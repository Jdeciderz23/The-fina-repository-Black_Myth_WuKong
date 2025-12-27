#ifndef __CHARACTER_COLLIDER_H__
#define __CHARACTER_COLLIDER_H__

#include "cocos2d.h"
#include "3d/CCMesh.h"
#include <vector>
#include <float.h>

using namespace cocos2d;

/**
 * @class CharacterCollider
 * @brief 角色/敌人碰撞组件，使用 AABB (轴对齐包围盒) 进行碰撞检测
 */
class CharacterCollider {
public:
    AABB aabb;          ///< 原始 AABB (模型局部空间)
    AABB worldAABB;     ///< 世界空间 AABB (跟随物体移动)

    /**
     * @brief 计算并初始化 AABB
     * @param characterModel 3D 角色模型
     * @param xzShrinkFactor XZ 轴收缩系数 (0-1)，用于减少空气墙感。例如 0.4 表示只保留中心 40% 的宽度
     */
    void calculateBoundingBox(Sprite3D* characterModel, float xzShrinkFactor = 1.0f) {
        if (!characterModel) return;
        
        // 尝试获取 AABB
        aabb = characterModel->getAABB();
        
        // 如果获取到的 AABB 为空或无限（常见于模型刚加载尚未计算的情况）
        // 强制重新计算一次
        if (aabb._min.x >= aabb._max.x) {
            auto meshes = characterModel->getMeshes();
            bool first = true;
            for (auto mesh : meshes) {
                mesh->calculateAABB();
                if (first) {
                    aabb = mesh->getAABB();
                    first = false;
                } else {
                    aabb.merge(mesh->getAABB());
                }
            }
        }

        // 如果依然无效，给一个默认的大小（例如 50x180x50），防止碰撞失效
        if (aabb._min.x >= aabb._max.x) {
            CCLOG("Warning: Failed to get AABB from model, using default box.");
            aabb._min = Vec3(-25, 0, -25);
            aabb._max = Vec3(25, 180, 25);
        }

        // 应用 XZ 轴收缩
        if (xzShrinkFactor < 1.0f && xzShrinkFactor > 0.0f) {
            Vec3 center = (aabb._min + aabb._max) * 0.5f;
            float halfWidth = (aabb._max.x - aabb._min.x) * 0.5f * xzShrinkFactor;
            float halfDepth = (aabb._max.z - aabb._min.z) * 0.5f * xzShrinkFactor;
            
            aabb._min.x = center.x - halfWidth;
            aabb._max.x = center.x + halfWidth;
            aabb._min.z = center.z - halfDepth;
            aabb._max.z = center.z + halfDepth;
        }
        
        // 调试日志
        CCLOG("Collider initialized. Local AABB: min(%.2f, %.2f, %.2f), max(%.2f, %.2f, %.2f)", 
              aabb._min.x, aabb._min.y, aabb._min.z, 
              aabb._max.x, aabb._max.y, aabb._max.z);
    }

    /**
     * @brief 每帧更新世界空间中的 AABB
     * @param owner 拥有该碰撞器的节点 (Character/Enemy)
     */
    void update(Node* owner) {
        if (!owner) return;
        
        // 获取节点的局部到世界变换矩阵
        const Mat4& transform = owner->getNodeToWorldTransform();
        
        // 将局部 AABB 变换到世界空间
        // 注意：AABB 变换后可能不再是轴对齐的，这里 transform 会返回包含变换后 AABB 的最小轴对齐 AABB
        worldAABB = aabb;
        worldAABB.transform(transform);
    }

    /**
     * @brief 检测与其他 AABB 的碰撞
     */
    bool checkCollision(const AABB& other) const {
        return worldAABB.intersects(other);
    }

    /**
     * @brief 获取碰撞后的位置修正偏移
     * @param other 其他物体的 AABB
     * @param sourceAABB 可选：指定检测碰撞的源 AABB (默认使用自身的 worldAABB)
     * @return Vec3 修正向量（让 sourceAABB 退出 other 的最短距离）
     */
    Vec3 getCollisionOffset(const AABB& other, const AABB* sourceAABB = nullptr) const {
        const AABB& src = sourceAABB ? *sourceAABB : worldAABB;
        
        if (!src.intersects(other)) return Vec3::ZERO;

        // 计算六个方向的重叠深度
        float overlapX1 = src._max.x - other._min.x;
        float overlapX2 = other._max.x - src._min.x;
        float overlapZ1 = src._max.z - other._min.z;
        float overlapZ2 = other._max.z - src._min.z;

        // 寻找水平方向最小重叠轴进行挤出
        float minOverlapX = (overlapX1 < overlapX2) ? overlapX1 : -overlapX2;
        float minOverlapZ = (overlapZ1 < overlapZ2) ? overlapZ1 : -overlapZ2;

        if (std::abs(minOverlapX) < std::abs(minOverlapZ)) {
            return Vec3(-minOverlapX, 0, 0);
        } else {
            return Vec3(0, 0, -minOverlapZ);
        }
    }
};

#endif // __CHARACTER_COLLIDER_H__

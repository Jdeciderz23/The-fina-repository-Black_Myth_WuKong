#ifndef __CHARACTER_COLLIDER_H__
#define __CHARACTER_COLLIDER_H__

#include "cocos2d.h"
#include "3d/CCMesh.h"
#include <vector>
#include <float.h>  // For FLT_MAX

using namespace cocos2d;

// 角色碰撞类
class CharacterCollider {
public:
    // 使用 Cocos2d-x 自带的 AABB 类来表示人物的碰撞盒
    cocos2d::AABB aabb;

    // 从 Sprite3D 模型中获取顶点数据并计算 AABB 包围盒
    void calculateBoundingBox(Sprite3D* characterModel) {
        // 获取人物模型的 Mesh 对象
        Mesh* mesh = characterModel->getMesh();

        if (mesh == nullptr) {
            CCLOG("模型没有加载 Mesh 数据！");
            return;
        }

        // 计算 Mesh 的 AABB
        mesh->calculateAABB();

        // 获取计算好的 AABB
        const AABB& calculatedAABB = mesh->getAABB();

        // 使用 Cocos2d-x 的 AABB 类设置最小点和最大点
        aabb = calculatedAABB;
    }

    // 为每一帧更新人物的 AABB 碰撞盒
    void updateBoundingBoxForAnimation(Sprite3D* characterModel) {
        // 获取人物的当前动画状态
        Mesh* mesh = characterModel->getMesh();

        if (mesh == nullptr) {
            CCLOG("模型没有加载 Mesh 数据！");
            return;
        }

        // 计算 Mesh 的 AABB
        mesh->calculateAABB();

        // 获取计算好的 AABB
        const AABB& calculatedAABB = mesh->getAABB();

        // 更新 AABB 包围盒
        aabb = calculatedAABB;
    }

    // 获取 AABB
    const cocos2d::AABB& getAABB() const {
        return aabb;
    }

    // 检测与其他 AABB 的碰撞
    bool checkCollision(const cocos2d::AABB& otherAABB) const {
        return aabb.intersects(otherAABB);
    }
};

#endif // __CHARACTER_COLLIDER_H__

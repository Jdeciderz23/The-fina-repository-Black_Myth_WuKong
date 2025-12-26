#ifndef __COLLIDER_H__
#define __COLLIDER_H__

#include "cocos2d.h"
#include <vector>

/**
 * @class CustomRay
 * @brief 简单的射线类，用于碰撞检测
 */
class CustomRay {
public:
    cocos2d::Vec3 origin;
    cocos2d::Vec3 direction;

    CustomRay(const cocos2d::Vec3& o, const cocos2d::Vec3& d) : origin(o), direction(d) {}
};

/**
 * @class TerrainCollider
 * @brief 处理 3D 地形碰撞的类
 */
class TerrainCollider : public cocos2d::Ref {
public:
    static TerrainCollider* create(cocos2d::Sprite3D* terrainModel, const std::string& objFilePath = "");
    
    bool init(cocos2d::Sprite3D* terrainModel, const std::string& objFilePath);

    /**
     * @brief 射线检测
     * @param ray 射线
     * @param hitDist 输出：碰撞距离
     * @return bool 是否碰撞
     */
    bool rayIntersects(const CustomRay& ray, float& hitDist);

private:
    cocos2d::Sprite3D* _terrain;
    // 这里可以存储简化的物理网格数据
    struct Triangle {
        cocos2d::Vec3 v0, v1, v2;
    };
    std::vector<Triangle> _triangles;

    void extractTriangles(cocos2d::Sprite3D* model);
    bool loadFromObj(const std::string& objFilePath);
    bool intersectTriangle(const CustomRay& ray, const Triangle& tri, float& t);
};

#endif // __COLLIDER_H__

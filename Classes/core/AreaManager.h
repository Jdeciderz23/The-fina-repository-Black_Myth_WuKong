#ifndef __AREA_MANAGER_H__
#define __AREA_MANAGER_H__

#include "cocos2d.h"
#include <vector>
#include <string>

/**
 * @class AreaManager
 * @brief 区域管理器
 * @details 管理游戏中的战斗区域、传送点，以及回血限制逻辑
 */
class AreaManager {
public:
    enum class AreaType {
        NONE,
        NORMAL_MONSTER,
        BOSS,
        TELEPORT_POINT
    };

    struct AreaInfo {
        std::string name;
        AreaType type;
        cocos2d::Rect bounds; // 简化为 2D 矩形判断 (x, z)
    };

    struct TeleportPoint {
        std::string name;
        cocos2d::Vec3 position;
    };

    static AreaManager* getInstance();

    /**
     * @brief 初始化区域和传送点
     */
    void init();

    /**
     * @brief 获取当前玩家所在区域类型
     */
    AreaType getCurrentAreaType(const cocos2d::Vec3& playerPos);

    /**
     * @brief 检查玩家是否在传送点附近（可交互范围）
     */
    bool isNearTeleportPoint(const cocos2d::Vec3& playerPos, int& outPointIndex);

    /**
     * @brief 执行传送逻辑
     * @param player 玩家对象
     */
    void teleport(cocos2d::Node* player);

    /**
     * @brief 检查当前是否允许回血
     */
    bool canHeal(const cocos2d::Vec3& playerPos);

    /**
     * @brief 获取传送点列表
     */
    const std::vector<TeleportPoint>& getTeleportPoints() const { return _teleportPoints; }

private:
    AreaManager();
    static AreaManager* _instance;

    std::vector<AreaInfo> _areas;
    std::vector<TeleportPoint> _teleportPoints;
    float _interactionDistance = 100.0f; // 交互距离
};

#endif // __AREA_MANAGER_H__

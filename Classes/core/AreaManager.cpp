#include "AreaManager.h"
#include "scene_ui/AudioManager.h"

USING_NS_CC;

AreaManager* AreaManager::_instance = nullptr;

AreaManager* AreaManager::getInstance() {
    if (!_instance) {
        _instance = new AreaManager();
        _instance->init();
    }
    return _instance;
}

AreaManager::AreaManager() {
}

void AreaManager::init() {
    _areas.clear();
    _teleportPoints.clear();

    // 定义小怪区域 (x: -500 to 500, z: -1500 to -500)
    _areas.push_back({"NormalMonsterArea", AreaType::NORMAL_MONSTER, Rect(-500, -1500, 1000, 1000)});

    // 定义 Boss 区域 (x: -1000 to 1000, z: -3000 to -2000)
    _areas.push_back({"BossArea", AreaType::BOSS, Rect(-1000, -3000, 2000, 1000)});

    // 定义两个传送点
    _teleportPoints.push_back({"Point_A_Spawn", Vec3(300, 0, 800)});
    _teleportPoints.push_back({"Point_B_BossGate", Vec3(0, 0, -960)});
}

AreaManager::AreaType AreaManager::getCurrentAreaType(const Vec3& playerPos) {
    // 检查是否在传送点附近
    int pointIdx;
    if (isNearTeleportPoint(playerPos, pointIdx)) {
        return AreaType::TELEPORT_POINT;
    }

    // 检查是否在战斗区域
    for (const auto& area : _areas) {
        if (area.bounds.containsPoint(Vec2(playerPos.x, playerPos.z))) {
            return area.type;
        }
    }

    return AreaType::NONE;
}

bool AreaManager::isNearTeleportPoint(const Vec3& playerPos, int& outPointIndex) {
    for (size_t i = 0; i < _teleportPoints.size(); ++i) {
        if (playerPos.distance(_teleportPoints[i].position) < _interactionDistance) {
            outPointIndex = (int)i;
            return true;
        }
    }
    outPointIndex = -1;
    return false;
}

void AreaManager::teleport(Node* player) {
    if (!player) return;

    Vec3 currentPos = player->getPosition3D();
    int currentIdx;
    if (isNearTeleportPoint(currentPos, currentIdx)) {
        // 如果在传送点 A，传送到 B；如果在 B，传送到 A
        int targetIdx = (currentIdx == 0) ? 1 : 0;
        Vec3 targetPos = _teleportPoints[targetIdx].position;
        
        player->setPosition3D(targetPos);
        
        // 切换背景音乐
        if (targetIdx == 0) {
            AudioManager::getInstance()->playBGM("Audio/game_bgm1.mp3");
        } else {
            AudioManager::getInstance()->playBGM("Audio/game_bgm2.mp3");
        }

        CCLOG("AreaManager: Teleported to %s, BGM switched.", _teleportPoints[targetIdx].name.c_str());
    } else {
        CCLOG("AreaManager: Not near any teleport point, cannot teleport.");
    }
}

bool AreaManager::canHeal(const Vec3& playerPos) {
    int pointIdx;
    // 只有在传送点附近才允许回血
    return isNearTeleportPoint(playerPos, pointIdx);
}

#include "InputController.h"

PlayerController* PlayerController::create(Wukong* target) {
    PlayerController* p = new (std::nothrow) PlayerController();
    if (p && p->init(target)) {
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

bool PlayerController::init(Wukong* target) {
    if (!cocos2d::Node::init()) {
        return false;
    }

    _target = target;
    _w = _a = _s = _d = false;
    _run = false;

    bindKeyboard();
    this->scheduleUpdate();
    return true;
}

void PlayerController::update(float dt) {
    (void)dt;
    if (!_target || _target->isDead()) {
        return;
    }

    /*
    if (_w) raw.z -= 1.0f;
    if (_s) raw.z += 1.0f;
    if (_a) raw.x -= 1.0f;
    if (_d) raw.x += 1.0f;*/

    // 1) 收集输入：A/D -> x, W/S -> z
    float x = 0.0f;
    float z = 0.0f;
    if (_a) x -= 1.0f;
    if (_d) x += 1.0f;
    if (_w) z += 1.0f;   // W 前
    if (_s) z -= 1.0f;   // S 后

    cocos2d::Vec3 moveWS = cocos2d::Vec3::ZERO;

    // 2) 有输入才算方向
    if (fabsf(x) > 1e-6f || fabsf(z) > 1e-6f)
    {
        // 斜向移动不加速：归一化输入
        float len = sqrtf(x * x + z * z);
        x /= len; z /= len;

        // 3) 取相机朝向（投影到地面）
        cocos2d::Vec3 forward = cocos2d::Vec3(0, 0, 1);
        if (_cam)
        {
            cocos2d::Vec3 camToPlayer = _target->getPosition3D() - _cam->getPosition3D();
            camToPlayer.y = 0.0f;
            if (camToPlayer.lengthSquared() > 1e-6f)
            {
                camToPlayer.normalize();
                forward = camToPlayer; // 镜头指向角色的方向作为“前”
            }
        }

        cocos2d::Vec3 right;
        cocos2d::Vec3::cross(forward, cocos2d::Vec3::UNIT_Y, &right);
        right.normalize();

        // 4) 组合：前后 + 左右
        moveWS = forward * z + right * (-x);
    }

    // 5) 交给角色
    Character::MoveIntent intent;
    intent.dirWS = moveWS;      // Vec3::ZERO 表示不移动
    intent.run = _run;          // 你已有的 Shift/跑步标记
    _target->setMoveIntent(intent);
    //cocos2d::Vec3 raw = cocos2d::Vec3::ZERO;
    // cocos2d::Vec3 worldDir = raw;

    //// 关键：相对镜头
    //if (_cam && raw.lengthSquared() > 1e-6f) {
    //    cocos2d::Vec3 camToPlayer = (_target->getPosition3D() - _cam->getPosition3D());
    //    camToPlayer.y = 0.0f;
    //    if (camToPlayer.lengthSquared() > 1e-6f) camToPlayer.normalize();
    //    else camToPlayer = cocos2d::Vec3(0, 0, -1);

    //    cocos2d::Vec3 forward = camToPlayer;                 // 镜头“朝向角色”的水平前方
    //    cocos2d::Vec3 right;
    //    cocos2d::Vec3::cross(forward, cocos2d::Vec3::UNIT_Y, &right);
    //    right.normalize();

    //    // raw.z: W=-1  S=+1，所以用 -raw.z
    //    worldDir = forward * (-raw.z) + right * (raw.x);
    //}

    //Character::MoveIntent intent;
    //intent.dirWS = worldDir;   // 现在是世界方向，但与镜头一致
    //intent.run = _run;
    //_target->setMoveIntent(intent);
}

void PlayerController::bindKeyboard() {
    auto* listener = cocos2d::EventListenerKeyboard::create();

    listener->onKeyPressed = [this](cocos2d::EventKeyboard::KeyCode code, cocos2d::Event*) {
        switch (code) {
            case cocos2d::EventKeyboard::KeyCode::KEY_W: _w = true; break;
            case cocos2d::EventKeyboard::KeyCode::KEY_A: _a = true; break;
            case cocos2d::EventKeyboard::KeyCode::KEY_S: _s = true; break;
            case cocos2d::EventKeyboard::KeyCode::KEY_D: _d = true; break;
            case cocos2d::EventKeyboard::KeyCode::KEY_SHIFT: _run = true; break;
            case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
                if (_target) _target->jump();
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_J:
                if (_target) _target->attackLight();
                break;
            case cocos2d::EventKeyboard::KeyCode::KEY_K:
                if (_target) _target->roll();
                break;
            default: break;
        }
        };

    listener->onKeyReleased = [this](cocos2d::EventKeyboard::KeyCode code, cocos2d::Event*) {
        switch (code) {
            case cocos2d::EventKeyboard::KeyCode::KEY_W: _w = false; break;
            case cocos2d::EventKeyboard::KeyCode::KEY_A: _a = false; break;
            case cocos2d::EventKeyboard::KeyCode::KEY_S: _s = false; break;
            case cocos2d::EventKeyboard::KeyCode::KEY_D: _d = false; break;
            case cocos2d::EventKeyboard::KeyCode::KEY_SHIFT: _run = false; break;
            default: break;
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

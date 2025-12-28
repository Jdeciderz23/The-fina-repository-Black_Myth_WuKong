#include "InputController.h"
#include"Wukong.h"
#include"cocos2d.h"
#include <cmath>
#include <new>

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

    _autoFollowYaw = false;
    bindKeyboard();
    bindMouse();

    this->scheduleUpdate();
    return true;
}
void PlayerController::setCamera(cocos2d::Camera* cam) {
    _cam = cam;
    if (_cam) {
        // 建议：你的 Wukong 用了 USER1 mask，这里相机也要 USER1，否则可能看不到模型
        _cam->setCameraFlag(cocos2d::CameraFlag::USER1);
    }

    // 用当前相机位置初始化 yaw（避免一上来跳镜头）
    if (_cam && _target) {
        cocos2d::Vec3 d = _cam->getPosition3D() - _target->getPosition3D();
        d.y = 0.0f;
        if (d.lengthSquared() > 1e-6f) {
            // offset = (sinYaw, 0, cosYaw) * dist  => yaw = atan2(x, z)
            _camYawDeg = CC_RADIANS_TO_DEGREES(std::atan2(d.x, d.z));
            _camDist = d.length();
        }
    }
}

float PlayerController::moveTowardAngleDeg(float cur, float target, float maxDeltaDeg) const {
    // 最短角度差（-180~180）
    float delta = std::fmod(target - cur + 540.0f, 360.0f) - 180.0f;
    if (delta > maxDeltaDeg) delta = maxDeltaDeg;
    if (delta < -maxDeltaDeg) delta = -maxDeltaDeg;
    return cur + delta;
}

void PlayerController::updateThirdPersonCamera(float dt) {
    if (!_cam || !_target) return;

    // 自动让相机“追随角色朝向”（更像魂类/悟空那种：你不动鼠标，镜头会慢慢回到身后）
    if (_autoFollowYaw && !_mouseRotating) {
        float targetYaw = _target->getRotation3D().y;
        _camYawDeg = moveTowardAngleDeg(_camYawDeg, targetYaw, _autoYawSpeed * dt);
    }

    _camPitchDeg = cocos2d::clampf(_camPitchDeg, _minPitch, _maxPitch);
    _camDist = cocos2d::clampf(_camDist, _minDist, _maxDist);

    const float yawRad = CC_DEGREES_TO_RADIANS(_camYawDeg);
    const float pitchRad = CC_DEGREES_TO_RADIANS(_camPitchDeg);

    cocos2d::Vec3 lookAtPos = _target->getPosition3D() + cocos2d::Vec3(0.0f, _lookAtHeight, 0.0f);

    // orbit offset：yaw=0 => offset.z=+dist（相机在角色后方）
    cocos2d::Vec3 offset;
    offset.x = _camDist * std::sinf(yawRad) * std::cosf(pitchRad);
    offset.y = -_camDist * std::sinf(pitchRad);
    offset.z = _camDist * std::cosf(yawRad) * std::cosf(pitchRad);

    cocos2d::Vec3 desiredPos = lookAtPos + offset;

    // 位置平滑（指数跟随）
    cocos2d::Vec3 cur = _cam->getPosition3D();
    float t = 1.0f - std::exp(-_camFollowPosK * dt);
    cocos2d::Vec3 newPos = cur + (desiredPos - cur) * t;

    _cam->setPosition3D(newPos);
    _cam->lookAt(lookAtPos, cocos2d::Vec3::UNIT_Y);
}
void PlayerController::update(float dt) {
    (void)dt;
    if (!_target || _target->isDead()) {
        return;
    }

    // 更新镜头
    updateThirdPersonCamera(dt);

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
        moveWS = forward * z + right * x;
    }
    cocos2d::Vec2 axis(x, z);
    if (axis.lengthSquared() > 1.0f) axis.normalize();

    // 关键：把输入方向交给 Wukong
    _target->setMoveAxis(axis);
    //镜头面向前方
    if (axis.lengthSquared() > 1e-4f) {
        auto rot = _target->getRotation3D();
        float newYaw = moveTowardAngleDeg(rot.y, _camYawDeg, 720.0f * dt);
        _target->setRotation3D(cocos2d::Vec3(0.0f, newYaw, 0.0f));
    }

    // 5) 交给角色
    Character::MoveIntent intent;
    intent.dirWS = moveWS;      // Vec3::ZERO 表示不移动
    intent.run = _run;          // 已有的 Shift/跑步标记
    _target->setMoveIntent(intent);

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
        case cocos2d::EventKeyboard::KeyCode::KEY_1:
            if (_target) _target->castSkill();
            break;
        case cocos2d::EventKeyboard::KeyCode::KEY_R:
            _camYawDeg = _target->getRotation3D().y;
            _camPitchDeg = -15.0f;
            _camDist = 180.0f;
            _mouseRotating = false;
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

void PlayerController::bindMouse() {
    auto* mouse = cocos2d::EventListenerMouse::create();

    mouse->onMouseDown = [this](cocos2d::EventMouse* e) {
        if (e->getMouseButton() == cocos2d::EventMouse::MouseButton::BUTTON_RIGHT) {
            _mouseRotating = true;
            _lastMouse = cocos2d::Vec2(e->getCursorX(), e->getCursorY());

            if (auto view = cocos2d::Director::getInstance()->getOpenGLView())
                view->setCursorVisible(false);
        }
        };

    mouse->onMouseUp = [this](cocos2d::EventMouse* e) {
        if (e->getMouseButton() == cocos2d::EventMouse::MouseButton::BUTTON_LEFT) {
            if (_target) _target->attackLight();
            return;
        }
        if (e->getMouseButton() == cocos2d::EventMouse::MouseButton::BUTTON_RIGHT) {
            _mouseRotating = false;

            if (auto view = cocos2d::Director::getInstance()->getOpenGLView())
                view->setCursorVisible(true);
        }
        };

    mouse->onMouseMove = [this](cocos2d::EventMouse* e) {
        cocos2d::Vec2 cur(e->getCursorX(), e->getCursorY());

        if (!_mouseRotating) {
            _lastMouse = cur;  // 不旋转时只更新 last，避免下一次按下右键 delta 爆炸
            return;
        }

        cocos2d::Vec2 delta = cur - _lastMouse;
        _lastMouse = cur;

        // 防止窗口切出去再回来时 cursor 瞬移导致镜头暴走
        if (std::fabs(delta.x) > 500.0f || std::fabs(delta.y) > 500.0f) return;

        _camYawDeg -= delta.x * _mouseSens;
        _camPitchDeg -= delta.y * _mouseSens;
        };

    mouse->onMouseScroll = [this](cocos2d::EventMouse* e) {
        _camDist -= e->getScrollY() * 18.0f;
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouse, this);
}
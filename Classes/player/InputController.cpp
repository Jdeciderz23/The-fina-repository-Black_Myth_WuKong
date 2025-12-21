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

    cocos2d::Vec3 dir = cocos2d::Vec3::ZERO;
    if (_w) dir.z -= 1.0f;
    if (_s) dir.z += 1.0f;
    if (_a) dir.x -= 1.0f;
    if (_d) dir.x += 1.0f;

    Character::MoveIntent intent;
    intent.dirWS = dir;
    intent.run = _run;
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

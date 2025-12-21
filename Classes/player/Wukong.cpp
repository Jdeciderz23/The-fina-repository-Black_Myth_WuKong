#include "Wukong.h"

Wukong* Wukong::create() {
    Wukong* p = new (std::nothrow) Wukong();
    if (p && p->init()) {
        p->autorelease();
        return p;
    }
    delete p;
    return nullptr;
}

bool Wukong::init() {
    if (!Character::init()) {
        return false;
    }

    _model = nullptr;
    // TODO: 有模型资源时再加载，例如：
    // _model = cocos2d::Sprite3D::create("models/wukong.c3b");
    // if (_model) { _visualRoot->addChild(_model); }

    return true;
}

void Wukong::playAnim(const std::string& name, bool loop) {
    // TODO: 接入 Animate3D/Animation3D
    cocos2d::log("[Wukong] playAnim: %s loop=%d", name.c_str(), (int)loop);
}

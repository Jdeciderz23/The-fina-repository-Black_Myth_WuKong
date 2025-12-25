#include "Wukong.h"
#include"cocos2d.h"

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

    this->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1, true);
    auto full = cocos2d::FileUtils::getInstance()->fullPathForFilename("WuKong/wukong.c3b");
    cocos2d::log("[Wukong] fullPath=%s", full.c_str());

    //加载模型
    _model = cocos2d::Sprite3D::create("WuKong/wukong.c3b");
    auto aabb = _model->getAABB();
    auto center = (aabb._min + aabb._max) * 0.5f;


    // XZ 居中，Y 方向把脚底抬到 y=0
    //_model->setPosition3D(cocos2d::Vec3(-center.x, -aabb._min.y, -center.z));

    _model->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1, true);

    if (_model) {
        _model->setScale(1.0f);
        _model->setPosition3D(cocos2d::Vec3::ZERO);
        _model->setRotation3D(cocos2d::Vec3(0.0f, 0.0f, 180.0f));
       // _model->setRotation3D(cocos2d::Vec3::ZERO);
        _model->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1, true);
        _model->setForceDepthWrite(true);
        _model->setCullFaceEnabled(false);
        _visualRoot->addChild(_model);

        // 预加载最基础两套
        _anims["idle"] = cocos2d::Animation3D::create("WuKong/Idle.c3b");
        _anims["run"] = cocos2d::Animation3D::create("WuKong/Jog_Fwd.c3b");
        _anims["jump_pad"] = cocos2d::Animation3D::create("WuKong/Jump_Pad.c3b");
        _anims["jump_start"] = cocos2d::Animation3D::create("WuKong/Jump_Start.c3b");
        _anims["jump_apex"] = cocos2d::Animation3D::create("WuKong/Jump_Apex.c3b");
        _anims["jump_land"] = cocos2d::Animation3D::create("WuKong/Jump_Land.c3b");
        _anims["jump_recovery"] = cocos2d::Animation3D::create("WuKong/Jump_Recovery.c3b");
        playAnim("idle", true);
        playAnim("run", true);
        playAnim("jump", false);

    }
    else {
        cocos2d::log("[Wukong] load model failed!");
    }
    return true;
}

void Wukong::loadAnimIfNeeded(const std::string& key,
    const std::string& c3bPath)
{
    if (_anims.count(key)) 
        return;

    // 你把文件放哪，就把这里路径改成对应 Resources 路径
    // 比如 "Models/Wukong/Idle.FBX"
    cocos2d::Animation3D* anim = cocos2d::Animation3D::create(c3bPath);
    _anims[key] = anim;

    if (!anim) {
        cocos2d::log("[Wukong] loadAnim failed: key=%s c3b=%s",
            key.c_str(), c3bPath.c_str());
    }
}

void Wukong::playAnim(const std::string& name, bool loop)
{
    if (!_model) return;
    if (_curAnim == name) return;

    auto it = _anims.find(name);
    if (it == _anims.end() || !it->second) return;

    _curAnim = name;

    _model->stopActionByTag(_animTag);

    auto animate = cocos2d::Animate3D::create(it->second);
    cocos2d::ActionInterval* act = loop
        ? (cocos2d::ActionInterval*)cocos2d::RepeatForever::create(animate)
        : (cocos2d::ActionInterval*)animate;

    act->setTag(_animTag);
    _model->runAction(act);
}

cocos2d::Animate3D* Wukong::makeAnimate(const std::string& key) const
{
    auto it = _anims.find(key);
    if (it == _anims.end() || !it->second) {
        cocos2d::log("[Wukong] anim not found: %s", key.c_str());
        return nullptr;
    }
    return cocos2d::Animate3D::create(it->second);
}

void Wukong::startJumpAnim()
{
    if (!_model) return;

    auto pad = makeAnimate("jump_pad");
    auto start = makeAnimate("jump_start");
    auto apex = makeAnimate("jump_apex");
    if (!pad || !start || !apex) return;

    _curAnim = "jump";
    _model->stopActionByTag(_animTag);

    // Pad -> Start -> Apex(循环，直到落地被 stop)
    auto apexLoop = cocos2d::RepeatForever::create(apex);
    auto seq = cocos2d::Sequence::create(pad, start, apexLoop, nullptr);

    seq->setTag(_animTag);
    _model->runAction(seq);
}

void Wukong::onJumpLanded()
{
    if (!_model) return;

    auto land = makeAnimate("jump_land");
    auto rec = makeAnimate("jump_recovery");

    // 先停掉空中循环（或者正在跑的跳跃序列）
    _model->stopActionByTag(_animTag);

    // 如果缺动画，至少别卡死：直接回 Idle/Move
    if (!land || !rec) {
        const auto intent = this->getMoveIntent();
        if (intent.dirWS.lengthSquared() > 1e-6f) this->getStateMachine().changeState("Move");
        else this->getStateMachine().changeState("Idle");
        return;
    }

    auto backToLocomotion = cocos2d::CallFunc::create([this]() {
        const auto intent = this->getMoveIntent();
        if (intent.dirWS.lengthSquared() > 1e-6f) this->getStateMachine().changeState("Move");
        else this->getStateMachine().changeState("Idle");
        });

    // Land -> Recovery -> 回到 Idle/Move
    auto seq = cocos2d::Sequence::create(land, rec, backToLocomotion, nullptr);
    seq->setTag(_animTag);
    _model->runAction(seq);

    _curAnim = "jump_land";
}

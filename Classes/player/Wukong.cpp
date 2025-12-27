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

    this->walkSpeed = 120.0f;
    this->runSpeed = 240.0f;

    // XZ 居中，Y 方向把脚底抬到 y=0
    //_model->setPosition3D(cocos2d::Vec3(-center.x, -aabb._min.y, -center.z));

    _model->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1, true);

    if (_model) {
        _model->setScale(0.1f);
        _model->setPosition3D(cocos2d::Vec3::ZERO);
        _model->setRotation3D(cocos2d::Vec3(0.0f, 180.0f, 0.0f));
        // _model->setRotation3D(cocos2d::Vec3::ZERO);
        _model->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1, true);
        _model->setForceDepthWrite(true);
        _model->setCullFaceEnabled(false);
        _visualRoot->addChild(_model);

        //预加载最基础两套
        _anims["idle"] = cocos2d::Animation3D::create("WuKong/Idle.c3b");
        _anims["run_fwd"] = cocos2d::Animation3D::create("WuKong/Jog_Fwd.c3b");
        _anims["run_bwd"] = cocos2d::Animation3D::create("WuKong/Jog_Bwd.c3b");
        _anims["run_left"] = cocos2d::Animation3D::create("WuKong/Jog_Left.c3b");   // 有就填
        _anims["run_right"] = cocos2d::Animation3D::create("WuKong/Jog_Right.c3b");        
        _anims["jump"] = cocos2d::Animation3D::create("WuKong/Jump.c3b");
        _anims["run"] = _anims["run_fwd"];
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

    auto jump = makeAnimate("jump");  
    if (!jump) return;

    _jumpAnimPlaying = true;

    // 停掉其它 locomotion 动画（同一个 tag）
    _model->stopActionByTag(_animTag);

    // jump 播完走落地逻辑
    auto done = cocos2d::CallFunc::create([this]() {
        _jumpAnimPlaying = false;
        this->onJumpLanded();
        });

    auto seq = cocos2d::Sequence::create(jump, done, nullptr);
    seq->setTag(_animTag);
    _model->runAction(seq);
}


void Wukong::onJumpLanded()
{
    const auto intent = this->getMoveIntent();  
    if (intent.dirWS.lengthSquared() > 1e-6f) {
        this->getStateMachine().changeState("Move");
    }
    else {
        this->getStateMachine().changeState("Idle");
    }
}

//根据输入轴计算方向，优先“幅度更大的轴”
void Wukong::setMoveAxis(const cocos2d::Vec2& axis) {
    _moveAxis = axis;
}

Wukong::LocomotionDir Wukong::calcLocomotionDir(const cocos2d::Vec2& axis) const {
    if (axis.lengthSquared() < 0.01f) return LocomotionDir::None;

    if (std::fabs(axis.y) >= std::fabs(axis.x)) {
        return axis.y >= 0.0f ? LocomotionDir::Fwd : LocomotionDir::Bwd;
    }
    return axis.x >= 0.0f ? LocomotionDir::Right : LocomotionDir::Left;
}

void Wukong::updateLocomotionAnim(bool running) {
    const auto dir = calcLocomotionDir(_moveAxis);
    if (dir == LocomotionDir::None) return;

    if (dir == _locoDir && running == _locoRun) return;
    _locoDir = dir;
    _locoRun = running;

    std::string key;
    switch (dir) {
    case LocomotionDir::Fwd:   key = "run_fwd"; break;
    case LocomotionDir::Bwd:   key = "run_bwd"; break;
    case LocomotionDir::Left:  key = "run_left"; break;
    case LocomotionDir::Right: key = "run_right"; break;
    default:                   key = "run_fwd"; break;
    }

    // 左右动画没放对名字时，至少别崩：回退到前跑
    auto it = _anims.find(key);
    if (it == _anims.end() || !it->second) key = "run_fwd";

    playAnim(key, true);
}

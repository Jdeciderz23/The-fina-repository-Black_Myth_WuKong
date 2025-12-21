#include "Character.h"
#include "WukongStates.h"

Character::Character()
    : _visualRoot(nullptr),
    _moveIntent(),
    _velocity(cocos2d::Vec3::ZERO),
    _onGround(true),
    _hp(100),
    _lifeState(LifeState::Alive),
    _comboBuffered(false),
    _fsm(this) {
}

Character::~Character() {
    // _ownedStates 会自动释放状态对象
}

bool Character::init() {
    if (!cocos2d::Node::init()) {
        return false;
    }

    _visualRoot = cocos2d::Node::create();
    this->addChild(_visualRoot);

    // ===== 注册状态（对象由 Character 持有，FSM 只保存裸指针映射）=====
    _ownedStates.emplace_back(std::make_unique<IdleState>());
    _ownedStates.emplace_back(std::make_unique<MoveState>());
    _ownedStates.emplace_back(std::make_unique<JumpState>());
    _ownedStates.emplace_back(std::make_unique<RollState>());
    _ownedStates.emplace_back(std::make_unique<AttackState>(1));
    _ownedStates.emplace_back(std::make_unique<AttackState>(2));
    _ownedStates.emplace_back(std::make_unique<AttackState>(3));
    _ownedStates.emplace_back(std::make_unique<HurtState>());
    _ownedStates.emplace_back(std::make_unique<DeadState>());

    for (auto& st : _ownedStates) {
        _fsm.registerState(st.get());
    }

    // 初始状态
    _fsm.init(_ownedStates[0].get()); // IdleState

    this->scheduleUpdate();
    return true;
}

void Character::update(float dt) {
    if (isDead()) {
        return;
    }

    _fsm.update(dt);

    applyGravity(dt);
    applyMovement(dt);
}

void Character::setMoveIntent(const MoveIntent& intent) {
    _moveIntent = intent;
}

Character::MoveIntent Character::getMoveIntent() const {
    return _moveIntent;
}

void Character::jump() {
    if (!_onGround || isDead()) {
        return;
    }
    _velocity.y = jumpSpeed;
    _onGround = false;
    _fsm.changeState("Jump");
}

void Character::roll() {
    if (isDead()) {
        return;
    }
    _fsm.changeState("Roll");
}

void Character::attackLight() {
    if (isDead()) {
        return;
    }

    BaseState<Character>* cur = _fsm.getCurrentState();
    const std::string curName = cur ? cur->getStateName() : "";

    // 若正在攻击，按一次只做“输入缓冲”，由 AttackState 在窗口内接续
    if (!curName.empty() && curName.rfind("Attack", 0) == 0) {
        _comboBuffered = true;
        return;
    }

    _comboBuffered = false;
    _fsm.changeState("Attack1");
}

void Character::takeHit(int damage) {
    if (isDead()) {
        return;
    }

    _hp -= damage;
    if (_hp <= 0) {
        die();
        return;
    }
    _fsm.changeState("Hurt");
}

void Character::die() {
    if (isDead()) {
        return;
    }
    _lifeState = LifeState::Dead;
    _fsm.changeState("Dead");
}

bool Character::isOnGround() const {
    return _onGround;
}

bool Character::isDead() const {
    return _lifeState == LifeState::Dead;
}

cocos2d::Vec3 Character::getVelocity() const {
    return _velocity;
}

void Character::setHorizontalVelocity(const cocos2d::Vec3& v) {
    _velocity.x = v.x;
    _velocity.z = v.z;
}

void Character::stopHorizontal() {
    _velocity.x = 0.0f;
    _velocity.z = 0.0f;
}

bool Character::consumeComboBuffered() {
    const bool had = _comboBuffered;
    _comboBuffered = false;
    return had;
}

StateMachine<Character>& Character::getStateMachine() {
    return _fsm;
}

void Character::applyGravity(float dt) {
    if (_onGround) {
        return;
    }

    _velocity.y -= gravity * dt;

    cocos2d::Vec3 pos = this->getPosition3D();
    const float nextY = pos.y + _velocity.y * dt;

    // 简化：y<=0 落地
    if (nextY <= 0.0f) {
        pos.y = 0.0f;
        this->setPosition3D(pos);
        _velocity.y = 0.0f;
        _onGround = true;
    }
}

void Character::applyMovement(float dt) {
    cocos2d::Vec3 pos = this->getPosition3D();
    pos += _velocity * dt;
    this->setPosition3D(pos);
}

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
    if (_onGround && _terrainCollider) {
        // 如果在地面上，且有碰撞器，我们通过 applyMovement 的射线检测来维持高度
        return;
    }

    _velocity.y -= gravity * dt;
}

void Character::applyMovement(float dt) {
    cocos2d::Vec3 oldPos = this->getPosition3D();
    cocos2d::Vec3 newPos = oldPos + _velocity * dt;

    if (_terrainCollider) {
        // 1. 射线检测新位置地面（从上方 500 个单位向下发射，覆盖更广的高度差）
        CustomRay ray(newPos + cocos2d::Vec3(0, 500, 0), cocos2d::Vec3(0, -1, 0));
        float hitDist;

        if (_terrainCollider->rayIntersects(ray, hitDist)) {
            float groundY = ray.origin.y - hitDist;
            const float MAX_STEP_HEIGHT = 40.0f; // 稍微增大跨越高度

            // 2. 坡度 / 台阶判断
            // 如果新位置的地面高度与当前位置高度差在允许范围内，或者正在下坡
            if (groundY - oldPos.y < MAX_STEP_HEIGHT) {
                newPos.y = groundY;
                this->setPosition3D(newPos);
                
                // 落地判定
                if (!_onGround && _velocity.y <= 0) {
                    _onGround = true;
                    _velocity.y = 0;
                }
            } else {
                // 坡度太陡（墙壁）
                // 限制水平位移，保持原位置，但允许垂直重力/跳跃
                cocos2d::Vec3 finalPos = oldPos;
                finalPos.y += _velocity.y * dt; 
                
                if (finalPos.y <= groundY) {
                    finalPos.y = groundY;
                    _onGround = true;
                    _velocity.y = 0;
                }
                this->setPosition3D(finalPos);
            }
        } else {
            // 3. 没检测到地面（可能出界）
            // 维持重力下降，但 _onGround 设为 false
            this->setPosition3D(newPos);
            _onGround = false;
        }
    } else {
        // 4. 无碰撞器，维持原有的简单 y=0 判定
        this->setPosition3D(newPos);
        if (newPos.y <= 0.0f) {
            cocos2d::Vec3 pos = this->getPosition3D();
            pos.y = 0.0f;
            this->setPosition3D(pos);
            _velocity.y = 0.0f;
            _onGround = true;
        }
    }
}

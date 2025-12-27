#include "Character.h"
#include "WukongStates.h"
#include "../combat/HealthComponent.h"
#include "../combat/CombatComponent.h"
#include "scene_ui/UIManager.h"
#include "enemy/Enemy.h"

Character::Character()
    : _visualRoot(nullptr),
    _moveIntent(),
    _velocity(cocos2d::Vec3::ZERO),
    _onGround(true),
    _healthComponent(nullptr),
    _combatComponent(nullptr),
    _comboBuffered(false),
    _fsm(this),
    _terrainCollider(nullptr),
    _enemies(nullptr) {
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

    // 注意：战斗系统组件将在子类中根据具体需求初始化
   // 例如：Wukong::init() 会调用 initCombatComponents(120.0f, 25.0f)

    this->scheduleUpdate();
    return true;
}

void Character::initCombatComponents(float maxHealth, float attackPower) {
    // 防止重复初始化组件
    if (_healthComponent || _combatComponent) {
        CCLOG("Character::initCombatComponents: Components already initialized, skipping duplicate initialization");
        return;
    }

    // 初始化健康组件
    _healthComponent = HealthComponent::create(maxHealth);
    if (!_healthComponent) {
        CCLOGERROR("Character::initCombatComponents: Failed to create HealthComponent");
        return;
    }

    // 添加健康组件到实体
    this->addComponent(_healthComponent);
    CCLOG("Character::initCombatComponents: HealthComponent created and added successfully");

    // 设置健康组件回调
    _healthComponent->setOnDeadCallback([this](cocos2d::Node* attacker) {
        this->die();
        });

    _healthComponent->setOnHurtCallback([this](float damage, cocos2d::Node* attacker) {
        // 可以在这里添加受伤音效、特效等
        CCLOG("Character took %.2f damage!", damage);
        });

    // 初始化战斗组件
    _combatComponent = CombatComponent::create();
    if (!_combatComponent) {
        CCLOGERROR("Character::initCombatComponents: Failed to create CombatComponent");
        return;
    }

    // 添加战斗组件到实体
    this->addComponent(_combatComponent);
    CCLOG("Character::initCombatComponents: CombatComponent created and added successfully");

    // 设置战斗属性
    _combatComponent->setAttackPower(attackPower);
    _combatComponent->setWeaponDamage(10.0f); // 基础武器伤害
    _combatComponent->setCritRate(0.15f);     // 15%暴击率
    _combatComponent->setCritDamage(1.8f);    // 180%暴击伤害

    CCLOG("Character::initCombatComponents: Combat components initialized successfully - Health: %.2f, Attack: %.2f", maxHealth, attackPower);
}


void Character::update(float dt) {
    if (isDead()) {
        return;
    }

    _fsm.update(dt);

    applyGravity(dt);
    applyMovement(dt);

    // 更新 AABB 碰撞盒到世界空间
    _collider.update(this);
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

void Character::takeHit(float damage, cocos2d::Node* attacker) {
    if (isDead() || !_healthComponent) {
        return;
    }

    _healthComponent->takeDamage(damage, attacker);

    if (_healthComponent->isDead()) {
        die();
    }
    else {
        _fsm.changeState("Hurt");
    }
}

void Character::die() {
    if (isDead()) {
        return;
    }

    if (_healthComponent) {
        _healthComponent->setInvincible(true); // 死亡后无敌
    }

    _fsm.changeState("Dead");

}


bool Character::isOnGround() const {
    return _onGround;
}

bool Character::isDead() const {
    return _healthComponent ? _healthComponent->isDead() : false;

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

    // 1. 与敌人的 AABB 碰撞检测
    if (_enemies && !_enemies->empty()) {
        // 先临时计算新位置下的世界 AABB
        // 获取当前变换并替换位置部分
        Mat4 nextTransform = this->getNodeToWorldTransform();
        nextTransform.m[12] = newPos.x;
        nextTransform.m[13] = newPos.y;
        nextTransform.m[14] = newPos.z;

        AABB nextWorldAABB = _collider.aabb;
        nextWorldAABB.transform(nextTransform);

        for (auto enemy : *_enemies) {
            if (!enemy || enemy->isDead()) continue;

            const AABB& enemyAABB = enemy->getCollider().worldAABB;

            if (nextWorldAABB.intersects(enemyAABB)) {
                // 计算碰撞偏移并修正 newPos
                Vec3 offset = _collider.getCollisionOffset(enemyAABB, &nextWorldAABB);

                if (offset != Vec3::ZERO) {
                    CCLOG("Collision detected with enemy! Offset: (%.2f, %.2f, %.2f)", offset.x, offset.y, offset.z);
                    newPos += offset;

                    // 修正后重新计算 nextWorldAABB 以便与下一个敌人检测
                    nextWorldAABB._min += offset;
                    nextWorldAABB._max += offset;
                }
            }
        }
    }

    if (_terrainCollider) {
        // 2. 射线检测新位置地面（从上方 500 个单位向下发射，覆盖更广的高度差）
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
            }
            else {
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
        }
        else {
            // 3. 没检测到地面（可能出界）
            // 维持重力下降，但 _onGround 设为 false
            this->setPosition3D(newPos);
            _onGround = false;
        }
    }
    else {
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

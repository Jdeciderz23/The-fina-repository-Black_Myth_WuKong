#include "EnemyStates.h"
#include "cocos2d.h"
#include <cfloat>

USING_NS_CC;

static inline bool HasTarget(const Enemy* e) {
    return e && e->getTarget() != nullptr;
}

// 敌人 world 坐标（你 Enemy.cpp 里已经实现了）
static inline cocos2d::Vec3 EnemyWorldPos(const Enemy* e) {
    return e->getWorldPosition3D();
}

// 玩家(悟空) world 坐标（你 Enemy.cpp 里已经实现了）
static inline cocos2d::Vec3 PlayerWorldPos(const Enemy* e) {
    return e->getTargetWorldPos();
}

static inline cocos2d::Vec3 BirthWorldPos(const Enemy* e) {
    auto p = e->getParent();
    if (!p) return e->getBirthPosition();  // 没父节点就当作世界坐标用

    cocos2d::Vec3 out = cocos2d::Vec3::ZERO;
    cocos2d::Mat4 m = p->getNodeToWorldTransform();   // parent local -> world
    m.transformPoint(e->getBirthPosition(), &out);     // birthPosition 是“父节点坐标系”的点
    return out;
}


// 把 world 坐标转换成“Enemy 父节点坐标”，用于 setPosition3D（很关键，避免坐标系错）
static inline cocos2d::Vec3 WorldToParentSpace(const cocos2d::Node* node,
    const cocos2d::Vec3& worldPos) {
    auto p = node->getParent();
    if (!p) return worldPos;

    cocos2d::Vec3 out = cocos2d::Vec3::ZERO;
    cocos2d::Mat4 inv = p->getWorldToNodeTransform(); // world -> parent local
    inv.transformPoint(worldPos, &out);
    return out;
}


// ==================== EnemyIdleState ====================

EnemyIdleState::EnemyIdleState()
    : _idleTimer(0.0f)
    , _maxIdleTime(2.0f) {
}

EnemyIdleState::~EnemyIdleState() {
}

void EnemyIdleState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered idle state");
    
    // 重置待机计时器
    _idleTimer = 0.0f;
    
    // 随机设置最大待机时间（1-3秒）
    _maxIdleTime = RandomHelper::random_real(1.0f, 3.0f);
    
    enemy->playAnim("idle", true);
}

void EnemyIdleState::onUpdate(Enemy* enemy, float deltaTime) {
    // 统一死亡判断
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }
    
    _idleTimer += deltaTime;
    
    // 待机时间结束后，切换到巡逻状态
    if (_idleTimer >= _maxIdleTime) {
        enemy->getStateMachine()->changeState("Patrol");
    }
    
    // 感知玩家：在视野范围内 -> 追击
    if (HasTarget(enemy)) {
        float d = EnemyWorldPos(enemy).distance(PlayerWorldPos(enemy));
        if (d <= enemy->getViewRange()) {
            enemy->getStateMachine()->changeState("Chase");
            return;
        }
    }
}

void EnemyIdleState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited idle state");
    
    // 清理待机动画（可选，因为下一个状态会停止并替换）
    if (enemy->getSprite()) {
        // 不需要在这里停止所有动作，因为下一个状态的onEnter会调用stopAllActions
    }
}

std::string EnemyIdleState::getStateName() const {
    return "Idle";
}

// ==================== EnemyPatrolState ====================

EnemyPatrolState::EnemyPatrolState()
    : _patrolTarget(Vec3::ZERO)
    , _patrolTimer(0.0f)
    , _maxPatrolTime(5.0f) {
}

EnemyPatrolState::~EnemyPatrolState() {
}

void EnemyPatrolState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered patrol state");

    _patrolTimer = 0.0f;
    _maxPatrolTime = RandomHelper::random_real(3.0f, 7.0f);

    // ★改：用父节点坐标系（Enemy 的 position3D 同一坐标系）
    const Vec3 birth = enemy->getBirthPosition();

    const float patrolRadius = 100.0f;
    const float a = RandomHelper::random_real(0.0f, (float)M_PI * 2.0f);
    _patrolTarget = birth + Vec3(cosf(a) * patrolRadius, 0.0f, sinf(a) * patrolRadius);

    enemy->playAnim("patrol", true);
}

void EnemyPatrolState::onUpdate(Enemy* enemy, float dt) {
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    _patrolTimer += dt;

    // 感知玩家仍然用 world 距离（可以保留你原来的写法）
    if (HasTarget(enemy)) {
        float d = EnemyWorldPos(enemy).distance(PlayerWorldPos(enemy));
        if (d <= enemy->getViewRange()) {
            enemy->getStateMachine()->changeState("Chase");
            return;
        }
    }

    if (enemy->canMove()) {
        Vec3 pos = enemy->getPosition3D();        // ★父节点坐标
        Vec3 dir = _patrolTarget - pos;
        dir.y = 0.0f;

        float dist = dir.length();
        if (dist > 10.0f) {
            dir.normalize();

            float step = enemy->getMoveSpeed() * dt;
            if (step > dist) step = dist;         // ★防止 dt 大时“跨过头”抖动

            enemy->setPosition3D(pos + dir * step);

            if (enemy->getSprite()) {
                float angle = atan2f(dir.x, dir.z) * 180.0f / M_PI;
                enemy->getSprite()->setRotation3D(Vec3(0, angle, 0));
            }
        }
        else {
            enemy->getStateMachine()->changeState("Idle");
            return;
        }
    }

    if (_patrolTimer >= _maxPatrolTime) {
        enemy->getStateMachine()->changeState("Idle");
    }
}


void EnemyPatrolState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited patrol state");
}

std::string EnemyPatrolState::getStateName() const {
    return "Patrol";
}

// ==================== EnemyChaseState ====================

EnemyChaseState::EnemyChaseState()
    : _chaseTimer(0.0f) {
}

EnemyChaseState::~EnemyChaseState() {
}

void EnemyChaseState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered chase state");
    
    // 重置追逐计时器
    _chaseTimer = 0.0f;
    
    // 追逐动画（如果有）
    enemy->playAnim("chase", false);
}

void EnemyChaseState::onUpdate(Enemy* enemy, float deltaTime) {
    // 统一死亡判断
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    _chaseTimer += deltaTime;

    // 没目标直接回家
    if (!HasTarget(enemy)) {
        enemy->getStateMachine()->changeState("Return");
        return;
    }

    // 用 world 坐标做所有距离判断
    const Vec3 enemyWorld = EnemyWorldPos(enemy);
    const Vec3 birthWorld = BirthWorldPos(enemy);

    // 距离出生点太远 -> Return
    float distanceFromBirth = enemyWorld.distance(birthWorld);
    if (distanceFromBirth > enemy->getMaxChaseRange()) {
        enemy->getStateMachine()->changeState("Return");
        return;
    }

    const Vec3 playerWorld = PlayerWorldPos(enemy);
    float distanceToPlayer = enemyWorld.distance(playerWorld);

    // 超出视野 -> Return
    if (distanceToPlayer > enemy->getViewRange()) {
        enemy->getStateMachine()->changeState("Return");
        return;
    }

    // 追上了再攻击（给一个简单攻击距离，避免远程“空挥”）
    const float kAttackRange = 60.0f;
    if (distanceToPlayer <= kAttackRange && enemy->canAttack()) {
        enemy->getStateMachine()->changeState("Attack");
        return;
    }

    // 继续追击移动
    if (enemy->canMove()) {
        Vec3 dir = playerWorld - enemyWorld;
        dir.y = 0;

        if (dir.lengthSquared() > 1e-6f) {
            dir.normalize();

            // 先在 world 空间算新位置，再转回父节点空间 setPosition3D（关键）
            Vec3 newWorld = enemyWorld + dir * enemy->getMoveSpeed() * deltaTime;
            enemy->setPosition3D(WorldToParentSpace(enemy, newWorld));

            // 朝向（沿用你 Patrol 的方式）
            if (enemy->getSprite()) {
                float angle = atan2f(dir.x, dir.z) * 180.0f / M_PI ;
                enemy->getSprite()->setRotation3D(Vec3(0, angle, 0));
            }
        }
    }
}

void EnemyChaseState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited chase state");
}

std::string EnemyChaseState::getStateName() const {
    return "Chase";
}

// ==================== EnemyAttackState ====================

EnemyAttackState::EnemyAttackState()
    : _attackTimer(0.0f)
    , _attackCooldown(1.0f) { // 1秒攻击冷却
}

EnemyAttackState::~EnemyAttackState() {
}

void EnemyAttackState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered attack state");
    
    // 重置攻击计时器
    _attackTimer = 0.0f;
    
    // 播放攻击动画
    enemy->playAnim("chase", true);
    
    // 进入攻击状态，不直接造成伤害
    // 伤害由外部系统触发
}

void EnemyAttackState::onUpdate(Enemy* enemy, float deltaTime) {
    // 统一死亡判断
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    _attackTimer += deltaTime;

    // 攻击冷却结束后，检查玩家是否仍在视野范围内
    if (_attackTimer >= _attackCooldown) {
        //获取玩家位置
        if (!HasTarget(enemy)) {
            enemy->getStateMachine()->changeState("Return");
            return;
        }
        float distance = EnemyWorldPos(enemy).distance(PlayerWorldPos(enemy));


        if (distance <= enemy->getViewRange()) {
            if (enemy->canAttack()) {
                // 再次攻击
                _attackTimer = 0.0f;
                enemy->playAnim("attack", false); //再播一次
            }
            else {
                // 无法攻击，切换到追逐状态
                enemy->getStateMachine()->changeState("Chase");
            }
        }
        else {
            // 玩家超出视野范围，切换到待机状态
            enemy->getStateMachine()->changeState("Return");
        }
    }
}

void EnemyAttackState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited attack state");
}

std::string EnemyAttackState::getStateName() const {
    return "Attack";
}

// ==================== EnemyHitState ====================

EnemyHitState::EnemyHitState()
    : _hitTimer(0.0f)
    , _hitDuration(0.5f) { // 0.5秒受击时间
}

EnemyHitState::~EnemyHitState() {
}

void EnemyHitState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered hit state");
    
    // 重置受击计时器
    _hitTimer = 0.0f;
    
    // 受击动画（如果有）
    enemy->playAnim("hited", false);
}

void EnemyHitState::onUpdate(Enemy* enemy, float deltaTime) {
    // 统一死亡判断
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    _hitTimer += deltaTime;

    // 受击时间结束后，根据情况切换状态
    if (_hitTimer >= _hitDuration) {
        //获取玩家位置
        if (!HasTarget(enemy)) {
            enemy->getStateMachine()->changeState("Return");
            return;
        }
        float distance = EnemyWorldPos(enemy).distance(PlayerWorldPos(enemy));

        if (distance <= enemy->getViewRange()) {
            if (enemy->canAttack()) {
                // 玩家在视野范围内，切换到攻击状态
                enemy->getStateMachine()->changeState("Attack");
            }
            else {
                // 无法攻击，切换到追逐状态
                enemy->getStateMachine()->changeState("Chase");
            }
        }
        else {
            // 玩家不在视野范围内，切换到待机状态
            enemy->getStateMachine()->changeState("Idle");
        }
    }
}

void EnemyHitState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited hit state");
}

std::string EnemyHitState::getStateName() const {
    return "Hit";
}

// ==================== EnemyDeadState ====================

EnemyDeadState::EnemyDeadState()
    : _isDeadProcessed(false) {
}

EnemyDeadState::~EnemyDeadState() {
}

void EnemyDeadState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered dead state");
    _isDeadProcessed = false;
    enemy->playAnim("dying", false); // 死亡动画

    // 死后消失
    // 注意：这个是跑在 enemy Node 上，不会被 playAnim stop 掉
    enemy->runAction(Sequence::create(
        DelayTime::create(1.5f),
        RemoveSelf::create(),
        nullptr
    ));
}

void EnemyDeadState::onUpdate(Enemy* enemy, float deltaTime) {
    // 死亡状态不再切换到任何其他状态
    if (!_isDeadProcessed) {
        // 等待一段时间后移除敌人
        enemy->runAction(Sequence::create(
            DelayTime::create(1.5f),
            RemoveSelf::create(),
            nullptr
        ));
        
        _isDeadProcessed = true;
    }
}

void EnemyDeadState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited dead state");
}

std::string EnemyDeadState::getStateName() const {
    return "Dead";
}

// ==================== ReturnState ====================
ReturnState::ReturnState()
    : _returnTarget(Vec3::ZERO) {
}

ReturnState::~ReturnState() {
}


void ReturnState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered return state");

    // 改：回家目标直接用父节点坐标系
    _returnTarget = enemy->getBirthPosition();
    enemy->playAnim("patrol", true);
}

void ReturnState::onUpdate(Enemy* enemy, float dt) {
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    // 玩家回到感知范围 -> Chase（保持你原逻辑）
    float distanceToPlayer = FLT_MAX;
    if (HasTarget(enemy)) {
        distanceToPlayer = EnemyWorldPos(enemy).distance(PlayerWorldPos(enemy));
    }
    if (distanceToPlayer <= enemy->getViewRange()) {
        enemy->getStateMachine()->changeState("Chase");
        return;
    }

    if (!enemy->canMove()) return;

    Vec3 pos = enemy->getPosition3D();     // 父节点坐标
    Vec3 dir = _returnTarget - pos;
    dir.y = 0.0f;

    float dist = dir.length();
    if (dist > 10.0f) {
        dir.normalize();

        float step = enemy->getMoveSpeed() * dt;
        if (step > dist) step = dist;      // 防止 overshoot 抖动/跳

        enemy->setPosition3D(pos + dir * step);

        if (enemy->getSprite()) {
            float angle = atan2f(dir.x, dir.z) * 180.0f / M_PI;
            enemy->getSprite()->setRotation3D(Vec3(0, angle, 0));
        }
    }
    else {
        // 锁死到出生点，再切 Patrol，避免“阈值边缘卡住”
        pos.x = _returnTarget.x;
        pos.z = _returnTarget.z;
        enemy->setPosition3D(pos);

        enemy->getStateMachine()->changeState("Patrol");
    }
}

void ReturnState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited return state");
}

std::string ReturnState::getStateName() const {
    return "Return";
}


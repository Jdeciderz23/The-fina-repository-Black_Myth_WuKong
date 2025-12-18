#include "EnemyStates.h"
#include "cocos2d.h"

USING_NS_CC;

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
    
    // 待机动画（如果有）
    // enemy->_sprite->runAction(Animation3D::create(...));
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
    
    // TODO: 检测玩家是否在感知范围内，如果是则切换到追逐状态
    // Vec3 playerPosition = ...;
    // float distance = enemy->getPosition3D().distance(playerPosition);
    // if (distance <= enemy->getViewRange()) {
    //     enemy->getStateMachine()->changeState("Chase");
    // }
}

void EnemyIdleState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited idle state");
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
    
    // 重置巡逻计时器
    _patrolTimer = 0.0f;
    
    // 随机设置最大巡逻时间和巡逻目标点
    _maxPatrolTime = RandomHelper::random_real(3.0f, 7.0f);
    
    // 在当前位置附近随机生成巡逻目标点
    Vec3 currentPos = enemy->getPosition3D();
    _patrolTarget.x = currentPos.x + RandomHelper::random_real(-100.0f, 100.0f);
    _patrolTarget.y = currentPos.y;
    _patrolTarget.z = currentPos.z + RandomHelper::random_real(-100.0f, 100.0f);
    
    // 巡逻动画（如果有）
    // enemy->_sprite->runAction(Animation3D::create(...));
}

void EnemyPatrolState::onUpdate(Enemy* enemy, float deltaTime) {
    // 统一死亡判断
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }
    
    _patrolTimer += deltaTime;
    
    // 移动向巡逻目标点
    if (enemy->canMove()) {
        Vec3 currentPos = enemy->getPosition3D();
        Vec3 direction = _patrolTarget - currentPos;
        float distance = direction.length();
        
        if (distance > 10.0f) { // 接近目标点（阈值10单位）
            direction.normalize();
            Vec3 newPos = currentPos + direction * enemy->getMoveSpeed() * deltaTime;
            enemy->setPosition(newPos);
        } else {
            // 到达目标点，切换到待机状态
            enemy->getStateMachine()->changeState("Idle");
        }
    }
    
    // 巡逻时间过长，切换到待机状态
    if (_patrolTimer >= _maxPatrolTime) {
        enemy->getStateMachine()->changeState("Idle");
    }
    
    // TODO: 检测玩家是否在感知范围内，如果是则切换到追逐状态
    // Vec3 playerPosition = ...;
    // float distance = enemy->getPosition3D().distance(playerPosition);
    // if (distance <= enemy->getViewRange()) {
    //     enemy->getStateMachine()->changeState("Chase");
    // }
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
    // enemy->_sprite->runAction(Animation3D::create(...));
}

void EnemyChaseState::onUpdate(Enemy* enemy, float deltaTime) {
    // 统一死亡判断
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }
    
    _chaseTimer += deltaTime;
    
    // TODO: 获取玩家位置
    Vec3 playerPosition = Vec3::ZERO; // 临时值，需要替换为实际玩家位置
    
    // 计算与玩家的距离
    Vec3 currentPos = enemy->getPosition3D();
    float distance = currentPos.distance(playerPosition);
    
    // 如果玩家在视野范围内且在移动范围内，尝试移动或攻击
    if (distance <= enemy->getViewRange()) {
        if (enemy->canAttack()) {
            // 进入攻击状态
            enemy->getStateMachine()->changeState("Attack");
        } else if (enemy->canMove()) {
            // 继续追逐
            Vec3 direction = playerPosition - currentPos;
            direction.normalize();
            Vec3 newPos = currentPos + direction * enemy->getMoveSpeed() * deltaTime;
            enemy->setPosition(newPos);
            
            // TODO: 实现路径寻找算法，避免障碍物
        }
    } else {
        // 玩家超出视野范围，切换到待机状态
        enemy->getStateMachine()->changeState("Idle");
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
    // enemy->getSprite()->runAction(Animation3D::create(...));
    
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
        // TODO: 获取玩家位置
        Vec3 playerPosition = Vec3::ZERO; // 临时值
        
        float distance = enemy->getPosition3D().distance(playerPosition);
        
        if (distance <= enemy->getViewRange()) {
            if (enemy->canAttack()) {
                // 再次攻击
                _attackTimer = 0.0f;
                // enemy->getSprite()->runAction(Animation3D::create(...));
            } else {
                // 无法攻击，切换到追逐状态
                enemy->getStateMachine()->changeState("Chase");
            }
        } else {
            // 玩家超出视野范围，切换到待机状态
            enemy->getStateMachine()->changeState("Idle");
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
    // enemy->getSprite()->runAction(Animation3D::create(...));
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
        // TODO: 获取玩家位置
        Vec3 playerPosition = Vec3::ZERO; // 临时值
        
        float distance = enemy->getPosition3D().distance(playerPosition);
        
        if (distance <= enemy->getViewRange()) {
            if (enemy->canAttack()) {
                // 玩家在视野范围内，切换到攻击状态
                enemy->getStateMachine()->changeState("Attack");
            } else {
                // 无法攻击，切换到追逐状态
                enemy->getStateMachine()->changeState("Chase");
            }
        } else {
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
    
    // 死亡动画（如果有）
    if (enemy->getSprite()) {
        auto rotate = RotateBy::create(1.0f, Vec3(180, 0, 0));
        auto fadeOut = FadeOut::create(1.0f);
        auto sequence = Sequence::create(rotate, fadeOut, nullptr);
        enemy->getSprite()->runAction(sequence);
    }
    
    // 发送敌人死亡事件
    // EventManager::getInstance()->dispatchEvent("EnemyDead", enemy);
    
    _isDeadProcessed = false;
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
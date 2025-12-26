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
    
    // 播放待机动画
    if (enemy->getSprite()) {
        // 停止当前所有动作
        enemy->getSprite()->stopAllActions();
        
        // 暂时移除待机动画，专注于基础模型显示
        // auto animation = Animation3D::create("Enemy/idle.c3b");
        // if (animation) {
        //     auto animate = Animate3D::create(animation);
        //     auto repeat = RepeatForever::create(animate);
        //     enemy->getSprite()->runAction(repeat);
        // }
    }
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
    
    // 重置巡逻计时器
    _patrolTimer = 0.0f;
    
    // 随机设置最大巡逻时间和巡逻目标点
    _maxPatrolTime = RandomHelper::random_real(3.0f, 7.0f);
    
    // 在当前位置附近随机生成巡逻目标点
    Vec3 birthPos = enemy->getBirthPosition();

    float patrolRadius = 100.0f;
    float angle = RandomHelper::random_real(0.0f, (float)M_PI * 2);

    _patrolTarget.x = birthPos.x + cosf(angle) * patrolRadius;
    _patrolTarget.y = birthPos.y;
    _patrolTarget.z = birthPos.z + sinf(angle) * patrolRadius;
    
    // 暂时移除巡逻动画，专注于基础模型显示
    // if (enemy->getSprite()) {
    //     enemy->getSprite()->stopAllActions();
    //     auto animation = Animation3D::create("Enemy/patrol.c3b");
    //     if (animation) {
    //         auto animate = Animate3D::create(animation);
    //         auto repeat = RepeatForever::create(animate);
    //         enemy->getSprite()->runAction(repeat);
    //     }
    // }
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
            
            // 根据移动方向调整模型朝向（考虑初始180度旋转）
            if (enemy->getSprite()) {
                float angle = atan2f(direction.x, direction.z) * 180.0f / M_PI+45.0f;
                enemy->getSprite()->setRotation3D(Vec3(0, angle, 0));
            }
            
            Vec3 newPos = currentPos + direction * enemy->getMoveSpeed() * deltaTime;
            enemy->setPosition3D(newPos);
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
    Vec3 currentPos = enemy->getPosition3D();
    
    // 计算与玩家的距离
    float distanceFromBirth = currentPos.distance(enemy->getBirthPosition());
    if (distanceFromBirth > enemy->getMaxChaseRange()) {
        // 追得太远，强制回家
        enemy->getStateMachine()->changeState("Return");
        return;
    }
    // TODO: 获取玩家位置
    Vec3 playerPosition = Vec3::ZERO; // 临时值，需要替换为实际玩家位置
    float distanceToPlayer = currentPos.distance(playerPosition);

    // 如果玩家在视野范围内且在移动范围内，尝试移动或攻击
    if (distanceToPlayer <= enemy->getViewRange()) {
        if (enemy->canAttack()) {
            // 进入攻击状态
            enemy->getStateMachine()->changeState("Attack");
        } else if (enemy->canMove()) {
            // 继续追逐
            Vec3 direction = playerPosition - currentPos;
            direction.normalize();
            Vec3 newPos = currentPos + direction * enemy->getMoveSpeed() * deltaTime;
            enemy->setPosition3D(newPos);
            
            // TODO: 实现路径寻找算法，避免障碍物
        }
    } else {
        // 玩家超出视野范围，切换到待机状态
        enemy->getStateMachine()->changeState("Return");
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

// ==================== ReturnState ====================
ReturnState::ReturnState()
    : _returnTarget(Vec3::ZERO) {
}

ReturnState::~ReturnState() {
}

void ReturnState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered return state");

    // 回家的目标 = 出生点
    _returnTarget = enemy->getBirthPosition();

    // 暂时移除回家动画，专注于基础模型显示
    // if (enemy->getSprite()) {
    //     enemy->getSprite()->stopAllActions();
    //     auto animation = Animation3D::create("Enemy/patrol.c3b");
    //     if (animation) {
    //         enemy->getSprite()->runAction(
    //             RepeatForever::create(Animate3D::create(animation))
    //         );
    //     }
    // }
}

void ReturnState::onUpdate(Enemy* enemy, float deltaTime) {
    // 统一死亡判断
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    Vec3 currentPos = enemy->getPosition3D();
    // ===== 玩家重新进入感知范围,立刻追击 =====
    // TODO: 替换为真实玩家坐标
    Vec3 playerPosition = Vec3::ZERO;
    float distanceToPlayer =
        currentPos.distance(playerPosition);

    if (distanceToPlayer <= enemy->getViewRange()) {
        enemy->getStateMachine()->changeState("Chase");
        return;
    }

    //回家逻辑
    if (!enemy->canMove()) return;

    Vec3 direction = _returnTarget - currentPos;
    float distance = direction.length();

    if (distance > 10.0f) {
        direction.normalize();

        // ===== 临时方案：直线位移（未来替换为寻路） =====
        Vec3 newPos =currentPos + direction * enemy->getMoveSpeed() * deltaTime;
        enemy->setPosition3D(newPos);
    }
    else {
        // 到家了 → 开始巡逻
        enemy->getStateMachine()->changeState("Patrol");
    }
}


void ReturnState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited return state");
}

std::string ReturnState::getStateName() const {
    return "Return";
}


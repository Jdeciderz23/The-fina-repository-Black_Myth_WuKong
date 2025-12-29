#include "EnemyStates.h"
#include "cocos2d.h"
#include <cfloat>
#include "combat/CombatComponent.h"
#include "player/Wukong.h"
#include "scene_ui/UIManager.h"

USING_NS_CC;

// 检查敌人是否有有效目标
// @param e 敌人指针
// @return 如果敌人有有效目标则返回true，否则返回false
static inline bool HasTarget(const Enemy* e) {
    if (!e || !e->getTarget()) return false;
    // 如果目标是悟空，且已经死亡，视为没有有效目标
    return !e->getTarget()->isDead();
}

// 获取敌人的世界坐标
// @param e 敌人指针
// @return 敌人的世界坐标
static inline cocos2d::Vec3 EnemyWorldPos(const Enemy* e) {
    return e->getWorldPosition3D();
}

// 获取玩家(悟空)的世界坐标
// @param e 敌人指针
// @return 玩家的世界坐标
static inline cocos2d::Vec3 PlayerWorldPos(const Enemy* e) {
    return e->getTargetWorldPos();
}

// 获取敌人出生点的世界坐标
// @param e 敌人指针
// @return 敌人出生点的世界坐标
static inline cocos2d::Vec3 BirthWorldPos(const Enemy* e) {
    auto p = e->getParent();
    if (!p) return e->getBirthPosition();  // 没父节点就当作世界坐标用

    cocos2d::Vec3 out = cocos2d::Vec3::ZERO;
    cocos2d::Mat4 m = p->getNodeToWorldTransform();   // parent local -> world
    m.transformPoint(e->getBirthPosition(), &out);     // birthPosition 是“父节点坐标系”的点
    return out;
}

// 把世界坐标转换成“Enemy父节点坐标”，用于setPosition3D
// @param node 节点指针
// @param worldPos 世界坐标
// @return 转换后的父节点坐标
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

// 构造函数：初始化待机计时器和最大待机时间
EnemyIdleState::EnemyIdleState()
    : _idleTimer(0.0f)
    , _maxIdleTime(2.0f) {
}

// 析构函数
EnemyIdleState::~EnemyIdleState() {
}

// 进入待机状态时执行的操作
// @param enemy 敌人指针
void EnemyIdleState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered idle state");
    
    // 重置待机计时器
    _idleTimer = 0.0f;
    
    // 随机设置最大待机时间（1-3秒）
    _maxIdleTime = RandomHelper::random_real(1.0f, 3.0f);
    
    // 播放待机动画
    enemy->playAnim("idle", true);
}

// 待机状态每一帧执行的操作
// @param enemy 敌人指针
// @param deltaTime 时间间隔
void EnemyIdleState::onUpdate(Enemy* enemy, float deltaTime) {
    // 统一死亡判断
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }
    
    // 更新待机计时器
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

// 离开待机状态时执行的操作
// @param enemy 敌人指针
void EnemyIdleState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited idle state");
    
    // 清理待机动画（可选，因为下一个状态会停止并替换）
    if (enemy->getSprite()) {
        // 不需要在这里停止所有动作，因为下一个状态的onEnter会调用stopAllActions
    }
}

// 获取状态名称
// @return 状态名称字符串
std::string EnemyIdleState::getStateName() const {
    return "Idle";
}

// ==================== EnemyPatrolState ====================

// 构造函数：初始化巡逻目标点、巡逻计时器和最大巡逻时间
EnemyPatrolState::EnemyPatrolState()
    : _patrolTarget(Vec3::ZERO)
    , _patrolTimer(0.0f)
    , _maxPatrolTime(5.0f) {
}

// 析构函数
EnemyPatrolState::~EnemyPatrolState() {
}

// 进入巡逻状态时执行的操作
// @param enemy 敌人指针
void EnemyPatrolState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered patrol state");
    
    // 重置巡逻计时器
    _patrolTimer = 0.0f;
    
    // 随机设置最大巡逻时间（3-7秒）
    _maxPatrolTime = RandomHelper::random_real(3.0f, 7.0f);
    
    // 在出生位置附近随机生成巡逻目标点
    Vec3 birthPos = enemy->getBirthPosition();

    float patrolRadius = 100.0f;
    float angle = RandomHelper::random_real(0.0f, (float)M_PI * 2);

    _patrolTarget.x = birthPos.x + cosf(angle) * patrolRadius;
    _patrolTarget.y = birthPos.y;
    _patrolTarget.z = birthPos.z + sinf(angle) * patrolRadius;
    
    // 播放巡逻动画
    enemy->playAnim("patrol", true);
}

// 巡逻状态每一帧执行的操作
// @param enemy 敌人指针
// @param deltaTime 时间间隔
void EnemyPatrolState::onUpdate(Enemy* enemy, float deltaTime) {
    // 统一死亡判断
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }
    
    // 更新巡逻计时器
    _patrolTimer += deltaTime;
    
    // 感知玩家：在视野范围内 -> 追击
    if (HasTarget(enemy)) {
        float d = EnemyWorldPos(enemy).distance(PlayerWorldPos(enemy));
        if (d <= enemy->getViewRange()) {
            enemy->getStateMachine()->changeState("Chase");
            return;
        }
    }

    // 移动向巡逻目标点
    if (enemy->canMove()) {
        Vec3 currentPos = enemy->getPosition3D();
        Vec3 direction = _patrolTarget - currentPos;
        float distance = direction.length();
        
        if (distance > 10.0f) { // 接近目标点（阈值10单位）
            direction.normalize();
            
            // 根据移动方向调整模型朝向（考虑初始180度旋转）
            if (enemy->getSprite()) {
                float angle = atan2f(direction.x, direction.z) * 180.0f / M_PI + 45.0f;
                enemy->getSprite()->setRotation3D(Vec3(0, angle, 0));
            }
            
            // 计算新位置并移动
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
}

// 离开巡逻状态时执行的操作
// @param enemy 敌人指针
void EnemyPatrolState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited patrol state");
}

// 获取状态名称
// @return 状态名称字符串
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

// 追逐状态每一帧执行的操作
// @param enemy 敌人指针
// @param deltaTime 时间间隔
void EnemyChaseState::onUpdate(Enemy* enemy, float deltaTime) {
    // 统一死亡判断
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    /*_chaseTimer += deltaTime;
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
    }*/
    // 更新追逐计时器
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

    // 追上了再攻击（给一个简单攻击距离，增加到 80，配合攻击判定的膨胀）
    const float kAttackRange = 80.0f;
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
                float angle = atan2f(dir.x, dir.z) * 180.0f / M_PI + 45.0f;
                enemy->getSprite()->setRotation3D(Vec3(0, angle, 0));
            }
        }
    }
}

// 离开追逐状态时执行的操作
// @param enemy 敌人指针
void EnemyChaseState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited chase state");
}

// 获取状态名称
// @return 状态名称字符串
std::string EnemyChaseState::getStateName() const {
    return "Chase";
}

// ==================== EnemyAttackState ====================

// 构造函数：初始化攻击计时器和攻击冷却时间
EnemyAttackState::EnemyAttackState()
    : _attackTimer(0.0f)
    , _attackCooldown(3.0f) { // 3秒攻击冷却
}

// 析构函数
EnemyAttackState::~EnemyAttackState() {
}

// 进入攻击状态时执行的操作
// @param enemy 敌人指针
void EnemyAttackState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered attack state");
    
    // 重置攻击计时器和标志
    _attackTimer = 0.0f;
    _attacked = false;
    
    // 播放攻击动画
    enemy->playAnim("attack", false);
}

// 攻击状态每一帧执行的操作
// @param enemy 敌人指针
// @param deltaTime 时间间隔
void EnemyAttackState::onUpdate(Enemy* enemy, float deltaTime) {
    // 统一死亡判断
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    // 更新攻击计时器
    _attackTimer += deltaTime;

    // 攻击命中检测：在动画播放到 0.3 秒左右执行一次判定
    if (!_attacked && _attackTimer >= 0.3f) {
        _attacked = true;
        auto combat = enemy->getCombat();
        auto target = enemy->getTarget();
        CCLOG("EnemyAttackState: Attempting attack. Combat: %p, Target: %p", combat, target);
        if (combat && target) {
            // 将目标（悟空）放入列表
            std::vector<cocos2d::Node*> targets = { static_cast<cocos2d::Node*>(target) };
            int hits = combat->executeMeleeAttack(enemy->getCollider(), targets);
            if (hits > 0) {
                CCLOG("Enemy hit player! Damage dealt. Hits: %d", hits);
            } else {
                CCLOG("Enemy attack missed.");
            }
        }
    }

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
                _attacked = false; // 重置标志位
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

// 离开攻击状态时执行的操作
// @param enemy 敌人指针
void EnemyAttackState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited attack state");
}

// 获取状态名称
// @return 状态名称字符串
std::string EnemyAttackState::getStateName() const {
    return "Attack";
}

// ==================== EnemyHitState ====================

// 构造函数：初始化受击计时器和受击持续时间
EnemyHitState::EnemyHitState()
    : _hitTimer(0.0f)
    , _hitDuration(0.5f) { // 0.5秒受击时间
}

// 析构函数
EnemyHitState::~EnemyHitState() {
}

// 进入受击状态时执行的操作
// @param enemy 敌人指针
void EnemyHitState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered hit state");
    
    // 重置受击计时器
    _hitTimer = 0.0f;
    
    // 播放受击动画
    enemy->playAnim("hited", false);
}

// 受击状态每一帧执行的操作
// @param enemy 敌人指针
// @param deltaTime 时间间隔
void EnemyHitState::onUpdate(Enemy* enemy, float deltaTime) {
    // 统一死亡判断
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    // 更新受击计时器
    _hitTimer += deltaTime;

    // 受击时间结束后，根据情况切换状态
    if (_hitTimer >= _hitDuration) {
        //获取玩家位置
        if (!HasTarget(enemy)) {
            enemy->getStateMachine()->changeState("Return");
            return;
        }
        float distance = EnemyWorldPos(enemy).distance(PlayerWorldPos(enemy));

        if (distance <= 80.0f) { // 使用与 ChaseState 一致的攻击距离
            if (enemy->canAttack()) {
                enemy->getStateMachine()->changeState("Attack");
            } else {
                enemy->getStateMachine()->changeState("Chase");
            }
        } else if (distance <= enemy->getViewRange()) {
            enemy->getStateMachine()->changeState("Chase");
        } else {
            enemy->getStateMachine()->changeState("Return");
        }
    }
}

// 离开受击状态时执行的操作
// @param enemy 敌人指针
void EnemyHitState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited hit state");
}

// 获取状态名称
// @return 状态名称字符串
std::string EnemyHitState::getStateName() const {
    return "Hit";
}

// ==================== EnemyDeadState ====================

// 构造函数：初始化死亡处理标志
EnemyDeadState::EnemyDeadState()
    : _isDeadProcessed(false) {
}

// 析构函数
EnemyDeadState::~EnemyDeadState() {
}

// 进入死亡状态时执行的操作
// @param enemy 敌人指针
void EnemyDeadState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered dead state");
    _isDeadProcessed = false;
    // 播放死亡动画
    enemy->playAnim("dying", false); 

    // 死亡动画结束后自动移除敌人
    // 注意：这个是跑在 enemy Node 上，不会被 playAnim stop 掉
    // 只有在onEnter中执行一次，onUpdate中不再执行
    enemy->runAction(Sequence::create(
        DelayTime::create(1.5f),
        CallFunc::create([enemy]() {
            CCLOG("Enemy is being removed after death animation");
            
            // 通知游戏管理器从敌人列表中移除此敌人
            // 通过一个全局事件通知系统
            cocos2d::EventCustom event("enemy_died");
            event.setUserData(enemy);
            cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);
            
            // 执行实际的移除操作
            enemy->removeFromParent();
        }),
        nullptr
    ));
}

// 死亡状态每一帧执行的操作
// @param enemy 敌人指针
// @param deltaTime 时间间隔
void EnemyDeadState::onUpdate(Enemy* enemy, float deltaTime) {
    // 死亡状态不再切换到任何其他状态
    // 移除操作已经在onEnter中执行，这里不再执行
    _isDeadProcessed = true; // 标记已处理
}

// 离开死亡状态时执行的操作
// @param enemy 敌人指针
void EnemyDeadState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited dead state");
}

// 获取状态名称
// @return 状态名称字符串
std::string EnemyDeadState::getStateName() const {
    return "Dead";
}

// ==================== ReturnState ====================

// 构造函数：初始化返回目标点
ReturnState::ReturnState()
    : _returnTarget(Vec3::ZERO) {
}

// 析构函数
ReturnState::~ReturnState() {
}

// 进入返回状态时执行的操作
// @param enemy 敌人指针
void ReturnState::onEnter(Enemy* enemy) {
    CCLOG("Enemy entered return state");

    // 设置返回目标点为出生位置（使用父节点坐标系）
    _returnTarget = enemy->getBirthPosition();
    // 播放巡逻动画（作为返回时的移动动画）
    enemy->playAnim("patrol", true);
}

// 返回状态每一帧执行的操作
// @param enemy 敌人指针
// @param dt 时间间隔
void ReturnState::onUpdate(Enemy* enemy, float dt) {
    // 统一死亡判断
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


// 离开返回状态时执行的操作
// @param enemy 敌人指针
void ReturnState::onExit(Enemy* enemy) {
    CCLOG("Enemy exited return state");
}

// 获取状态名称
// @return 状态名称字符串
std::string ReturnState::getStateName() const {
    return "Return";
}


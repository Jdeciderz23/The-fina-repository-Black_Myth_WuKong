/**
 * EnemyStates
 * --------------------
 * 仅负责 Enemy 的 AI 行为状态切换与移动决策（执行动画+行为）
 * 不负责：
 * - 伤害计算
 * - 攻击命中
 * - 冷却判定
 * - 血量 / 死亡逻辑
 *
 * 所有战斗与数值相关判断，必须通过 Enemy 对外接口完成
 */

#pragma once

#include "BaseState.h"
#include "Enemy.h"

/**
 * @class EnemyStates
 * @brief 敌人状态集合，包含所有敌人状态类的定义
 */

/**
 * @class EnemyIdleState
 * @brief 敌人待机状态类
 */
class EnemyIdleState : public BaseState<Enemy> {
public:
    EnemyIdleState();
    virtual ~EnemyIdleState();
    virtual void onEnter(Enemy* enemy) override;                    //刚进入这个状态做什么
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;  //每一帧这个状态下应该做什么
    virtual void onExit(Enemy* enemy) override;                     //离开这个状态之前做什么
    virtual std::string getStateName() const override;
    
private:
    float _idleTimer;       ///< 待机计时器
    float _maxIdleTime;     ///< 最大待机时间
};

/**
 * @class EnemyPatrolState
 * @brief 敌人巡逻状态类
 */
class EnemyPatrolState : public BaseState<Enemy> {
public:
    EnemyPatrolState();
    virtual ~EnemyPatrolState();
    virtual void onEnter(Enemy* enemy) override;
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    virtual void onExit(Enemy* enemy) override;
    virtual std::string getStateName() const override;
    
private:
    Vec3 _patrolTarget;     ///< 巡逻目标点
    float _patrolTimer;     ///< 巡逻计时器
    float _maxPatrolTime;   ///< 最大巡逻时间
};

/**
 * @class EnemyChaseState
 * @brief 敌人追逐状态类
 */
class EnemyChaseState : public BaseState<Enemy> {
public:
    EnemyChaseState();
    virtual ~EnemyChaseState();
    virtual void onEnter(Enemy* enemy) override;
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    virtual void onExit(Enemy* enemy) override;
    virtual std::string getStateName() const override;
    
private:
    float _chaseTimer;      ///< 追逐计时器
};

/**
 * @class EnemyAttackState
 * @brief 敌人攻击状态类
 */
class EnemyAttackState : public BaseState<Enemy> {
public:
    EnemyAttackState();
    virtual ~EnemyAttackState();
    virtual void onEnter(Enemy* enemy) override;
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    virtual void onExit(Enemy* enemy) override;
    virtual std::string getStateName() const override;
    
private:
    float _attackTimer;     ///< 攻击计时器
    float _attackCooldown;  ///< 攻击冷却时间
};

/**
 * @class EnemyHitState
 * @brief 敌人受击状态类
 */
class EnemyHitState : public BaseState<Enemy> {
public:
    EnemyHitState();
    virtual ~EnemyHitState();
    virtual void onEnter(Enemy* enemy) override;
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    virtual void onExit(Enemy* enemy) override;
    virtual std::string getStateName() const override;
    
private:
    float _hitTimer;        ///< 受击计时器
    float _hitDuration;     ///< 受击持续时间
};

/**
 * @class EnemyDeadState
 * @brief 敌人死亡状态类
 */
class EnemyDeadState : public BaseState<Enemy> {
public:
    EnemyDeadState();
    virtual ~EnemyDeadState();
    virtual void onEnter(Enemy* enemy) override;
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    virtual void onExit(Enemy* enemy) override;
    virtual std::string getStateName() const override;
    
private:
    bool _isDeadProcessed;  ///< 死亡处理是否完成
};

/**
 * @class ReturnState
 * @brief 敌人死亡状态类
 */

class ReturnState : public BaseState<Enemy> {
public:
    ReturnState();
    virtual ~ReturnState();

    virtual void onEnter(Enemy* enemy) override;
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    virtual void onExit(Enemy* enemy) override;

    virtual std::string getStateName() const override;

private:
    Vec3 _returnTarget;
};

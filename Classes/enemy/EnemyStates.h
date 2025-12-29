// EnemyStates
// --------------------
// 仅负责 Enemy 的 AI 行为状态切换与移动决策（执行动画+行为）
// 不负责：
// - 伤害计算
// - 攻击命中
// - 冷却判定
// - 血量 / 死亡逻辑
//
// 所有战斗与数值相关判断，必须通过 Enemy 对外接口完成

#pragma once

#include "BaseState.h"
#include "Enemy.h"

// EnemyStates 命名空间：敌人状态集合，包含所有敌人状态类的定义

// EnemyIdleState 类：敌人待机状态类
class EnemyIdleState : public BaseState<Enemy> {
public:
    EnemyIdleState();
    virtual ~EnemyIdleState();
    // 刚进入这个状态时执行的操作
    virtual void onEnter(Enemy* enemy) override;
    // 每一帧在这个状态下执行的操作
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    // 离开这个状态之前执行的操作
    virtual void onExit(Enemy* enemy) override;
    // 获取状态名称
    virtual std::string getStateName() const override;
    
private:
    float _idleTimer;      // 待机计时器
    float _maxIdleTime;    // 最大待机时间
};

// EnemyPatrolState 类：敌人巡逻状态类
class EnemyPatrolState : public BaseState<Enemy> {
public:
    EnemyPatrolState();
    virtual ~EnemyPatrolState();
    // 刚进入这个状态时执行的操作
    virtual void onEnter(Enemy* enemy) override;
    // 每一帧在这个状态下执行的操作
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    // 离开这个状态之前执行的操作
    virtual void onExit(Enemy* enemy) override;
    // 获取状态名称
    virtual std::string getStateName() const override;
    
private:
    Vec3 _patrolTarget;     // 巡逻目标点
    float _patrolTimer;     // 巡逻计时器
    float _maxPatrolTime;   // 最大巡逻时间
};

// EnemyChaseState 类：敌人追逐状态类
class EnemyChaseState : public BaseState<Enemy> {
public:
    EnemyChaseState();
    virtual ~EnemyChaseState();
    // 刚进入这个状态时执行的操作
    virtual void onEnter(Enemy* enemy) override;
    // 每一帧在这个状态下执行的操作
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    // 离开这个状态之前执行的操作
    virtual void onExit(Enemy* enemy) override;
    // 获取状态名称
    virtual std::string getStateName() const override;
    
private:
    float _chaseTimer;      // 追逐计时器
};

// EnemyAttackState 类：敌人攻击状态类
class EnemyAttackState : public BaseState<Enemy> {
public:
    EnemyAttackState();
    virtual ~EnemyAttackState();
    // 刚进入这个状态时执行的操作
    virtual void onEnter(Enemy* enemy) override;
    // 每一帧在这个状态下执行的操作
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    // 离开这个状态之前执行的操作
    virtual void onExit(Enemy* enemy) override;
    // 获取状态名称
    virtual std::string getStateName() const override;
    
private:
    float _attackTimer;     // 攻击计时器
    float _attackCooldown;  // 攻击冷却时间
    bool _attacked;         // 是否已执行攻击判定
};

// EnemyHitState 类：敌人受击状态类
class EnemyHitState : public BaseState<Enemy> {
public:
    EnemyHitState();
    virtual ~EnemyHitState();
    // 刚进入这个状态时执行的操作
    virtual void onEnter(Enemy* enemy) override;
    // 每一帧在这个状态下执行的操作
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    // 离开这个状态之前执行的操作
    virtual void onExit(Enemy* enemy) override;
    // 获取状态名称
    virtual std::string getStateName() const override;
    
private:
    float _hitTimer;        // 受击计时器
    float _hitDuration;     // 受击持续时间
};

// EnemyDeadState 类：敌人死亡状态类
class EnemyDeadState : public BaseState<Enemy> {
public:
    EnemyDeadState();
    virtual ~EnemyDeadState();
    // 刚进入这个状态时执行的操作
    virtual void onEnter(Enemy* enemy) override;
    // 每一帧在这个状态下执行的操作
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    // 离开这个状态之前执行的操作
    virtual void onExit(Enemy* enemy) override;
    // 获取状态名称
    virtual std::string getStateName() const override;
    
private:
    bool _isDeadProcessed;  // 死亡处理是否完成
};

// ReturnState 类：敌人返回出生点状态类
class ReturnState : public BaseState<Enemy> {
public:
    ReturnState();
    virtual ~ReturnState();
    // 刚进入这个状态时执行的操作
    virtual void onEnter(Enemy* enemy) override;
    // 每一帧在这个状态下执行的操作
    virtual void onUpdate(Enemy* enemy, float deltaTime) override;
    // 离开这个状态之前执行的操作
    virtual void onExit(Enemy* enemy) override;
    // 获取状态名称
    virtual std::string getStateName() const override;
    
private:
    Vec3 _returnTarget;     // 返回目标点（出生点）
};

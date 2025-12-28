// BossStates.h
#pragma once

#include "Enemy.h"
#include "core/StateMachine.h"   // 你工程里 BaseState/StateMachine 很可能在这里
#include <string>

// ========== 技能配置（AttackState 用）==========
struct BossSkillConfig {
    std::string skill;   // "Combo3" / "DashSlash" / "GroundSlam" / "Roar" / "LeapSlam"
    std::string anim;    // 对应动画文件名（不带 .c3b）

    float windup = 0.f;    // 前摇
    float moveTime = 0.f;  // 位移时间（Dash/Leap 用）
    float active = 0.f;    // 生效窗口
    float recovery = 0.f;  // 后摇

    float dashDistance = 0.f; // Dash/Leap 跳到“离玩家多近”（世界单位）
    float hitRadius = 0.f;    // 命中球半径（世界单位）
    float damage = 0.f;       // 伤害（先打印，后面再接你的扣血）
    bool  lockTarget = true;  // 是否锁定落点
};

// ========== Boss Idle ==========
class BossIdleState : public BaseState<Enemy> {
public:
    void onEnter(Enemy* enemy) override;
    void onUpdate(Enemy* enemy, float dt) override;
    void onExit(Enemy* enemy) override;
    std::string getStateName() const override { return "Idle"; }
};

// ========== Boss Chase ==========
class BossChaseState : public BaseState<Enemy> {
public:
    void onEnter(Enemy* enemy) override;
    void onUpdate(Enemy* enemy, float dt) override;
    void onExit(Enemy* enemy) override;
    std::string getStateName() const override { return "Chase"; }
};

// ========== Phase Change (Roar 1s) ==========
class BossPhaseChangeState : public BaseState<Enemy> {
public:
    void onEnter(Enemy* enemy) override;
    void onUpdate(Enemy* enemy, float dt) override;
    void onExit(Enemy* enemy) override;
    std::string getStateName() const override { return "PhaseChange"; }
private:
    float _timer = 0.f;
};

// ========== Boss Attack ==========
class BossAttackState : public BaseState<Enemy> {
public:
    void onEnter(Enemy* enemy) override;
    void onUpdate(Enemy* enemy, float dt) override;
    void onExit(Enemy* enemy) override;
    std::string getStateName() const override { return "Attack"; }

private:
    enum class Stage { Windup, Move, Active, Recovery };
    Stage _stage = Stage::Windup;

    float _timer = 0.f;
    bool  _didHit = false;

    BossSkillConfig _cfg;

    cocos2d::Vec3 _startW = cocos2d::Vec3::ZERO;
    cocos2d::Vec3 _targetW = cocos2d::Vec3::ZERO;  // 位移目标（锁定）
};

// ========== Boss Hit ==========
class BossHitState : public BaseState<Enemy> {
public:
    void onEnter(Enemy* enemy) override;
    void onUpdate(Enemy* enemy, float dt) override;
    void onExit(Enemy* enemy) override;
    std::string getStateName() const override { return "Hit"; }
private:
    float _timer = 0.f;
};

// ========== Boss Dead ==========
class BossDeadState : public BaseState<Enemy> {
public:
    void onEnter(Enemy* enemy) override;
    void onUpdate(Enemy* enemy, float dt) override;
    void onExit(Enemy* enemy) override;
    std::string getStateName() const override { return "Dead"; }
};

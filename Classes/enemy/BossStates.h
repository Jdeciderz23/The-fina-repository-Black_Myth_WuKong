// BossStates.h
// 定义Boss实体的所有状态类，包括Idle、Chase、Attack等状态
#pragma once

#include "Enemy.h"
#include "core/StateMachine.h"   // BaseState/StateMachine 所在文件
#include <string>
#include "combat/HealthComponent.h"
#include "combat/CombatComponent.h"

// ========== 技能配置（AttackState 用）==========
// BossSkillConfig 结构体定义了Boss技能的各项参数配置
struct BossSkillConfig {
  std::string skill;   // 技能名称，如"Combo3" / "DashSlash" / "GroundSlam" / "Roar" / "LeapSlam"
  std::string anim;    // 对应动画文件名（不带 .c3b）

  float windup = 0.f;    // 技能前摇时间（秒）
  float moveTime = 0.f;  // 位移时间（Dash/Leap 用，秒）
  float active = 0.f;    // 伤害判定窗口时间（秒）
  float recovery = 0.f;  // 技能后摇时间（秒）

  float dashDistance = 0.f; // Dash/Leap 跳跃到玩家的距离（世界单位）
  float hitRadius = 0.f;    // 命中判定半径（世界单位）
  float damage = 0.f;       // 技能伤害值
  bool  lockTarget = true;  // 是否锁定跳跃目标位置
};

// ========== Boss Idle ==========
// BossIdleState 处理Boss的待机状态逻辑
class BossIdleState : public BaseState<Enemy> {
public:
  // 进入状态时调用，设置Boss的动画和状态标志
  void onEnter(Enemy* enemy) override;
  
  // 更新状态时调用，处理死亡检测等逻辑
  void onUpdate(Enemy* enemy, float dt) override;
  
  // 退出状态时调用
  void onExit(Enemy* enemy) override;
  
  // 获取状态名称
  std::string getStateName() const override { return "Idle"; }
};

// ========== Boss Chase ==========
// BossChaseState 处理Boss追击玩家的状态逻辑
class BossChaseState : public BaseState<Enemy> {
public:
  // 进入状态时调用，设置Boss的动画和状态标志
  void onEnter(Enemy* enemy) override;
  
  // 更新状态时调用，处理移动、转向和目标检测
  void onUpdate(Enemy* enemy, float dt) override;
  
  // 退出状态时调用
  void onExit(Enemy* enemy) override;
  
  // 获取状态名称
  std::string getStateName() const override { return "Chase"; }
};

// ========== Phase Change (Roar 1s) ==========
// BossPhaseChangeState 处理Boss阶段转换时的状态逻辑
class BossPhaseChangeState : public BaseState<Enemy> {
public:
  // 进入状态时调用，开始计时并播放转换动画
  void onEnter(Enemy* enemy) override;
  
  // 更新状态时调用，处理阶段转换计时和属性提升
  void onUpdate(Enemy* enemy, float dt) override;
  
  // 退出状态时调用
  void onExit(Enemy* enemy) override;
  
  // 获取状态名称
  std::string getStateName() const override { return "PhaseChange"; }
  
private:
  float _timer = 0.f;  // 状态计时器
};

// ========== Boss Attack ==========
// BossAttackState 处理Boss攻击状态的逻辑，包括技能执行、位移和伤害判定
class BossAttackState : public BaseState<Enemy> {
public:
  // 进入状态时调用，初始化攻击参数和动画
  void onEnter(Enemy* enemy) override;
  
  // 更新状态时调用，处理攻击阶段切换和位移
  void onUpdate(Enemy* enemy, float dt) override;
  
  // 退出状态时调用，重置状态标志
  void onExit(Enemy* enemy) override;
  
  // 获取状态名称
  std::string getStateName() const override { return "Attack"; }

private:
  // 攻击阶段枚举：前摇、位移、伤害判定、后摇
  enum class Stage { Windup, Move, Active, Recovery };
  Stage _stage = Stage::Windup;  // 当前攻击阶段

  float _timer = 0.f;     // 状态计时器
  bool  _didHit = false;  // 是否已触发伤害判定

  BossSkillConfig _cfg;  // 当前技能配置

  cocos2d::Vec3 _startW = cocos2d::Vec3::ZERO;    // 起始世界位置
  cocos2d::Vec3 _targetW = cocos2d::Vec3::ZERO;   // 目标世界位置（用于位移）
};

// ========== Boss Hit ==========
// BossHitState 处理Boss被攻击时的状态逻辑
class BossHitState : public BaseState<Enemy> {
public:
  // 进入状态时调用，播放受击动画并设置状态标志
  void onEnter(Enemy* enemy) override;
  
  // 更新状态时调用，处理受击硬直时间
  void onUpdate(Enemy* enemy, float dt) override;
  
  // 退出状态时调用，重置状态标志
  void onExit(Enemy* enemy) override;
  
  // 获取状态名称
  std::string getStateName() const override { return "Hit"; }
private:
  float _timer = 0.f;  // 受击硬直计时器
};

// ========== Boss Dead ==========
// BossDeadState 处理Boss死亡时的状态逻辑
class BossDeadState : public BaseState<Enemy> {
public:
  // 进入状态时调用，播放死亡动画并设置移除延迟
  void onEnter(Enemy* enemy) override;
  
  // 更新状态时调用（死亡状态无需更新逻辑）
  void onUpdate(Enemy* enemy, float dt) override;
  
  // 退出状态时调用
  void onExit(Enemy* enemy) override;
  
  // 获取状态名称
  std::string getStateName() const override { return "Dead"; }
};

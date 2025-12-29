#ifndef BOSS_H
#define BOSS_H

#pragma once

#include "Enemy.h"
#include <string>

class BossAI;

// 游戏中的Boss敌人类型，继承自Enemy类，
// 具有阶段性变化、特殊能力和增强的AI行为。
class Boss : public Enemy {
 public:
  // 创建Boss实例
  // @param resRoot 资源根目录路径，例如 "Enemy/boss"
  // @param modelFile 模型文件路径，例如 "boss.c3b"
  // @return 返回初始化成功的Boss指针，如果失败则返回nullptr
  static Boss* createBoss(const std::string& resRoot, const std::string& modelFile);

  Boss();
  virtual ~Boss();

  // 初始化Boss，设置基础参数和资源
  // @param resRoot 资源根目录路径
  // @param modelFile 模型文件路径
  // @return 初始化是否成功
  bool initBoss(const std::string& resRoot, const std::string& modelFile);

  // 每帧更新Boss状态和AI逻辑
  // @param dt 帧间隔时间
  void update(float dt) override;

  // 初始化Boss的状态机，注册特定状态
  void initStateMachine() override;

  // ============ 阶段/增益系统 ============
  // 获取Boss当前阶段
  int getPhase() const { return _phase; }
  // 设置Boss当前阶段
  void setPhase(int p) { _phase = p; }

  // 获取移动速度倍率
  float getMoveMul() const { return _moveMul; }
  // 获取伤害倍率
  float getDmgMul() const { return _dmgMul; }

  // 应用第二阶段增益效果
  // @param moveMul 移动速度倍率
  // @param dmgMul 伤害倍率
  void applyPhase2Buff(float moveMul = 1.2f, float dmgMul = 1.15f) {
    _moveMul = moveMul;
    _dmgMul = dmgMul;
  }

  // ============ 忙碌状态 ============
  // 检查Boss是否处于忙碌状态（技能、演出、硬直等）
  bool isBusy() const { return _busy || isDead(); }
  // 设置Boss忙碌状态
  void setBusy(bool b) { _busy = b; }

  // ============ 待处理技能系统 ============
  // 设置待执行的技能名称
  void setPendingSkill(const std::string& s) { _pendingSkill = s; }
  // 检查是否有待执行技能
  bool hasPendingSkill() const { return !_pendingSkill.empty(); }
  // 消费并返回待执行技能名称，同时清空
  std::string consumePendingSkill() {
    std::string out = _pendingSkill;
    _pendingSkill.clear();
    return out;
  }

  // ============ 距离辅助方法 ============
  // 计算Boss到玩家的距离
  // @return 距离值
  float distanceToPlayer() const;

  // ============ AI系统 ============
  // 设置Boss的AI控制器
  // @param ai BossAI控制器指针，Boss负责管理其生命周期
  void setAI(BossAI* ai);
  // 获取Boss的AI控制器
  // @return BossAI控制器指针
  BossAI* getAI() const { return _ai; }

  // 重置Boss到初始状态
  void resetEnemy() override;

 private:
  // Boss的AI控制器
  BossAI* _ai = nullptr;
  
  // 当前阶段
  int _phase = 1;
  // 移动速度倍率
  float _moveMul = 1.0f;
  // 伤害倍率
  float _dmgMul = 1.0f;

  // 是否忙碌状态
  bool _busy = false;
  // 是否已触发半血回满
  bool _hasHealed = false;

  // 待执行技能名称
  std::string _pendingSkill;
};

#endif // BOSS_H

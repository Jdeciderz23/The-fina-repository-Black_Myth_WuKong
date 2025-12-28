#ifndef BOSS_H
#define BOSS_H

#pragma once

#include "Enemy.h"
#include <string>

class BossAI;

/**
 * @class Boss
 * @brief Boss 继承 Enemy：拥有 phase(阶段)、倍率、pendingSkill(给Attack状态读) 等
 */
class Boss : public Enemy {
public:
    /**
     * @brief 创建 Boss（指定资源根目录与模型文件）
     * @param resRoot 例如 "Enemy/boss"
     * @param modelFile 例如 "boss.c3b"
     */
    static Boss* createBoss(const std::string& resRoot, const std::string& modelFile);

    Boss();
    virtual ~Boss();

    /**
     * @brief 初始化 Boss：复用 Enemy::initWithResRoot，并设置 Boss 基础参数
     */
    bool initBoss(const std::string& resRoot, const std::string& modelFile);

    /**
     * @brief 每帧更新：先 Enemy::update，再 AI 决策（AI可后续接入）
     */
    void update(float dt) override;

    /**
     * @brief Boss 注册自己的状态（后续你写 BossStates 后在 cpp 里改这里）
     * 现在先默认复用 Enemy 的状态机，保证能跑起来
     */
    void initStateMachine() override;

public:
    // ============ Phase / Buff ============
    int  getPhase() const { return _phase; }
    void setPhase(int p) { _phase = p; }

    float getMoveMul() const { return _moveMul; }
    float getDmgMul() const { return _dmgMul; }

    void applyPhase2Buff(float moveMul = 1.2f, float dmgMul = 1.15f) {
        _moveMul = moveMul;
        _dmgMul = dmgMul;
    }

    // ============ Busy（AI决策用：正在技能/演出/硬直等） ============
    bool isBusy() const { return _busy || isDead(); }
    void setBusy(bool b) { _busy = b; }

    // ============ Pending skill（AI选完技能后写入，AttackState进入时读） ============
    void setPendingSkill(const std::string& s) { _pendingSkill = s; }
    bool hasPendingSkill() const { return !_pendingSkill.empty(); }
    std::string consumePendingSkill() {
        std::string out = _pendingSkill;
        _pendingSkill.clear();
        return out;
    }

    // ============ Distance helpers ============
    float distanceToPlayer() const;

    // ============ AI ============
    void setAI(BossAI* ai);     // Boss 拥有 ai 的生命周期（内部会 delete）
    BossAI* getAI() const { return _ai; }

private:
    BossAI* _ai = nullptr;

    int _phase = 1;
    float _moveMul = 1.0f;
    float _dmgMul = 1.0f;

    bool _busy = false;

    std::string _pendingSkill; // 例如 "Combo3" / "DashSlash" / ...
};

#endif // BOSS_H

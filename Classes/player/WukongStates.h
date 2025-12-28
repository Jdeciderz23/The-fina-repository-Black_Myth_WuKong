#pragma once
#ifndef WUKONGSTATES_H
#define WUKONGSTATES_H

#include "BaseState.h"
#include "Character.h"
#include "Wukong.h"
#include "enemy/Enemy.h"
#include "../combat/CombatComponent.h"
#include "../scene_ui/UIManager.h"
#include <string>
#include <cmath>
#include <unordered_map>
#include <vector>

/**
 * @class IdleState
 * @brief 待机状态，播放 idle，等待移动/攻击/跳跃/翻滚等输入切换
 */
class IdleState : public BaseState<Character> {
public:
    void onEnter(Character* entity) override {
        if (!entity) return;
        entity->stopHorizontal();
        entity->playAnim("idle", true);
    }

    void onUpdate(Character* entity, float deltaTime) override {
        (void)deltaTime;
        if (!entity) return;

        const auto intent = entity->getMoveIntent();
        if (intent.dirWS.lengthSquared() > 1e-6f) {
            entity->getStateMachine().changeState("Move");
        }
    }

    void onExit(Character* entity) override {
        (void)entity;
    }

    std::string getStateName() const override {
        return "Idle";
    }
};

/**
 * @class MoveState
 * @brief 移动状态，根据 MoveIntent 更新速度，播放 run 动画（也可以是 walk）
 */
class MoveState : public BaseState<Character> {
public:
    void onEnter(Character* entity) override {
        if (!entity) return;
        static_cast<Wukong*>(entity)->updateLocomotionAnim(entity->getMoveIntent().run);
    }

    static float moveTowardAngleDeg(float cur, float target, float maxDeltaDeg)
    {
        float delta = std::fmod(target - cur + 540.0f, 360.0f) - 180.0f; // [-180,180]
        if (delta > maxDeltaDeg) delta = maxDeltaDeg;
        if (delta < -maxDeltaDeg) delta = -maxDeltaDeg;
        return cur + delta;
    }

    void onUpdate(Character* entity, float deltaTime) override {
        (void)deltaTime;
        if (!entity) return;

        auto intent = entity->getMoveIntent();
        const float len2 = intent.dirWS.lengthSquared();

        if (len2 <= 1e-6f) {
            entity->stopHorizontal();
            entity->getStateMachine().changeState("Idle");
            return;
        }

        cocos2d::Vec3 dir = intent.dirWS;
        dir.normalize();

        const float spd = intent.run ? entity->runSpeed : entity->walkSpeed;
        entity->setHorizontalVelocity(cocos2d::Vec3(dir.x * spd, 0.0f, dir.z * spd));
        static_cast<Wukong*>(entity)->updateLocomotionAnim(intent.run);

    }

    void onExit(Character* entity) override {
        if (!entity) return;
        entity->stopHorizontal();
    }

    std::string getStateName() const override {
        return "Move";
    }
};

/**
 * @class JumpState
 * @brief 跳跃状态，播放 jump，落地后切换 Idle/Move
 */
class JumpState : public BaseState<Character> {
public:
    JumpState() : _landTriggered(false), _t(0.0f), _leftGround(false) {}

    void onEnter(Character* entity) override {
        if (!entity) return;
        _landTriggered = false;
        _leftGround = false;
        _t = 0.0f;
        static_cast<Wukong*>(entity)->startJumpAnim();
    }

    void onUpdate(Character* entity, float deltaTime) override {
        if (!entity) return;
        _t += deltaTime;
        if (_landTriggered) return;

        // 起跳保护时间：防止刚跳起就判定落地
        if (_t < 0.08f) return;

        // 关键逻辑：先观察到“离地”，之后再判定“着地”。
        if (!_leftGround) {
            if (!entity->isOnGround()) _leftGround = true;

            // 兜底：如果一直没检测到离地，某个时间后也强制允许判定
            if (!_leftGround && _t < 0.35f) return;
        }

    }

    void onExit(Character* entity) override { (void)entity; }
    std::string getStateName() const override { return "Jump"; }

private:
    bool  _landTriggered;
    float _t;
    bool  _leftGround;
 };


/**
 * @class RollState
 * @brief 翻滚状态，短时位移 + 播放 roll，结束后 Idle/Move
 */


class RollState : public BaseState<Character> {
public:
    RollState() : _t(0.0f), _dur(0.45f), _moveEnd(0.25f), _stopped(false) {}

    void onEnter(Character* entity) override {
        if (!entity) return;

        entity->stopHorizontal();
        entity->playAnim("roll", false);

        _t = 0.0f;
        _stopped = false;

        // 1) 时长改成“动画真实时长”
        if (auto* wk = dynamic_cast<Wukong*>(entity)) {
            _dur = wk->getAnimDuration("roll");
        }
        else {
            _dur = 0.45f;
        }
        if (_dur < 0.05f) _dur = 0.45f;

        // 2) 位移只在前半段给（后半段一般是收势，不要一直滑）
        _moveEnd = 0.55f * _dur;

        // 3) 翻滚方向：有输入用输入；没输入用“角色朝向”
        auto intent = entity->getMoveIntent();
        cocos2d::Vec3 dir = intent.dirWS;

        if (dir.lengthSquared() <= 1e-6f) {
            // 用角色 yaw 推 forward（yaw=0 => +Z）
            const float yawRad = CC_DEGREES_TO_RADIANS(entity->getRotation3D().y);
            dir = cocos2d::Vec3(std::sinf(yawRad), 0.0f, std::cosf(yawRad));
        }

        if (dir.lengthSquared() <= 1e-6f) {
            dir = cocos2d::Vec3(0.0f, 0.0f, 1.0f); // 再兜底一次
        }
        dir.normalize();

        const float rollSpeed = entity->runSpeed * 1.25f;
        entity->setHorizontalVelocity(cocos2d::Vec3(dir.x * rollSpeed, 0.0f, dir.z * rollSpeed));
    }

    void onUpdate(Character* entity, float dt) override {
        if (!entity) return;
        _t += dt;

        // 位移到点就停，避免一直滑
        if (!_stopped && _t >= _moveEnd) {
            entity->stopHorizontal();
            _stopped = true;
        }

        // 到动画末尾再切状态，避免截断 roll 动画
        const float endTime = 0.95f * _dur;
        if (_t >= endTime) {
            entity->stopHorizontal();
            const auto intent = entity->getMoveIntent();
            entity->getStateMachine().changeState(
                intent.dirWS.lengthSquared() > 1e-6f ? "Move" : "Idle"
            );
        }
    }

    void onExit(Character* entity) override {
        if (!entity) return;
        entity->stopHorizontal();
    }

    std::string getStateName() const override { return "Roll"; }

private:
    float _t;
    float _dur;
    float _moveEnd;
    bool  _stopped;
};

/**
 * @class AttackState
 * @brief 攻击状态，支持 1/2/3 段，期间触发 consumeComboBuffered() 为 true 则进下一段
 */
class AttackState : public BaseState<Character> {
public:
    /**
     * @brief 构造攻击状态
     * @param step 连招段数1/2/3
     */
    explicit AttackState(int step)
        : _step(step), _t(0.0f), _queuedNext(false), _dur(0.6f), _damageDealt(false) {
    }

    void onEnter(Character* entity) override {
        if (!entity) return;

        _t = 0.0f;
        _queuedNext = false;
        _damageDealt = false;
        entity->stopHorizontal();

        std::string key = (_step == 1) ? "attack1" : ((_step == 2) ? "attack2" : "attack3");
        entity->playAnim(key, false);

        // 用真实动画时长（需要 entity 是 Wukong）
        if (auto* wk = dynamic_cast<Wukong*>(entity)) {
            _dur = wk->getAnimDuration(key);
        }
        else {
            _dur = 0.6f;
        }
    }

    void onUpdate(Character* entity, float dt) override {
        if (!entity) return;
        _t += dt;

        // 攻击伤害检测时间窗口：不同段数攻击的伤害检测时机不同
        performAttackHitCheck(entity);

        // 连段输入窗口：按时长比例更稳（你也能自己调）
        const float winStart = 0.20f * _dur;
        const float winEnd = 0.65f * _dur;

        if (_t >= winStart && _t <= winEnd) {
            if (entity->consumeComboBuffered()) {
                _queuedNext = true;
            }
        }

        // 让当前段“基本播完”再切下一段
        const float endTime = 0.95f * _dur;
        if (_t >= endTime) {
            if (_queuedNext && _step < 3) {
                entity->getStateMachine().changeState(_step == 1 ? "Attack2" : "Attack3");
                return;
            }

            const auto intent = entity->getMoveIntent();
            if (intent.dirWS.lengthSquared() > 1e-6f) entity->getStateMachine().changeState("Move");
            else                                      entity->getStateMachine().changeState("Idle");
        }
    }

    void onExit(Character* entity) override {
        (void)entity;
    }

    std::string getStateName() const override {
        if (_step == 1) return "Attack1";
        if (_step == 2) return "Attack2";
        return "Attack3";
    }

private:
    /**
     * @brief 执行攻击伤害检测
     * @param entity 攻击者实体
     */
    void performAttackHitCheck(Character* entity) {
        if (!entity || _damageDealt) return;

        // 根据攻击段数设置不同的伤害检测时机和范围
        float hitTimeRatio = 0.0f;
        float hitWindow = 0.1f; // 伤害检测窗口宽度（秒）
        
        switch (_step) {
        case 1: 
            hitTimeRatio = 0.35f; // 第一段攻击早期检测（快速出手）
            hitWindow = 0.08f;    // 较短的检测窗口
            break;
        case 2: 
            hitTimeRatio = 0.45f; // 第二段攻击中期检测（蓄力攻击）
            hitWindow = 0.12f;    // 中等检测窗口
            break;
        case 3: 
            hitTimeRatio = 0.40f; // 第三段攻击早期检测（终结技）
            hitWindow = 0.15f;    // 较长的检测窗口（确保命中）
            break;
        default: 
            hitTimeRatio = 0.40f; 
            hitWindow = 0.1f; 
            break;
        }

        float hitTime = hitTimeRatio * _dur;

        // 在合适的时机执行一次伤害检测
        if (_t >= hitTime && _t <= hitTime + hitWindow) {
            auto* combat = entity->getCombat();
            if (combat) {
                // 获取敌人列表
                auto* enemies = entity->getEnemies();
                if (enemies && !enemies->empty()) {
                    // 将 Enemy* 转换为 Node* 以匹配函数参数类型
                    // 同时只保留存活的敌人
                    std::vector<Node*> nodeTargets;
                    nodeTargets.reserve(enemies->size());
                    for (auto* enemy : *enemies) {
                        // 只对存活的敌人进行攻击检测
                        if (enemy && !enemy->isDead()) {
                            nodeTargets.push_back(dynamic_cast<Node*>(enemy));
                        }
                    }
                    
                    // 执行近战攻击
                    if (!nodeTargets.empty()) {
                        int hitCount = combat->executeMeleeAttack(
                            entity->getCollider(), 
                            nodeTargets
                        );

                        if (hitCount > 0) {
                            CCLOG("AttackState: %s hit %d enemies!", getStateName().c_str(), hitCount);
                            
                            // 可以在这里添加攻击命中特效或音效
                            // TODO: 添加攻击命中反馈
                        }
                    } else {
                        CCLOG("AttackState: %s - no alive enemies to attack", getStateName().c_str());
                    }
                }
            }
            _damageDealt = true; // 标记已经执行过伤害检测，避免重复伤害
        }
    }

private:
    int _step;  ///< 连招段数
    float _t;   ///< 计时
    bool _queuedNext;
    bool _damageDealt;  ///< 是否已经执行过伤害检测
    float _dur;
};

/**
 * @class HurtState
 * @brief 受击状态，播放 hurt，短时硬直后回到 Idle/Move
 */
class HurtState : public BaseState<Character> {
public:
    HurtState() : _t(0.0f), _dur(0.35f) {}

    void onEnter(Character* entity) override {
        if (!entity) return;
        _t = 0.0f;
        entity->stopHorizontal();
        entity->playAnim("hurt", false);

        if (auto* wk = dynamic_cast<Wukong*>(entity)) {
            _dur = wk->getAnimDuration("hurt");
        }
        else {
            _dur = 0.35f;
        }
        if (_dur < 0.05f) _dur = 0.35f;
    }

    void onUpdate(Character* entity, float dt) override {
        if (!entity) return;
        _t += dt;

        if (_t >= 0.95f * _dur) {
            const auto intent = entity->getMoveIntent();
            entity->getStateMachine().changeState(
                intent.dirWS.lengthSquared() > 1e-6f ? "Move" : "Idle"
            );
        }
    }

    void onExit(Character* entity) override {
        (void)entity;
    }

    std::string getStateName() const override {
        return "Hurt";
    }

private:
    float _t; ///< 状态计时
    float _dur;
};

/**
 * @class DeadState
 * @brief 死亡状态，播放 dead，停止移动
 */
class DeadState : public BaseState<Character> {
public:
    DeadState() : _t(0.0f), _dur(1.0f), _menuShown(false) {}

    void onEnter(Character* entity) override {
        if (!entity) return;
        entity->stopHorizontal();
        entity->playAnim("dead", false);
        
        _t = 0.0f;
        _menuShown = false;
        
        // 获取死亡动画持续时间
        if (auto* wk = dynamic_cast<Wukong*>(entity)) {
            _dur = wk->getAnimDuration("dead");
        }
        else {
            _dur = 1.0f;
        }
        if (_dur < 0.5f) _dur = 1.0f; // 确保至少有一定时间播放动画
    }

    void onUpdate(Character* entity, float deltaTime) override {
        if (!entity || _menuShown) return;
        
        _t += deltaTime;
        
        // 当动画播放完成后显示死亡菜单
        if (_t >= _dur) {
            _menuShown = true;
            // 弹出死亡界面
            UIManager::getInstance()->showDeathMenu();
        }
    }

    void onExit(Character* entity) override {
        (void)entity;
    }

    std::string getStateName() const override {
        return "Dead";
    }
    
private:
    float _t; // 计时器
    float _dur; // 动画持续时间
    bool _menuShown; // 是否已显示菜单
};

class SkillState : public BaseState<Character> {
public:
    SkillState() : _t(0.0f), _dur(0.8f) {}

    void onEnter(Character* entity) override {
        if (!entity) return;
        _t = 0.0f;

        entity->stopHorizontal();
        entity->playAnim("skill", false);

        if (auto* wk = dynamic_cast<Wukong*>(entity)) {
            _dur = wk->getAnimDuration("skill");
        }
        else {
            _dur = 0.8f;
        }
        if (_dur < 0.05f) _dur = 0.8f;

        // TODO：如果你要做“技能伤害窗口/特效/震屏”，建议在 onUpdate 用时间窗触发
    }

    void onUpdate(Character* entity, float dt) override {
        if (!entity) return;
        _t += dt;

        // TODO：示例：在 25%~45% 动画区间做判定（你后续接 hitbox 就放这里）

        if (_t >= 0.95f * _dur) {
            const auto intent = entity->getMoveIntent();
            entity->getStateMachine().changeState(
                intent.dirWS.lengthSquared() > 1e-6f ? "Move" : "Idle"
            );
        }
    }

    void onExit(Character* entity) override { (void)entity; }
    std::string getStateName() const override { return "Skill"; }

private:
    float _t;
    float _dur;
};

#endif // WUKONGSTATES_H

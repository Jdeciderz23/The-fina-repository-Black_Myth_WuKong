#pragma once
#ifndef WUKONGSTATES_H
#define WUKONGSTATES_H

#include "BaseState.h"
#include "Character.h"
#include "Wukong.h"
#include <string>
#include <cmath>
#include <unordered_map>

/**
 * @class IdleState
 * @brief 待机状态：播放 idle，等待移动/攻击/跳跃/翻滚触发切换
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
 * @brief 移动状态：根据 MoveIntent 计算速度，并播放 run 动画（也可换 walk）
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
 * @brief 跳跃状态：播放 jump，落地后切回 Idle/Move
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

        // 起跳保护时间：避免刚进 JumpState 地面状态还没刷新
        if (_t < 0.08f) return;

        // 关键：必须观察到“离地”之后，才允许触发“落地”
        if (!_leftGround) {
            if (!entity->isOnGround()) _leftGround = true;

            // 兜底：如果一直检测不到离地（某些地面判定写法会这样），
            // 也别立刻落地打断动画，至少让起跳动画播一段时间
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
 * @brief 翻滚状态：短时间冲刺 + 播放 roll，结束后回 Idle/Move
 */
class RollState : public BaseState<Character> {
public:
    RollState() : _t(0.0f) {}

    void onEnter(Character* entity) override {
        if (!entity) return;

        entity->playAnim("roll", false);
        _t = 0.0f;

        // 沿当前 MoveIntent 方向翻滚；没有方向则沿前方（-Z）
        auto intent = entity->getMoveIntent();
        cocos2d::Vec3 dir = intent.dirWS;
        if (dir.lengthSquared() <= 1e-6f) {
            dir = cocos2d::Vec3(0.0f, 0.0f, -1.0f);
        }
        dir.normalize();

        const float rollSpeed = entity->runSpeed * 1.25f;
        entity->setHorizontalVelocity(cocos2d::Vec3(dir.x * rollSpeed, 0.0f, dir.z * rollSpeed));
    }

    void onUpdate(Character* entity, float deltaTime) override {
        if (!entity) return;
        _t += deltaTime;

        // 简化：0.45s 结束
        if (_t >= 0.45f) {
            entity->stopHorizontal();
            const auto intent = entity->getMoveIntent();
            if (intent.dirWS.lengthSquared() > 1e-6f) {
                entity->getStateMachine().changeState("Move");
            }
            else {
                entity->getStateMachine().changeState("Idle");
            }
        }
    }

    void onExit(Character* entity) override {
        if (!entity) return;
        entity->stopHorizontal();
    }

    std::string getStateName() const override {
        return "Roll";
    }

private:
    float _t; ///< 状态计时
};

/**
 * @class AttackState
 * @brief 攻击状态（支持 1/2/3 段）：在窗口内若 consumeComboBuffered() 为 true，则接下一段
 */
class AttackState : public BaseState<Character> {
public:
    explicit AttackState(int step)
        : _step(step), _t(0.0f), _queuedNext(false), _dur(0.6f) {
    }

    void onEnter(Character* entity) override {
        if (!entity) return;

        _t = 0.0f;
        _queuedNext = false;
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

    void onExit(Character* entity)override {
        (void)entity;
    }
    std::string getStateName() const override {
        if (_step == 1) return "Attack1";
        if (_step == 2) return "Attack2";
        return "Attack3";
    }

private:
    int _step;
    float _t;
    bool _queuedNext;
    float _dur;
};

/**
 * @class HurtState
 * @brief 受击状态：播放 hurt，短暂硬直后回 Idle/Move
 */
class HurtState : public BaseState<Character> {
public:
    HurtState() : _t(0.0f) {}

    void onEnter(Character* entity) override {
        if (!entity) return;
        _t = 0.0f;
        entity->stopHorizontal();
        entity->playAnim("hurt", false);
    }

    void onUpdate(Character* entity, float deltaTime) override {
        if (!entity) return;
        _t += deltaTime;

        if (_t >= 0.35f) {
            const auto intent = entity->getMoveIntent();
            if (intent.dirWS.lengthSquared() > 1e-6f) {
                entity->getStateMachine().changeState("Move");
            }
            else {
                entity->getStateMachine().changeState("Idle");
            }
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
};

/**
 * @class DeadState
 * @brief 死亡状态：播放 dead，停止移动
 */
class DeadState : public BaseState<Character> {
public:
    void onEnter(Character* entity) override {
        if (!entity) return;
        entity->stopHorizontal();
        entity->playAnim("dead", false);
    }

    void onUpdate(Character* entity, float deltaTime) override {
        (void)entity;
        (void)deltaTime;
        // 死亡一般不再切状态
    }

    void onExit(Character* entity) override {
        (void)entity;
    }

    std::string getStateName() const override {
        return "Dead";
    }
};

#endif // WUKONGSTATES_H

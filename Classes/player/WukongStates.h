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
        entity->playAnim("run", true);
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

        // （可选）朝向：让角色面向移动方向
        const float yawRad = std::atan2(dir.x, -dir.z);
        const float yawDeg = yawRad * 180.0f / 3.1415926f;
        entity->setRotation3D(cocos2d::Vec3(0.0f, yawDeg, 0.0f));
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
//class JumpState : public BaseState<Character> {
//public:
//    void onEnter(Character* entity) override {
//        if (!entity) return;
//        entity->playAnim("jump", false);
//    }
//
//    void onUpdate(Character* entity, float deltaTime) override {
//        (void)deltaTime;
//        if (!entity) return;
//
//        if (entity->isOnGround()) {
//            const auto intent = entity->getMoveIntent();
//            if (intent.dirWS.lengthSquared() > 1e-6f) {
//                entity->getStateMachine().changeState("Move");
//            }
//            else {
//                entity->getStateMachine().changeState("Idle");
//            }
//        }
//    }
//
//    void onExit(Character* entity) override {
//        (void)entity;
//    }
//
//    std::string getStateName() const override {
//        return "Jump";
//    }
//};
class JumpState : public BaseState<Character> {
public:
    JumpState() : _landTriggered(false), _t(0.0f) {}

    void onEnter(Character* entity) override {
        if (!entity) return;
        _landTriggered = false;
        _t = 0.0f;

        // 进入跳跃：Pad -> Start -> Apex(循环)
        static_cast<Wukong*>(entity)->startJumpAnim();
    }

    void onUpdate(Character* entity, float deltaTime) override {
        if (!entity) return;
        _t += deltaTime;

        // 防止刚切进 JumpState 的第一帧地面判定还没更新，导致立刻触发落地
        if (_t < 0.05f) return;

        if (_landTriggered) return;

        if (entity->isOnGround()) {
            _landTriggered = true;

            // 落地：Land -> Recovery -> 最后回 Idle/Move（在 Wukong::onJumpLanded 里做）
            static_cast<Wukong*>(entity)->onJumpLanded();
        }
    }

    void onExit(Character* entity) override { (void)entity; }

    std::string getStateName() const override { return "Jump"; }

private:
    bool  _landTriggered;
    float _t;
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
    /**
     * @brief 构造攻击状态
     * @param step 连招段数（1/2/3）
     */
    explicit AttackState(int step) : _step(step), _t(0.0f) {}

    void onEnter(Character* entity) override {
        if (!entity) return;

        _t = 0.0f;
        entity->stopHorizontal();

        if (_step == 1) entity->playAnim("atk1", false);
        if (_step == 2) entity->playAnim("atk2", false);
        if (_step == 3) entity->playAnim("atk3", false);
    }

    void onUpdate(Character* entity, float deltaTime) override {
        if (!entity) return;
        _t += deltaTime;

        // 简化：0.15~0.35 秒为接续窗口
        const bool inWindow = (_t >= 0.15f && _t <= 0.35f);

        bool wantNext = false;
        if (inWindow) {
            wantNext = entity->consumeComboBuffered();
        }

        // 简化：0.55 秒动作结束
        if (_t >= 0.55f) {
            if (wantNext && _step < 3) {
                if (_step == 1) entity->getStateMachine().changeState("Attack2");
                if (_step == 2) entity->getStateMachine().changeState("Attack3");
            }
            else {
                const auto intent = entity->getMoveIntent();
                if (intent.dirWS.lengthSquared() > 1e-6f) {
                    entity->getStateMachine().changeState("Move");
                }
                else {
                    entity->getStateMachine().changeState("Idle");
                }
            }
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
    int _step;  ///< 连招段数
    float _t;   ///< 状态计时
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

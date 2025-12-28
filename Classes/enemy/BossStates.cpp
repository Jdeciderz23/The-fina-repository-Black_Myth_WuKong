// BossStates.cpp
#include "BossStates.h"
#include "Boss.h"
#include "cocos2d.h"
#include <algorithm>
#include <cmath>

USING_NS_CC;

static constexpr float PI_F = 3.1415926f;

// 你项目里通常 1m≈100 世界单位（之前 viewRange/moveSpeed 很像）
// 如果你场景不是这个比例，把这个系数改一下就行
static inline float M(float meters) { return meters * 100.0f; }

// worldPos -> parent local（因为 setPosition3D 是 parent space）
static Vec3 worldToParentSpace(const Node* node, const Vec3& worldPos) {
    auto p = node->getParent();
    if (!p) return worldPos;
    Vec3 out = Vec3::ZERO;
    Mat4 inv = p->getWorldToNodeTransform();
    inv.transformPoint(worldPos, &out);
    return out;
}

// 面向某个世界方向（沿用你小怪的 +45 偏移，如果 Boss 朝向不对就改这里）
static void faceToWorldDir(Enemy* e, Vec3 dirW, float yOffsetDeg = 45.0f) {
    if (!e || !e->getSprite()) return;
    dirW.y = 0;
    if (dirW.lengthSquared() < 1e-6f) return;
    dirW.normalize();

    float yaw = atan2f(dirW.x, dirW.z) * 180.0f / PI_F + yOffsetDeg;
    e->getSprite()->setRotation3D(Vec3(0, yaw, 0));
}

// 技能表：按你给的设计参数
static BossSkillConfig getCfg(const std::string& skill) {
    if (skill == "Combo3") {
        return BossSkillConfig{
            "Combo3", "combo3",
            0.25f, 0.0f, 0.35f, 0.45f,
            0.f, M(2.5f), 12.f, false
        };
    }
    if (skill == "DashSlash") {
        return BossSkillConfig{
            "DashSlash", "dashslash",
            0.30f, 0.25f, 0.15f, 0.50f,
            M(2.0f), M(2.8f), 16.f, true
        };
    }
    if (skill == "GroundSlam") {
        return BossSkillConfig{
            "GroundSlam", "groundslam",
            0.60f, 0.0f, 0.20f, 0.80f,
            0.f, M(3.5f), 20.f, false
        };
    }
    if (skill == "Roar") {
        return BossSkillConfig{
            "Roar", "roar",
            1.00f, 0.0f, 0.0f, 0.0f,
            0.f, 0.f, 0.f, false
        };
    }
    if (skill == "LeapSlam") {
        return BossSkillConfig{
            "LeapSlam", "leapslam",
            0.35f, 0.35f, 0.20f, 0.90f,
            M(2.0f), M(4.0f), 26.f, true
        };
    }

    // 默认兜底
    return getCfg("Combo3");
}

static void applyHitOnce(Enemy* enemy, const BossSkillConfig& cfg, float dmgMul) {
    if (!enemy) return;

    Vec3 pW = enemy->getTargetWorldPos();
    Vec3 eW = enemy->getWorldPosition3D();
    float dist = (pW - eW).length();

    if (dist <= cfg.hitRadius) {
        float dmg = cfg.damage * dmgMul;
        CCLOG("[BossAttack] %s HIT dmg=%.2f dist=%.1f", cfg.skill.c_str(), dmg, dist);

        // TODO：接你真实扣血接口（HealthComponent / CombatComponent）
        // auto w = enemy->getTarget();
        // w->takeDamage(dmg);
    }
    else {
        CCLOG("[BossAttack] %s miss dist=%.1f", cfg.skill.c_str(), dist);
    }
}

// ================= Idle =================
void BossIdleState::onEnter(Enemy* enemy) {
    if (!enemy) return;
    enemy->playAnim("idle", true);

    auto boss = static_cast<Boss*>(enemy);
    boss->setBusy(false);
}

void BossIdleState::onUpdate(Enemy* enemy, float) {
    if (!enemy) return;
    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }
}

void BossIdleState::onExit(Enemy*) {}

// ================= Chase =================
void BossChaseState::onEnter(Enemy* enemy) {
    if (!enemy) return;
    enemy->playAnim("chase", true);

    auto boss = static_cast<Boss*>(enemy);
    boss->setBusy(false);
}

void BossChaseState::onUpdate(Enemy* enemy, float dt) {
    if (!enemy) return;

    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    Vec3 pW = enemy->getTargetWorldPos();
    if (pW == Vec3::ZERO) {
        enemy->getStateMachine()->changeState("Idle");
        return;
    }

    auto boss = static_cast<Boss*>(enemy);
    Vec3 eW = enemy->getWorldPosition3D();

    Vec3 dir = pW - eW;
    dir.y = 0;
    if (dir.lengthSquared() < 1e-6f) return;
    dir.normalize();

    faceToWorldDir(enemy, dir);

    float speed = enemy->getMoveSpeed() * boss->getMoveMul();
    Vec3 newW = eW + dir * speed * dt;
    newW.y = eW.y;

    enemy->setPosition3D(worldToParentSpace(enemy, newW));
}

void BossChaseState::onExit(Enemy*) {}

// ================= PhaseChange =================
void BossPhaseChangeState::onEnter(Enemy* enemy) {
    if (!enemy) return;

    _timer = 0.f;
    enemy->playAnim("roar", false);

    auto boss = static_cast<Boss*>(enemy);
    boss->setBusy(true);
}

void BossPhaseChangeState::onUpdate(Enemy* enemy, float dt) {
    if (!enemy) return;

    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    _timer += dt;
    if (_timer >= 1.0f) {
        auto boss = static_cast<Boss*>(enemy);
        boss->applyPhase2Buff(1.2f, 1.15f); // 二阶段加速/增伤
        boss->setBusy(false);

        enemy->getStateMachine()->changeState("Chase");
    }
}

void BossPhaseChangeState::onExit(Enemy*) {}

// ================= Attack =================
void BossAttackState::onEnter(Enemy* enemy) {
    if (!enemy) return;

    _timer = 0.f;
    _didHit = false;
    _stage = Stage::Windup;

    auto boss = static_cast<Boss*>(enemy);
    boss->setBusy(true);

    // 读取 AI 预写入的技能（没有就默认 Combo3）
    std::string skill = boss->hasPendingSkill() ? boss->consumePendingSkill() : "Combo3";
    _cfg = getCfg(skill);

    enemy->playAnim(_cfg.anim, false);

    _startW = enemy->getWorldPosition3D();

    // 锁定落点（Dash/Leap）
    _targetW = enemy->getTargetWorldPos();
    if (_cfg.moveTime > 0.f && _cfg.lockTarget) {
        // 调整目标：冲到“离玩家 dashDistance”处，不要穿过去
        Vec3 toP = _targetW - _startW;
        toP.y = 0;
        if (toP.lengthSquared() > 1e-6f) {
            float len = toP.length();
            toP.normalize();
            float want = std::max(0.0f, len - _cfg.dashDistance);
            _targetW = _startW + toP * want;
        }
    }
}

void BossAttackState::onUpdate(Enemy* enemy, float dt) {
    if (!enemy) return;

    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    auto boss = static_cast<Boss*>(enemy);

    _timer += dt;

    auto gotoStage = [&](Stage s) {
        _stage = s;
        _timer = 0.f;
        if (s == Stage::Active) _didHit = false;
        };

    // 1) Windup
    if (_stage == Stage::Windup) {
        if (_timer >= _cfg.windup) {
            if (_cfg.moveTime > 0.f) gotoStage(Stage::Move);
            else gotoStage(Stage::Active);
        }
        return;
    }

    // 2) Move（Dash/Leap）
    if (_stage == Stage::Move) {
        float denom = std::max(0.0001f, _cfg.moveTime);
        float t01 = std::min(1.0f, _timer / denom);

        Vec3 newW = _startW + (_targetW - _startW) * t01;
        newW.y = enemy->getWorldPosition3D().y;

        faceToWorldDir(enemy, _targetW - _startW);
        enemy->setPosition3D(worldToParentSpace(enemy, newW));

        if (_timer >= _cfg.moveTime) {
            gotoStage(Stage::Active);
        }
        return;
    }

    // 3) Active（生效窗口：只判定一次最稳）
    if (_stage == Stage::Active) {
        if (!_didHit) {
            applyHitOnce(enemy, _cfg, boss->getDmgMul());
            _didHit = true;
        }

        if (_timer >= _cfg.active) {
            gotoStage(Stage::Recovery);
        }
        return;
    }

    // 4) Recovery
    if (_stage == Stage::Recovery) {
        if (_timer >= _cfg.recovery) {
            boss->setBusy(false);
            enemy->getStateMachine()->changeState("Chase");
        }
        return;
    }
}

void BossAttackState::onExit(Enemy* enemy) {
    if (!enemy) return;
    auto boss = static_cast<Boss*>(enemy);
    boss->setBusy(false);
}

// ================= Hit =================
void BossHitState::onEnter(Enemy* enemy) {
    if (!enemy) return;
    _timer = 0.f;

    auto boss = static_cast<Boss*>(enemy);
    boss->setBusy(true);

    enemy->playAnim("hited", false);
}

void BossHitState::onUpdate(Enemy* enemy, float dt) {
    if (!enemy) return;

    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    _timer += dt;
    if (_timer >= 0.5f) {
        auto boss = static_cast<Boss*>(enemy);
        boss->setBusy(false);
        enemy->getStateMachine()->changeState("Chase");
    }
}

void BossHitState::onExit(Enemy* enemy) {
    if (!enemy) return;
    auto boss = static_cast<Boss*>(enemy);
    boss->setBusy(false);
}

// ================= Dead =================
void BossDeadState::onEnter(Enemy* enemy) {
    if (!enemy) return;

    auto boss = static_cast<Boss*>(enemy);
    boss->setBusy(true);

    enemy->playAnim("dying", false);
    enemy->runAction(Sequence::create(DelayTime::create(2.0f), RemoveSelf::create(), nullptr));
}

void BossDeadState::onUpdate(Enemy*, float) {}
void BossDeadState::onExit(Enemy*) {}
  
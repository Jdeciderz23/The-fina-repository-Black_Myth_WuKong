// BossStates.cpp
#include "BossStates.h"
#include "Boss.h"
#include "Wukong.h"
#include "cocos2d.h"
#include <algorithm>
#include <cmath>

USING_NS_CC;

static constexpr float PI_F = 3.1415926f;

// ����Ŀ��ͨ�� 1m��100 ���絥λ��֮ǰ viewRange/moveSpeed ����
// ����㳡��������������������ϵ����һ�¾���
static inline float M(float meters) { return meters * 100.0f; }

// worldPos -> parent local����Ϊ setPosition3D �� parent space��
static Vec3 worldToParentSpace(const Node* node, const Vec3& worldPos) {
    auto p = node->getParent();
    if (!p) return worldPos;
    Vec3 out = Vec3::ZERO;
    Mat4 inv = p->getWorldToNodeTransform();
    inv.transformPoint(worldPos, &out);
    return out;
}

// ����ĳ�����緽��������С�ֵ� +45 ƫ�ƣ���� Boss ���򲻶Ծ͸����
static void faceToWorldDir(Enemy* e, Vec3 dirW, float yOffsetDeg = 45.0f) {
    if (!e || !e->getSprite()) return;
    dirW.y = 0;
    if (dirW.lengthSquared() < 1e-6f) return;
    dirW.normalize();

    float yaw = atan2f(dirW.x, dirW.z) * 180.0f / PI_F + yOffsetDeg;
    e->getSprite()->setRotation3D(Vec3(0, yaw, 0));
}

// ���ܱ������������Ʋ���
static BossSkillConfig getCfg(const std::string& skill) {
    if (skill == "Combo3") {
        return BossSkillConfig{
            "Combo3", "combo3",
            0.35f, 0.0f, 0.50f, 0.65f,  // 增加所有时间参数以延长动画播放时间
            0.f, M(1.2f), 12.f, false
        };
    }
    if (skill == "DashSlash") {
        return BossSkillConfig{
            "DashSlash", "rush",
            0.30f, 0.25f, 0.15f, 0.50f,
            M(2.0f), M(1.4f), 16.f, true
        };
    }
    if (skill == "GroundSlam") {
        return BossSkillConfig{
            "GroundSlam", "groundslam",
            0.60f, 0.0f, 0.20f, 0.80f,
            0.f, M(1.7f), 20.f, false
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
            "LeapSlam", "rush",  // 首先播放rush动画
            0.35f, 0.35f, 0.15f, 1.30f,  // 延长recovery时间以容纳第二个动画
            M(2.0f), M(3.0f), 26.f, true
        };
    }

    // Ĭ�϶���
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

        // 使用与普通敌人相同的减血逻辑
        auto target = enemy->getTarget();
        if (target && target->getHealth()) {
            target->getHealth()->takeDamage(dmg, enemy);
            CCLOG("Boss dealt %.2f damage to player", dmg);
        }
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
    CCLOG("Boss phase change triggered, playing roar animation");

    // 播放roar.c3b动画，这是BOSS血量降到50%以下时的特殊动画
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
    // 延长时间从1.0秒到1.5秒，确保roar.c3b动画完整播放
    if (_timer >= 3.5f) {
        auto boss = static_cast<Boss*>(enemy);
        boss->applyPhase2Buff(1.2f, 1.15f); // ���׶μ���/����
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

    // ��ȡ AI Ԥд��ļ��ܣ�û�о�Ĭ�� Combo3��
    std::string skill = boss->hasPendingSkill() ? boss->consumePendingSkill() : "Combo3";
    _cfg = getCfg(skill);

    enemy->playAnim(_cfg.anim, false);

    _startW = enemy->getWorldPosition3D();

    // ������㣨Dash/Leap��
    _targetW = enemy->getTargetWorldPos();
    if (_cfg.moveTime > 0.f && _cfg.lockTarget) {
        // ����Ŀ�꣺�嵽������� dashDistance��������Ҫ����ȥ
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

    // 2) Move��Dash/Leap��
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

    // 3) Active����Ч���ڣ�ֻ�ж�һ�����ȣ�
    if (_stage == Stage::Active) {
        if (!_didHit) {
            applyHitOnce(enemy, _cfg, boss->getDmgMul());
            _didHit = true;
        }

        if (_timer >= _cfg.active) {
            gotoStage(Stage::Recovery);

            // 如果是LeapSlam技能，播放groundslam动画作为第二个动画
            if (_cfg.skill == "LeapSlam") {
                enemy->playAnim("groundslam", false);
            }
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

    // 播放受击动画，确保使用正确的文件名
    enemy->playAnim("hited", false);
    CCLOG("Boss hit state triggered, playing hited animation");
}

void BossHitState::onUpdate(Enemy* enemy, float dt) {
    if (!enemy) return;

    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");
        return;
    }

    _timer += dt;
    // 延长受击状态时间从0.5秒到0.8秒，确保hited.c3b动画能够完整播放
    if (_timer >= 0.8f) {
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

    CCLOG("Boss entering death state, playing dying animation");

    // 播放死亡动画 dying.c3b
    enemy->playAnim("dying", false);

    // 与普通敌人一样的死亡处理流程：发送事件 + 延迟移除
    // 延长延迟时间至3秒以确保死亡动画完整播放
    enemy->runAction(Sequence::create(
        DelayTime::create(3.0f),
        CallFunc::create([enemy]() {
            CCLOG("Boss is being removed after death animation");

            // 通知游戏管理器从敌人列表中移除此敌人
            // 通过一个全局事件通知系统
            cocos2d::EventCustom event("enemy_died");
            event.setUserData(enemy);
            cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);

            // 执行实际的移除操作
            enemy->removeFromParent();
            }),
        nullptr
    ));
}

void BossDeadState::onUpdate(Enemy*, float) {}
void BossDeadState::onExit(Enemy*) {}

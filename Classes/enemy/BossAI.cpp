// BossAI.cpp
#include "BossAI.h"
#include "Boss.h"
#include "cocos2d.h"
#include <algorithm>

USING_NS_CC;

// 默认：1m ≈ 100 世界单位（如果你项目不是这个比例，改这里）
static inline float M(float meters) { return meters * 100.0f; }

BossAI::BossAI(Boss* boss) : _boss(boss) {
    initSkills();
    for (auto& s : _skills) _cdLeft[s.name] = 0.f;
}

void BossAI::initSkills() {
    // 距离分层：
    // 近：dist < 2.5m
    // 中：2.5m ~ 6m
    // 远：> 6m

    // Phase 1 技能
    _skills.push_back(BossAISkill{ "Combo3",     0.f,      M(0.5f), 2.0f, 1.00f, 1 });
    _skills.push_back(BossAISkill{ "DashSlash",  M(2.5f),  M(3.0f), 4.0f, 0.90f, 1 });
    _skills.push_back(BossAISkill{ "GroundSlam", 0.f,      M(1.0f), 6.0f, 0.70f, 1 });

    // Phase 2 技能（使用rush+groundslam组合替代leapslam）
    _skills.push_back(BossAISkill{ "LeapSlam",   M(2.5f),  M(5.f), 10.0f, 1.20f, 2 }); // 增加冷却时间以匹配延长的动画


    // 你也可以让 GroundSlam 在二阶段还能用（phaseMask=3）
    // 这里为了简单：Phase2 仍然允许 Combo3/DashSlash/GroundSlam
    // 所以我们把它们 phaseMask 改成 3（1|2）
    for (auto& s : _skills) {
        if (s.name == "Combo3" || s.name == "DashSlash" || s.name == "GroundSlam") {
            s.phaseMask = 3; // 1阶段和2阶段都可用
        }
    }
}

const BossAISkill* BossAI::pickByWeight(const std::vector<const BossAISkill*>& cands) {
    float sum = 0.f;
    for (auto* s : cands) sum += std::max(0.f, s->weight);
    if (sum <= 0.f) return cands.front();

    float r = RandomHelper::random_real(0.f, sum);
    for (auto* s : cands) {
        r -= std::max(0.f, s->weight);
        if (r <= 0.f) return s;
    }
    return cands.back();
}

void BossAI::update(float dt) {
    if (!_enabled || !_boss) return;

    // 1) 冷却递减
    for (auto& kv : _cdLeft) {
        kv.second = std::max(0.0f, kv.second - dt);
    }

    // 2) 死亡/忙碌（Attack/Hit/PhaseChange）则不决策
    if (_boss->isDead()) return;
    if (_boss->isBusy()) return;

    // 3) 降频决策
    _thinkTimer += dt;
    if (_thinkTimer < _thinkInterval) return;
    _thinkTimer = 0.f;

    // 4) 阶段切换：HP <= 50%
    if (_boss->getPhase() == 1 && _boss->getHealthRatio() <= 0.5f) {
        _boss->setPhase(2);
        _boss->getStateMachine()->changeState("PhaseChange"); // Roar 1s + buff 在 PhaseChangeState 里做
        return;
    }

    // 5) 距离判断
    float dist = _boss->distanceToPlayer();
    int phase = _boss->getPhase();

    // 6) 收集候选技能（阶段+距离+冷却）
    std::vector<const BossAISkill*> cands;
    cands.reserve(_skills.size());

    for (auto& s : _skills) {
        if (!(s.phaseMask & phase)) continue;
        if (dist < s.rangeMin || dist > s.rangeMax) continue;
        if (_cdLeft[s.name] > 0.f) continue;

        cands.push_back(&s);
    }

    // 7) 远距离：Phase2 更偏向 LeapSlam（压迫感）
    if (phase == 2 && dist > M(6.0f)) {
        for (auto* s : cands) {
            if (s->name == "LeapSlam") {
                _boss->setPendingSkill("LeapSlam");
                _boss->getStateMachine()->changeState("Attack");
                _cdLeft["LeapSlam"] = s->cd;
                return;
            }
        }
    }

    // 8) 有技能就选一个（加权随机）
    if (!cands.empty()) {
        const BossAISkill* pick = pickByWeight(cands);

        _boss->setPendingSkill(pick->name);
        _boss->getStateMachine()->changeState("Attack");
        _cdLeft[pick->name] = pick->cd;
        return;
    }

    // 9) 没技能：追击（你想加 strafe 也可以在这里切别的状态）
    _boss->getStateMachine()->changeState("Chase");
}

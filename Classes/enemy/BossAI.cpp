#include "BossAI.h"
#include "Boss.h"
#include "cocos2d.h"
#include <algorithm>

USING_NS_CC;

// 世界单位转换辅助函数
// 将米转换为世界单位（假设1米≈100世界单位）
// @param meters 米数
// @return 对应的世界单位
static inline float M(float meters) { return meters * 100.0f; }

// BossAI构造函数
// 初始化技能和冷却时间
// @param boss 控制的Boss实例
BossAI::BossAI(Boss* boss) : _boss(boss) {
  initSkills();
  // 初始化所有技能的冷却时间为0
  for (auto& s : _skills) _cdLeft[s.name] = 0.f;
}

// 初始化Boss可用的所有技能
// 定义不同阶段的技能，包括名称、距离范围、冷却时间和权重
void BossAI::initSkills() {
  // 距离分层：
  // 近：dist < 2.5m
  // 中：2.5m ~ 6m
  // 远：> 6m

  // Phase 1 技能
  _skills.push_back(BossAISkill{ "Combo3",     0.f,      M(0.5f), 2.0f, 1.00f, 1 });
  _skills.push_back(BossAISkill{ "DashSlash",  M(2.5f),  M(3.0f), 4.0f, 0.90f, 1 });
  _skills.push_back(BossAISkill{ "GroundSlam", 0.f,      M(1.0f), 6.0f, 0.70f, 1 });

  // Phase 2 技能（使用LeapSlam作为新技能）
  _skills.push_back(BossAISkill{ "LeapSlam",   M(2.5f),  M(5.f), 10.0f, 1.20f, 2 });

  // 让部分Phase 1技能在Phase 2也可用
  for (auto& s : _skills) {
    if (s.name == "Combo3" || s.name == "DashSlash" || s.name == "GroundSlam") {
      s.phaseMask = 3; // 1阶段和2阶段都可用
    }
  }
}

// 根据权重随机选择一个技能
// 权重越高，被选中的概率越大
// @param cands 候选技能列表
// @return 选中的技能指针，如果列表为空或权重总和为0则返回第一个
const BossAISkill* BossAI::pickByWeight(const std::vector<const BossAISkill*>& cands) {
  // 计算权重总和
  float sum = 0.f;
  for (auto* s : cands) sum += std::max(0.f, s->weight);
  if (sum <= 0.f) return cands.front();

  // 加权随机选择
  float r = RandomHelper::random_real(0.f, sum);
  for (auto* s : cands) {
    r -= std::max(0.f, s->weight);
    if (r <= 0.f) return s;
  }
  return cands.back();
}

// 每帧更新AI决策逻辑
// 处理冷却时间、阶段切换、技能选择等
// @param dt 帧间隔时间
void BossAI::update(float dt) {
  if (!_enabled || !_boss) return;

  // 1) 冷却时间递减
  for (auto& kv : _cdLeft) {
    kv.second = std::max(0.0f, kv.second - dt);
  }

  // 2) 死亡或忙碌状态则不进行决策
  if (_boss->isDead()) return;
  if (_boss->isBusy()) return;

  // 3) 降低决策频率，避免每帧都决策
  _thinkTimer += dt;
  if (_thinkTimer < _thinkInterval) return;
  _thinkTimer = 0.f;

  // 4) 检查并触发阶段切换
  if (_boss->getPhase() == 1 && _boss->getHealthRatio() <= 0.5f) {
    _boss->setPhase(2);
    _boss->getStateMachine()->changeState("PhaseChange");
    return;
  }

  // 5) 计算与玩家的距离
  float dist = _boss->distanceToPlayer();
  int phase = _boss->getPhase();

  // 6) 收集符合当前阶段、距离范围且不在冷却的候选技能
  std::vector<const BossAISkill*> cands;
  cands.reserve(_skills.size());

  for (auto& s : _skills) {
    // 检查阶段是否匹配
    if (!(s.phaseMask & phase)) continue;
    // 检查距离是否在范围内
    if (dist < s.rangeMin || dist > s.rangeMax) continue;
    // 检查是否在冷却中
    if (_cdLeft[s.name] > 0.f) continue;

    cands.push_back(&s);
  }

  // 7) 特殊处理：Phase 2远距离优先使用LeapSlam
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

  // 8) 有可用技能时，根据权重随机选择一个
  if (!cands.empty()) {
    const BossAISkill* pick = pickByWeight(cands);

    _boss->setPendingSkill(pick->name);
    _boss->getStateMachine()->changeState("Attack");
    _cdLeft[pick->name] = pick->cd;
    return;
  }

  // 9) 没有可用技能时，追击玩家
  _boss->getStateMachine()->changeState("Chase");
}

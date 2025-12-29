#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class Boss;

// 定义Boss技能数据结构
struct BossAISkill {
  std::string name;   // 技能名称，如"Combo3"/"DashSlash"/"GroundSlam"/"LeapSlam"
  float rangeMin = 0.f;    // 技能最小可用距离
  float rangeMax = 0.f;    // 技能最大可用距离
  float cd = 0.f;          // 技能冷却时间（秒）
  float weight = 1.f;      // 技能权重（用于随机选择时的概率）
  int phaseMask = 1;       // 可用阶段掩码：1表示阶段1，2表示阶段2，3(1|2)表示两阶段都可用
};

// Boss人工智能控制器，负责技能选择和行为决策
class BossAI {
 public:
  // 构造函数
  // @param boss 控制的Boss实例
  explicit BossAI(Boss* boss);

  // 每帧更新AI决策逻辑
  // @param dt 帧间隔时间
  void update(float dt);

  // 设置AI是否启用
  // @param e 启用状态
  void setEnabled(bool e) { _enabled = e; }
  // 获取AI启用状态
  // @return AI是否启用
  bool isEnabled() const { return _enabled; }

 private:
  // 初始化所有可用技能
  void initSkills();
  
  // 根据权重随机选择一个技能
  // @param cands 候选技能列表
  // @return 选中的技能指针，若列表为空则返回nullptr
  const BossAISkill* pickByWeight(const std::vector<const BossAISkill*>& cands);

 private:
  // 控制的Boss实例
  Boss* _boss = nullptr;
  // AI是否启用
  bool _enabled = true;

  // 决策计时器
  float _thinkTimer = 0.f;
  // 决策间隔（秒）
  float _thinkInterval = 0.10f; // 每0.1秒决策一次

  // 所有可用技能列表
  std::vector<BossAISkill> _skills;
  // 技能冷却时间映射表
  std::unordered_map<std::string, float> _cdLeft; // 名称 -> 剩余冷却时间
};

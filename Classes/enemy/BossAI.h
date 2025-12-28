#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class Boss;

struct BossAISkill {
    std::string name;   // "Combo3" / "DashSlash" / "GroundSlam" / "LeapSlam"
    float rangeMin = 0.f;
    float rangeMax = 0.f;
    float cd = 0.f;
    float weight = 1.f;
    int phaseMask = 1;  // 1 or 2 or 3(=1|2)
};

class BossAI {
public:
    explicit BossAI(Boss* boss);

    void update(float dt);

    void setEnabled(bool e) { _enabled = e; }
    bool isEnabled() const { return _enabled; }

private:
    void initSkills();
    const BossAISkill* pickByWeight(const std::vector<const BossAISkill*>& cands);

private:
    Boss* _boss = nullptr;
    bool _enabled = true;

    float _thinkTimer = 0.f;
    float _thinkInterval = 0.10f; // 0.1s 决策一次

    std::vector<BossAISkill> _skills;
    std::unordered_map<std::string, float> _cdLeft; // name -> time left
};

#pragma once

#include "cocos2d.h"
#include "CharacterCollider.h"
#include <vector>
#include <functional>
#include <unordered_map>

USING_NS_CC;

/**
 * @class CombatComponent
 * @brief 战斗组件，负责处理实体的攻击行为、战斗属性和技能管理
 */
class CombatComponent : public Component {
public:
    /**
     * @brief 攻击回调类型定义
     */
    typedef std::function<bool(Node* target)> AttackCallback;

    static CombatComponent* create();
    CombatComponent();
    virtual ~CombatComponent();

    virtual const std::string& getName() const { return Component::getName(); }
    bool init() override;

    // ... (属性 getter/setter)
    void setAttackPower(float attackPower);
    float getAttackPower() const;
    void setDefense(float defense);
    float getDefense() const;
    void setCritRate(float critRate);
    float getCritRate() const;
    void setCritDamage(float critDamage);
    float getCritDamage() const;

    /**
     * @brief 执行攻击结算（对单个目标造成伤害）
     * @param target 目标节点
     * @return bool 是否成功造成伤害
     */
    bool attack(Node* target);

    /**
     * @brief 执行近战范围攻击
     * @param attackerCollider 攻击者的碰撞器（提供当前 AABB）
     * @param potentialTargets 潜在的目标列表
     * @return int 命中的目标数量
     */
    int executeMeleeAttack(const CharacterCollider& attackerCollider, const std::vector<Node*>& potentialTargets);

    void setAttackCallback(const AttackCallback& callback);
    bool castSkill(const std::string& skillName, Node* target = nullptr);
    float calculateDamage(float baseDamage, float targetDefense) const;
    float getWeaponDamage() const;
    void setWeaponDamage(float damage);

protected:
    float _attackPower;
    float _defense;
    float _critRate;
    float _critDamage;
    float _weaponDamage;

    AttackCallback _attackCallback;
};




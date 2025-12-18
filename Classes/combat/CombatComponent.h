#pragma once

#include "cocos2d.h"
#include "Skill.h"
#include <vector>
#include <functional>
#include <unordered_map>

USING_NS_CC;

/**
 * @class CombatComponent
 * @brief 战斗组件，负责处理实体的攻击行为、战斗属性和技能管理
 * @details 该组件是战斗系统的核心部分，为实体提供了完整的战斗能力支持
 *          包括基础攻击、暴击系统、防御计算和技能释放等功能
 */
class CombatComponent : public Component {
public:
    /**
     * @brief 攻击回调类型定义
     * @param target 攻击目标节点
     * @return bool 攻击是否成功执行
     */
    typedef std::function<bool(Node* target)> AttackCallback;
    
    /**
     * @brief 创建CombatComponent实例的工厂方法
     * @return CombatComponent* 创建的实例指针，失败返回nullptr
     */
    static CombatComponent* create();
    
    /**
     * @brief 构造函数
     * @details 初始化所有战斗属性为默认值
     */
    CombatComponent();
    
    /**
     * @brief 析构函数
     * @details 释放所有技能资源，防止内存泄漏
     */
    virtual ~CombatComponent();
    
    /**
     * @brief 组件初始化方法
     * @return bool 初始化是否成功
     */
    bool init() override;
    
    /**
     * @brief 设置实体的基础攻击强度
     * @param attackPower 攻击强度值，为正数，默认值10.0
     */
    void setAttackPower(float attackPower);
    
    /**
     * @brief 获取实体的基础攻击强度
     * @return float 当前攻击强度值
     */
    float getAttackPower() const;
    
    /**
     * @brief 设置实体的基础防御值
     * @param defense 防御值，为非负数，默认值0.0
     */
    void setDefense(float defense);
    
    /**
     * @brief 获取实体的基础防御值
     * @return float 当前防御值
     */
    float getDefense() const;
    
    /**
     * @brief 设置暴击率
     * @param critRate 暴击率，取值范围[0.0, 1.0]，默认值0.05（5%）
     */
    void setCritRate(float critRate);
    
    /**
     * @brief 获取暴击率
     * @return float 当前暴击率
     */
    float getCritRate() const;
    
    /**
     * @brief 设置暴击伤害倍率
     * @param critDamage 暴击伤害倍率，通常大于1.0，默认值2.0（200%）
     */
    void setCritDamage(float critDamage);
    
    /**
     * @brief 获取暴击伤害倍率
     * @return float 当前暴击伤害倍率
     */
    float getCritDamage() const;
    
    /**
     * @brief 执行攻击动作
     * @param target 攻击目标节点，必须包含HealthComponent组件
     * @return bool 攻击是否成功命中目标
     */
    bool attack(Node* target);
    
    /**
     * @brief 设置自定义攻击回调函数
     * @param callback 攻击回调函数，用于实现自定义攻击逻辑
     * @details 如果设置了回调函数，attack()方法将调用该回调而不是默认攻击逻辑
     */
    void setAttackCallback(const AttackCallback& callback);
   
    /**
     * @brief 释放指定名称的技能
     * @param skillName 技能名称，必须与添加时的名称一致
     * @param target 技能目标节点（可选）
     * @return bool 技能是否成功释放
     * @details 如果技能正在冷却或不存在，返回false
     */
    bool castSkill(const std::string& skillName, Node* target = nullptr);
    
    /**
     * @brief 根据基础伤害和目标防御计算最终伤害
     * @param baseDamage 基础伤害值
     * @param targetDefense 目标防御值
     * @return float 最终伤害值
     * @details 使用伤害计算公式：伤害 = 基础伤害 * (1 - 防御/(防御+100))
     */
    float calculateDamage(float baseDamage, float targetDefense) const;
    
    /**
     * @brief 获取当前武器提供的额外伤害
     * @return float 当前武器伤害值
     */
    float getWeaponDamage() const;
    
    /**
     * @brief 设置当前武器提供的额外伤害
     * @param damage 武器伤害值，可为0
     */
    void setWeaponDamage(float damage);
    
protected:
    float _attackPower; ///< 实体的基础攻击强度
    float _defense; ///< 实体的基础防御值
    float _critRate; ///< 暴击率（0.0-1.0）
    float _critDamage; ///< 暴击伤害倍率
    float _weaponDamage; ///< 当前装备武器提供的额外伤害
    
    AttackCallback _attackCallback; ///< 自定义攻击回调函数，用于扩展攻击逻辑
  
};


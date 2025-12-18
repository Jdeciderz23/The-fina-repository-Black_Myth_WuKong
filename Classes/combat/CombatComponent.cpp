#include "CombatComponent.h"
#include "HealthComponent.h"

/**
 * @brief CombatComponent构造函数
 * @details 初始化所有战斗属性为默认值
 * - 默认攻击强度：10.0
 * - 默认防御值：0.0
 * - 默认暴击率：5%
 * - 默认暴击伤害倍率：200%
 * - 默认武器伤害：0.0
 */
CombatComponent::CombatComponent() : 
    _attackPower(10.0f),   // 基础攻击强度10
    _defense(0.0f),        // 基础防御值0
    _critRate(0.05f),      // 5% 暴击率
    _critDamage(2.0f),     // 200% 暴击伤害
    _weaponDamage(0.0f)    // 武器附加伤害
{
}

/**
 * @brief CombatComponent析构函数
 * @details 释放所有技能资源，防止内存泄漏
 * - 遍历技能列表，逐个删除技能实例
 * - 清空技能列表和技能映射表
 */
CombatComponent::~CombatComponent() {
  
}

/**
 * @brief CombatComponent实例创建函数
 * @details 使用工厂模式创建组件实例，并自动管理内存
 * @return CombatComponent* 创建的实例指针，失败返回nullptr
 */
CombatComponent* CombatComponent::create() {
    CombatComponent* component = new CombatComponent();
    if (component && component->init()) {
        component->autorelease();  // 自动释放内存
        return component;
    }
    delete component;  // 初始化失败，手动释放内存
    return nullptr;
}

/**
 * @brief 组件初始化函数
 * @details CombatComponent的初始化非常简单，因为大部分属性已在构造函数中设置
 * @return bool 始终返回true，表示初始化成功
 */
bool CombatComponent::init() {
    return true;  // 初始化成功
}

/**
 * @brief 设置攻击强度
 * @details 更新实体的基础攻击强度属性
 * @param attackPower 新的攻击强度值（应为正数）
 */
void CombatComponent::setAttackPower(float attackPower) {
    _attackPower = attackPower;
}

/**
 * @brief 获取攻击强度
 * @details 返回当前实体的基础攻击强度值
 * @return float 当前攻击强度值
 */
float CombatComponent::getAttackPower() const {
    return _attackPower;
}

/**
 * @brief 设置防御值
 * @details 更新实体的基础防御值属性
 * @param defense 新的防御值（应为非负数）
 */
void CombatComponent::setDefense(float defense) {
    _defense = defense;
}

/**
 * @brief 获取防御值
 * @details 返回当前实体的基础防御值
 * @return float 当前防御值
 */
float CombatComponent::getDefense() const {
    return _defense;
}

/**
 * @brief 设置暴击率
 * @details 更新实体的暴击率属性
 * @param critRate 新的暴击率值（范围：0.0-1.0）
 */
void CombatComponent::setCritRate(float critRate) {
    _critRate = critRate;
}

/**
 * @brief 获取暴击率
 * @details 返回当前实体的暴击率
 * @return float 当前暴击率（范围：0.0-1.0）
 */
float CombatComponent::getCritRate() const {
    return _critRate;
}

/**
 * @brief 设置暴击伤害倍率
 * @details 更新实体的暴击伤害倍率属性
 * @param critDamage 新的暴击伤害倍率（通常大于1.0）
 */
void CombatComponent::setCritDamage(float critDamage) {
    _critDamage = critDamage;
}

/**
 * @brief 获取暴击伤害倍率
 * @details 返回当前实体的暴击伤害倍率
 * @return float 当前暴击伤害倍率
 */
float CombatComponent::getCritDamage() const {
    return _critDamage;
}

/**
 * @brief 执行攻击动作
 * @details 实现实体的攻击逻辑
 * @param target 攻击目标节点
 * @return bool 攻击是否成功执行
 */
bool CombatComponent::attack(Node* target) {
    if (!target) {
        return false;  // 目标不存在，攻击失败
    }
    
    // 检查是否设置了自定义攻击回调
    if (_attackCallback) {
        return _attackCallback(target);  // 调用自定义攻击逻辑
    }
    
    // 默认攻击逻辑
    // 1. 获取目标的健康组件
    HealthComponent* targetHealth = dynamic_cast<HealthComponent*>(target->getComponent("HealthComponent"));
    if (!targetHealth) {
        return false;  // 目标没有健康组件，攻击失败
    }
    
    // 2. 计算总伤害
    float totalDamage = _attackPower + _weaponDamage;
    
    // 3. 检查是否触发暴击
    // 使用rand()生成0-99的随机数，与暴击率比较
    if (rand() % 100 < _critRate * 100) {
        totalDamage *= _critDamage;  // 暴击时伤害翻倍
        CCLOG("Critical hit! Damage: %f", totalDamage);
    }
    
    // 4. 获取目标的防御值（从目标的CombatComponent获取）
    float targetDefense = 0.0f;
    CombatComponent* targetCombat = dynamic_cast<CombatComponent*>(target->getComponent("CombatComponent"));
    if (targetCombat) {
        targetDefense = targetCombat->getDefense();
    }
    
    // 5. 计算防御减免后的最终伤害
    float finalDamage = calculateDamage(totalDamage, targetDefense);
    
    // 5. 对目标造成伤害
    targetHealth->takeDamage(finalDamage, this->getOwner());
    
    return true;  // 攻击成功
}

/**
 * @brief 设置自定义攻击回调
 * @details 允许外部定义自定义的攻击逻辑
 * @param callback 攻击回调函数
 */
void CombatComponent::setAttackCallback(const AttackCallback& callback) {
    _attackCallback = callback;
}



/**
 * @brief 计算最终伤害
 * @details 根据基础伤害和目标防御计算实际伤害值
 * @param baseDamage 基础伤害值
 * @param targetDefense 目标防御值
 * @return float 最终伤害值
 * @note 使用公式：伤害 = 基础伤害 * (1 - 防御/(防御+100))
 *       这种公式确保防御越高，收益递减，避免出现防御无敌的情况
 */
float CombatComponent::calculateDamage(float baseDamage, float targetDefense) const {
    // 伤害减免比例 = 防御 / (防御 + 100)
    // 当防御为0时，减免0%；当防御为100时，减免50%；当防御为200时，减免66.7%...
    float damageReduction = targetDefense / (targetDefense + 100.0f);
    
    // 最终伤害 = 基础伤害 * (1 - 伤害减免比例)
    float finalDamage = baseDamage * (1.0f - damageReduction);
    
    // 确保伤害不小于1（防止出现0伤害的情况）
    return std::max(1.0f, finalDamage);
}

/**
 * @brief 获取武器伤害
 * @details 返回当前武器提供的额外伤害
 * @return float 武器伤害值
 */
float CombatComponent::getWeaponDamage() const {
    return _weaponDamage;
}

/**
 * @brief 设置武器伤害
 * @details 更新当前武器提供的额外伤害
 * @param damage 武器伤害值
 */
void CombatComponent::setWeaponDamage(float damage) {
    _weaponDamage = damage;
    CCLOG("Weapon damage updated: %f", damage);
}
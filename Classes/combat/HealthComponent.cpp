#include "HealthComponent.h"

/**
 * @brief HealthComponent构造函数
 * 
 * 初始化生命值组件的默认属性：
 * - 最大生命值：0
 * - 当前生命值：0
 * - 无敌状态：false
 * - 死亡状态：false
 */
HealthComponent::HealthComponent() :
    _maxHealth(0.0f),            ///< 默认最大生命值为0
    _currentHealth(0.0f),        ///< 默认当前生命值为0
    _isInvincible(false),        ///< 默认不处于无敌状态
    _isDead(false)               ///< 默认未死亡
{
}

/**
 * @brief HealthComponent析构函数
 * 
 * 当前版本的析构函数比较简单，没有需要特别释放的资源。
 */
HealthComponent::~HealthComponent() {
}

/**
 * @brief 创建HealthComponent实例的工厂方法
 * 
 * Cocos2d-x标准的对象创建模式：
 * 1. 分配内存创建实例
 * 2. 调用init方法进行初始化
 * 3. 如果初始化成功，加入自动释放池
 * 4. 如果初始化失败，释放内存并返回nullptr
 * 
 * @param maxHealth 实体的最大生命值
 * @return HealthComponent* 创建成功的实例或nullptr
 */
HealthComponent* HealthComponent::create(float maxHealth) {
    HealthComponent* health = new HealthComponent();  // 1. 分配内存
    if (health && health->init(maxHealth)) {  // 2. 初始化组件
        health->autorelease();  // 3. 加入自动释放池
        return health;
    }
    delete health;  // 4. 初始化失败，释放内存
    return nullptr;
}

/**
 * @brief 初始化HealthComponent组件
 * 
 * 设置实体的最大生命值和当前生命值，并确保它们在合理范围内。
 * 
 * @param maxHealth 实体的最大生命值
 * @return bool 初始化是否成功
 */
bool HealthComponent::init(float maxHealth) {
    // 确保最大生命值为正数
    if (maxHealth <= 0.0f) {
        CCLOG("HealthComponent::init: maxHealth must be positive");
        return false;
    }
    
    _maxHealth = maxHealth;          // 设置最大生命值
    _currentHealth = maxHealth;      // 初始时当前生命值等于最大生命值
    _isInvincible = false;           // 初始时不无敌
    _isDead = false;                 // 初始时未死亡
    
    CCLOG("HealthComponent::init: Initialized with max health %.2f", maxHealth);
    return true;
}

/**
 * @brief 处理实体受伤逻辑
 * 
 * 受伤处理流程：
 * 1. 检查实体是否无敌
 * 2. 检查实体是否已经死亡
 * 3. 计算实际伤害值（确保为正数）
 * 4. 更新当前生命值
 * 5. 确保当前生命值不会小于0
 * 6. 触发受伤回调
 * 7. 检查是否死亡
 * 8. 如果死亡，触发死亡回调
 * 
 * @param damage 伤害值
 * @param attacker 攻击者节点（可选，用于追踪伤害来源）
 */
void HealthComponent::takeDamage(float damage, Node* attacker) {
    // 1. 检查实体是否无敌
    if (_isInvincible) {
        CCLOG("HealthComponent::takeDamage: Entity is invincible, damage ignored");
        return;
    }
    
    // 2. 检查实体是否已经死亡
    if (_isDead) {
        CCLOG("HealthComponent::takeDamage: Entity is already dead, damage ignored");
        return;
    }
    
    // 3. 计算实际伤害值（确保为正数）
    float actualDamage = std::max(damage, 0.0f);
    
    // 4. 更新当前生命值
    float oldHealth = _currentHealth;
    _currentHealth -= actualDamage;
    
    // 5. 确保当前生命值不会小于0
    _currentHealth = std::max(_currentHealth, 0.0f);
    
    CCLOG("HealthComponent::takeDamage: Entity took %.2f damage, health: %.2f/%.2f", 
          actualDamage, _currentHealth, _maxHealth);
    
    // 6. 触发受伤回调
    if (_onHurtCallback) {
        _onHurtCallback(actualDamage, attacker);
    }
    
    // 7. 检查是否死亡
    if (_currentHealth <= 0.0f && !_isDead) {
        _isDead = true;
        CCLOG("HealthComponent::takeDamage: Entity died");
        
        // 8. 如果死亡，触发死亡回调
        if (_onDeadCallback) {
            _onDeadCallback(attacker);
        }
    }
    
    // 9. 触发生命值变化回调
    if (_onHealthChangeCallback) {
        _onHealthChangeCallback(_currentHealth, oldHealth);
    }
}

/**
 * @brief 恢复实体的生命值
 * 
 * 治疗处理流程：
 * 1. 检查实体是否已经死亡
 * 2. 计算实际恢复值（确保为正数）
 * 3. 更新当前生命值
 * 4. 确保当前生命值不会超过最大生命值
 * 5. 触发生命值变化回调
 * 
 * @param amount 恢复的生命值
 */
void HealthComponent::heal(float amount) {
    // 1. 检查实体是否已经死亡
    if (_isDead) {
        CCLOG("HealthComponent::heal: Entity is dead, healing ignored");
        return;
    }
    
    // 2. 计算实际恢复值（确保为正数）
    float actualHeal = std::max(amount, 0.0f);
    
    // 3. 更新当前生命值
    float oldHealth = _currentHealth;
    _currentHealth += actualHeal;
    
    // 4. 确保当前生命值不会超过最大生命值
    _currentHealth = std::min(_currentHealth, _maxHealth);
    
    CCLOG("HealthComponent::heal: Entity healed %.2f health, health: %.2f/%.2f", 
          actualHeal, _currentHealth, _maxHealth);
    
    // 5. 触发生命值变化回调
    if (_onHealthChangeCallback) {
        _onHealthChangeCallback(_currentHealth, oldHealth);
    }
}

/**
 * @brief 设置实体的最大生命值
 * 
 * 同时调整当前生命值，确保其不会超过新的最大生命值。
 * 
 * @param maxHealth 新的最大生命值
 */
void HealthComponent::setMaxHealth(float maxHealth) {
    // 确保最大生命值为正数
    if (maxHealth <= 0.0f) {
        CCLOG("HealthComponent::setMaxHealth: maxHealth must be positive");
        return;
    }
    
    float oldHealth = _currentHealth;
    _maxHealth = maxHealth;
    
    // 确保当前生命值不会超过新的最大生命值
    _currentHealth = std::min(_currentHealth, _maxHealth);
    
    CCLOG("HealthComponent::setMaxHealth: Max health set to %.2f", maxHealth);
    
    // 触发生命值变化回调
    if (_onHealthChangeCallback) {
        _onHealthChangeCallback(_currentHealth, oldHealth);
    }
}

/**
 * @brief 获取实体的最大生命值
 * @return float 最大生命值
 */
float HealthComponent::getMaxHealth() const {
    return _maxHealth;
}

/**
 * @brief 获取实体的当前生命值
 * @return float 当前生命值
 */
float HealthComponent::getCurrentHealth() const {
    return _currentHealth;
}

/**
 * @brief 获取实体的生命值百分比
 * @return float 生命值百分比（0.0-1.0）
 */
float HealthComponent::getHealthPercentage() const {
    if (_maxHealth <= 0.0f) {
        return 0.0f;
    }
    return _currentHealth / _maxHealth;
}

/**
 * @brief 检查实体是否死亡
 * @return bool 是否死亡
 */
bool HealthComponent::isDead() const {
    return _isDead;
}

/**
 * @brief 设置实体的无敌状态
 * 
 * 处于无敌状态的实体不会受到任何伤害。
 * 
 * @param invincible 是否无敌
 */
void HealthComponent::setInvincible(bool invincible) {
    _isInvincible = invincible;
    CCLOG("HealthComponent::setInvincible: Invincible set to %s", invincible ? "true" : "false");
}

/**
 * @brief 检查实体是否处于无敌状态
 * @return bool 是否无敌
 */
bool HealthComponent::isInvincible() const {
    return _isInvincible;
}

/**
 * @brief 设置受伤回调函数
 * 
 * 当实体受到伤害时，会调用此回调函数。
 * 
 * @param callback 受伤时要执行的回调函数
 */
void HealthComponent::setOnHurtCallback(const std::function<void(float, Node*)>& callback) {
    _onHurtCallback = callback;
}

/**
 * @brief 设置死亡回调函数
 * 
 * 当实体死亡时，会调用此回调函数。
 * 
 * @param callback 死亡时要执行的回调函数
 */
void HealthComponent::setOnDeadCallback(const std::function<void(Node*)>& callback) {
    _onDeadCallback = callback;
}

/**
 * @brief 设置生命值变化回调函数
 * 
 * 当实体的生命值发生变化时，会调用此回调函数。
 * 
 * @param callback 生命值变化时要执行的回调函数
 */
void HealthComponent::setOnHealthChangeCallback(const std::function<void(float, float)>& callback) {
    _onHealthChangeCallback = callback;
}


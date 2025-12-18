#pragma once

#include "cocos2d.h"
#include <functional>

USING_NS_CC;

/**
 * @class HealthComponent
 * @brief 生命值组件，负责处理实体的生命值、受伤和死亡逻辑
 */
class HealthComponent : public Component {
public:
    /**
     * @brief 创建HealthComponent实例
     * @param maxHealth 最大生命值
     * @return HealthComponent* 实例指针
     */
    static HealthComponent* create(float maxHealth);

    /**
     * @brief 构造函数
     */
    HealthComponent();

    /**
     * @brief 析构函数
     */
    virtual ~HealthComponent();

    /**
     * @brief 初始化组件
     * @param maxHealth 最大生命值
     * @return bool 初始化是否成功
     */
    bool init(float maxHealth);

    /**
     * @brief 处理实体受伤
     * @param damage 伤害值
     * @param attacker 攻击者（可选）
     */
    void takeDamage(float damage, Node* attacker = nullptr);

    /**
     * @brief 恢复生命值
     * @param amount 恢复的生命值
     */
    void heal(float amount);

    /**
     * @brief 设置最大生命值
     * @param maxHealth 最大生命值
     */
    void setMaxHealth(float maxHealth);

    /**
     * @brief 获取最大生命值
     * @return float 最大生命值
     */
    float getMaxHealth() const;

    /**
     * @brief 获取当前生命值
     * @return float 当前生命值
     */
    float getCurrentHealth() const;

    /**
     * @brief 获取生命值百分比
     * @return float 生命值百分比（0.0-1.0）
     */
    float getHealthPercentage() const;

    /**
     * @brief 检查实体是否死亡
     * @return bool 是否死亡
     */
    bool isDead() const;

    /**
     * @brief 设置无敌状态
     * @param invincible 是否无敌
     */
    void setInvincible(bool invincible);

    /**
     * @brief 检查是否处于无敌状态
     * @return bool 是否无敌
     */
    bool isInvincible() const;

    /**
     * @brief 设置受伤回调
     * @param callback 受伤时的回调函数
     */
    void setOnHurtCallback(const std::function<void(float, Node*)>& callback);

    /**
     * @brief 设置死亡回调
     * @param callback 死亡时的回调函数
     */
    void setOnDeadCallback(const std::function<void(Node*)>& callback);

    /**
     * @brief 设置生命值变化回调
     * @param callback 生命值变化时的回调函数
     */
    void setOnHealthChangeCallback(const std::function<void(float, float)>& callback);

protected:
    float _maxHealth; ///< 最大生命值
    float _currentHealth; ///< 当前生命值
    bool _isInvincible; ///< 是否无敌
    bool _isDead; ///< 是否死亡

    std::function<void(float, Node*)> _onHurtCallback; ///< 受伤回调
    std::function<void(Node*)> _onDeadCallback; ///< 死亡回调
    std::function<void(float, float)> _onHealthChangeCallback; ///< 生命值变化回调
};


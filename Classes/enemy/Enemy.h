#ifndef ENEMY_H
#define ENEMY_H

#pragma once

#include "cocos2d.h"
#include "core/StateMachine.h"

USING_NS_CC;
class HealthComponent;
class CombatComponent;
class Wukong;
/**
 * @class Enemy
 * @brief 敌人基类，所有敌人类型都继承自此类
 */
class Enemy : public Node {
public:
    /**
     * @enum EnemyType
     * @brief 敌人类型枚举
     */
    enum class EnemyType {
        NORMAL,  ///< 普通敌人
        BOSS     ///< BOSS敌人
    };

    /**
     * @brief 创建敌人实例
     * @return Enemy* 敌人实例指针
     */
    static Enemy* create();

    /**
     * @brief 构造函数
     */
    Enemy();

    /**
     * @brief 析构函数
     */
    virtual ~Enemy();

    /**
     * @brief 初始化敌人(初始化血量,创建 3D 模型,初始化状态机,scheduleUpdate)
     * @return bool 初始化是否成功
     */
    virtual bool init() override;

    /**
     * @brief 更新敌人状态(状态切换,移动,攻击冷却,AI 判断)
     * @param deltaTime 帧间隔时间
     */
    virtual void update(float deltaTime) override;

    /**
     * @brief 获取移动速度
     * @return float 移动速度
     */
    float getMoveSpeed() const;

    /**
     * @brief 获取旋转速度
     * @return float 旋转速度
     */
    float getRotateSpeed() const;

    /**
     * @brief 获取视野范围
     * @return float 视野范围
     */
    float getViewRange() const;

    /**
     * @brief 获取是否可以移动
     * @return bool 是否可以移动
     */
    bool canMove() const;

    /**
     * @brief 获取是否可以攻击
     * @return bool 是否可以攻击
     */
    bool canAttack() const;

    /**
     * @brief 获取是否死亡
     * @return bool 是否死亡
     */
    bool isDead() const;

    /**
     * @brief 设置敌人类型
     * @param type 敌人类型
     */
    void setEnemyType(EnemyType type);

    /**
     * @brief 获取敌人类型
     * @return EnemyType 敌人类型
     */
    EnemyType getEnemyType() const;

    /**
     * @brief 获取状态机
     * @return StateMachine<Enemy>* 状态机指针
     */
    StateMachine<Enemy>* getStateMachine() const;

    /**
     * @brief 获取3D精灵
     * @return Sprite3D* 3D精灵指针
     */
    Sprite3D* getSprite() const;

    /**
    * @brief 设置出生点（一般在 init 中调用）
    */
    void setBirthPosition(const Vec3& pos);

    /**
     * @brief 获取出生点
     */
    const Vec3& getBirthPosition() const;

    /**
     * @brief 获取最大追击距离
     */
    float getMaxChaseRange() const;

    void setTarget(Wukong* w);
    Wukong* getTarget() const;

    cocos2d::Vec3 getTargetWorldPos() const; // 获取悟空的世界坐标
    cocos2d::Vec3 getWorldPosition3D() const; // 敌人自己的世界坐标

    //动画资源加载
    static Enemy* createWithResRoot(const std::string& resRoot,
        const std::string& modelFile);

    bool initWithResRoot(const std::string& resRoot,
        const std::string& modelFile);

    const std::string& getResRoot() const { return _resRoot; }

    void playAnim(const std::string& name, bool loop); // name="idle"/"chase"..

protected:
    /**
     * @brief 检查是否处于低血量状态
     * @return bool 是否处于低血量状态
     */
    bool isLowHealth() const;

    /**
     * @brief 获取当前生命值比例
     * @return float 生命值比例 (0.0f - 1.0f)
     */
    float getHealthRatio() const;

protected:
    /**
     * @brief 初始化状态机
     */
    virtual void initStateMachine();

    /**
     * @brief 初始化生命值组件
     */
    virtual void initHealthComponent();

    /**
     * @brief 初始化战斗组件
     */
    virtual void initCombatComponent();

    /**
     * @brief 受伤回调函数
     */
    void onHurtCallback(float damage, Node* attacker);

    /**
     * @brief 死亡回调函数
     */
    void onDeadCallback(Node* attacker);

    EnemyType _enemyType;              // 敌人类型
    StateMachine<Enemy>* _stateMachine; // 状态机指针

    HealthComponent* _health;          // 生命值组件
    CombatComponent* _combat;          // 战斗组件

    // 空间与移动相关
    float _moveSpeed;                  // 移动速度
    float _rotateSpeed;                // 旋转速度
    float _viewRange;                  // 视野范围（原感知范围）

    // 行为控制开关
    bool _canMove;                     // 是否可以移动
    bool _canAttack;                   // 是否可以攻击

    cocos2d::Sprite3D* _sprite;        // 3D模型精灵
    Vec3 _birthPosition;               //出生点
    float _maxChaseRange;              //最大追击距离

    Wukong* _target = nullptr; // 只是引用，不负责释放
    std::string _resRoot;   // 例如 "Enemy/enemy1" 或 "Enemy/boss"
    std::string _modelFile; // 例如 "enemy1.c3b" 或 "boss.c3b"
};

#endif // ENEMY_H
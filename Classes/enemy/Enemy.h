// 功能描述：
// 实现游戏中所有敌人的基类Enemy。此类提供了敌人的基本属性和行为，
// 包括状态管理、移动、战斗、碰撞检测等功能。
#ifndef ENEMY_H
#define ENEMY_H

#pragma once

#include "cocos2d.h"
#include "core/StateMachine.h"
#include "combat/CharacterCollider.h"

USING_NS_CC;
class HealthComponent;
class CombatComponent;
class TerrainCollider;
class Wukong;

/// Enemy 类：敌人基类，所有敌人类型都继承自此类
class Enemy : public Node {
 public:
  /// EnemyType 枚举：敌人类型
  enum class EnemyType {
    NORMAL,  // 普通敌人
    BOSS     // BOSS敌人
  };
  
  /// 创建敌人实例
  static Enemy* create();
  
  /// 构造函数
  Enemy();
  
  /// 析构函数
  virtual ~Enemy();
  
  /// 初始化敌人(初始化血量,创建 3D 模型,初始化状态机,scheduleUpdate)
  virtual bool init() override;
  
  /// 更新敌人状态(状态切换,移动,攻击冷却,AI 判断)
  virtual void update(float deltaTime) override;
    
    // 获取移动速度
    // @return float 移动速度
    float getMoveSpeed() const;
    
    // 获取旋转速度
    // @return float 旋转速度
    float getRotateSpeed() const;
    
    // 获取视野范围
    // @return float 视野范围
    float getViewRange() const;
    
    // 获取是否可以移动
    // @return bool 是否可以移动
    bool canMove() const;
    
    // 获取是否可以攻击
    // @return bool 是否可以攻击
    bool canAttack() const;
    
    // 获取是否死亡
    // @return bool 是否死亡
    bool isDead() const;
    
    // 设置敌人类型
    // @param type 敌人类型
    void setEnemyType(EnemyType type);
    
    // 获取敌人类型
    // @return EnemyType 敌人类型
    EnemyType getEnemyType() const;

    // 设置地形碰撞器
    // @param collider 地形碰撞器指针
    void setTerrainCollider(TerrainCollider* collider) { _terrainCollider = collider; }

    // 获取碰撞组件
    // @return CharacterCollider& 碰撞组件引用
    CharacterCollider& getCollider() { return _collider; }
    
    // 获取碰撞组件（常量版本）
    // @return const CharacterCollider& 碰撞组件常量引用
    const CharacterCollider& getCollider() const { return _collider; }
    
    // 获取战斗组件
    // @return CombatComponent* 战斗组件指针
    CombatComponent* getCombat() const { return _combat; }

    // 获取健康组件
    // @return HealthComponent* 健康组件指针
    HealthComponent* getHealth() const { return _health; }
    
    
    //获取当前生命值比例
    //@return float 生命值比例 (0.0f - 1.0f)
    float getHealthRatio() const;

    /**
     * @brief 设置敌人位置
     * @param position 目标位置
     */
    virtual void setPosition3D(const Vec3& position);
    
    /**
     * @brief 获取敌人位置
     * @return Vec3 敌人位置
     */
    virtual Vec3 getPosition3D() const;
    
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
     * @brief 设置出生位置
     * @param pos 出生位置
     */
    void setBirthPosition(const Vec3& pos);

    /**
     * @brief 获取出生位置
     * @return const Vec3& 出生位置常量引用
     */
    const Vec3& getBirthPosition() const;

    /**
     * @brief 获取最大追击距离
     * @return float 最大追击距离
     */
    float getMaxChaseRange() const;

    /**
     * @brief 设置目标（玩家）
     * @param w 玩家角色指针
     */
    void setTarget(Wukong* w);
    
    /**
     * @brief 获取目标（玩家）
     * @return Wukong* 玩家角色指针
     */
    Wukong* getTarget() const;

    /**
     * @brief 获取目标（玩家）的世界坐标
     * @return Vec3 目标世界坐标
     */
    cocos2d::Vec3 getTargetWorldPos() const;
    
    /**
     * @brief 获取敌人自己的世界坐标
     * @return Vec3 敌人世界坐标
     */
    cocos2d::Vec3 getWorldPosition3D() const;

    // 使用资源根目录创建敌人实例
    // @param resRoot 资源根目录路径，例如 "Enemy/enemy1" 或 "Enemy/boss"
    // @param modelFile 模型文件路径，例如 "enemy1.c3b" 或 "boss.c3b"
    // @return Enemy* 敌人实例指针
    static Enemy* createWithResRoot(const std::string& resRoot,
        const std::string& modelFile);

    // 使用资源根目录初始化敌人
    // @param resRoot 资源根目录路径
    // @param modelFile 模型文件路径
    // @return bool 初始化是否成功
    bool initWithResRoot(const std::string& resRoot,
        const std::string& modelFile);

    const std::string& getResRoot() const { return _resRoot; }

    // 播放动画
    // @param name 动画名称，如"idle"、"chase"等
    // @param loop 是否循环播放
    void playAnim(const std::string& name, bool loop);
    
    // 重置敌人状态（用于复活时重置）
    virtual void resetEnemy();

    /**
     * @brief 设置模型 Y 轴额外偏移（用于微调）
     */
    void setSpriteOffsetY(float offset) { _spriteOffsetY = offset; updateSpritePosition(); }
    float getSpriteOffsetY() const { return _spriteOffsetY; }

protected:
    // 更新精灵位置
    void updateSpritePosition();
    
    // 应用重力效果
    // @param dt 帧间隔时间
    void applyGravity(float dt);
    
    // 应用移动效果
    // @param dt 帧间隔时间
    void applyMovement(float dt);
    
    // 检查是否低生命值
    // @return bool 是否低生命值
    bool isLowHealth() const;
    
    
protected:
    // 初始化状态机
    virtual void initStateMachine();
    
    // 初始化生命值组件
    virtual void initHealthComponent();
    
    // 初始化战斗组件
    virtual void initCombatComponent();
    
    // 受伤回调函数
    // @param damage 受伤数值
    // @param attacker 攻击者节点指针
    void onHurtCallback(float damage, Node* attacker);
    
    // 死亡回调函数
    // @param attacker 攻击者节点指针
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
  Vec3 _targetPosition;              // 目标位置
  Vec3 _birthPosition;               // 出生点
  float _maxChaseRange;              // 最大追击距离

  Wukong* _target = nullptr;         // 只是引用，不负责释放
  std::string _resRoot;              // 例如 "Enemy/enemy1" 或 "Enemy/boss"
  std::string _modelFile;            // 例如 "enemy1.c3b" 或 "boss.c3b"

  // 物理与碰撞
  TerrainCollider* _terrainCollider = nullptr; // 地形碰撞器
  CharacterCollider _collider;       // 角色碰撞器
  Vec3 _velocity = Vec3::ZERO;       // 速度向量
  bool _onGround = true;             // 是否在地面上
  const float _gravity = 980.0f;     // 重力加速度
  float _spriteOffsetY = 0.0f;       // 模型额外偏移

};

#endif // ENEMY_H
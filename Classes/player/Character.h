#ifndef CHARACTER_H
#define CHARACTER_H

#include "StateMachine.h"
#include "cocos2d.h"
#include "../combat/Collider.h"
#include <string>
#include <vector>
#include <memory>

/**
 * @class Character
 * @brief 角色基类（继承 cocos2d::Node），提供移动、跳跃、翻滚、普攻连招、受击、死亡等通用接口
 *
 * @note
 * 1) Controller / AI / 战斗系统等应只通过本类公开接口驱动角色
 * 2) 角色内部使用 StateMachine<Character> 管理状态切换
 */
class Character : public cocos2d::Node {
public:
    /**
     * @brief 移动意图（由输入或 AI 生成）
     */
    struct MoveIntent {
        cocos2d::Vec3 dirWS = cocos2d::Vec3::ZERO; ///< 世界空间方向（可不归一化）
        bool run = false;                          ///< 是否奔跑
    };

    /**
     * @brief 生命状态
     */
    enum class LifeState {
        Alive = 0, ///< 存活
        Dead       ///< 死亡
    };

public:
    /**
     * @brief 构造函数
     */
    Character();

    /**
     * @brief 析构函数
     */
    virtual ~Character();

    /**
     * @brief cocos2d 初始化
     * @return bool 是否初始化成功
     */
    virtual bool init() override;

    /**
     * @brief cocos2d 每帧更新
     * @param dt 帧间隔时间（秒）
     */
    virtual void update(float dt) override;

    // ======================= 对外动作接口（外部系统只调用这些） =======================

    /**
     * @brief 设置移动意图（WASD/摇杆/AI）
     * @param intent 移动意图
     */
    void setMoveIntent(const MoveIntent& intent);

    /**
     * @brief 获取移动意图
     * @return MoveIntent 当前移动意图
     */
    MoveIntent getMoveIntent() const;

    /**
     * @brief 设置地形碰撞器
     */
    void setTerrainCollider(TerrainCollider* collider) { _terrainCollider = collider; }

    /**
     * @brief 发起跳跃请求（最终是否能跳由状态/是否在地面决定）
     */
    void jump();

    /**
     * @brief 发起翻滚/闪避请求
     */
    void roll();

    /**
     * @brief 发起轻攻击（基础连招入口）
     */
    void attackLight();

    /**
     * @brief 受到伤害/进入受击
     * @param damage 伤害值
     */
    void takeHit(int damage);

    /**
     * @brief 死亡（进入死亡状态）
     */
    void die();

    /**
     * @brief 是否在地面
     * @return bool 是否在地面
     */
    bool isOnGround() const;

    /**
     * @brief 是否死亡
     * @return bool 是否死亡
     */
    bool isDead() const;

    /**
     * @brief 获取当前速度
     * @return cocos2d::Vec3 当前速度
     */
    cocos2d::Vec3 getVelocity() const;

    /**
     * @brief 设置水平速度（只改 x/z，不改 y）
     * @param v 水平速度向量
     */
    void setHorizontalVelocity(const cocos2d::Vec3& v);

    /**
     * @brief 停止水平移动（x/z 置 0）
     */
    void stopHorizontal();

    /**
     * @brief 消耗一次连招输入缓冲（AttackState 用）
     * @return bool 本次是否存在缓冲输入
     */
    bool consumeComboBuffered();

    /**
     * @brief 获取角色状态机
     * @return StateMachine<Character>& 状态机引用
     */
    StateMachine<Character>& getStateMachine();

    // ======================= 派生类需实现（体现多态） =======================

    /**
     * @brief 播放动画（派生类实现，体现多态）
     * @param name 动画名
     * @param loop 是否循环
     */
    virtual void playAnim(const std::string& name, bool loop) = 0;

public:
    // ======================= 参数（可后续改为读配置/数值表） =======================

    float walkSpeed = 260.0f; ///< 行走速度
    float runSpeed = 420.0f; ///< 奔跑速度
    float jumpSpeed = 520.0f; ///< 起跳初速度
    float gravity = 1400.0f;///< 重力加速度（简化）

protected:
    /**
     * @brief 应用重力与落地判定（简化：y<=0 认为落地）
     * @param dt 帧间隔时间（秒）
     */
    void applyGravity(float dt);

    /**
     * @brief 应用位移（pos += velocity * dt）
     * @param dt 帧间隔时间（秒）
     */
    void applyMovement(float dt);

protected:
    cocos2d::Node* _visualRoot;          ///< 模型/骨骼/特效挂载根节点
    MoveIntent _moveIntent;              ///< 当前帧移动意图
    cocos2d::Vec3 _velocity;             ///< 当前速度
    bool _onGround;                      ///< 是否在地面

    int _hp;                             ///< 生命值
    LifeState _lifeState;                ///< 生命状态

    bool _comboBuffered;                 ///< 连招输入缓冲

    StateMachine<Character> _fsm;         ///< 角色状态机（拥有状态映射）

    std::vector<std::unique_ptr<BaseState<Character>>> _ownedStates; ///< 状态对象所有权（由角色持有）

    TerrainCollider* _terrainCollider = nullptr; ///< 地形碰撞器
};

#endif // CHARACTER_H

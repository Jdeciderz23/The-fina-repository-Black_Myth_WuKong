#pragma once

#include "EnemyAI.h"
#include <string>

/**
 * @class BossEnemyAI
 * @brief Boss敌人AI决策层
 * @details BossEnemyAI是Boss敌人的纯决策中枢，负责：
 *          - 战斗节奏管理（行为阶段）
 *          - 技能/行为意图表达
 *          - 基于玩家行为的决策
 *          作为纯决策层，不执行任何具体行为，只通过状态机表达意图
 */
class BossEnemyAI : public EnemyAI {
public:
    /**
     * @enum CombatPhase
     * @brief Boss战斗行为阶段
     * @details 表示Boss当前的战斗节奏阶段，用于AI内部决策上下文
     */
    enum class CombatPhase {
        OPENING,    ///< 开场阶段（初始行为）
        PRESSURE,   ///< 施压阶段（主动进攻）
        RECOVERY,   ///< 恢复阶段（短暂休整）
        PUNISH      ///< 惩罚阶段（玩家失误时反击）
    };
    
    /**
     * @enum ActionIntent
     * @brief Boss行为意图
     * @details 表示Boss当前希望执行的行为类型，不是具体状态
     */
    enum class ActionIntent {
        MELEE,      ///< 近战攻击意图
        RANGED,     ///< 远程攻击意图
        DASH,       ///< 冲刺意图
        AOE,        ///< 范围攻击意图
        COUNTER     ///< 反击意图
    };
    
    /**
     * @brief 创建BossEnemyAI实例
     * @return BossEnemyAI* 创建的AI实例指针
     */
    static BossEnemyAI* create();
    
    /**
     * @brief 构造函数
     */
    BossEnemyAI();
    
    /**
     * @brief 析构函数
     */
    virtual ~BossEnemyAI() override;
    
    /**
     * @brief 更新AI决策逻辑
     * @param enemy 此AI控制的敌人实例
     * @param perception 当前帧的感知信息
     * @param deltaTime 自上一帧以来经过的时间
     */
    virtual void updateAI(Enemy* enemy, const EnemyPerception& perception, float deltaTime) override;
    
protected:
    /**
     * @brief 初始化AI实例
     * @return bool 初始化是否成功
     */
    bool init();
    
private:
    /**
     * @brief 从状态机获取敌人的当前状态
     * @param enemy 敌人实例
     * @return std::string 当前状态名称
     */
    std::string getCurrentEnemyState(Enemy* enemy);
    
    /**
     * @brief 请求敌人状态变更
     * @param enemy 敌人实例
     * @param newState 要转换到的新状态
     */
    void requestStateChange(Enemy* enemy, const std::string& newState);
    
    // ===== 战斗节奏管理 =====
    CombatPhase _currentPhase;        ///< 当前战斗行为阶段
    float _phaseTimer;                ///< 当前阶段持续时间计时器
    float _phaseChangeCooldown;       ///< 阶段切换冷却计时器
    
    // ===== 行为意图管理 =====
    ActionIntent _currentIntent;      ///< 当前行为意图
    float _intentExecutionTimer;      ///< 意图执行计时器
    float _intentChangeCooldown;      ///< 意图切换冷却计时器
    
    // ===== 决策稳定性 =====
    float _decisionCooldownTimer;     ///< 决策冷却计时器
    static constexpr float DECISION_COOLDOWN = 0.5f; ///< 决策冷却时间
    
    // ===== 玩家行为感知（AI记忆） =====
    float _playerDistanceMemory;      ///< 玩家距离记忆（抽象值）
    bool _playerAggressive;           ///< 玩家是否具有攻击性
    bool _playerFrequentAttacks;      ///< 玩家是否频繁攻击
    bool _playerRecentlyDodged;       ///< 玩家最近是否闪避
    bool _playerRecentlyBlocked;      ///< 玩家最近是否格挡
    
    // ===== Boss专属状态 =====
    bool _isOpeningCompleted;         ///< 开场阶段是否完成
    bool _inCounterWindow;            ///< 是否处于反击窗口
    float _counterWindowTimer;        ///< 反击窗口计时器
};
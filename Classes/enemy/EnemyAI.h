/**
 * 本文件用于定义 Enemy 的 AI 决策逻辑接口，用于：
 *  - 根据外部信息（玩家位置、战斗结果、环境状态等）
 *    决定 Enemy 应当进入的行为状态
 *  - 协调 Enemy 状态机（StateMachine<Enemy>）的状态切换
 *
 * 设计说明：
 *  1. EnemyAI 不直接控制 Enemy 的数值（血量、攻击、防御等），
 *     这些由 HealthComponent / CombatComponent 等模块负责。
 *  2. EnemyAI 只做“决策”，不做“执行”：
 *     - 不直接移动 Enemy
 *     - 不直接造成伤害
 *     - 不直接播放动画
 *  3. Enemy 的具体行为（移动、攻击、受击、死亡）由 EnemyStates 实现，
 *     EnemyAI 仅通过 Enemy 提供的接口触发状态切换。
 *
 * 模块边界：
 *  - EnemyAI ← 输入：Enemy 状态、外部感知信息（玩家、环境）
 *  - EnemyAI → 输出：Enemy 状态机切换指令
 *
 * 使用建议：
 *  - 普通敌人、精英敌人、Boss 可使用不同的 EnemyAI 派生类
 *  - Boss 的复杂阶段逻辑应优先放在 EnemyAI，而不是 Enemy 本体
 */
#ifndef __ENEMY_AI_H__
#define __ENEMY_AI_H__

/**
 * @file EnemyAI.h
 * @brief Enemy AI 纯决策层接口
 * @details EnemyAI 是敌人 AI 的纯决策中枢，仅负责基于输入的感知信息和内部记忆状态做出决策
 *          - 不负责感知：所有环境信息通过 EnemyPerception 结构体传入
 *          - 不负责执行：仅通过 Enemy 接口或状态机表达行为意图
 *          - 只保存内部记忆：不保存外部状态或 Enemy 属性
 *          此接口设计确保了 AI 决策与感知、执行的完全解耦，支持多实例安全复用
 */
#pragma once

#include "Enemy.h"
#include <cocos2d.h>

USING_NS_CC;

/**
 * @struct EnemyPerception
 * @brief 敌人感知信息结构体
 * @details 封装敌人每帧所需的所有感知输入信息，作为 AI 决策的唯一外部数据源
 *          设计为轻量级结构体，包含最基本的环境感知要素
 */
struct EnemyPerception {
    /**
     * @brief 默认构造函数
     */
    EnemyPerception() 
        : playerPosition(Vec3::ZERO)
        , isPlayerDetected(false)
    {}
    
    /**
     * @brief 带参数的构造函数
     * @param playerPos 玩家 3D 位置
     * @param detected 是否检测到玩家
     */
    EnemyPerception(const Vec3& playerPos, bool detected) 
        : playerPosition(playerPos)
        , isPlayerDetected(detected)
    {}
    
    Vec3 playerPosition;    ///< 玩家的 3D 位置
    bool isPlayerDetected;  ///< 是否检测到玩家
};

/**
 * @class EnemyAI
 * @brief Enemy AI 抽象基类
 * @details 定义敌人 AI 的纯决策接口，所有具体敌人 AI 都需要继承并实现此接口
 *          设计特点：
 *          - 无外部感知输入成员变量：所有感知信息通过 EnemyPerception 传入
 *          - 仅保存内部记忆状态：如警戒状态、决策标记等
 *          - 不直接执行行为：仅通过 Enemy 接口表达意图
 *          - 支持多实例安全复用：无特定于单个 Enemy 的状态
 */
class EnemyAI {
public:
    /**
     * @brief 构造函数
     */
    EnemyAI() 
        : _isAlert(false)
        , _alertTimer(0.0f)
        , _isPatrolling(false)
        , _patrolDirection(1)
    {}
    
    /**
     * @brief 析构函数
     */
    virtual ~EnemyAI() = default;
    
    /**
     * @brief AI 决策更新方法
     * @details 核心决策方法，每帧调用，基于感知信息和内部记忆做出决策
     * @param enemy 所属的敌人实例
     * @param perception 当前帧的感知信息（只读）
     * @param deltaTime 帧间隔时间
     */
    virtual void updateAI(Enemy* enemy, const EnemyPerception& perception, float deltaTime) = 0;
    
protected:
    // ===== AI 内部记忆状态（仅保存决策相关的内部状态）=====
    bool _isAlert;          ///< 是否处于警戒状态
    float _alertTimer;      ///< 警戒状态计时器（用于警戒状态的持续时间管理）
    bool _isPatrolling;     ///< 是否处于巡逻状态
    int _patrolDirection;   ///< 巡逻方向标记（1: 正向, -1: 反向）
    // 可根据需要添加更多 AI 内部记忆状态，如：
    // float _decisionCooldown;  ///< 决策冷却时间（防止频繁切换状态）
    // int _combatPhase;         ///< 战斗阶段标记（用于多阶段 AI）
};

#endif // __ENEMY_AI_H__





/**
 * @file EnemyAI.h
 * @brief Enemy 行为决策接口层（AI 决策中枢）
 *
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

#pragma once


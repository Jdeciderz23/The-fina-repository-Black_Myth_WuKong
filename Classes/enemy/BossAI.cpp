/**
 * @file BossAI.cpp
 * @brief Boss敌人AI决策层实现
 * @details 实现Boss敌人的纯决策逻辑，作为战斗节奏的导演和技能选择的意图决策者
 */

#include "BossAI.h"
#include "Enemy.h"

// 定义常量
constexpr float PHASE_MIN_DURATION = 2.0f;
constexpr float INTENT_MIN_DURATION = 1.0f;
constexpr float COUNTER_WINDOW_DURATION = 1.5f;

BossEnemyAI::BossEnemyAI()
    : _currentPhase(CombatPhase::OPENING)
    , _phaseTimer(0.0f)
    , _phaseChangeCooldown(0.0f)
    , _currentIntent(ActionIntent::MELEE)
    , _intentExecutionTimer(0.0f)
    , _intentChangeCooldown(0.0f)
    , _decisionCooldownTimer(0.0f)
    , _playerDistanceMemory(0.0f)
    , _playerAggressive(false)
    , _playerFrequentAttacks(false)
    , _playerRecentlyDodged(false)
    , _playerRecentlyBlocked(false)
    , _isOpeningCompleted(false)
    , _inCounterWindow(false)
    , _counterWindowTimer(0.0f)
{
    // 初始化基类中的警戒状态
    _isAlert = false;
    _alertTimer = 0.0f;
    _isPatrolling = false; // Boss不巡逻
    _patrolDirection = 1;
}

BossEnemyAI::~BossEnemyAI()
{
}

BossEnemyAI* BossEnemyAI::create()
{
    auto ai = new (std::nothrow) BossEnemyAI();
    if (ai && ai->init()) {
        return ai;
    }
    delete ai;
    return nullptr;
}

bool BossEnemyAI::init()
{
    return true;
}

void BossEnemyAI::updateAI(Enemy* enemy, const EnemyPerception& perception, float deltaTime)
{
    // 安全检查：敌人不存在或已死亡则直接返回
    if (enemy == nullptr || enemy->isDead()) {
        return;
    }

    // =========================================
    // 【阶段 1】世界事实更新（World Facts Update）
    // =========================================
    
    // 更新当前状态
    std::string currentState = getCurrentEnemyState(enemy);
    
    // 更新计时器
    _phaseTimer += deltaTime;
    _intentExecutionTimer += deltaTime;
    _decisionCooldownTimer += deltaTime;
    
    // 更新反击窗口
    if (_inCounterWindow) {
        _counterWindowTimer += deltaTime;
        if (_counterWindowTimer > COUNTER_WINDOW_DURATION) {
            _inCounterWindow = false;
        }
    }
    
    // 根据感知更新玩家行为记忆
    if (perception.isPlayerDetected) {
        _playerDistanceMemory = 0.0f; // 玩家在感知范围内（抽象为近距离）
        _playerAggressive = true; // 玩家主动进入战斗范围，标记为具有攻击性
    } else {
        _playerDistanceMemory = 1.0f; // 玩家不在感知范围内（抽象为远距离）
        _playerAggressive = false;
    }
    
    // 更新特殊标志
    if (currentState == "Hit") {
        _inCounterWindow = true;
        _counterWindowTimer = 0.0f;
        _playerRecentlyDodged = true; // 假设玩家攻击导致Boss受击，因此玩家刚刚进攻
    }
    
    // =========================================
    // 【阶段 2】高层决策（Phase & Intent Decision）
    // =========================================
    
    // CombatPhase 决策
    CombatPhase nextPhase = _currentPhase;
    
    switch (_currentPhase) {
        case CombatPhase::OPENING:
            // 开场阶段只出现一次
            if (_phaseTimer > PHASE_MIN_DURATION) {
                _isOpeningCompleted = true;
                nextPhase = CombatPhase::PRESSURE;
                _phaseTimer = 0.0f;
            }
            break;
            
        case CombatPhase::PRESSURE:
            // 主动进攻阶段，持续一定时间后进入恢复
            if (_phaseTimer > PHASE_MIN_DURATION * 3.0f) {
                nextPhase = CombatPhase::RECOVERY;
                _phaseTimer = 0.0f;
            }
            break;
            
        case CombatPhase::RECOVERY:
            // 恢复阶段，持续短暂时间后回到主动进攻
            if (_phaseTimer > PHASE_MIN_DURATION) {
                nextPhase = CombatPhase::PRESSURE;
                _phaseTimer = 0.0f;
            }
            break;
            
        case CombatPhase::PUNISH:
            // 惩罚阶段，玩家失误后进入，持续短暂时间
            if (_phaseTimer > PHASE_MIN_DURATION) {
                nextPhase = CombatPhase::PRESSURE;
                _phaseTimer = 0.0f;
            }
            break;
    }
    
    // 根据玩家行为调整阶段
    if (_playerFrequentAttacks || _playerRecentlyDodged) {
        // 玩家频繁攻击或闪避，进入惩罚阶段
        nextPhase = CombatPhase::PUNISH;
        _phaseTimer = 0.0f;
    }
    
    // 更新战斗阶段
    _currentPhase = nextPhase;
    
    // ActionIntent 决策
    ActionIntent nextIntent = _currentIntent;
    
    // 根据当前阶段和玩家行为决策意图
    switch (_currentPhase) {
        case CombatPhase::OPENING:
            // 开场阶段，Boss展示姿态
            nextIntent = ActionIntent::MELEE;
            break;
            
        case CombatPhase::PRESSURE:
            // 主动进攻阶段，多样化攻击
            if (_inCounterWindow) {
                nextIntent = ActionIntent::COUNTER;
            } else if (_playerDistanceMemory < 0.5f) { // 近距离
                nextIntent = ActionIntent::MELEE;
            } else { // 中远距离
                nextIntent = ActionIntent::RANGED;
            }
            break;
            
        case CombatPhase::RECOVERY:
            // 恢复阶段，减少攻击性
            nextIntent = ActionIntent::DASH;
            break;
            
        case CombatPhase::PUNISH:
            // 惩罚阶段，使用强力攻击
            nextIntent = ActionIntent::AOE;
            break;
    }
    
    // 更新行为意图
    if (_intentExecutionTimer > INTENT_MIN_DURATION) {
        _currentIntent = nextIntent;
        _intentExecutionTimer = 0.0f;
    }
    
    // =========================================
    // 【阶段 3】意图 → 状态映射（Intent → State）
    // =========================================
    
    // 将行为意图映射为具体状态
    std::string targetState = "";
    
    switch (_currentIntent) {
        case ActionIntent::MELEE:
        case ActionIntent::RANGED:
        case ActionIntent::AOE:
        case ActionIntent::COUNTER:
            targetState = "Attack";
            break;
            
        case ActionIntent::DASH:
            targetState = "Chase";
            break;
    }
    
    // 检查决策冷却和状态是否相同
    if (_decisionCooldownTimer > DECISION_COOLDOWN && currentState != targetState) {
        // 统一通过requestStateChange切换状态
        requestStateChange(enemy, targetState);
        _decisionCooldownTimer = 0.0f;
    }
}

std::string BossEnemyAI::getCurrentEnemyState(Enemy* enemy)
{
    if (enemy == nullptr || enemy->getStateMachine() == nullptr) {
        return "";
    }
    
    // 从状态机获取当前状态
    auto currentState = enemy->getStateMachine()->getCurrentState();
    if (currentState != nullptr) {
        return currentState->getStateName();
    }
    
    return "";
}

void BossEnemyAI::requestStateChange(Enemy* enemy, const std::string& newState)
{
    if (enemy == nullptr || enemy->getStateMachine() == nullptr) {
        return;
    }
    
    // 避免重复切换
    std::string currentState = getCurrentEnemyState(enemy);
    if (currentState == newState) {
        return;
    }
    
    // 请求状态机切换状态
    enemy->getStateMachine()->changeState(newState);
}
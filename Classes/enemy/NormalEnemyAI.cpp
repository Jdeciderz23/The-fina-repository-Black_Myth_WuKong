/**
 * @file NormalEnemyAI.cpp
 * @brief 普通敌人 AI 决策层实现
 * @details 实现普通敌人的纯决策逻辑，控制状态机切换
 */

#include "NormalEnemyAI.h"
#include "Enemy.h"

NormalEnemyAI::NormalEnemyAI()
    : _currentState("")
{
    // 初始化基类中的警戒状态
    _isAlert = false;
    _alertTimer = 0.0f;
    _isPatrolling = true;
    _patrolDirection = 1;
}

NormalEnemyAI::~NormalEnemyAI()
{
}

NormalEnemyAI* NormalEnemyAI::create()
{
    auto ai = new (std::nothrow) NormalEnemyAI();
    if (ai && ai->init()) {
        return ai;
    }
    delete ai;
    return nullptr;
}

bool NormalEnemyAI::init()
{
    return true;
}

void NormalEnemyAI::updateAI(Enemy* enemy, const EnemyPerception& perception, float deltaTime)
{
    // 安全检查：敌人不存在或已死亡则直接返回
    if (enemy == nullptr || enemy->isDead()) {
        return;
    }
    
    // 获取当前状态，避免冗余切换
    _currentState = getCurrentEnemyState(enemy);
    
    // 根据感知信息做出决策
    if (perception.isPlayerDetected) {
        // 发现玩家：进入警戒状态并切换到追逐状态
        _isAlert = true;
        _alertTimer = 0.0f;
        requestStateChange(enemy, "Chase");
    } else {
        if (_isAlert) {
            // 丢失玩家但处于警戒状态：计时，时间到后返回巡逻
            _alertTimer += deltaTime;
            if (_alertTimer >= ALERT_DURATION) {
                _isAlert = false;
                requestStateChange(enemy, "Patrol");
            }
        } else {
            // 未警戒：默认保持巡逻状态
            requestStateChange(enemy, "Patrol");
        }
    }
}

std::string NormalEnemyAI::getCurrentEnemyState(Enemy* enemy)
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

void NormalEnemyAI::requestStateChange(Enemy* enemy, const std::string& newState)
{
    if (enemy == nullptr || enemy->getStateMachine() == nullptr) {
        return;
    }
    
    // 检查是否已经是目标状态，避免频繁切换
    if (_currentState == newState) {
        return;
    }
    
    // 请求状态机切换状态
    enemy->getStateMachine()->changeState(newState);
}
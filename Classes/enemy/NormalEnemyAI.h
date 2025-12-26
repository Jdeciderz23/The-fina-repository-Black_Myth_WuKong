#pragma once

#include "EnemyAI.h"
#include <string>

/**
 * @class NormalEnemyAI
 * @brief 普通敌人 AI（非精英、非 Boss）
 * @details 这个类实现了普通敌人的决策逻辑，基于接收到的感知信息决定状态切换。
 *          遵循纯决策层原则：只决定状态转换，不执行任何操作（移动、攻击等）。
 */
class NormalEnemyAI : public EnemyAI {
public:
    /**
     * @brief 创建新的 NormalEnemyAI 实例
     * @return NormalEnemyAI* 创建的 AI 实例指针
     */
    static NormalEnemyAI* create();
    
    /**
     * @brief 默认构造函数
     */
    NormalEnemyAI();
    
    /**
     * @brief 默认析构函数
     */
    virtual ~NormalEnemyAI() override;
    
    /**
     * @brief 更新 AI 决策逻辑
     * @param enemy 此 AI 控制的敌人实例
     * @param perception 当前帧的感知信息
     * @param deltaTime 自上一帧以来经过的时间
     */
    virtual void updateAI(Enemy* enemy, const EnemyPerception& perception, float deltaTime) override;
    
protected:
    /**
     * @brief 初始化 AI 实例
     * @return bool 初始化是否成功
     */
    bool init();
    
private:
    // 当前状态跟踪（避免冗余状态切换）
    std::string _currentState; ///< 敌人当前的状态
    static constexpr float ALERT_DURATION = 2.0f;
    
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
};

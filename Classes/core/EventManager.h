#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "cocos2d.h"
#include "cocos2d.h"
#include "cocos2d.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>

USING_NS_CC;

USING_NS_CC;

USING_NS_CC;

/**
 * @class EventManager
 * @brief 事件管理器，负责事件的注册、触发和处理
 */
class EventManager {
public:
    /**
     * @brief 事件监听器类型
     */
    typedef std::function<void(void*)> EventListener;

    /**
     * @brief 构造函数
     */
    EventManager();

    /**
     * @brief 析构函数
     */
    ~EventManager();

    /**
     * @brief 初始化事件管理器
     * @return bool 初始化是否成功
     */
    bool init();

    /**
     * @brief 更新事件管理器
     * @param deltaTime 帧间隔时间
     */
    void update(float deltaTime);

    /**
     * @brief 注册事件监听器
     * @param eventName 事件名称
     * @param listener 事件监听器函数
     */
    void addEventListener(const std::string& eventName, const EventListener& listener);

    /**
     * @brief 移除事件监听器
     * @param eventName 事件名称
     * @param listener 要移除的事件监听器函数
     */
    void removeEventListener(const std::string& eventName, const EventListener& listener);

    /**
     * @brief 移除指定事件的所有监听器
     * @param eventName 事件名称
     */
    void removeAllEventListeners(const std::string& eventName);

    /**
     * @brief 移除所有事件监听器
     */
    void removeAllEventListeners();

    /**
     * @brief 触发事件
     * @param eventName 事件名称
     * @param data 事件数据（可选）
     */
    void triggerEvent(const std::string& eventName, void* data = nullptr);

    /**
     * @brief 延迟触发事件
     * @param eventName 事件名称
     * @param delay 延迟时间（秒）
     * @param data 事件数据（可选）
     */
    void triggerEventDelayed(const std::string& eventName, float delay, void* data = nullptr);

private:
    /**
     * @brief 延迟事件结构
     */
    struct DelayedEvent {
        std::string eventName; ///< 事件名称
        float delayTime; ///< 延迟时间
        float elapsedTime; ///< 已流逝时间
        void* data; ///< 事件数据
    };

    std::unordered_map<std::string, std::vector<EventListener>> _eventListeners; ///< 事件监听器映射
    std::vector<DelayedEvent> _delayedEvents; ///< 延迟事件列表
};

// 定义游戏中常用的事件名称
#define EVENT_PLAYER_HURT "player_hurt"              ///< 玩家受伤事件
#define EVENT_PLAYER_DEAD "player_dead"              ///< 玩家死亡事件
#define EVENT_ENEMY_HURT "enemy_hurt"                ///< 敌人受伤事件
#define EVENT_ENEMY_DEAD "enemy_dead"                ///< 敌人死亡事件
#define EVENT_BOSS_HURT "boss_hurt"                  ///< Boss受伤事件
#define EVENT_BOSS_PHASE_CHANGE "boss_phase_change"  ///< Boss阶段变化事件
#define EVENT_BOSS_DEAD "boss_dead"                  ///< Boss死亡事件
#define EVENT_GAME_START "game_start"                ///< 游戏开始事件
#define EVENT_GAME_PAUSE "game_pause"                ///< 游戏暂停事件
#define EVENT_GAME_RESUME "game_resume"              ///< 游戏继续事件
#define EVENT_GAME_OVER "game_over"                  ///< 游戏结束事件
#define EVENT_SCENE_CHANGE "scene_change"            ///< 场景切换事件
#define EVENT_SKILL_CAST "skill_cast"                ///< 技能释放事件
#define EVENT_SKILL_COOLDOWN "skill_cooldown"        ///< 技能冷却事件

#endif // EVENTMANAGER_H
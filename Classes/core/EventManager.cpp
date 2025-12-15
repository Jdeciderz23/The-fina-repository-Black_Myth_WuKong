#include "EventManager.h"
#include <algorithm>

/**
 * @brief EventManager构造函数
 */
EventManager::EventManager() {
}

/**
 * @brief EventManager析构函数
 */
EventManager::~EventManager() {
    removeAllEventListeners();
    _delayedEvents.clear();
}

/**
 * @brief 初始化事件管理器
 * @return bool 初始化是否成功
 */
bool EventManager::init() {
    // 事件管理器初始化成功
    return true;
}

/**
 * @brief 更新事件管理器
 * @param deltaTime 帧间隔时间
 */
void EventManager::update(float deltaTime) {
    // 处理延迟事件
    auto it = _delayedEvents.begin();
    while (it != _delayedEvents.end()) {
        it->elapsedTime += deltaTime;
        if (it->elapsedTime >= it->delayTime) {
            // 延迟时间到，触发事件
            triggerEvent(it->eventName, it->data);
            // 移除已处理的延迟事件
            it = _delayedEvents.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * @brief 注册事件监听器
 * @param eventName 事件名称
 * @param listener 事件监听器函数
 */
void EventManager::addEventListener(const std::string& eventName, const EventListener& listener) {
    _eventListeners[eventName].push_back(listener);
}

/**
 * @brief 移除事件监听器
 * @param eventName 事件名称
 * @param listener 要移除的事件监听器函数
 * @note std::function不能直接比较，所以这个函数目前只是占位符，实际使用中可能需要更复杂的管理方式
 */
void EventManager::removeEventListener(const std::string& eventName, const EventListener& listener) {
    // CCLOG("Warning: EventListener removal not implemented due to std::function comparison limitations");
}

/**
 * @brief 移除指定事件的所有监听器
 * @param eventName 事件名称
 */
void EventManager::removeAllEventListeners(const std::string& eventName) {
    _eventListeners.erase(eventName);
}

/**
 * @brief 移除所有事件监听器
 */
void EventManager::removeAllEventListeners() {
    _eventListeners.clear();
}

/**
 * @brief 触发事件
 * @param eventName 事件名称
 * @param data 事件数据（可选）
 */
void EventManager::triggerEvent(const std::string& eventName, void* data) {
    auto it = _eventListeners.find(eventName);
    if (it != _eventListeners.end()) {
        // 调用所有监听器
        for (const auto& listener : it->second) {
            listener(data);
        }
    }
}

/**
 * @brief 延迟触发事件
 * @param eventName 事件名称
 * @param delay 延迟时间（秒）
 * @param data 事件数据（可选）
 */
void EventManager::triggerEventDelayed(const std::string& eventName, float delay, void* data) {
    DelayedEvent delayedEvent;
    delayedEvent.eventName = eventName;
    delayedEvent.delayTime = delay;
    delayedEvent.elapsedTime = 0.0f;
    delayedEvent.data = data;
    
    _delayedEvents.push_back(delayedEvent);
}
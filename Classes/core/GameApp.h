#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "cocos2d.h"
#include "SceneManager.h"
#include "EventManager.h"

USING_NS_CC;

/**
 * @class GameApp
 * @brief 游戏核心应用类，管理整个游戏的生命周期
 * @details 负责初始化游戏环境、管理场景、事件系统等核心功能
 */
class GameApp {
public:
    /**
     * @brief 获取单例实例
     * @return GameApp* 单例指针
     */
    static GameApp* getInstance();

    /**
     * @brief 初始化游戏
     * @param director 导演实例
     * @return bool 初始化是否成功
     */
    bool init(Director* director);

    /**
     * @brief 游戏主循环更新
     * @param deltaTime 帧间隔时间
     */
    void update(float deltaTime);

    /**
     * @brief 处理游戏暂停
     */
    void pause();

    /**
     * @brief 处理游戏继续
     */
    void resume();

    /**
     * @brief 处理游戏退出
     */
    void exit();

    /**
     * @brief 获取场景管理器
     * @return SceneManager* 场景管理器指针
     */
    SceneManager* getSceneManager() const;

    /**
     * @brief 获取事件管理器
     * @return EventManager* 事件管理器指针
     */
    EventManager* getEventManager() const;

private:
    /**
     * @brief 构造函数（私有，单例模式）
     */
    GameApp();

    /**
     * @brief 析构函数（私有，单例模式）
     */
    ~GameApp();

    static GameApp* _instance; ///< 单例实例
    Director* _director; ///< 导演实例
    SceneManager* _sceneManager; ///< 场景管理器
    EventManager* _eventManager; ///< 事件管理器
    bool _isPaused; ///< 游戏是否暂停
};

#endif // GAMEAPP_H
#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "cocos2d.h"
#include <unordered_map>
#include <string>

USING_NS_CC;

/**
 * @class SceneManager
 * @brief 场景管理器，负责场景的创建、切换和管理
 */
class SceneManager {
public:
    /**
     * @brief 场景类型枚举
     */
    enum class SceneType {
        NONE,       ///< 无场景
        TITLE,      ///< 标题场景
        GAMEPLAY,   ///< 游戏场景
        BOSS_FIGHT, ///< Boss战斗场景
        GAME_OVER   ///< 游戏结束场景
    };

    /**
     * @brief 构造函数
     */
    SceneManager();

    /**
     * @brief 析构函数
     */
    ~SceneManager();

    /**
     * @brief 初始化场景管理器
     * @return bool 初始化是否成功
     */
    bool init();

    /**
     * @brief 更新场景管理器
     * @param deltaTime 帧间隔时间
     */
    void update(float deltaTime);

    /**
     * @brief 注册场景
     * @param type 场景类型
     * @param sceneCreator 创建场景的函数指针
     */
    void registerScene(SceneType type, std::function<Scene*()> sceneCreator);

    /**
     * @brief 切换场景
     * @param type 目标场景类型
     * @param transition 是否使用过渡动画
     */
    void switchScene(SceneType type, bool transition = true);

    /**
     * @brief 获取当前场景类型
     * @return SceneType 当前场景类型
     */
    SceneType getCurrentSceneType() const;

    /**
     * @brief 获取当前场景
     * @return Scene* 当前场景指针
     */
    Scene* getCurrentScene() const;

private:
    /**
     * @brief 创建过渡动画
     * @return TransitionScene* 过渡动画场景
     */
    TransitionScene* createTransition(Scene* scene);

    std::unordered_map<SceneType, std::function<Scene*()>> _sceneCreators; ///< 场景创建函数映射
    SceneType _currentSceneType; ///< 当前场景类型
    Scene* _currentScene; ///< 当前场景指针
    Director* _director; ///< 导演实例
};

#endif // SCENEMANAGER_H
#include "SceneManager.h"

/**
 * @brief SceneManager构造函数
 */
SceneManager::SceneManager() : 
    _currentSceneType(SceneType::NONE),
    _currentScene(nullptr) {
    _director = Director::getInstance();
}

/**
 * @brief SceneManager析构函数
 */
SceneManager::~SceneManager() {
    _sceneCreators.clear();
}

/**
 * @brief 初始化场景管理器
 * @return bool 初始化是否成功
 */
bool SceneManager::init() {
    // 初始化场景创建函数映射
    // 具体场景的注册将在GameApp中完成
    return true;
}

/**
 * @brief 更新场景管理器
 * @param deltaTime 帧间隔时间
 */
void SceneManager::update(float deltaTime) {
    // 场景管理器本身不需要更新
    // 更新逻辑在各个场景中处理
}

/**
 * @brief 注册场景
 * @param type 场景类型
 * @param sceneCreator 创建场景的函数指针
 */
void SceneManager::registerScene(SceneType type, std::function<Scene*()> sceneCreator) {
    _sceneCreators[type] = sceneCreator;
}

/**
 * @brief 切换场景
 * @param type 目标场景类型
 * @param transition 是否使用过渡动画
 */
void SceneManager::switchScene(SceneType type, bool transition) {
    // 检查是否为当前场景
    if (type == _currentSceneType) {
        return;
    }
    
    // 查找场景创建函数
    auto it = _sceneCreators.find(type);
    if (it == _sceneCreators.end()) {
        CCLOG("Scene type not registered: %d", static_cast<int>(type));
        return;
    }
    
    // 创建新场景
    Scene* newScene = it->second();
    if (!newScene) {
        CCLOG("Failed to create scene: %d", static_cast<int>(type));
        return;
    }
    
    if (transition) {
        // 使用过渡动画切换场景
        auto transition = createTransition(newScene);
        _director->replaceScene(transition);
    } else {
        // 直接切换场景
        _director->replaceScene(newScene);
    }
    
    // 更新当前场景信息
    _currentSceneType = type;
    _currentScene = newScene;
    
    CCLOG("Scene switched to: %d", static_cast<int>(type));
}

/**
 * @brief 获取当前场景类型
 * @return SceneType 当前场景类型
 */
SceneManager::SceneType SceneManager::getCurrentSceneType() const {
    return _currentSceneType;
}

/**
 * @brief 获取当前场景
 * @return Scene* 当前场景指针
 */
Scene* SceneManager::getCurrentScene() const {
    return _currentScene;
}

/**
 * @brief 创建过渡动画
 * @param scene 目标场景
 * @return TransitionScene* 过渡动画场景
 */
TransitionScene* SceneManager::createTransition(Scene* scene) {
    // 创建淡入淡出过渡动画，持续时间0.5秒
    return TransitionFade::create(0.5f, scene, Color3B::BLACK);
}

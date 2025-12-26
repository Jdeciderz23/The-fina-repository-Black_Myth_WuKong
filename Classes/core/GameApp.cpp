#include "GameApp.h"                                   // 游戏应用主类
#include "scene_ui/UIManager.h"                        // UI 管理器（用于注册标题场景）
#include "scene_ui/BaseScene.h"                        // 第一个游戏场景

// 初始化单例指针
GameApp* GameApp::_instance = nullptr;

/**
 * @brief 获取GameApp单例实例
 * @return GameApp* 单例指针
 */
GameApp* GameApp::getInstance() {
    if (_instance == nullptr) {
        _instance = new GameApp();
    }
    return _instance;
}

/**
 * @brief GameApp构造函数
 */
GameApp::GameApp() : 
    _director(nullptr),
    _sceneManager(nullptr),
    _eventManager(nullptr),
    _isPaused(false) {
    // 构造函数初始化
}

/**
 * @brief GameApp析构函数
 */
GameApp::~GameApp() {
    // 释放资源
    if (_sceneManager) {
        delete _sceneManager;
        _sceneManager = nullptr;
    }
    
    if (_eventManager) {
        delete _eventManager;
        _eventManager = nullptr;
    }
}

/**
 * @brief 初始化游戏
 * @param director 导演实例
 * @return bool 初始化是否成功
 */
bool GameApp::init(Director* director) {
    if (!director) {
        return false;
    }
    
    // 保存导演实例
    _director = director;
    
    // 创建场景管理器
    _sceneManager = new SceneManager();
    if (!_sceneManager->init()) {
        delete _sceneManager;
        _sceneManager = nullptr;
        return false;
    }
    // 注册标题场景（开始菜单）
    _sceneManager->registerScene(SceneManager::SceneType::TITLE, []() {
        return UIManager::getInstance()->createStartMenuScene();
    });
    _sceneManager->registerScene(SceneManager::SceneType::GAMEPLAY, []() {
        return CampScene::createScene();
    });
    
    // 创建事件管理器
    _eventManager = new EventManager();
    if (!_eventManager->init()) {
        delete _eventManager;
        _eventManager = nullptr;
        return false;
    }
    
    // 初始化成功
    return true;
}

/**
 * @brief 游戏主循环更新
 * @param deltaTime 帧间隔时间
 */
void GameApp::update(float deltaTime) {
    if (_isPaused) {
        return;
    }
    
    // 更新场景管理器
    if (_sceneManager) {
        _sceneManager->update(deltaTime);
    }
    
    // 更新事件管理器
    if (_eventManager) {
        _eventManager->update(deltaTime);
    }
}

/**
 * @brief 处理游戏暂停
 */
void GameApp::pause() {
    _isPaused = true;
    _director->pause();
}

/**
 * @brief 处理游戏继续
 */
void GameApp::resume() {
    _isPaused = false;
    _director->resume();
}

/**
 * @brief 处理游戏退出
 */
void GameApp::exit() {
    _director->end();
}

/**
 * @brief 获取场景管理器
 * @return SceneManager* 场景管理器指针
 */
SceneManager* GameApp::getSceneManager() const {
    return _sceneManager;
}

/**
 * @brief 获取事件管理器
 * @return EventManager* 事件管理器指针
 */
EventManager* GameApp::getEventManager() const {
    return _eventManager;
}

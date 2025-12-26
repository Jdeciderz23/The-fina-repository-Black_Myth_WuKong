#pragma execution_character_set("utf-8")                // 指示编译器按 UTF-8 解析源文件中的字符串字面量
#include "UIManager.h"
#include "GameApp.h"
#include "SceneManager.h"
#include "scene_ui/BaseScene.h"

USING_NS_CC;                                             // 使用 cocos2d 命名空间简写
using namespace cocos2d::ui;                             // 使用 ui 子命名空间

UIManager* UIManager::_instance = nullptr;               // 定义并初始化单例静态指针

UIManager* UIManager::getInstance() {                    // 获取单例实例
    if (_instance == nullptr) {                          // 如果尚未创建实例
        _instance = new (std::nothrow) UIManager();      // 使用不抛异常的 new 创建实例
    }                                                    // 判断结束
    return _instance;                                     // 返回单例指针
}                                                        // getInstance 结束

void UIManager::destroyInstance() {                      // 销毁单例实例
    CC_SAFE_DELETE(_instance);                           // 安全删除并置空
}                                                        // destroyInstance 结束

UIManager::UIManager() {                                 // 构造函数
    // 此处可初始化成员变量，目前暂无需要                         // 构造说明
}                                                        // 构造结束

UIManager::~UIManager() {                                // 析构函数
    // 此处可释放资源，目前暂无需要                             // 析构说明
}                                                        // 析构结束

Scene* UIManager::createStartMenuScene() {               // 创建开始菜单场景
    auto scene = Scene::create();                        // 新建一个场景对象

    auto layer = Layer::create();                        // 创建一个承载 UI 的层
    scene->addChild(layer);                              // 将层添加到场景

    auto visibleSize = Director::getInstance()->getVisibleSize(); // 获取可见区域大小
    Vec2 origin = Director::getInstance()->getVisibleOrigin();    // 获取可见区域原点

    // ---------------- 背景图片 -----------------------------
    auto background = Sprite::create("StartMenu.png"); // 从资源中加载背景图片（文件需位于 Resources）
    if (background) {                                    // 如果加载成功
        background->setPosition(                         // 设置背景到屏幕中心
            Vec2(visibleSize.width/2 + origin.x, 
                 visibleSize.height/2 + origin.y));      
        
        float scaleX = visibleSize.width / background->getContentSize().width;   // 计算横向缩放
        float scaleY = visibleSize.height / background->getContentSize().height; // 计算纵向缩放
        float scale = std::max(scaleX, scaleY);          // 取较大值以实现填充（避免黑边）
        background->setScale(scale);                     // 设置缩放
        
        layer->addChild(background, -1);                 // 背景层级设为 -1，置于最底层
    } else {                                             // 如果加载失败
        CCLOG("Error: StartMenu.png not found in Resources!"); // 打印错误日志
        auto bgLayer = LayerColor::create(Color4B(20, 20, 20, 255)); // 兜底纯色背景
        layer->addChild(bgLayer, -2);                    // 添加纯色背景到场景
    }                                                    // 背景处理结束

    // ---------------- 标题文字（可选） ----------------------
    auto titleLabel = Label::createWithTTF(              // 创建 TTF 字体标签
        "Black Myth: Wukong",                            // 文本内容
        "fonts/arial.ttf",                         // 字体文件
        100);                                             // 字号
    if (titleLabel) {                                    // 如果创建成功
        titleLabel->setPosition(                         // 设置标题位置在屏幕 80% 高度
            Vec2(visibleSize.width/2 + origin.x, 
                 visibleSize.height * 0.8 + origin.y));
        titleLabel->enableShadow();                      // 开启阴影效果增强可读性
        layer->addChild(titleLabel, 1);                  // 添加到层，层级为 1
    }                                                    // 标题处理结束

    // ---------------- 菜单按钮 ------------------------------
    MenuItemFont::setFontName("fonts/arial.ttf");        // 设置全局菜单字体
    MenuItemFont::setFontSize(40);                       // 设置全局菜单字号

    auto startItem = MenuItemFont::create(               // 创建“开始游戏”菜单项
        "Start Game",                                    // 按钮文本
        CC_CALLBACK_1(UIManager::onStartGame, this));    // 点击回调函数
    startItem->setColor(Color3B::WHITE);                 // 设置按钮文字颜色为白色

    auto settingsItem = MenuItemFont::create(            // 创建“设置”菜单项
        "Settings",                                      // 按钮文本
        CC_CALLBACK_1(UIManager::onSettings, this));     // 点击回调函数
    settingsItem->setColor(Color3B::WHITE);              // 设置按钮文字颜色为白色

    auto exitItem = MenuItemFont::create(                // 创建“退出”菜单项
        "Exit",                                          // 按钮文本
        CC_CALLBACK_1(UIManager::onExitGame, this));     // 点击回调函数
    exitItem->setColor(Color3B(255, 100, 100));          // 将退出按钮设为红色以示区分

    auto menu = Menu::create(                            // 创建菜单容器并加入三个按钮
        startItem, settingsItem, exitItem, nullptr);     
    menu->setPosition(                                   // 将菜单放置在屏幕中心
        Vec2(visibleSize.width/2 + origin.x, 
             visibleSize.height/2 + origin.y));
    
    menu->alignItemsVerticallyWithPadding(50);           // 垂直排列按钮，设置间距为 50 像素
    
    layer->addChild(menu, 1);                            // 将菜单添加到层，层级为 1

    return scene;
}

void UIManager::showPauseMenu()
{
    auto running = Director::getInstance()->getRunningScene();
    if (!running) return;

    GameApp::getInstance()->pause();

    auto layer = Layer::create();
    layer->setName("PauseMenuLayer");
    running->addChild(layer, 9999);

    auto vs = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto bg = Sprite::create("pause.png");
    if (bg) {
        bg->setPosition(Vec2(vs.width/2 + origin.x, vs.height/2 + origin.y));
        float sx = vs.width / bg->getContentSize().width;
        float sy = vs.height / bg->getContentSize().height;
        bg->setScale(std::max(sx, sy));
        layer->addChild(bg, -1);
    }

    MenuItemFont::setFontName("fonts/arial.ttf");
    MenuItemFont::setFontSize(32);

    auto healItem = MenuItemFont::create("\xe5\x9b\x9e\xe8\xa1\x80", CC_CALLBACK_1(UIManager::onPauseHeal, this));
    auto teleportItem = MenuItemFont::create("\xe4\xbc\xa0\xe9\x80\x81", CC_CALLBACK_1(UIManager::onPauseTeleport, this));
    auto resumeItem = MenuItemFont::create("\xe7\xbb\xa7\xe7\xbb\xad\xe6\xb8\xb8\xe6\x88\x8f", CC_CALLBACK_1(UIManager::onPauseResume, this));
    auto titleItem = MenuItemFont::create("\xe5\x9b\x9e\xe5\x88\xb0\xe5\xbc\x80\xe5\xa7\x8b\xe8\x8f\x9c\xe5\x8d\x95", CC_CALLBACK_1(UIManager::onPauseReturnTitle, this));

    auto menu = Menu::create(healItem, teleportItem, resumeItem, titleItem, nullptr);
    menu->alignItemsVerticallyWithPadding(30);
    menu->setPosition(Vec2(vs.width/2 + origin.x, vs.height/2 + origin.y));
    layer->addChild(menu, 1);
}

void UIManager::onStartGame(Ref* sender)
{
    auto sceneMgr = GameApp::getInstance()->getSceneManager();
    if (sceneMgr) {
        sceneMgr->switchScene(SceneManager::SceneType::GAMEPLAY, true);
    }
}

void UIManager::onSettings(Ref* sender)
{
}

void UIManager::onExitGame(Ref* sender)
{
    Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

void UIManager::onPauseHeal(Ref* sender)
{
}

void UIManager::onPauseTeleport(Ref* sender)
{
    auto scene = GameApp::getInstance()->getSceneManager()->getCurrentScene();
    auto base = dynamic_cast<BaseScene*>(scene);
    if (base) base->teleportPlayerToCenter();
}

void UIManager::onPauseResume(Ref* sender)
{
    auto running = Director::getInstance()->getRunningScene();
    if (running) {
        auto layer = running->getChildByName("PauseMenuLayer");
        if (layer) running->removeChild(layer);
    }
    GameApp::getInstance()->resume();
}

void UIManager::onPauseReturnTitle(Ref* sender)
{
    auto running = Director::getInstance()->getRunningScene();
    if (running) {
        auto layer = running->getChildByName("PauseMenuLayer");
        if (layer) running->removeChild(layer);
    }
    GameApp::getInstance()->resume();
    auto sceneMgr = GameApp::getInstance()->getSceneManager();
    if (sceneMgr) sceneMgr->switchScene(SceneManager::SceneType::TITLE, true);
}

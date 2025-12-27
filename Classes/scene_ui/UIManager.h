#ifndef __UI_MANAGER_H__                         // 头文件防重定义宏开始
#define __UI_MANAGER_H__                         // 定义防重宏

#pragma execution_character_set("utf-8")         // 指示 MSVC 按 UTF-8 解释源文件中的字符串字面量

#include "cocos2d.h"                             // 引入 cocos2d 核心头文件
#include "ui/CocosGUI.h"                         // 引入 Cocos UI 组件头文件

/**
 * @class UIManager
 * @brief UI 管理器
 * @details 管理游戏中的各类 UI（开始菜单、HUD、暂停菜单等），采用单例模式便于全局访问
 */
class UIManager {
public:
    /**
     * @brief 获取 UIManager 单例实例
     * @return UIManager* 单例指针
     */
    static UIManager* getInstance();

    /**
     * @brief 销毁 UIManager 单例实例
     * @details 在游戏结束或重启时调用，用于释放内存
     */
    static void destroyInstance();

    /**
     * @brief 创建“开始菜单”场景
     * @details 包含背景图片与主菜单按钮（开始游戏 / 设置 / 退出）
     * @return cocos2d::Scene* 创建好的场景对象
     */
    cocos2d::Scene* createStartMenuScene();
    void showPauseMenu();
    void showDeathMenu();

    /**
     * @brief 显示 HUD（血条等）
     * @param parent HUD 挂载的父节点
     */
    void showHUD(cocos2d::Node* parent);

    /**
     * @brief 更新玩家血条显示
     * @param percent 生命值百分比（0.0-1.0）
     */
    void updatePlayerHP(float percent);

private:
    /**
     * @brief 构造函数（私有）
     * @details 防止外部直接实例化，仅通过 getInstance 获取
     */
    UIManager();

    /**
     * @brief 析构函数（私有）
     */
    ~UIManager();
    
    static UIManager* _instance;                 // 单例指针

    cocos2d::DrawNode* _hpBarDrawNode = nullptr; // 血条绘制节点
    cocos2d::Label* _hpLabel = nullptr;          // 血量文字标签
    float _hpBarWidth = 400.0f;                  // 血条宽度
    float _hpBarHeight = 20.0f;                  // 血条高度

    // ---------------- 菜单按钮回调 ----------------

    /**
     * @brief “开始游戏”按钮点击回调
     * @param sender 触发事件的节点对象
     */
    void onStartGame(cocos2d::Ref* sender);

    /**
     * @brief “设置”按钮点击回调
     * @param sender 触发事件的节点对象
     */
    void onSettings(cocos2d::Ref* sender);

    /**
     * @brief “退出”按钮点击回调
     * @param sender 触发事件的节点对象
     */
    void onExitGame(cocos2d::Ref* sender);

    void onPauseHeal(cocos2d::Ref* sender);
    void onPauseTeleport(cocos2d::Ref* sender);
    void onPauseResume(cocos2d::Ref* sender);
    void onPauseReturnTitle(cocos2d::Ref* sender);

    void onDeathRespawn(cocos2d::Ref* sender);
    void onDeathReturnTitle(cocos2d::Ref* sender);

    // ---------------- 设置菜单相关 ----------------
    void showSettingsMenu();
    void onVolumeSliderChanged(cocos2d::Ref* sender, cocos2d::ui::Slider::EventType type);
    void onCloseSettings(cocos2d::Ref* sender);
};

#endif // __UI_MANAGER_H__                        // 防重宏结束

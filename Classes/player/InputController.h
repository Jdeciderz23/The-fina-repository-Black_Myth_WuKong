#pragma once
#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "cocos2d.h"
#include "Wukong.h"

/**
 * @class PlayerController
 * @brief 玩家控制器：采样键盘输入 -> 生成 MoveIntent 并驱动角色动作
 *
 * @note
 * Controller 不包含战斗判定业务逻辑，只负责输入翻译：
 * - WASD/Shift -> setMoveIntent
 * - Space -> jump
 * - J -> attackLight
 * - K -> roll
 */
class PlayerController : public cocos2d::Node {
public:
    /**
     * @brief 创建控制器并绑定目标角色
     * @param target 被控制的悟空角色
     * @return PlayerController* 创建成功返回指针，失败返回 nullptr
     */
    static PlayerController* create(Wukong* target);

    /**
     * @brief 初始化
     * @param target 被控制的悟空角色
     * @return bool 是否初始化成功
     */
    bool init(Wukong* target);

    /**
     * @brief 每帧更新：输出 MoveIntent
     * @param dt 帧间隔时间（秒）
     */
    void update(float dt) override;

    /**
    * @brief 随相对镜头移动
    * @param cam
    */
    void setCamera(cocos2d::Camera* cam) { _cam = cam; }

private:
    /**
     * @brief 绑定键盘事件监听
     */
    void bindKeyboard();

private:
    Wukong* _target; ///< 目标角色（不拥有）
    cocos2d::Camera* _cam = nullptr;

    bool _w;   ///< W 是否按下
    bool _a;   ///< A 是否按下
    bool _s;   ///< S 是否按下
    bool _d;   ///< D 是否按下
    bool _run; ///< Shift 是否按下（奔跑）
};

#endif // PLAYERCONTROLLER_H

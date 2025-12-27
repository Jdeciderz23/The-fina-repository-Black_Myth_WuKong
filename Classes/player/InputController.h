#pragma once
#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "cocos2d.h"
#include "Wukong.h"

class Wukong;
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
    void setCamera(cocos2d::Camera* cam);

    void setOwner(Wukong* w) { _wukong = w; }   // 或者放构造函数里传入

private:
    /**
     * @brief 绑定键盘事件监听
     */
    void bindKeyboard();
    void bindMouse();

    void updateThirdPersonCamera(float dt);
    float moveTowardAngleDeg(float cur, float target, float maxDeltaDeg) const;

private:
    Wukong* _target=nullptr; /// 目标角色
    Wukong* _wukong = nullptr;
    cocos2d::Camera* _cam = nullptr;

    bool _w;   ///< W 是否按下
    bool _a;   ///< A 是否按下
    bool _s;   ///< S 是否按下
    bool _d;   ///< D 是否按下
    bool _run; ///< Shift 是否按下（奔跑）

    // ===== Third person camera (orbit) =====
    bool _mouseRotating = false;
    cocos2d::Vec2 _lastMouse{ 0, 0 };

    float _camYawDeg = 0.0f;    // 0 表示相机在角色“后方”（你的坐标下相机 offset 是 +Z）
    float _camPitchDeg = -15.0f;  // 负值：略向下俯视
    float _camDist = 32.0f;

    float _camFollowPosK = 12.0f;     // 位置跟随平滑（越大越“跟手”）
    float _autoYawSpeed = 240.0f;    // 不转鼠标时，相机自动追随角色朝向（度/秒）
    bool  _autoFollowYaw = true;      // 想完全自由相机就改 false

    float _mouseSens = 0.12f;         // 鼠标灵敏度
    float _minPitch = -35.0f;
    float _maxPitch = -8.0f;
    float _minDist = 18.0f;
    float _maxDist = 90.0f;

    float _lookAtHeight = 12.0f;      // 镜头看向角色“胸口”高度
};

#endif // PLAYERCONTROLLER_H

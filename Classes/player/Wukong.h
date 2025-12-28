#pragma once
#ifndef WUKONG_H
#define WUKONG_H

#include "Character.h"
#include <string>
#include"cocos2d.h"
#include <unordered_map>
#include <functional>

/**
 * @class Wukong
 * @brief 战斗角色类（Character 的子类），实现悟空的模型加载、动画控制和状态管理。
 */
class Wukong : public Character {
public:
    /**
     * @brief 创建悟空实例的静态方法（Cocos2d-x 惯例）
     * @return Wukong* 创建成功的对象指针，失败返回 nullptr
     */
    static Wukong* create();

    /**
     * @brief 初始化方法
     * @return bool 是否初始化成功
     */
    virtual bool init() override;

    /**
     * @brief 播放指定名称的动画
     * @param name 动画名称
     * @param loop 是否循环播放
     */
    virtual void playAnim(const std::string& name, bool loop) override;

    // 跳跃动画序列：Start -> Apex (循环)
    void startJumpAnim();

    // 落地处理：停止 Apex -> Land -> Recovery -> 回到 Idle/Move
    void onJumpLanded();

    enum class MoveDir { None, Fwd, Bwd, Left, Right };

    // x: 右为+，y: 前为+（你也可以用 z，这里用 Vec2 更直观）
    void setMoveAxis(const cocos2d::Vec2& axis);
    void updateLocomotionAnim(bool running);
    float getAnimDuration(const std::string& key) const;

    // 给敌人/AI 用：返回悟空“世界坐标系”的位置（推荐用这个做距离/追击判断）
    cocos2d::Vec3 getWorldPosition3D() const;

    void castSkill();
    void triggerHurt();
    void triggerDead();

private:
    cocos2d::Sprite3D* _model; ///< 角色模型指针
    std::string _curAnim;
    int _animTag = 1001;
    std::unordered_map<std::string, cocos2d::Animation3D*> _anims;
    cocos2d::Action* _curAnimAction = nullptr;
    cocos2d::Animate3D* makeAnimate(const std::string& key) const;
    enum class LocomotionDir { None, Fwd, Bwd, Left, Right };
    LocomotionDir calcLocomotionDir(const cocos2d::Vec2& axis) const;
    bool _jumpAnimPlaying = false;

    cocos2d::Vec2 _moveAxis{ 0.0f, 0.0f };
    LocomotionDir _locoDir = LocomotionDir::None;
    bool _locoRun = false;


    void loadAnimIfNeeded(const std::string& key,
        const std::string& c3bPath);
    MoveDir _runDir = MoveDir::None;        // 当前奔跑方向（防止每帧重复切）
    std::string _curAnimKey;                // 当前动画 key（防止重复播放）

};

#endif // WUKONG_H
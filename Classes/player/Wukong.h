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
 * @brief 锟斤拷战锟缴拷啵–haracter 锟斤拷锟斤拷锟洁）锟斤拷实锟街讹拷锟斤拷锟斤拷锟斤拷锟斤拷模锟酵癸拷锟截ｏ拷锟斤拷态实锟街ｏ拷
 */
class Wukong : public Character {
public:
    /**
     * @brief 锟斤拷锟斤拷锟斤拷锟绞碉拷锟斤拷锟絚ocos2d 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
     * @return Wukong* 锟斤拷锟斤拷锟缴癸拷锟斤拷锟截讹拷锟斤拷指锟诫，失锟杰凤拷锟斤拷 nullptr
     */
    static Wukong* create();

    /**
     * @brief 锟斤拷始锟斤拷
     * @return bool 锟角凤拷锟绞硷拷锟斤拷晒锟?
     */
    virtual bool init() override;

    /**
     * @brief 锟斤拷锟脚讹拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷实锟街ｏ拷
     * @param name 锟斤拷锟斤拷锟斤拷
     * @param loop 锟角凤拷循锟斤拷
     */
    virtual void playAnim(const std::string& name, bool loop) override;

    // 锟斤拷锟斤拷锟皆撅拷锟絇ad -> Start -> Apex(循锟斤拷)
    void startJumpAnim();

    // 锟斤拷兀锟酵Ｖ笰pex -> Land -> Recovery -> 锟截碉拷Idle/Move
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
    cocos2d::Sprite3D* _model; ///< 锟斤拷色模锟酵ｏ拷锟斤拷为锟秸ｏ拷
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
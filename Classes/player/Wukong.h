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
 * @brief 悟空角色类（Character 派生类），实现动画播放与模型挂载（多态实现）
 */
class Wukong : public Character {
public:
    /**
     * @brief 创建悟空实例（cocos2d 工厂函数）
     * @return Wukong* 创建成功返回对象指针，失败返回 nullptr
     */
    static Wukong* create();

    /**
     * @brief 初始化
     * @return bool 是否初始化成功
     */
    virtual bool init() override;

    /**
     * @brief 播放动画（派生类实现）
     * @param name 动画名
     * @param loop 是否循环
     */
    virtual void playAnim(const std::string& name, bool loop) override;

    // 多段跳跃：Pad -> Start -> Apex(循环)
    void startJumpAnim();

    // 落地：停止Apex -> Land -> Recovery -> 回到Idle/Move
    void onJumpLanded();


private:
    cocos2d::Sprite3D* _model; ///< 角色模型（可为空）
    std::string _curAnim;
    int _animTag = 1001;
    std::unordered_map<std::string, cocos2d::Animation3D*> _anims;
    cocos2d::Action* _curAnimAction = nullptr;
    cocos2d::Animate3D* makeAnimate(const std::string& key) const;

    void loadAnimIfNeeded(const std::string& key,
        const std::string& c3bPath);
};

#endif // WUKONG_H

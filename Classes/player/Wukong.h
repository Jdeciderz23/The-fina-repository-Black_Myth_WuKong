#pragma once
#ifndef WUKONG_H
#define WUKONG_H

#include "Character.h"
#include <string>

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

private:
    cocos2d::Sprite3D* _model; ///< 角色模型（可为空）
};

#endif // WUKONG_H

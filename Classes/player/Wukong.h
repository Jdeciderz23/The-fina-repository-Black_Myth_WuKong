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
 * @brief ÔøΩÔøΩ’ΩÔøΩ…´ÔøΩ‡£®Character ÔøΩÔøΩÔøΩÔøΩÔøΩ‡£©ÔøΩÔøΩ µÔøΩ÷∂ÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩƒ£ÔøΩÕπÔøΩÔøΩÿ£ÔøΩÔøΩÔøΩÃ¨ µÔøΩ÷£ÔøΩ
 */
class Wukong : public Character {
public:
    /**
     * @brief ÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩ µÔøΩÔøΩÔøΩÔøΩcocos2d ÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩ
     * @return Wukong* ÔøΩÔøΩÔøΩÔøΩÔøΩ…πÔøΩÔøΩÔøΩÔøΩÿ∂ÔøΩÔøΩÔøΩ÷∏ÔøΩÎ£¨ ßÔøΩ‹∑ÔøΩÔøΩÔøΩ nullptr
     */
    static Wukong* create();

    /**
     * @brief ÔøΩÔøΩ ºÔøΩÔøΩ
     * @return bool ÔøΩ«∑ÔøΩÔøΩ ºÔøΩÔøΩÔøΩ…πÔø?
     */
    virtual bool init() override;

    /**
     * @brief ÔøΩÔøΩÔøΩ≈∂ÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩ µÔøΩ÷£ÔøΩ
     * @param name ÔøΩÔøΩÔøΩÔøΩÔøΩÔøΩ
     * @param loop ÔøΩ«∑ÔøΩ—≠ÔøΩÔøΩ
     */
    virtual void playAnim(const std::string& name, bool loop) override;

    // ÔøΩÔøΩÔøΩÔøΩÔøΩ‘æÔøΩÔøΩPad -> Start -> Apex(—≠ÔøΩÔøΩ)
    void startJumpAnim();

    // ÔøΩÔøΩÿ£ÔøΩÕ£÷πApex -> Land -> Recovery -> ÔøΩÿµÔøΩIdle/Move
    void onJumpLanded();

    enum class MoveDir { None, Fwd, Bwd, Left, Right };

    // x: ”“Œ™+£¨y: «∞Œ™+£®ƒ„“≤ø…“‘”√ z£¨’‚¿Ô”√ Vec2 ∏¸÷±π€£©
    void setMoveAxis(const cocos2d::Vec2& axis);
    void updateLocomotionAnim(bool running);
    // ‘⁄ RunState ªÚ update ¿Ôµ˜”√À¸£¨”√”⁄«–ªªÀƒœÚ±º≈‹∂Øª≠
    void updateRunAnimation();


private:
    MoveDir calcMoveDirFromAxis(const cocos2d::Vec2& axis) const;


private:
    cocos2d::Sprite3D* _model; ///< ÔøΩÔøΩ…´ƒ£ÔøΩÕ£ÔøΩÔøΩÔøΩŒ™ÔøΩ’£ÔøΩ
    std::string _curAnim;
    int _animTag = 1001;
    std::unordered_map<std::string, cocos2d::Animation3D*> _anims;
    cocos2d::Action* _curAnimAction = nullptr;
    cocos2d::Animate3D* makeAnimate(const std::string& key) const;
    enum class LocomotionDir { None, Fwd, Bwd, Left, Right };
    LocomotionDir calcLocomotionDir(const cocos2d::Vec2& axis) const;

    cocos2d::Vec2 _moveAxis{ 0.0f, 0.0f };
    LocomotionDir _locoDir = LocomotionDir::None;
    bool _locoRun = false;


    void loadAnimIfNeeded(const std::string& key,
        const std::string& c3bPath);
    MoveDir _runDir = MoveDir::None;        // µ±«∞±º≈‹∑ΩœÚ£®∑¿÷π√ø÷°÷ÿ∏¥«–£©
    std::string _curAnimKey;                // µ±«∞∂Øª≠ key£®∑¿÷π÷ÿ∏¥≤•∑≈£©

};

#endif // WUKONG_H
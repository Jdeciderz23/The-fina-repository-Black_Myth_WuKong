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
 * @brief ï¿½ï¿½Õ½ï¿½É«ï¿½à£¨Character ï¿½ï¿½ï¿½ï¿½ï¿½à£©ï¿½ï¿½Êµï¿½Ö¶ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä£ï¿½Í¹ï¿½ï¿½Ø£ï¿½ï¿½ï¿½Ì¬Êµï¿½Ö£ï¿½
 */
class Wukong : public Character {
public:
    /**
     * @brief ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Êµï¿½ï¿½ï¿½ï¿½cocos2d ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
     * @return Wukong* ï¿½ï¿½ï¿½ï¿½ï¿½É¹ï¿½ï¿½ï¿½ï¿½Ø¶ï¿½ï¿½ï¿½Ö¸ï¿½ë£¬Ê§ï¿½Ü·ï¿½ï¿½ï¿½ nullptr
     */
    static Wukong* create();

    /**
     * @brief ï¿½ï¿½Ê¼ï¿½ï¿½
     * @return bool ï¿½Ç·ï¿½ï¿½Ê¼ï¿½ï¿½ï¿½É¹ï¿?
     */
    virtual bool init() override;

    /**
     * @brief ï¿½ï¿½ï¿½Å¶ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Êµï¿½Ö£ï¿½
     * @param name ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
     * @param loop ï¿½Ç·ï¿½Ñ­ï¿½ï¿½
     */
    virtual void playAnim(const std::string& name, bool loop) override;

    // ï¿½ï¿½ï¿½ï¿½ï¿½Ô¾ï¿½ï¿½Pad -> Start -> Apex(Ñ­ï¿½ï¿½)
    void startJumpAnim();

    // ï¿½ï¿½Ø£ï¿½Í£Ö¹Apex -> Land -> Recovery -> ï¿½Øµï¿½Idle/Move
    void onJumpLanded();

    enum class MoveDir { None, Fwd, Bwd, Left, Right };

    // x: ÓÒÎª+£¬y: Ç°Îª+£¨ÄãÒ²¿ÉÒÔÓÃ z£¬ÕâÀïÓÃ Vec2 ¸üÖ±¹Û£©
    void setMoveAxis(const cocos2d::Vec2& axis);
    void updateLocomotionAnim(bool running);

    // ¸øµĞÈË/AI ÓÃ£º·µ»ØÎò¿Õ¡°ÊÀ½ç×ø±êÏµ¡±µÄÎ»ÖÃ£¨ÍÆ¼öÓÃÕâ¸ö×ö¾àÀë/×·»÷ÅĞ¶Ï£©
    cocos2d::Vec3 getWorldPosition3D() const;
private:
    cocos2d::Sprite3D* _model; ///< ï¿½ï¿½É«Ä£ï¿½Í£ï¿½ï¿½ï¿½Îªï¿½Õ£ï¿½
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
    MoveDir _runDir = MoveDir::None;        // µ±Ç°±¼ÅÜ·½Ïò£¨·ÀÖ¹Ã¿Ö¡ÖØ¸´ÇĞ£©
    std::string _curAnimKey;                // µ±Ç°¶¯»­ key£¨·ÀÖ¹ÖØ¸´²¥·Å£©

};

#endif // WUKONG_H
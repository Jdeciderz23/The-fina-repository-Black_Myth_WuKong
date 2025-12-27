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
 * @brief ��ս�ɫ�ࣨCharacter �����ࣩ��ʵ�ֶ���������ģ�͹��أ���̬ʵ�֣�
 */
class Wukong : public Character {
public:
    /**
     * @brief �������ʵ����cocos2d ����������
     * @return Wukong* �����ɹ����ض���ָ�룬ʧ�ܷ��� nullptr
     */
    static Wukong* create();

    /**
     * @brief ��ʼ��
     * @return bool �Ƿ��ʼ���ɹ�?
     */
    virtual bool init() override;

    /**
     * @brief ���Ŷ�����������ʵ�֣�
     * @param name ������
     * @param loop �Ƿ�ѭ��
     */
    virtual void playAnim(const std::string& name, bool loop) override;

    // �����Ծ��Pad -> Start -> Apex(ѭ��)
    void startJumpAnim();

    // ��أ�ֹͣApex -> Land -> Recovery -> �ص�Idle/Move
    void onJumpLanded();

    enum class MoveDir { None, Fwd, Bwd, Left, Right };

    // x: ��Ϊ+��y: ǰΪ+����Ҳ������ z�������� Vec2 ��ֱ�ۣ�
    void setMoveAxis(const cocos2d::Vec2& axis);
    void updateLocomotionAnim(bool running);

private:
    cocos2d::Sprite3D* _model; ///< ��ɫģ�ͣ���Ϊ�գ�
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
    MoveDir _runDir = MoveDir::None;        // ��ǰ���ܷ��򣨷�ֹÿ֡�ظ��У�
    std::string _curAnimKey;                // ��ǰ���� key����ֹ�ظ����ţ�

};

#endif // WUKONG_H
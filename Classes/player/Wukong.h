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
     * @return bool �Ƿ��ʼ���ɹ�
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
 

private:
    cocos2d::Sprite3D* _model; ///< ��ɫģ�ͣ���Ϊ�գ�
    std::string _curAnim;
    int _animTag = 1001;
    std::unordered_map<std::string, cocos2d::Animation3D*> _anims;
    cocos2d::Action* _curAnimAction = nullptr;
    cocos2d::Animate3D* makeAnimate(const std::string& key) const;

    void loadAnimIfNeeded(const std::string& key,
        const std::string& c3bPath);
};

#endif // WUKONG_H

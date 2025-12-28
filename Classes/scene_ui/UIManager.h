// Copyright 2025 The Black Myth Wukong Authors. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __UI_MANAGER_H__
#define __UI_MANAGER_H__

#pragma execution_character_set("utf-8")

#include "cocos2d.h"
#include "ui/CocosGUI.h"

// UIManager 处理游戏的 UI，包括菜单、HUD 和通知。
// 它被实现为单例，方便全局访问。
class UIManager {
 public:
  // 获取 UIManager 的单例实例。
  static UIManager* getInstance();

  // 销毁 UIManager 的单例实例。
  static void destroyInstance();

  // 创建开始菜单场景。
  cocos2d::Scene* createStartMenuScene();

  // 显示暂停菜单。
  void showPauseMenu();

  // 显示死亡菜单。
  void showDeathMenu();

  // 在指定的父节点上显示 HUD（抬头显示）。
  void showHUD(cocos2d::Node* parent);

  // 更新玩家的血条。
  // |percent| 是生命值百分比（0.0 到 1.0）。
  void updatePlayerHP(float percent);

  // 更新 Boss 的血条。
  // |percent| 是生命值百分比（0.0 到 1.0）。
  void updateBossHP(float percent);

  // 显示或隐藏 Boss 血条。
  void showBossHPBar(bool show);

  // 在击败 Boss 时显示胜利 UI。
  void showVictoryUI();

  // 在屏幕上显示临时的通知消息。
  void showNotification(const std::string& text,
                        const cocos2d::Color3B& color = cocos2d::Color3B::WHITE);

 private:
  UIManager();
  ~UIManager();

  // 按钮回调。
  void onStartGame(cocos2d::Ref* sender);
  void onSettings(cocos2d::Ref* sender);
  void onExitGame(cocos2d::Ref* sender);
  void onPauseResume(cocos2d::Ref* sender);
  void onPauseReturnTitle(cocos2d::Ref* sender);
  void onPauseHeal(cocos2d::Ref* sender);
  void onPauseTeleport(cocos2d::Ref* sender);
  void onDeathRespawn(cocos2d::Ref* sender);
  void onDeathReturnTitle(cocos2d::Ref* sender);
  void onCloseSettings(cocos2d::Ref* sender);
  void onVolumeSliderChanged(cocos2d::Ref* sender,
                             cocos2d::ui::Slider::EventType type);

  // UI 辅助方法。
  void showSettingsMenu();

  static UIManager* _instance;

  cocos2d::DrawNode* _hpBarDrawNode = nullptr;
  cocos2d::Label* _hpLabel = nullptr;
  float _hpBarWidth = 400.0f;
  float _hpBarHeight = 20.0f;

  cocos2d::DrawNode* _bossHpBarDrawNode = nullptr;
  cocos2d::Label* _bossNameLabel = nullptr;
  float _bossHpBarWidth = 800.0f;
  float _bossHpBarHeight = 15.0f;
};

#endif  // __UI_MANAGER_H__

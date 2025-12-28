// Copyright 2025 The Black Myth Wukong Authors. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __UI_MANAGER_H__
#define __UI_MANAGER_H__

#pragma execution_character_set("utf-8")

#include "cocos2d.h"
#include "ui/CocosGUI.h"

// UIManager handles the game's UI, including menus, HUD, and notifications.
// It is implemented as a singleton for easy global access.
class UIManager {
 public:
  // Returns the singleton instance of UIManager.
  static UIManager* getInstance();

  // Destroys the singleton instance of UIManager.
  static void destroyInstance();

  // Creates the start menu scene.
  cocos2d::Scene* createStartMenuScene();

  // Displays the pause menu.
  void showPauseMenu();

  // Displays the death menu.
  void showDeathMenu();

  // Shows the HUD (Heads-Up Display) on the specified parent node.
  void showHUD(cocos2d::Node* parent);

  // Updates the player's health bar.
  // |percent| is the health percentage (0.0 to 1.0).
  void updatePlayerHP(float percent);

  // Updates the Boss's health bar.
  // |percent| is the health percentage (0.0 to 1.0).
  void updateBossHP(float percent);

  // Shows or hides the Boss health bar.
  void showBossHPBar(bool show);

  // Displays the victory UI when the Boss is defeated.
  void showVictoryUI();

  // Shows a temporary notification message on the screen.
  void showNotification(const std::string& text,
                        const cocos2d::Color3B& color = cocos2d::Color3B::WHITE);

 private:
  UIManager();
  ~UIManager();

  // Button callbacks.
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

  // UI helper methods.
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

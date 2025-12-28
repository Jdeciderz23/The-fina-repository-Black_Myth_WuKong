// Copyright 2025 The Black Myth Wukong Authors. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma execution_character_set("utf-8")

#include "UIManager.h"

#include <algorithm>
#include <cstdio>

#include "AudioManager.h"
#include "GameApp.h"
#include "combat/HealthComponent.h"
#include "core/AreaManager.h"
#include "player/Wukong.h"
#include "scene_ui/BaseScene.h"

USING_NS_CC;
using namespace cocos2d::ui;

UIManager* UIManager::_instance = nullptr;

UIManager* UIManager::getInstance() {
  if (_instance == nullptr) {
    _instance = new (std::nothrow) UIManager();
  }
  return _instance;
}

void UIManager::destroyInstance() { CC_SAFE_DELETE(_instance); }

UIManager::UIManager() {}

UIManager::~UIManager() {}

Scene* UIManager::createStartMenuScene() {
  auto scene = Scene::create();
  auto layer = Layer::create();
  scene->addChild(layer);

  // 播放菜单背景音乐。
  AudioManager::getInstance()->playBGM("Audio/menu_bgm.mp3");

  auto visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 背景图片。
  auto background = Sprite::create("StartMenu.png");
  if (background) {
    background->setPosition(
        Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
    float scaleX = visibleSize.width / background->getContentSize().width;
    float scaleY = visibleSize.height / background->getContentSize().height;
    float scale = std::max(scaleX, scaleY);
    background->setScale(scale);
    layer->addChild(background, -1);
  } else {
    CCLOG("错误：Resources 中未找到 StartMenu.png！");
    auto bgLayer = LayerColor::create(Color4B(20, 20, 20, 255));
    layer->addChild(bgLayer, -2);
  }

  // 标题。
  auto titleLabel = Label::createWithSystemFont("黑神话：悟空", "Arial", 80);
  if (titleLabel) {
    titleLabel->setPosition(Vec2(visibleSize.width / 2 + origin.x,
                                 visibleSize.height * 0.8 + origin.y));
    titleLabel->enableShadow();
    layer->addChild(titleLabel, 1);
  }

  // 菜单按钮。
  MenuItemFont::setFontName("Arial");
  MenuItemFont::setFontSize(50);

  auto startItem = MenuItemFont::create(
      "开始游戏", CC_CALLBACK_1(UIManager::onStartGame, this));
  startItem->setColor(Color3B::YELLOW);

  auto settingsItem = MenuItemFont::create(
      "设置", CC_CALLBACK_1(UIManager::onSettings, this));
  settingsItem->setColor(Color3B::WHITE);

  auto exitItem = MenuItemFont::create(
      "退出游戏", CC_CALLBACK_1(UIManager::onExitGame, this));
  exitItem->setColor(Color3B(255, 100, 100));

  auto menu = Menu::create(startItem, settingsItem, exitItem, nullptr);
  menu->setPosition(
      Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
  menu->alignItemsVerticallyWithPadding(50);
  layer->addChild(menu, 1);

  return scene;
}

void UIManager::showHUD(Node* parent) {
  if (!parent) return;

  auto vs = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  // 1. 血条背景。
  auto bg = DrawNode::create();
  bg->drawSolidRect(Vec2(-_hpBarWidth / 2 - 2, -_hpBarHeight / 2 - 2),
                    Vec2(_hpBarWidth / 2 + 2, _hpBarHeight / 2 + 2),
                    Color4F(0, 0, 0, 0.5f));
  bg->setPosition(Vec2(vs.width / 2 + origin.x, 50 + origin.y));
  parent->addChild(bg, 999);

  // 2. 血条 DrawNode。
  _hpBarDrawNode = DrawNode::create();
  _hpBarDrawNode->setPosition(Vec2(vs.width / 2 + origin.x, 50 + origin.y));
  parent->addChild(_hpBarDrawNode, 1000);

  // 3. 生命值文本。
  _hpLabel = Label::createWithSystemFont("100 / 100", "Arial", 16);
  _hpLabel->setPosition(Vec2(vs.width / 2 + origin.x, 50 + origin.y));
  _hpLabel->setTextColor(Color4B::WHITE);
  parent->addChild(_hpLabel, 1001);

  updatePlayerHP(1.0f);

  // 4. Boss 血条（顶部）。
  _bossHpBarDrawNode = DrawNode::create();
  _bossHpBarDrawNode->setPosition(
      Vec2(vs.width / 2 + origin.x, vs.height - 60 + origin.y));
  _bossHpBarDrawNode->setVisible(false);
  parent->addChild(_bossHpBarDrawNode, 1000);

  _bossNameLabel = Label::createWithSystemFont("BOSS", "Arial", 24);
  _bossNameLabel->setPosition(
      Vec2(vs.width / 2 + origin.x, vs.height - 35 + origin.y));
  _bossNameLabel->setTextColor(Color4B::YELLOW);
  _bossNameLabel->setVisible(false);
  parent->addChild(_bossNameLabel, 1001);
}

void UIManager::updatePlayerHP(float percent) {
  if (!_hpBarDrawNode) return;

  _hpBarDrawNode->clear();
  percent = std::max(0.0f, std::min(1.0f, percent));
  float currentWidth = _hpBarWidth * percent;

  _hpBarDrawNode->drawSolidRect(
      Vec2(-_hpBarWidth / 2, -_hpBarHeight / 2),
      Vec2(-_hpBarWidth / 2 + currentWidth, _hpBarHeight / 2), Color4F::RED);

  if (_hpLabel) {
    char buf[32];
    sprintf(buf, "%d / 100", (int)(percent * 100));
    _hpLabel->setString(buf);
  }
}

void UIManager::updateBossHP(float percent) {
  if (!_bossHpBarDrawNode) return;

  _bossHpBarDrawNode->clear();
  percent = std::max(0.0f, std::min(1.0f, percent));

  _bossHpBarDrawNode->drawSolidRect(
      Vec2(-_bossHpBarWidth / 2 - 2, -_bossHpBarHeight / 2 - 2),
      Vec2(_bossHpBarWidth / 2 + 2, _bossHpBarHeight / 2 + 2),
      Color4F(0, 0, 0, 0.6f));

  _bossHpBarDrawNode->drawSolidRect(
      Vec2(-_bossHpBarWidth / 2, -_bossHpBarHeight / 2),
      Vec2(_bossHpBarWidth / 2, _bossHpBarHeight / 2), Color4F(0.3f, 0, 0, 1.0f));

  float currentWidth = _bossHpBarWidth * percent;
  _bossHpBarDrawNode->drawSolidRect(
      Vec2(-_bossHpBarWidth / 2, -_bossHpBarHeight / 2),
      Vec2(-_bossHpBarWidth / 2 + currentWidth, _bossHpBarHeight / 2),
      Color4F(1.0f, 0.7f, 0.0f, 1.0f));

  if (percent > 0 && percent < 1.0f) {
    showBossHPBar(true);
  }
}

void UIManager::showBossHPBar(bool show) {
  if (_bossHpBarDrawNode) _bossHpBarDrawNode->setVisible(show);
  if (_bossNameLabel) _bossNameLabel->setVisible(show);
}

void UIManager::showNotification(const std::string& text,
                                 const cocos2d::Color3B& color) {
  auto running = Director::getInstance()->getRunningScene();
  if (!running) return;

  auto vs = Director::getInstance()->getVisibleSize();
  auto origin = Director::getInstance()->getVisibleOrigin();

  auto label = Label::createWithSystemFont(text, "Arial", 36);
  if (label) {
    label->setColor(color);
    label->setPosition(
        Vec2(vs.width / 2 + origin.x, vs.height * 0.7f + origin.y));
    running->addChild(label, 10000);

    label->setOpacity(0);
    auto fadeIn = FadeIn::create(0.2f);
    auto delay = DelayTime::create(1.5f);
    auto fadeOut = FadeOut::create(0.5f);
    auto remove = RemoveSelf::create();
    label->runAction(Sequence::create(fadeIn, delay, fadeOut, remove, nullptr));
  }
}

void UIManager::showDeathMenu() {
  auto running = Director::getInstance()->getRunningScene();
  if (!running || running->getChildByName("DeathMenuLayer")) return;

  auto layer = Layer::create();
  layer->setName("DeathMenuLayer");
  running->addChild(layer, 10000);

  auto vs = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto bg = Sprite::create("death.png");
  if (bg) {
    bg->setPosition(Vec2(vs.width / 2 + origin.x, vs.height / 2 + origin.y));
    float sx = vs.width / bg->getContentSize().width;
    float sy = vs.height / bg->getContentSize().height;
    bg->setScale(std::max(sx, sy));
    layer->addChild(bg, -1);
  } else {
    auto colorBg = LayerColor::create(Color4B(0, 0, 0, 180));
    layer->addChild(colorBg, -1);
  }

  MenuItemFont::setFontName("Arial");
  MenuItemFont::setFontSize(40);

  auto respawnItem = MenuItemFont::create(
      "重新开始", CC_CALLBACK_1(UIManager::onDeathRespawn, this));
  respawnItem->setColor(Color3B(100, 255, 100));

  auto titleItem = MenuItemFont::create(
      "返回菜单", CC_CALLBACK_1(UIManager::onDeathReturnTitle, this));
  titleItem->setColor(Color3B(100, 100, 255));

  auto menu = Menu::create(respawnItem, titleItem, nullptr);
  menu->alignItemsVerticallyWithPadding(40);
  menu->setPosition(
      Vec2(vs.width / 2 + origin.x, vs.height / 2 + origin.y - 50));
  layer->addChild(menu, 1);

  Director::getInstance()->pause();
}

void UIManager::onDeathRespawn(Ref* sender) {
  Director::getInstance()->resume();
  auto running = Director::getInstance()->getRunningScene();
  if (running) {
    running->removeChildByName("DeathMenuLayer");
  }
  auto baseScene = dynamic_cast<BaseScene*>(running);
  if (baseScene) {
    baseScene->teleportPlayerToCenter();
  }
}

void UIManager::onDeathReturnTitle(Ref* sender) {
  Director::getInstance()->resume();
  auto sceneMgr = GameApp::getInstance()->getSceneManager();
  if (sceneMgr) {
    sceneMgr->switchScene(SceneManager::SceneType::TITLE, true);
  }
}

void UIManager::showPauseMenu() {
  auto running = Director::getInstance()->getRunningScene();
  if (!running) return;

  GameApp::getInstance()->pause();

  auto layer = Layer::create();
  layer->setName("PauseMenuLayer");
  running->addChild(layer, 9999);

  auto vs = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto bg = Sprite::create("pause.png");
  if (bg) {
    bg->setPosition(Vec2(vs.width / 2 + origin.x, vs.height / 2 + origin.y));
    float sx = vs.width / bg->getContentSize().width;
    float sy = vs.height / bg->getContentSize().height;
    bg->setScale(std::max(sx, sy));
    layer->addChild(bg, -1);
  }

  MenuItemFont::setFontName("Arial");
  MenuItemFont::setFontSize(32);

  auto healItem =
      MenuItemFont::create("治疗", CC_CALLBACK_1(UIManager::onPauseHeal, this));
  auto teleportItem = MenuItemFont::create(
      "传送", CC_CALLBACK_1(UIManager::onPauseTeleport, this));
  auto resumeItem = MenuItemFont::create(
      "继续", CC_CALLBACK_1(UIManager::onPauseResume, this));
  auto titleItem = MenuItemFont::create(
      "返回菜单", CC_CALLBACK_1(UIManager::onPauseReturnTitle, this));

  auto menu =
      Menu::create(healItem, teleportItem, resumeItem, titleItem, nullptr);
  menu->alignItemsVerticallyWithPadding(30);
  menu->setPosition(
      Vec2(vs.width / 2 + origin.x, vs.height / 2 + origin.y));
  layer->addChild(menu, 1);
}

void UIManager::onStartGame(Ref* sender) {
  auto sceneMgr = GameApp::getInstance()->getSceneManager();
  if (sceneMgr) {
    sceneMgr->switchScene(SceneManager::SceneType::GAMEPLAY, true);
  }
}

void UIManager::onSettings(Ref* sender) { showSettingsMenu(); }

void UIManager::showSettingsMenu() {
  auto running = Director::getInstance()->getRunningScene();
  if (!running) return;

  auto layer = Layer::create();
  layer->setName("SettingsMenuLayer");
  running->addChild(layer, 10001);

  auto vs = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();

  auto background = Sprite::create("StartMenu.png");
  if (background) {
    background->setPosition(
        Vec2(vs.width / 2 + origin.x, vs.height / 2 + origin.y));
    float sx = vs.width / background->getContentSize().width;
    float sy = vs.height / background->getContentSize().height;
    background->setScale(std::max(sx, sy));
    layer->addChild(background, -1);
  }

  auto title = Label::createWithSystemFont("设置", "Arial", 60);
  title->setPosition(Vec2(vs.width / 2 + origin.x, vs.height * 0.75f + origin.y));
  layer->addChild(title, 1);

  static float currentVolume = 1.0f;
  auto volumeLabel = Label::createWithSystemFont(
      "音量: " + std::to_string((int)std::round(currentVolume * 100)) + "%",
      "Arial", 40);
  volumeLabel->setPosition(
      Vec2(vs.width / 2 + origin.x, vs.height * 0.55f + origin.y));
  volumeLabel->setName("VolumeLabel");
  layer->addChild(volumeLabel, 1);

  auto volumeDown = MenuItemFont::create(" - ", [volumeLabel](Ref* sender) {
    currentVolume = std::max(0.0f, currentVolume - 0.1f);
    currentVolume = std::round(currentVolume * 10.0f) / 10.0f;
    volumeLabel->setString(
        "音量: " + std::to_string((int)std::round(currentVolume * 100)) + "%");
    AudioManager::getInstance()->setBGMVolume(currentVolume);
    AudioManager::getInstance()->setEffectVolume(currentVolume);
  });

  auto volumeUp = MenuItemFont::create(" + ", [volumeLabel](Ref* sender) {
    currentVolume = std::min(1.0f, currentVolume + 0.1f);
    currentVolume = std::round(currentVolume * 10.0f) / 10.0f;
    volumeLabel->setString(
        "音量: " + std::to_string((int)std::round(currentVolume * 100)) + "%");
    AudioManager::getInstance()->setBGMVolume(currentVolume);
    AudioManager::getInstance()->setEffectVolume(currentVolume);
  });

  auto closeItem = MenuItemFont::create(
      "返回", CC_CALLBACK_1(UIManager::onCloseSettings, this));
  closeItem->setColor(Color3B(100, 255, 100));

  auto menu = Menu::create(volumeDown, volumeUp, closeItem, nullptr);
  menu->alignItemsHorizontallyWithPadding(40);
  menu->setPosition(Vec2(vs.width / 2 + origin.x, vs.height * 0.4f + origin.y));
  layer->addChild(menu, 1);

  volumeDown->setPosition(Vec2(-100, 50));
  volumeUp->setPosition(Vec2(100, 50));
  closeItem->setPosition(Vec2(0, -100));
}

void UIManager::onCloseSettings(Ref* sender) {
  auto running = Director::getInstance()->getRunningScene();
  if (running) {
    auto layer = running->getChildByName("SettingsMenuLayer");
    if (layer) running->removeChild(layer);
  }
}

void UIManager::onVolumeSliderChanged(Ref* sender,
                                      cocos2d::ui::Slider::EventType type) {}

void UIManager::onExitGame(Ref* sender) {
  Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
  exit(0);
#endif
}

void UIManager::onPauseHeal(Ref* sender) {
  auto scene = Director::getInstance()->getRunningScene();
  auto baseScene = dynamic_cast<BaseScene*>(scene);
  if (!baseScene) return;

  auto player = baseScene->getChildByName<Wukong*>("Wukong");
  if (!player) {
    for (auto& child : baseScene->getChildren()) {
      player = dynamic_cast<Wukong*>(child);
      if (player) break;
    }
  }

  if (player) {
    Vec3 pos = player->getPosition3D();
    if (AreaManager::getInstance()->canHeal(pos)) {
      auto health = player->getComponent("HealthComponent");
      auto healthComp = dynamic_cast<HealthComponent*>(health);
      if (healthComp) {
        healthComp->fullHeal();
        CCLOG("UIManager: 在传送点恢复了生命值。");
        showNotification("生命值已恢复", Color3B::GREEN);
      }
    } else {
      showNotification("只有在传送点才能休息", Color3B::RED);
    }
  }
}

void UIManager::onPauseTeleport(Ref* sender) {
  auto scene = Director::getInstance()->getRunningScene();
  auto baseScene = dynamic_cast<BaseScene*>(scene);
  if (baseScene) {
    baseScene->teleportPlayerToCenter();
    onPauseResume(nullptr);
  }
}

void UIManager::onPauseResume(Ref* sender) {
  GameApp::getInstance()->resume();
  auto running = Director::getInstance()->getRunningScene();
  if (running) {
    running->removeChildByName("PauseMenuLayer");
  }
}

void UIManager::onPauseReturnTitle(Ref* sender) {
  GameApp::getInstance()->resume();
  auto sceneMgr = GameApp::getInstance()->getSceneManager();
  if (sceneMgr) {
    sceneMgr->switchScene(SceneManager::SceneType::TITLE, true);
  }
}

void UIManager::showVictoryUI() {
  auto running = Director::getInstance()->getRunningScene();
  if (!running) return;

  auto vs = Director::getInstance()->getVisibleSize();
  auto origin = Director::getInstance()->getVisibleOrigin();

  auto label = Label::createWithSystemFont("胜利！", "Arial", 120);
  if (label) {
    label->setColor(Color3B(255, 215, 0));  // 金色。
    label->setPosition(Vec2(vs.width / 2 + origin.x, vs.height / 2 + origin.y));
    label->enableOutline(Color4B::BLACK, 4);
    running->addChild(label, 10000);

    label->setScale(0.1f);
    label->setOpacity(0);
    auto fadeIn = FadeIn::create(0.5f);
    auto scaleTo = ScaleTo::create(0.5f, 1.0f);
    auto spawn = Spawn::create(fadeIn, scaleTo, nullptr);
    label->runAction(spawn);
  }
}

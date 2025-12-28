// Copyright 2025 The Black Myth Wukong Authors. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma execution_character_set("utf-8")

#include "BaseScene.h"

#include <algorithm>

#include "3d/CCSprite3D.h"
#include "3d/CCTerrain.h"
#include "AudioManager.h"
#include "Boss.h"
#include "BossAI.h"
#include "Enemy.h"
#include "GameApp.h"
#include "HealthComponent.h"
#include "InputController.h"
#include "SceneManager.h"
#include "UIManager.h"
#include "Wukong.h"
#include "core/AreaManager.h"
#include "renderer/CCTexture2D.h"

USING_NS_CC;

// 摄像机投影参数。
static float s_fov = 30.0f;
static float s_aspect = 1.0f;
static float s_nearPlane = 1.0f;
static float s_farPlane = 2000.0f;

Scene* BaseScene::createScene() { return BaseScene::create(); }

bool BaseScene::init() {
  if (!Scene::init()) return false;

  initCamera();
  initSkybox();
  initLights();
  initInput();
  
  // 基础场景不再在这里初始化游戏对象，因为它们依赖于地形碰撞器。
  // 子类（如 CampScene）在加载完地形后应显式调用 initGameObjects()。

  // 初始化传送点标记。
  auto points = AreaManager::getInstance()->getTeleportPoints();
  for (const auto& pt : points) {
    auto marker = Sprite3D::create("WuKong/wukong.c3b");
    if (marker) {
      marker->setPosition3D(pt.position);
      marker->setScale(0.5f);
      marker->setColor(Color3B(255, 215, 0));  // 金色。
      marker->setCameraMask((unsigned short)CameraFlag::USER1);
      this->addChild(marker);

      // 为标记添加简单的旋转动画。
      marker->runAction(
          RepeatForever::create(RotateBy::create(2.0f, Vec3(0, 180, 0))));
    }
  }

  // 播放背景音乐。
  AudioManager::getInstance()->playBGM("Audio/game_bgm1.mp3");

  this->scheduleUpdate();

  // 在 HUD 中设置暂停按钮。
  auto vs = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  auto label = Label::createWithSystemFont("暂停", "Arial", 24);
  auto item = MenuItemLabel::create(label, [](Ref*) {
    UIManager::getInstance()->showPauseMenu();
  });
  auto menu = Menu::create(item, nullptr);
  menu->setPosition(origin + Vec2(30, vs.height - 30));
  menu->setCameraMask((unsigned short)CameraFlag::DEFAULT);
  addChild(menu, 1000);

  return true;
}

void BaseScene::initGameObjects() {
  initPlayer();
  initEnemy();
  initBoss();

  // 初始化 HUD。
  UIManager::getInstance()->showHUD(this);
}

/* ==================== 天空盒 ==================== */

void BaseScene::initSkybox() {
  std::array<std::string, 6> faces;
  if (!chooseSkyboxFaces(faces) || !verifyCubeFacesSquare(faces)) {
    CCLOG("天空盒无效，回退到颜色刷。");
    auto brush = CameraBackgroundBrush::createColorBrush(
        Color4F(0.08f, 0.09f, 0.11f, 1.0f), 1.0f);
    _mainCamera->setBackgroundBrush(brush);
    return;
  }

  _skybox = Skybox::create(faces[0], faces[1], faces[2], faces[3], faces[4],
                           faces[5]);
  _skybox->setCameraMask((unsigned short)CameraFlag::USER1);
  _skybox->setRotation3D(Vec3::ZERO);
  addChild(_skybox, -100);
}

// 选择天空盒贴图。如果所有贴图都存在，返回 true。
bool BaseScene::chooseSkyboxFaces(std::array<std::string, 6>& outFaces) {
  auto fu = FileUtils::getInstance();

  std::array<std::string, 6> set1 = {
      "SkyBox/Skybox_right.png", "SkyBox/Skybox_left.png",
      "SkyBox/Skybox_top.png",   "SkyBox/Skybox_bottom.png",
      "SkyBox/Skybox_front.png", "SkyBox/Skybox_back.png"};

  // 辅助 lambda 函数检查文件是否存在。
  auto existsAll = [&](const std::array<std::string, 6>& s) -> bool {
    for (const auto& f : s) {
      if (!fu->isFileExist(f)) return false;
    }
    return true;
  };

  if (existsAll(set1)) {
    outFaces = set1;
    return true;
  }
  return false;
}

// 验证所有立方体贴图面是否为正方形且尺寸一致。
bool BaseScene::verifyCubeFacesSquare(const std::array<std::string, 6>& faces) {
  int faceSize = -1;
  for (int i = 0; i < 6; ++i) {
    std::string full = FileUtils::getInstance()->fullPathForFilename(faces[i]);
    if (full.empty()) return false;

    auto img = new (std::nothrow) Image();
    if (!img || !img->initWithImageFile(full)) {
      CC_SAFE_DELETE(img);
      return false;
    }

    // 检查图像是否为正方形。
    if (img->getWidth() != img->getHeight()) {
      CC_SAFE_DELETE(img);
      return false;
    }

    // 检查所有面是否大小相同。
    if (faceSize < 0)
      faceSize = img->getWidth();
    else if (faceSize != img->getWidth()) {
      CC_SAFE_DELETE(img);
      return false;
    }

    CC_SAFE_DELETE(img);
  }
  return true;
}

/* ==================== 摄像机 ==================== */

void BaseScene::initCamera() {
  auto vs = Director::getInstance()->getVisibleSize();
  s_aspect = vs.width / std::max(1.0f, vs.height);
  s_fov = 30.0f;
  s_nearPlane = 0.1f;
  s_farPlane = 10000.0f;

  auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
  _mainCamera = Camera::createPerspective(
      60.0f, visibleSize.width / visibleSize.height, 1.0f, 2000.0f);
  _mainCamera->setCameraFlag(CameraFlag::USER1);

  _mainCamera->setPosition3D(cocos2d::Vec3(0.0f, 140.0f, 260.0f));
  _mainCamera->lookAt(cocos2d::Vec3(0.0f, 90.0f, 0.0f), cocos2d::Vec3::UNIT_Y);

  addChild(_mainCamera);
}

/* ==================== 光照 ==================== */

void BaseScene::initLights() {
  auto ambient = AmbientLight::create(Color3B(180, 180, 180));
  ambient->setIntensity(0.6f);
  addChild(ambient);

  auto dirLight =
      DirectionLight::create(Vec3(-0.7f, -1.0f, -0.3f), Color3B::WHITE);
  dirLight->setIntensity(1.0f);
  dirLight->setCameraMask((unsigned short)CameraFlag::USER1);
  addChild(dirLight);
}

/* ==================== 输入 ==================== */

void BaseScene::initInput() {
  // 输入初始化逻辑（例如键盘/鼠标监听器）将放在这里。
  // 目前使用 scheduleUpdate 来轮询输入。
  scheduleUpdate();
}

/* ==================== 更新 ==================== */

void BaseScene::update(float dt) {
  // 更新 HUD 和游戏状态。
  if (_player) {
    // 检查玩家是否掉出世界。
    if (_player->getPositionY() < -500.0f && !_player->isDead()) {
      _player->die();
    }

    float hp = (float)_player->getHP();
    float maxHp = (float)_player->getMaxHP();
    UIManager::getInstance()->updatePlayerHP(hp / maxHp);
  }

  // 更新天空盒位置以跟随摄像机。
  if (_skybox && _mainCamera) {
    _skybox->setPosition3D(_mainCamera->getPosition3D());
    _skybox->setRotation3D(cocos2d::Vec3::ZERO);
  }
}

static float moveTowardAngleDeg(float cur, float target, float maxDeltaDeg) {
  float delta = std::fmod(target - cur + 540.0f, 360.0f) - 180.0f;  // [-180,180]
  if (delta > maxDeltaDeg) delta = maxDeltaDeg;
  if (delta < -maxDeltaDeg) delta = -maxDeltaDeg;
  return cur + delta;
}

void BaseScene::updateCamera(float dt) {
  if (!_mainCamera || !_player) return;

  // 自动跟随偏航角逻辑。
  if (_autoFollowYaw && _mouseIdleTime > 0.12f) {
    auto intent = _player->getMoveIntent();
    cocos2d::Vec3 d = intent.dirWS;
    d.y = 0.0f;
    if (d.lengthSquared() > 1e-6f) {
      d.normalize();
      float desiredYaw = CC_RADIANS_TO_DEGREES(std::atan2f(d.z, d.x));
      _yaw = moveTowardAngleDeg(_yaw, desiredYaw, _autoYawSpeed * dt);
    }
  }

  cocos2d::Vec3 playerPos = _player->getPosition3D();
  cocos2d::Vec3 target = playerPos + cocos2d::Vec3(0.0f, _followHeight, 0.0f);

  float yawRad = CC_DEGREES_TO_RADIANS(_yaw);
  float pitchRad = CC_DEGREES_TO_RADIANS(_pitch);

  cocos2d::Vec3 front(cosf(yawRad) * cosf(pitchRad), sinf(pitchRad),
                      sinf(yawRad) * cosf(pitchRad));
  front.normalize();

  cocos2d::Vec3 desiredPos = target - front * _followDistance;

  // 平滑插值摄像机位置。
  float t = 1.0f - expf(-_followSmooth * dt);
  _camPos = _camPos.lerp(desiredPos, t);

  _mainCamera->setPosition3D(_camPos);
  _mainCamera->lookAt(target, cocos2d::Vec3::UNIT_Y);

  if (_skybox) {
    _skybox->setPosition3D(_camPos);
    _skybox->setRotation3D(cocos2d::Vec3::ZERO);
  }
}

void BaseScene::teleportPlayerToCenter() {
  if (_player) {
    cocos2d::Vec3 teleportPos(0, 0, -960);
    if (_terrainCollider) {
        CustomRay ray(teleportPos + cocos2d::Vec3(0, 500, 0), cocos2d::Vec3(0, -1, 0));
        float hitDist;
        if (_terrainCollider->rayIntersects(ray, hitDist)) {
            teleportPos.y = ray.origin.y - hitDist;
        }
    }
    _player->setPosition3D(teleportPos);  // 回到传送点 2。
    _player->respawn();
  }

  // 重置所有敌人。
  for (auto enemy : _enemies) {
    if (enemy) {
      enemy->resetEnemy();
    }
  }

  CCLOG("BaseScene: 玩家已重生，所有敌人已重置。");
}

/* ==================== 调试 ==================== */

Scene* CampScene::createScene() { return CampScene::create(); }

bool CampScene::init() {
  if (!BaseScene::init()) return false;

  // 加载地形模型。
  auto terrain = Sprite3D::create("scene/terrain.obj");
  if (terrain) {
    terrain->setPosition3D(Vec3(0, 0, 0));
    terrain->setScale(100.0f);
    terrain->setCameraMask((unsigned short)CameraFlag::USER1);
    addChild(terrain);

    // 初始化地形碰撞器。
    _terrainCollider = TerrainCollider::create(terrain, "scene/terrain.obj");
    if (_terrainCollider) {
      _terrainCollider->retain();
      if (_player) {
        _player->setTerrainCollider(_terrainCollider);
      }
      for (auto enemy : _enemies) {
        enemy->setTerrainCollider(_terrainCollider);
      }

      // 初始化游戏对象（玩家、敌人、Boss）。
      initGameObjects();
    }
  }

  return true;
}

/* ---------- 玩家 ---------- */

void BaseScene::initPlayer() {
  _player = Wukong::create();
  if (!_player) {
    CCLOG("错误：悟空创建失败！");
    return;
  }

  // 在传送点 2 生成玩家。
  cocos2d::Vec3 playerSpawnPos(0.0f, 0.0f, -960.0f);
  if (_terrainCollider) {
      CustomRay ray(playerSpawnPos + cocos2d::Vec3(0, 500, 0), cocos2d::Vec3(0, -1, 0));
      float hitDist;
      if (_terrainCollider->rayIntersects(ray, hitDist)) {
          playerSpawnPos.y = ray.origin.y - hitDist;
          CCLOG("Player spawned at ground Y: %f (hitDist: %f)", playerSpawnPos.y, hitDist);
      } else {
          CCLOG("Warning: Player terrain raycast failed!");
      }
  }
  _player->setPosition3D(playerSpawnPos);
  _player->setRotation3D(cocos2d::Vec3::ZERO);

  if (_terrainCollider) {
    _player->setTerrainCollider(_terrainCollider);
  }

  addChild(_player, 10);

  // 初始化玩家控制器。
  auto controller = PlayerController::create(_player);
  if (controller) {
    controller->setCamera(_mainCamera);
    addChild(controller, 20);
  }
}

void BaseScene::initEnemy() {
  struct Spawn {
    const char* root;
    const char* model;
    cocos2d::Vec3 pos;
  };

  const Spawn spawns[] = {
      {"Enemy/enemy1", "enemy1.c3b", cocos2d::Vec3(400, 0, -400)},
      {"Enemy/enemy2", "enemy2.c3b", cocos2d::Vec3(450, 0, -420)},
      {"Enemy/enemy3", "enemy3.c3b", cocos2d::Vec3(380, 0, -450)},
  };

  for (auto& s : spawns) {
    auto e = Enemy::createWithResRoot(s.root, s.model);
    if (!e) continue;

    cocos2d::Vec3 spawnPos = s.pos;
    if (_terrainCollider) {
        CustomRay ray(spawnPos + cocos2d::Vec3(0, 500, 0), cocos2d::Vec3(0, -1, 0));
        float hitDist;
        if (_terrainCollider->rayIntersects(ray, hitDist)) {
            spawnPos.y = ray.origin.y - hitDist;
            CCLOG("Enemy spawned at ground Y: %f (hitDist: %f)", spawnPos.y, hitDist);
        } else {
            CCLOG("Warning: Enemy terrain raycast failed!");
        }
    }

    e->setPosition3D(spawnPos);
    e->setBirthPosition(e->getPosition3D());
    e->setTarget(_player);
    e->setTerrainCollider(_terrainCollider);

    // 设置小怪血量为 10。
    if (e->getHealth()) {
      e->getHealth()->setMaxHealth(10.0f);
    }

    this->addChild(e);
    _enemies.push_back(e);
  }

  if (_player) {
    _player->setEnemies(&_enemies);
  }

  // 敌人死亡监听器。
  auto enemyDeathListener = cocos2d::EventListenerCustom::create(
      "enemy_died", [this](cocos2d::EventCustom* event) {
        CCLOG("BaseScene: 触发敌人死亡事件");
        Enemy* deadEnemy = static_cast<Enemy*>(event->getUserData());
        if (deadEnemy) {
          CCLOG("BaseScene: 正在移除死亡敌人 %p", (void*)deadEnemy);
          this->removeDeadEnemy(deadEnemy);
        }
      });

  _eventDispatcher->addEventListenerWithFixedPriority(enemyDeathListener, 1);
}

void BaseScene::removeDeadEnemy(Enemy* deadEnemy) {
  if (!deadEnemy) {
    CCLOG("BaseScene::removeDeadEnemy: 无效的死亡敌人指针");
    return;
  }

  CCLOG("BaseScene::removeDeadEnemy: 正在移除敌人 %p", (void*)deadEnemy);

  // 从敌人向量中移除。
  auto it = std::find(_enemies.begin(), _enemies.end(), deadEnemy);
  if (it != _enemies.end()) {
    _enemies.erase(it);
    CCLOG("BaseScene::removeDeadEnemy: 敌人已移除，剩余 %zu 个",
          _enemies.size());
  } else {
    CCLOG("BaseScene::removeDeadEnemy: 列表中未找到该敌人");
  }
}

void BaseScene::initBoss() {
  // 初始化 Boss。
  auto boss = Boss::createBoss("Enemy/boss", "boss.c3b");
  if (!boss) {
    CCLOG("错误：Boss 创建失败！");
    return;
  }

  // 设置 Boss 在 (-200, 0, 600) 附近生成。
  boss->setPosition3D(cocos2d::Vec3(-200, 0, 600));
  boss->setBirthPosition(boss->getPosition3D());
  boss->setTarget(_player);

  if (_terrainCollider) {
    boss->setTerrainCollider(_terrainCollider);
  }

  // 设置 Boss AI。
  boss->setAI(new BossAI(boss));

  auto sprite = boss->getSprite();
  if (sprite) {
    sprite->setScale(0.5f);
    // 移除手动偏移，由 Enemy::updateSpritePosition 自动处理模型对齐
    boss->setSpriteOffsetY(0.0f);
    // 显式更新 AABB 修正，确保缩放后位置依然正确
    // setSpriteOffsetY 内部已经调用了 updateSpritePosition
  }

  // 尝试根据地形对齐初始高度
  if (_terrainCollider) {
      CustomRay ray(cocos2d::Vec3(-200, 500, 600), cocos2d::Vec3(0, -1, 0));
      float hitDist;
      if (_terrainCollider->rayIntersects(ray, hitDist)) {
          float groundY = ray.origin.y - hitDist;
          boss->setPosition3D(cocos2d::Vec3(-200, groundY, 600));
          boss->setBirthPosition(boss->getPosition3D());
          CCLOG("Boss spawned at ground Y: %f (hitDist: %f)", groundY, hitDist);
      } else {
          CCLOG("Warning: Boss terrain raycast failed!");
      }
  }

  this->addChild(boss);
  _enemies.push_back(boss);

  if (_player) {
    _player->setEnemies(&_enemies);
  }

  CCLOG("Boss 已初始化在: %f, %f, %f 带有 AI", boss->getPositionX(),
        boss->getPositionY(), boss->getPositionZ());

  // 添加 Boss 死亡事件监听器。
  auto bossDeathListener = cocos2d::EventListenerCustom::create(
      "enemy_died", [this](cocos2d::EventCustom* event) {
        CCLOG("BaseScene: 触发 Boss 死亡事件");
        Enemy* deadEnemy = static_cast<Enemy*>(event->getUserData());
        if (deadEnemy && deadEnemy->getEnemyType() == Enemy::EnemyType::BOSS) {
          CCLOG("BaseScene: 正在移除死亡 Boss %p", (void*)deadEnemy);
          this->removeDeadEnemy(deadEnemy);

          // 显示胜利界面。
          UIManager::getInstance()->showVictoryUI();
        }
      });

  _eventDispatcher->addEventListenerWithFixedPriority(bossDeathListener, 1);
}

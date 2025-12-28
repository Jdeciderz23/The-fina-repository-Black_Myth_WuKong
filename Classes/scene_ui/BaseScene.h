// Copyright 2025 The Black Myth Wukong Authors. All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __BASE_SCENE_H__
#define __BASE_SCENE_H__

#include <array>
#include <string>
#include <vector>

#include "../combat/Collider.h"
#include "Enemy.h"
#include "Wukong.h"
#include "cocos2d.h"

class Wukong;
class TerrainCollider;

// BaseScene 是所有 3D 游戏场景的基础类。
// 它处理摄像机、天空盒、光照、输入、玩家和敌人管理。
class BaseScene : public cocos2d::Scene {
 public:
  static cocos2d::Scene* createScene();
  virtual bool init() override;

  // 将玩家传送到重生点并重置敌人。
  void teleportPlayerToCenter();

  CREATE_FUNC(BaseScene);

 protected:
  // 初始化方法。
  void initCamera();
  void initSkybox();
  void initLights();
  void initInput();
  void initGameObjects(); // 初始化玩家、敌人等游戏对象
  void initEnemy();
  void initBoss();
  void initPlayer();

  // 更新循环。
  virtual void update(float dt) override;
  void updateCamera(float dt);

  // 天空盒辅助方法。
  bool chooseSkyboxFaces(std::array<std::string, 6>& outFaces);
  bool verifyCubeFacesSquare(const std::array<std::string, 6>& faces);

  // 敌人管理。
  void removeDeadEnemy(Enemy* deadEnemy);

 protected:
  // 摄像机相关成员。
  cocos2d::Camera* _mainCamera = nullptr;
  cocos2d::Skybox* _skybox = nullptr;
  bool _autoFollowYaw = true;
  float _autoYawSpeed = 240.0f;  // 度/秒。
  float _mouseIdleTime = 999.0f; // 自上次鼠标移动以来的时间。

  cocos2d::Vec3 _camPos = cocos2d::Vec3(0.0f, 120.0f, 220.0f);
  cocos2d::Vec3 _camFront = cocos2d::Vec3(0.0f, 0.0f, -1.0f);
  cocos2d::Vec3 _camUp = cocos2d::Vec3::UNIT_Y;

  float _yaw = -90.0f;
  float _pitch = -15.0f;

  float _moveSpeed = 200.0f;
  float _mouseSensitivity = 0.15f;

  // 透视投影参数。
  float _fov = 60.0f;         // 视野（度）。
  float _aspect = 1.0f;       // 宽高比。
  float _nearPlane = 1.0f;    // 近裁剪面。
  float _farPlane = 1000.0f;  // 远裁剪面。
  float _followDistance = 220.0f;  // 摄像机离角色的距离。
  float _followHeight = 80.0f;     // 摄像机观察高度。
  float _followSmooth = 12.0f;     // 摄像机跟随平滑度。

  // 输入状态。
  bool _keyW = false;
  bool _keyS = false;
  bool _keyA = false;
  bool _keyD = false;
  bool _keyQ = false;
  bool _keyE = false;
  bool _rotating = false;

  cocos2d::Vec2 _lastMousePos;
  bool _hasLastMouse = false;

  // 游戏对象。
  Wukong* _player = nullptr;
  TerrainCollider* _terrainCollider = nullptr;
  std::vector<Enemy*> _enemies;
};

// CampScene 是 BaseScene 的特定实现，用于营地场景。
class CampScene : public BaseScene {
 public:
  static cocos2d::Scene* createScene();
  virtual bool init() override;
  CREATE_FUNC(CampScene);
};

#endif  // __BASE_SCENE_H__

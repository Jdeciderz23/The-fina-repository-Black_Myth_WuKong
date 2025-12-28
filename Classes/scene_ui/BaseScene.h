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

// BaseScene is the foundation class for all 3D game scenes.
// It handles camera, skybox, lighting, input, player, and enemy management.
class BaseScene : public cocos2d::Scene {
 public:
  static cocos2d::Scene* createScene();
  virtual bool init() override;

  // Teleports the player back to the respawn point and resets enemies.
  void teleportPlayerToCenter();

  CREATE_FUNC(BaseScene);

 protected:
  // Initialization methods.
  void initCamera();
  void initSkybox();
  void initLights();
  void initInput();
  void initEnemy();
  void initBoss();
  void initPlayer();

  // Update loop.
  virtual void update(float dt) override;
  void updateCamera(float dt);

  // Skybox helpers.
  bool chooseSkyboxFaces(std::array<std::string, 6>& outFaces);
  bool verifyCubeFacesSquare(const std::array<std::string, 6>& faces);

  // Enemy management.
  void removeDeadEnemy(Enemy* deadEnemy);

 protected:
  // Camera members.
  cocos2d::Camera* _mainCamera = nullptr;
  cocos2d::Skybox* _skybox = nullptr;
  bool _autoFollowYaw = true;
  float _autoYawSpeed = 240.0f;  // Degrees/second.
  float _mouseIdleTime = 999.0f; // Time since last mouse movement.

  cocos2d::Vec3 _camPos = cocos2d::Vec3(0.0f, 120.0f, 220.0f);
  cocos2d::Vec3 _camFront = cocos2d::Vec3(0.0f, 0.0f, -1.0f);
  cocos2d::Vec3 _camUp = cocos2d::Vec3::UNIT_Y;

  float _yaw = -90.0f;
  float _pitch = -15.0f;

  float _moveSpeed = 200.0f;
  float _mouseSensitivity = 0.15f;

  // Perspective projection parameters.
  float _fov = 60.0f;         // Field of view in degrees.
  float _aspect = 1.0f;       // Aspect ratio.
  float _nearPlane = 1.0f;    // Near clipping plane.
  float _farPlane = 1000.0f;  // Far clipping plane.
  float _followDistance = 220.0f;  // Camera distance from character.
  float _followHeight = 80.0f;     // Camera look-at height.
  float _followSmooth = 12.0f;     // Camera following smoothness.

  // Input state.
  bool _keyW = false;
  bool _keyS = false;
  bool _keyA = false;
  bool _keyD = false;
  bool _keyQ = false;
  bool _keyE = false;
  bool _rotating = false;

  cocos2d::Vec2 _lastMousePos;
  bool _hasLastMouse = false;

  // Game objects.
  Wukong* _player = nullptr;
  TerrainCollider* _terrainCollider = nullptr;
  std::vector<Enemy*> _enemies;
};

// CampScene is a specific implementation of BaseScene for the camp area.
class CampScene : public BaseScene {
 public:
  static cocos2d::Scene* createScene();
  virtual bool init() override;
  CREATE_FUNC(CampScene);
};

#endif  // __BASE_SCENE_H__

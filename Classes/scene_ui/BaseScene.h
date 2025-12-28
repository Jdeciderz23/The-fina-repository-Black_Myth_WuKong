#ifndef __BASE_SCENE_H__
#define __BASE_SCENE_H__

#include "cocos2d.h"
#include <array>
#include "Wukong.h"
#include "../combat/Collider.h"
#include "Enemy.h"
class Wukong;
class TerrainCollider;
class BaseScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    void teleportPlayerToCenter();
    CREATE_FUNC(BaseScene);

protected:
    /* ---------- Init ---------- */
    void initCamera();
    void initSkybox();
    void initLights();
    void initInput();
    void initEnemy();
    void initBoss();
    /* ---------- Update ---------- */
    virtual void update(float dt) override;
    void updateCamera(float dt);

    /* ---------- Skybox ---------- */
    bool chooseSkyboxFaces(std::array<std::string, 6>& outFaces);
    bool verifyCubeFacesSquare(const std::array<std::string, 6>& faces);

    /* ---------- Player ---------- */
    void initPlayer();

protected:
    /* ---------- Camera ---------- */
    cocos2d::Camera* _mainCamera = nullptr;
    cocos2d::Skybox* _skybox = nullptr;
    bool  _autoFollowYaw = true;
    float _autoYawSpeed = 240.0f;   // 度/秒（越大越快回正）
    float _mouseIdleTime = 999.0f;   // 距离上次鼠标移动时间

    cocos2d::Vec3 _camPos = cocos2d::Vec3(0.0f, 120.0f, 220.0f);
    cocos2d::Vec3 _camFront = cocos2d::Vec3(0.0f, 0.0f, -1.0f);
    cocos2d::Vec3 _camUp = cocos2d::Vec3::UNIT_Y;

    float _yaw = -90.0f;
    float _pitch = -15.0f;

    float _moveSpeed = 200.0f;
    float _mouseSensitivity = 0.15f;

    // 透视投影参数（用于滚轮缩放 FOV）
    float _fov = 60.0f;        // 视野角（度）
    float _aspect = 1.0f;      // 宽高比
    float _nearPlane = 1.0f;   // 近裁剪面
    float _farPlane = 1000.0f; // 远裁剪面
    float _followDistance = 220.0f;   // 相机离人物距离
    float _followHeight = 80.0f;    // 看向人物的高度（头部高度）
    float _followSmooth = 12.0f;    // 跟随平滑（越大越跟手）


    /* ---------- Input ---------- */
    bool _keyW = false;
    bool _keyS = false;
    bool _keyA = false;
    bool _keyD = false;
    bool _keyQ = false;
    bool _keyE = false;

    bool _rotating = false;

    cocos2d::Vec2 _lastMousePos;
    bool _hasLastMouse = false;

    /* ---------- Player ---------- */
    Wukong* _player = nullptr;
    TerrainCollider* _terrainCollider = nullptr;
    std::vector<Enemy*> _enemies;
    
    /* ---------- Enemy Management ---------- */
    void removeDeadEnemy(Enemy* deadEnemy);
};

class CampScene : public BaseScene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(CampScene);
};


#endif // __BASE_SCENE_H__

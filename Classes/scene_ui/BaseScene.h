#ifndef __BASE_SCENE_H__
#define __BASE_SCENE_H__

#include "cocos2d.h"
#include <array>

class BaseScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(BaseScene);

protected:
    /* ---------- Init ---------- */
    void initCamera();
    void initSkybox();
    void initLights();
    void initInput();
    void initDebugObjects();

    /* ---------- Update ---------- */
    virtual void update(float dt) override;
    void updateCamera(float dt);

    /* ---------- Skybox ---------- */
    bool chooseSkyboxFaces(std::array<std::string, 6>& outFaces);
    bool verifyCubeFacesSquare(const std::array<std::string, 6>& faces);

protected:
    /* ---------- Camera ---------- */
    cocos2d::Camera* _mainCamera = nullptr;
    cocos2d::Skybox* _skybox = nullptr;

    cocos2d::Vec3 _camPos = cocos2d::Vec3(0.0f, 50.0f, 200.0f);
    cocos2d::Vec3 _camFront = cocos2d::Vec3(0.0f, 0.0f, -1.0f);
    cocos2d::Vec3 _camUp = cocos2d::Vec3::UNIT_Y;

    float _yaw = -90.0f;
    float _pitch = 0.0f;

    float _moveSpeed = 200.0f;
    float _mouseSensitivity = 0.15f;
    
    // 透视投影参数（用于滚轮缩放 FOV）
    float _fov = 60.0f;        // 视野角（度）
    float _aspect = 1.0f;      // 宽高比
    float _nearPlane = 1.0f;   // 近裁剪面
    float _farPlane = 1000.0f; // 远裁剪面

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
};

class CampScene : public BaseScene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(CampScene);
};

class BossScene : public BaseScene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(BossScene);
};

#endif // __BASE_SCENE_H__

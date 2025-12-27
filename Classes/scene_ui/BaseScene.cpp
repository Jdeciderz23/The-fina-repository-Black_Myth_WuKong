#pragma execution_character_set("utf-8")
#include "BaseScene.h"
#include "GameApp.h"
#include "SceneManager.h"
#include "3d/CCSprite3D.h"
#include "3d/CCTerrain.h"
#include "renderer/CCTexture2D.h"
#include "2d/CCLight.h"
#include "Wukong.h"
#include "InputController.h"
#include "scene_ui/UIManager.h"
#include "../combat/Collider.h"
#include "Enemy.h"
#include "AudioManager.h"

USING_NS_CC;

// 相机投影参数（文件内静态变量，避免因头文件差异导致未声明错误）
static float s_fov = 30.0f;          // 视野角（度，调小以增强“空间巨大”的感觉）
static float s_aspect = 1.0f;        // 宽高比
static float s_nearPlane = 1.0f;     // 近裁剪面（适当拉近）
static float s_farPlane = 2000.0f;   // 远裁剪面（适当拉远）

Scene* BaseScene::createScene()
{
    return BaseScene::create();
}

bool BaseScene::init()
{
    if (!Scene::init())
        return false;

    initCamera();
    initSkybox();
    initLights();
    initPlayer();
    initInput();
    initEnemy();

    // 显示 HUD (血条)
    UIManager::getInstance()->showHUD(this);

    // 播放游戏场景背景音乐
    AudioManager::getInstance()->playBGM("Audio/game_bgm.mp3");

    this->scheduleUpdate();

    auto vs = Director::getInstance()->getVisibleSize(); // 获取屏幕可见区域大小
    Vec2 origin = Director::getInstance()->getVisibleOrigin(); // 获取可见区域原点坐标
    auto label = Label::createWithSystemFont("\xe6\x9a\x82\xe5\x81\x9c", "Arial", 24); // 创建“暂停”按钮文字标签
    auto item = MenuItemLabel::create(label, [](Ref*) {   // 创建菜单项并绑定暂停回调
        UIManager::getInstance()->showPauseMenu();       // 点击时显示暂停菜单
        });
    auto menu = Menu::create(item, nullptr);             // 创建菜单容器
    menu->setPosition(origin + Vec2(30, vs.height - 30)); // 设置菜单位置在左上角
    menu->setCameraMask((unsigned short)CameraFlag::DEFAULT);
    addChild(menu, 1000);                                // 添加菜单到场景，层级设为最高

    return true;
}

/* ==================== Skybox ==================== */

void BaseScene::initSkybox()
{
    std::array<std::string, 6> faces;
    if (!chooseSkyboxFaces(faces) || !verifyCubeFacesSquare(faces))
    {
        CCLOG("Skybox invalid, fallback to color brush.");
        auto brush = CameraBackgroundBrush::createColorBrush(
            Color4F(0.08f, 0.09f, 0.11f, 1.0f), 1.0f);
        _mainCamera->setBackgroundBrush(brush);
        return;
    }

    _skybox = Skybox::create(
        faces[0], faces[1], faces[2],
        faces[3], faces[4], faces[5]
    );
    _skybox->setCameraMask((unsigned short)CameraFlag::USER1);
    _skybox->setRotation3D(Vec3::ZERO);
    addChild(_skybox, -100);
}

// 选择可用的天空盒六面贴图（返回 true 表示找到并填充 outFaces）
bool BaseScene::chooseSkyboxFaces(std::array<std::string, 6>& outFaces)
{
    auto fu = FileUtils::getInstance();                        // 文件工具

    std::array<std::string, 6> set1 = {
        "SkyBox/Skybox_right.png", "SkyBox/Skybox_left.png", "SkyBox/Skybox_top.png",
        "SkyBox/Skybox_bottom.png", "SkyBox/Skybox_front.png", "SkyBox/Skybox_back.png"
    };


    // 检查一组文件是否全部存在
    auto existsAll = [&](const std::array<std::string, 6>& s) -> bool {
        for (const auto& f : s)
        {
            if (!fu->isFileExist(f))
                return false;
        }
        return true;
        };

    if (existsAll(set1)) { outFaces = set1; return true; }
    return false;                                          // 都不存在则返回失败
}

// 校验六面贴图是否满足立方体要求（每张为正方形，且尺寸一致）
bool BaseScene::verifyCubeFacesSquare(const std::array<std::string, 6>& faces)
{
    int faceSize = -1;                                               // 记录第一张的尺寸
    for (int i = 0; i < 6; ++i)
    {
        std::string full = FileUtils::getInstance()->fullPathForFilename(faces[i]); // 解析完整路径
        if (full.empty())
            return false;

        auto img = new (std::nothrow) Image();                       // 加载图片以检查尺寸
        if (!img || !img->initWithImageFile(full))
        {
            CC_SAFE_DELETE(img);
            return false;
        }

        // 要求正方形
        if (img->getWidth() != img->getHeight())
        {
            CC_SAFE_DELETE(img);
            return false;
        }

        // 要求六面尺寸一致
        if (faceSize < 0) faceSize = img->getWidth();
        else if (faceSize != img->getWidth())
        {
            CC_SAFE_DELETE(img);
            return false;
        }

        CC_SAFE_DELETE(img);
    }
    return true;                                                     // 通过校验
}

/* ==================== Camera ==================== */

void BaseScene::initCamera()
{
    auto vs = Director::getInstance()->getVisibleSize();
    s_aspect = vs.width / std::max(1.0f, vs.height);              // 计算可视区域宽高比（避免除零）
    s_fov = 30.0f;                                                // 初始视野角（较小视角）
    s_nearPlane = 0.1f;                                           // 初始近裁剪面
    s_farPlane = 10000.0f;                                         // 初始远裁剪面

    //_mainCamera = Camera::createPerspective(s_fov, s_aspect, s_nearPlane, s_farPlane);
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    _mainCamera = Camera::createPerspective(60.0f, visibleSize.width / visibleSize.height, 1.0f, 2000.0f);
    _mainCamera->setCameraFlag(CameraFlag::USER1);

    _mainCamera->setPosition3D(cocos2d::Vec3(0.0f, 140.0f, 260.0f));
    _mainCamera->lookAt(cocos2d::Vec3(0.0f, 90.0f, 0.0f), cocos2d::Vec3::UNIT_Y);

    //_mainCamera->setPosition3D(_camPos);
    //mainCamera->lookAt(_camPos + _camFront, Vec3::UNIT_Y);

    addChild(_mainCamera);

    // 关掉默认相机
    //this->getDefaultCamera()->setVisible(false);

}

/* ==================== Lights ==================== */

void BaseScene::initLights()
{
    auto ambient = AmbientLight::create(Color3B(180, 180, 180));
    ambient->setIntensity(0.6f);
    addChild(ambient);

    auto dirLight = DirectionLight::create(
        Vec3(-0.7f, -1.0f, -0.3f),
        Color3B::WHITE
    );
    dirLight->setIntensity(1.0f);
    dirLight->setCameraMask((unsigned short)CameraFlag::USER1);
    addChild(dirLight);
}

/* ==================== Input ==================== */

void BaseScene::initInput()
{
    //// ---------- Mouse ----------
    //auto mouse = EventListenerMouse::create();

    //mouse->onMouseDown = [this](EventMouse* e)
    //    {
    //        /*if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
    //        {
    //            _rotating = true;
    //            _lastMousePos = e->getLocationInView();
    //            _hasLastMouse = true;
    //        }*/
    //        _hasLastMouse = true;
    //    };

    //mouse->onMouseUp = [this](EventMouse* e)
    //    {
    //       /* if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
    //        {
    //            _rotating = false;
    //            _hasLastMouse = false;
    //        }*/
    //        _hasLastMouse = false;
    //    };

    //mouse->onMouseMove = [this](EventMouse* e)
    //    {
    //        /*if (!_rotating || !_hasLastMouse) return;

    //        Vec2 cur = e->getLocationInView();
    //        Vec2 delta = cur - _lastMousePos;
    //        _lastMousePos = cur;

    //        _yaw += delta.x * _mouseSensitivity;
    //        _pitch -= delta.y * _mouseSensitivity;
    //        _pitch = clampf(_pitch, -80.0f, 80.0f);

    //        float yawRad = CC_DEGREES_TO_RADIANS(_yaw);
    //        float pitchRad = CC_DEGREES_TO_RADIANS(_pitch);

    //        Vec3 front(
    //            cosf(yawRad) * cosf(pitchRad),
    //            sinf(pitchRad),
    //            sinf(yawRad) * cosf(pitchRad)
    //        );
    //        _camFront = front.getNormalized();*/
    //        Vec2 cur = e->getLocationInView();
    //        if (!_hasLastMouse) { _lastMousePos = cur; _hasLastMouse = true; return; }

    //        Vec2 delta = cur - _lastMousePos;
    //        _lastMousePos = cur;

    //        _yaw += delta.x * _mouseSensitivity;
    //        _pitch -= delta.y * _mouseSensitivity;
    //        _pitch = clampf(_pitch, -80.0f, 80.0f);
    //        _mouseIdleTime = 0.0f;
    //    };

    //mouse->onMouseScroll = [this](EventMouse* e)
    //    {
    //        //// 滚轮缩放视野角（FOV），让“变焦”更直观
    //        //// 正值向前滚：缩小 FOV（放大画面），负值向后滚：增大 FOV（缩小画面）
    //        //float delta = e->getScrollY();                          // 获取滚轮增量
    //        //s_fov -= delta * 2.0f;                                  // 调整 FOV（适度灵敏度）
    //        //s_fov = clampf(s_fov, 25.0f, 80.0f);                    // 限制 FOV 范围，避免过大/过小

    //        //// 计算新的投影矩阵，并通过 setAdditionalProjection 精确替换当前投影
    //        //Mat4 newProj;
    //        //Mat4::createPerspective(s_fov, s_aspect, s_nearPlane, s_farPlane, &newProj);
    //        //const Mat4& oldProj = _mainCamera->getProjectionMatrix();
    //        //Mat4 deltaProj = newProj * oldProj.getInversed();       // 求出投影变换增量
    //        //_mainCamera->setAdditionalProjection(deltaProj);        // 应用新的投影（不累计误差）
    //        _followDistance = clampf(_followDistance - e->getScrollY() * 25.0f, 140.0f, 380.0f);
    //    };

    //_eventDispatcher->addEventListenerWithSceneGraphPriority(mouse, this);

    scheduleUpdate();
}

/* ==================== Update ==================== */

void BaseScene::update(float dt)
{
    /*_mouseIdleTime += dt;
    updateCamera(dt);*/
    
    // 更新 HUD 血条
    if (_player) {
        // 掉落死亡检测
        if (_player->getPositionY() < -500.0f && !_player->isDead()) {
            _player->die();
        }

        float hp = (float)_player->getHP();
        float maxHp = (float)_player->getMaxHP();
        UIManager::getInstance()->updatePlayerHP(hp / maxHp);
    }

    // 相机由 PlayerController 更新，这里只同步 skybox
    if (_skybox && _mainCamera) {
        _skybox->setPosition3D(_mainCamera->getPosition3D());
        _skybox->setRotation3D(cocos2d::Vec3::ZERO);
    }
}

static float moveTowardAngleDeg(float cur, float target, float maxDeltaDeg)
{
    float delta = std::fmod(target - cur + 540.0f, 360.0f) - 180.0f; // [-180,180]
    if (delta > maxDeltaDeg) delta = maxDeltaDeg;
    if (delta < -maxDeltaDeg) delta = -maxDeltaDeg;
    return cur + delta;
}

void BaseScene::updateCamera(float dt)
{
    if (!_mainCamera || !_player) return;
    // 不动鼠标时，镜头自动回到人物移动方向后方
    if (_autoFollowYaw && _mouseIdleTime > 0.12f)
    {
        auto intent = _player->getMoveIntent();
        cocos2d::Vec3 d = intent.dirWS;
        d.y = 0.0f;
        if (d.lengthSquared() > 1e-6f)
        {
            d.normalize();
            float desiredYaw = CC_RADIANS_TO_DEGREES(std::atan2f(d.z, d.x));
            _yaw = moveTowardAngleDeg(_yaw, desiredYaw, _autoYawSpeed * dt);
        }
    }

    // 目标点：人物位置 + 头部高度
    cocos2d::Vec3 playerPos = _player->getPosition3D();
    cocos2d::Vec3 target = playerPos + cocos2d::Vec3(0.0f, _followHeight, 0.0f);

    // 根据 yaw/pitch 得到相机“朝向”
    float yawRad = CC_DEGREES_TO_RADIANS(_yaw);
    float pitchRad = CC_DEGREES_TO_RADIANS(_pitch);

    cocos2d::Vec3 front(
        cosf(yawRad) * cosf(pitchRad),
        sinf(pitchRad),
        sinf(yawRad) * cosf(pitchRad)
    );
    front.normalize();

    // 相机期望位置：在人物“后方”一定距离
    cocos2d::Vec3 desiredPos = target - front * _followDistance;

    // 平滑跟随（指数插值，帧率稳定）
    float t = 1.0f - expf(-_followSmooth * dt);
    _camPos = _camPos.lerp(desiredPos, t);

    _mainCamera->setPosition3D(_camPos);
    _mainCamera->lookAt(target, cocos2d::Vec3::UNIT_Y);

    if (_skybox) {
        _skybox->setPosition3D(_camPos);
        _skybox->setRotation3D(cocos2d::Vec3::ZERO);
    }

}
void BaseScene::teleportPlayerToCenter()
{
    if (_player) {
        _player->setPosition3D(Vec3(0, 200, 0)); // 传送到高空防止直接掉下去
        _player->respawn();
    }
}


/* ==================== Debug ==================== */


Scene* CampScene::createScene()
{
    return CampScene::create();
}

bool CampScene::init()
{
    if (!BaseScene::init())
        return false;

    // 使用3D模型作为地形
    auto terrain = Sprite3D::create("scene/terrain.obj");

    // 设置地形位置和缩放
    terrain->setPosition3D(Vec3(0, 0, 0));
    terrain->setScale(100.0f);  // 根据实际模型大小调整缩放比例

    // 设置渲染相机
    terrain->setCameraMask((unsigned short)CameraFlag::USER1);
    addChild(terrain);

    // 创建地形碰撞器
    _terrainCollider = TerrainCollider::create(terrain, "scene/terrain.obj");
    if (_terrainCollider) {
        _terrainCollider->retain();
        if (_player) {
            _player->setTerrainCollider(_terrainCollider);
        }
        // 设置所有敌人的地形碰撞器
        for (auto enemy : _enemies) {
            enemy->setTerrainCollider(_terrainCollider);
        }
    }

    return true;
}



/* ---------- Player ---------- */

void BaseScene::initPlayer()
{
    _player = Wukong::create();
    if (!_player) {
        CCLOG("Error: Wukong create failed!");
        return;
    }

    // 放到场景中心，地面 y=0
    _player->setPosition3D(cocos2d::Vec3(0.0f, 0.0f, 0.0f));
    _player->setRotation3D(cocos2d::Vec3::ZERO);

    if (_terrainCollider) {
        _player->setTerrainCollider(_terrainCollider);
    }

    addChild(_player, 10);
    // 绑定键盘控制（WASD/Shift/Space/J/K）
    auto controller = PlayerController::create(_player);
    controller->setCamera(_mainCamera);
    addChild(controller, 20);
}
void BaseScene::initEnemy()
{
    struct Spawn {
        const char* root;
        const char* model;
        cocos2d::Vec3 pos;
    };

    const Spawn spawns[] = {
        { "Enemy/enemy1", "enemy1.c3b", cocos2d::Vec3(0, 30, 0) },
        { "Enemy/enemy1", "enemy1.c3b", cocos2d::Vec3(200, 30, 50) },
        { "Enemy/enemy1", "enemy1.c3b", cocos2d::Vec3(-200, 30, 50) },
    };

    for (auto& s : spawns) {
        auto e = Enemy::createWithResRoot(s.root, s.model);
        if (!e) continue;

        e->setPosition3D(s.pos);
        e->setBirthPosition(e->getPosition3D());
        e->setTarget(_player);
        e->setTerrainCollider(_terrainCollider); // 设置地形碰撞

        this->addChild(e);
        _enemies.push_back(e); // 添加到敌人列表
    }

    // 将敌人列表同步给玩家，用于碰撞检测
    if (_player) {
        _player->setEnemies(&_enemies);
    }
}
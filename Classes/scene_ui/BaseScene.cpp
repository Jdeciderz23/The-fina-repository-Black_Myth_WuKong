#pragma execution_character_set("utf-8")
#include "BaseScene.h"
#include "GameApp.h"
#include "SceneManager.h"
#include "3d/CCSprite3D.h"
#include "3d/CCTerrain.h"
#include "renderer/CCTexture2D.h"
#include "2d/CCLight.h"
#include "Wukong.h"
#include"InputController.h"

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
    initLights();                                       // 初始化光照（环境光 + 平行光）
    initPlayer();
    // initDebugObjects();                              // 调试用的参照物（会使用 StartMenu.png），这里先关闭
    initInput();                                        // 初始化输入（键盘 + 鼠标）

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

    // 方案一：常见命名（右、左、上、下、前、后）
    std::array<std::string, 6> set1 = {
        "Skybox_right.png", "Skybox_left.png", "Skybox_top.png",
        "Skybox_bottom.png", "Skybox_front.png", "Skybox_back.png"
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

    _mainCamera = Camera::createPerspective(s_fov, s_aspect, s_nearPlane, s_farPlane);
    _mainCamera->setCameraFlag(CameraFlag::USER1);

    _mainCamera->setPosition3D(_camPos);
    _mainCamera->lookAt(_camPos + _camFront, Vec3::UNIT_Y);

    addChild(_mainCamera);
    // 关掉默认相机
    this->getDefaultCamera()->setVisible(false);

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
    // ---------- Mouse ----------
    auto mouse = EventListenerMouse::create();

    mouse->onMouseDown = [this](EventMouse* e)
        {
            if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
            {
                _rotating = true;
                _lastMousePos = e->getLocationInView();
                _hasLastMouse = true;
            }
        };

    mouse->onMouseUp = [this](EventMouse* e)
        {
            if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
            {
                _rotating = false;
                _hasLastMouse = false;
            }
        };

    mouse->onMouseMove = [this](EventMouse* e)
        {
            if (!_rotating || !_hasLastMouse) return;

            Vec2 cur = e->getLocationInView();
            Vec2 delta = cur - _lastMousePos;
            _lastMousePos = cur;

            _yaw += delta.x * _mouseSensitivity;
            _pitch -= delta.y * _mouseSensitivity;
            _pitch = clampf(_pitch, -80.0f, 80.0f);

            float yawRad = CC_DEGREES_TO_RADIANS(_yaw);
            float pitchRad = CC_DEGREES_TO_RADIANS(_pitch);

            Vec3 front(
                cosf(yawRad) * cosf(pitchRad),
                sinf(pitchRad),
                sinf(yawRad) * cosf(pitchRad)
            );
            _camFront = front.getNormalized();
        };

    mouse->onMouseScroll = [this](EventMouse* e)
        {
            // 滚轮缩放视野角（FOV），让“变焦”更直观
            // 正值向前滚：缩小 FOV（放大画面），负值向后滚：增大 FOV（缩小画面）
            float delta = e->getScrollY();                          // 获取滚轮增量
            s_fov -= delta * 2.0f;                                  // 调整 FOV（适度灵敏度）
            s_fov = clampf(s_fov, 25.0f, 80.0f);                    // 限制 FOV 范围，避免过大/过小

            // 计算新的投影矩阵，并通过 setAdditionalProjection 精确替换当前投影
            Mat4 newProj;
            Mat4::createPerspective(s_fov, s_aspect, s_nearPlane, s_farPlane, &newProj);
            const Mat4& oldProj = _mainCamera->getProjectionMatrix();
            Mat4 deltaProj = newProj * oldProj.getInversed();       // 求出投影变换增量
            _mainCamera->setAdditionalProjection(deltaProj);        // 应用新的投影（不累计误差）
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouse, this);

    scheduleUpdate();
}

/* ==================== Update ==================== */

void BaseScene::update(float dt)
{
    updateCamera(dt);
}

void BaseScene::updateCamera(float dt)
{
    if (!_mainCamera || !_player) return;

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
    cocos2d::Vec3 desiredPos = target - front *_followDistance;

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


/* ==================== Debug ==================== */

void BaseScene::initDebugObjects()
{
    auto bb = BillBoard::create("StartMenu.png", BillBoard::Mode::VIEW_POINT_ORIENTED);
    bb->setPosition3D(Vec3(0, 0, -200));
    bb->setScale(2.0f);
    bb->setCameraMask((unsigned short)CameraFlag::USER1);
    addChild(bb);
}

Scene* CampScene::createScene()
{
    return CampScene::create();
}

bool CampScene::init()
{
    if (!BaseScene::init())
        return false;

    // 使用3D模型作为地形
    auto terrain = Sprite3D::create("terrain.obj");

    // 设置地形位置和缩放
    terrain->setPosition3D(Vec3(0, 0, 0));
    terrain->setScale(100.0f);  // 根据实际模型大小调整缩放比例

    // 设置渲染相机
    terrain->setCameraMask((unsigned short)CameraFlag::USER1);
    addChild(terrain);

    auto kb = EventListenerKeyboard::create();
    kb->onKeyPressed = [](EventKeyboard::KeyCode code, Event*)
        {
            if (code == EventKeyboard::KeyCode::KEY_B)
            {
                auto mgr = GameApp::getInstance()->getSceneManager();
                if (mgr) mgr->switchScene(SceneManager::SceneType::BOSS_FIGHT, true);
            }
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(kb, this);

    return true;
}

Scene* BossScene::createScene()
{
    return BossScene::create();
}

bool BossScene::init()
{
    if (!BaseScene::init())
        return false;

    auto banner = BillBoard::create("1.png", BillBoard::Mode::VIEW_POINT_ORIENTED);
    banner->setPosition3D(Vec3(-50, 0, -250));
    banner->setScale(2.5f);
    banner->setCameraMask((unsigned short)CameraFlag::USER1);
    addChild(banner);

    auto kb = EventListenerKeyboard::create();
    kb->onKeyPressed = [](EventKeyboard::KeyCode code, Event*)
        {
            if (code == EventKeyboard::KeyCode::KEY_C)
            {
                auto mgr = GameApp::getInstance()->getSceneManager();
                if (mgr) mgr->switchScene(SceneManager::SceneType::GAMEPLAY, true);
            }
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(kb, this);

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

    // 放到场景中心，地面 y=0（你当前重力/落地简化也是 y=0）
    _player->setPosition3D(cocos2d::Vec3(0.0f, 0.0f, 0.0f));
    _player->setRotation3D(cocos2d::Vec3::ZERO); 

    addChild(_player, 10);
    // 绑定键盘控制（WASD/Shift/Space/J/K）
    auto controller = PlayerController::create(_player);
    controller->setCamera(_mainCamera);
    addChild(controller, 20);
}
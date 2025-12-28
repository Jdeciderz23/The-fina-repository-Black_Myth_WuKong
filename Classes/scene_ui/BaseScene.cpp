#pragma execution_character_set("utf-8")
#include "BaseScene.h"
#include "GameApp.h"
#include "core/AreaManager.h"
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
#include "Boss.h"
#include "BossAI.h"
#include "AudioManager.h"

USING_NS_CC;

// ?????????????????????????????????????????
static float s_fov = 30.0f;          // ????????��???????????????��?
static float s_aspect = 1.0f;        // ?????
static float s_nearPlane = 1.0f;     // ???��??��??????????
static float s_farPlane = 2000.0f;   // ??��??��??????????

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
    initBoss();

    // ??? HUD (???)
    UIManager::getInstance()->showHUD(this);

    // ????????????????
    auto points = AreaManager::getInstance()->getTeleportPoints();
    for (const auto& pt : points) {
        auto marker = Sprite3D::create("WuKong/wukong.c3b"); 
        if (marker) {
            marker->setPosition3D(pt.position);
            marker->setScale(0.5f); // ????????0.5 ????????????
            marker->setColor(Color3B(255, 215, 0)); // ??????
            marker->setCameraMask((unsigned short)CameraFlag::USER1);
            this->addChild(marker);
            
            // ??????????????????????????????
            marker->runAction(RepeatForever::create(RotateBy::create(2.0f, Vec3(0, 180, 0))));
        }
    }

    // ?????????????
    // ???????????
    AudioManager::getInstance()->playBGM("Audio/game_bgm1.mp3");

    this->scheduleUpdate();

    auto vs = Director::getInstance()->getVisibleSize(); // ??????????????��
    Vec2 origin = Director::getInstance()->getVisibleOrigin(); // ?????????????????
    auto label = Label::createWithSystemFont("\xe6\x9a\x82\xe5\x81\x9c", "Arial", 24); // ????????????????
    auto item = MenuItemLabel::create(label, [](Ref*) {   // ??????????????????
        UIManager::getInstance()->showPauseMenu();       // ?????????????
        });
    auto menu = Menu::create(item, nullptr);             // ???????????
    menu->setPosition(origin + Vec2(30, vs.height - 30)); // ???��??��?????????
    menu->setCameraMask((unsigned short)CameraFlag::DEFAULT);
    addChild(menu, 1000);                                // ???????????��?????

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

// ??????????????????????? true ????? outFaces
bool BaseScene::chooseSkyboxFaces(std::array<std::string, 6>& outFaces)
{
    auto fu = FileUtils::getInstance();                        // ?????????

    std::array<std::string, 6> set1 = {
        "SkyBox/Skybox_right.png", "SkyBox/Skybox_left.png", "SkyBox/Skybox_top.png",
        "SkyBox/Skybox_bottom.png", "SkyBox/Skybox_front.png", "SkyBox/Skybox_back.png"
    };


    // ???????????????????
    auto existsAll = [&](const std::array<std::string, 6>& s) -> bool {
        for (const auto& f : s)
        {
            if (!fu->isFileExist(f))
                return false;
        }
        return true;
        };

    if (existsAll(set1)) { outFaces = set1; return true; }
    return false;                                          // ??????????????????
}

// ��??????????????????????????????????????????
bool BaseScene::verifyCubeFacesSquare(const std::array<std::string, 6>& faces)
{
    int faceSize = -1;                                               // ???????????
    for (int i = 0; i < 6; ++i)
    {
        std::string full = FileUtils::getInstance()->fullPathForFilename(faces[i]); // ???????��??
        if (full.empty())
            return false;

        auto img = new (std::nothrow) Image();                       // ?????????????
        if (!img || !img->initWithImageFile(full))
        {
            CC_SAFE_DELETE(img);
            return false;
        }

        // ???????????
        if (img->getWidth() != img->getHeight())
        {
            CC_SAFE_DELETE(img);
            return false;
        }

        // ??????????????
        if (faceSize < 0) faceSize = img->getWidth();
        else if (faceSize != img->getWidth())
        {
            CC_SAFE_DELETE(img);
            return false;
        }

        CC_SAFE_DELETE(img);
    }
    return true;                                                     // ???��??
}

/* ==================== Camera ==================== */

void BaseScene::initCamera()
{
    auto vs = Director::getInstance()->getVisibleSize();
    s_aspect = vs.width / std::max(1.0f, vs.height);              // ?????????????????
    s_fov = 30.0f;                                                // ????????
    s_nearPlane = 0.1f;                                           // ??????��???
    s_farPlane = 10000.0f;                                         // ?????��???

    //_mainCamera = Camera::createPerspective(s_fov, s_aspect, s_nearPlane, s_farPlane);
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
    _mainCamera = Camera::createPerspective(60.0f, visibleSize.width / visibleSize.height, 1.0f, 2000.0f);
    _mainCamera->setCameraFlag(CameraFlag::USER1);

    _mainCamera->setPosition3D(cocos2d::Vec3(0.0f, 140.0f, 260.0f));
    _mainCamera->lookAt(cocos2d::Vec3(0.0f, 90.0f, 0.0f), cocos2d::Vec3::UNIT_Y);

    //_mainCamera->setPosition3D(_camPos);
    //mainCamera->lookAt(_camPos + _camFront, Vec3::UNIT_Y);

    addChild(_mainCamera);

    // ???????????????????????
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
    //        //// ?????????????????FOV?????????????????????????
    //        //// ?????????????????�� FOV??????��???????????????????? FOV????��???��
    //        //float delta = e->getScrollY();                          // ???????????
    //        //s_fov -= delta * 2.0f;                                  // ???? FOV???????
    //        //s_fov = clampf(s_fov, 25.0f, 80.0f);                    // ???? FOV ??��

    //        //// ?????????????????? setAdditionalProjection ????�I?????
    //        //Mat4 newProj;
    //        //Mat4::createPerspective(s_fov, s_aspect, s_nearPlane, s_farPlane, &newProj);
    //        //const Mat4& oldProj = _mainCamera->getProjectionMatrix();
    //        //Mat4 deltaProj = newProj * oldProj.getInversed();       // ???????��????
    //        //_mainCamera->setAdditionalProjection(deltaProj);        // ???????
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
    
    // ???? HUD ???
    if (_player) {
        // ??????????
        if (_player->getPositionY() < -500.0f && !_player->isDead()) {
            _player->die();
        }

        float hp = (float)_player->getHP();
        float maxHp = (float)_player->getMaxHP();
        UIManager::getInstance()->updatePlayerHP(hp / maxHp);
    }

    // ????????��????????????
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
    // ??????????????????????????????
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

    // ???????��?? + ??????
    cocos2d::Vec3 playerPos = _player->getPosition3D();
    cocos2d::Vec3 target = playerPos + cocos2d::Vec3(0.0f, _followHeight, 0.0f);

    // ???? yaw/pitch ?????????��??
    float yawRad = CC_DEGREES_TO_RADIANS(_yaw);
    float pitchRad = CC_DEGREES_TO_RADIANS(_pitch);

    cocos2d::Vec3 front(
        cosf(yawRad) * cosf(pitchRad),
        sinf(pitchRad),
        sinf(yawRad) * cosf(pitchRad)
    );
    front.normalize();

    // ?????��????????????????��???
    cocos2d::Vec3 desiredPos = target - front * _followDistance;

    // ??????????????????????
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
        _player->setPosition3D(Vec3(0, 200, 0)); // ?????????????????
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

    // ??? 3D ??????????
    auto terrain = Sprite3D::create("scene/terrain.obj");

    // ???????��?��?????
    terrain->setPosition3D(Vec3(0, 0, 0));
    terrain->setScale(100.0f);  // ???????????��????????

    // ???????????
    terrain->setCameraMask((unsigned short)CameraFlag::USER1);
    addChild(terrain);

    // ??????????????
    _terrainCollider = TerrainCollider::create(terrain, "scene/terrain.obj");
    if (_terrainCollider) {
        _terrainCollider->retain();
        if (_player) {
            _player->setTerrainCollider(_terrainCollider);
        }
        // ????????��????????????????
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

    // ????????��??
    _player->setPosition3D(cocos2d::Vec3(0.0f, 0.0f, 0.0f));
    _player->setRotation3D(cocos2d::Vec3::ZERO);

    if (_terrainCollider) {
        _player->setTerrainCollider(_terrainCollider);
    }

    addChild(_player, 10);
    // ????????
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
        { "Enemy/enemy1", "enemy1.c3b", cocos2d::Vec3(200, 0, -150) },
        { "Enemy/enemy2", "enemy2.c3b", cocos2d::Vec3(200, 0, 50) },
        { "Enemy/enemy3", "enemy3.c3b", cocos2d::Vec3(300, 0, 50) },
    };

    for (auto& s : spawns) {
        auto e = Enemy::createWithResRoot(s.root, s.model);
        if (!e) continue;

        e->setPosition3D(s.pos);
        e->setBirthPosition(e->getPosition3D());
        e->setTarget(_player);
        e->setTerrainCollider(_terrainCollider); // ??????????

        this->addChild(e);
        _enemies.push_back(e); // ??????????��?
    }

    // ??????��?????????��?????????????
    if (_player) {
        _player->setEnemies(&_enemies);
    }
    
    // ??????????????????
    auto enemyDeathListener = cocos2d::EventListenerCustom::create("enemy_died", [this](cocos2d::EventCustom* event) {
        CCLOG("BaseScene: ??????????????");
        Enemy* deadEnemy = static_cast<Enemy*>(event->getUserData());
        if (deadEnemy) {
            CCLOG("BaseScene: ??????? %p", (void*)deadEnemy);
            this->removeDeadEnemy(deadEnemy);
        }
    });
    
    _eventDispatcher->addEventListenerWithFixedPriority(enemyDeathListener, 1);
}

void BaseScene::removeDeadEnemy(Enemy* deadEnemy) {
    if (!deadEnemy) {
        CCLOG("BaseScene::removeDeadEnemy: ??��????????");
        return;
    }
    
    CCLOG("BaseScene::removeDeadEnemy: ??????? %p", (void*)deadEnemy);
    
    // ??????��???????????
    auto it = std::find(_enemies.begin(), _enemies.end(), deadEnemy);
    if (it != _enemies.end()) {
        _enemies.erase(it);
        CCLOG("BaseScene::removeDeadEnemy: ????????��??????????? %zu ??????", _enemies.size());
    } else {
        CCLOG("BaseScene::removeDeadEnemy: ????��???��??????");
    }
}

void BaseScene::initBoss()
{
    // ???? Boss
    auto boss = Boss::createBoss("Enemy/boss", "boss.c3b");
    if (!boss) {
        CCLOG("Error: Boss create failed!");
        return;
    }

    // ????��?????????????????????
    // ????????????????????
    boss->setPosition3D(cocos2d::Vec3(0, 50, -500)); 
    boss->setBirthPosition(boss->getPosition3D());
    boss->setTarget(_player);
    
    // ????????????
    if (_terrainCollider) {
        boss->setTerrainCollider(_terrainCollider);
    }

    // ?? AI
    boss->setAI(new BossAI(boss));

    auto sprite = boss->getSprite();
    if (sprite) {
        sprite->setScale(0.5f); 
        boss->setSpriteOffsetY(40.0f); 
    }

    this->addChild(boss);
    _enemies.push_back(boss);

    // ????????
    if (_player) {
        _player->setEnemies(&_enemies);
    }

    CCLOG("Boss initialized at: %f, %f, %f with AI", boss->getPositionX(), boss->getPositionY(), boss->getPositionZ());

    // 添加Boss死亡事件监听器
    auto bossDeathListener = cocos2d::EventListenerCustom::create("enemy_died", [this](cocos2d::EventCustom* event) {
        CCLOG("BaseScene: Boss死亡事件触发");
        Enemy* deadEnemy = static_cast<Enemy*>(event->getUserData());
        if (deadEnemy && deadEnemy->getEnemyType() == Enemy::EnemyType::BOSS) {
            CCLOG("BaseScene: 移除死亡的Boss %p", (void*)deadEnemy);
            this->removeDeadEnemy(deadEnemy);
        }
    });
    
    _eventDispatcher->addEventListenerWithFixedPriority(bossDeathListener, 1);
}



#include "Enemy.h"
#include "EnemyStates.h"
#include "combat/HealthComponent.h"
#include "combat/CombatComponent.h"
#include "combat/Collider.h"
#include "player/Wukong.h"

Enemy* Enemy::create() {
    auto enemy = new (std::nothrow) Enemy();
    if (enemy && enemy->init()) {
        enemy->autorelease();
        return enemy;
    }
    CC_SAFE_DELETE(enemy);
    return nullptr;
}

Enemy::Enemy()
    : _enemyType(EnemyType::NORMAL)
    , _stateMachine(nullptr)
    , _health(nullptr)
    , _combat(nullptr)
    , _moveSpeed(50.0f)
    , _rotateSpeed(180.0f) // 默认旋转速度
    , _viewRange(200.0f)   // 默认视野范围
    , _canMove(true)
    , _canAttack(true)
    , _sprite(nullptr)
    , _targetPosition(Vec3::ZERO)
    , _birthPosition(0, 100, 0)
    , _maxChaseRange(1000.0f)
    , _terrainCollider(nullptr)
    , _velocity(Vec3::ZERO)
    , _onGround(true)
{
}

Enemy::~Enemy() {
    if (_stateMachine) {
        delete _stateMachine;
        _stateMachine = nullptr;
    }
}

bool Enemy::init() {
    if (!Node::init()) {
        return false;
    }

    // 初始化状态机
    initStateMachine();
    
    // 初始化生命值组件
    initHealthComponent();
    
    // 初始化战斗组件
    initCombatComponent();
    
    // 创建3D精灵
    _birthPosition = this->getPosition3D();

    // 开启更新循环
    this->scheduleUpdate();

    // 初始化 AABB 碰撞器，收缩 XZ 轴到 40%
    _collider.calculateBoundingBox(_sprite, 0.4f);

    // 开启更新循环
    this->scheduleUpdate();
    
    return true;
}

void Enemy::update(float deltaTime) {
    Node::update(deltaTime);
    
    // 更新状态机
    if (_stateMachine) {
        _stateMachine->update(deltaTime);
    }

    applyGravity(deltaTime);
    applyMovement(deltaTime);

    // 更新 AABB 碰撞盒到世界空间
    _collider.update(this);
}

void Enemy::applyGravity(float dt) {
    if (_onGround && _terrainCollider) {
        return;
    }
    _velocity.y -= _gravity * dt;
}

void Enemy::applyMovement(float dt) {
    Vec3 oldPos = this->getPosition3D();
    Vec3 newPos = oldPos + _velocity * dt;

    if (_terrainCollider) {
        // 射线检测新位置地面
        CustomRay ray(newPos + Vec3(0, 500, 0), Vec3(0, -1, 0));
        float hitDist;

        if (_terrainCollider->rayIntersects(ray, hitDist)) {
            float groundY = ray.origin.y - hitDist;
            const float MAX_STEP_HEIGHT = 40.0f;

            if (groundY - oldPos.y < MAX_STEP_HEIGHT) {
                newPos.y = groundY;
                this->setPosition3D(newPos);
                
                if (!_onGround && _velocity.y <= 0) {
                    _onGround = true;
                    _velocity.y = 0;
                }
            } else {
                // 坡度太陡
                Vec3 finalPos = oldPos;
                finalPos.y += _velocity.y * dt; 
                
                if (finalPos.y <= groundY) {
                    finalPos.y = groundY;
                    _onGround = true;
                    _velocity.y = 0;
                }
                this->setPosition3D(finalPos);
            }
        } else {
            // 没检测到地面
            this->setPosition3D(newPos);
            _onGround = false;
        }
    } else {
        this->setPosition3D(newPos);
        if (newPos.y <= 0.0f) {
            Vec3 pos = this->getPosition3D();
            pos.y = 0.0f;
            this->setPosition3D(pos);
            _velocity.y = 0.0f;
            _onGround = true;
        }
    }
}

float Enemy::getMoveSpeed() const {
    return _moveSpeed;
}

float Enemy::getRotateSpeed() const {
    return _rotateSpeed;
}

float Enemy::getViewRange() const {
    return _viewRange;
}

bool Enemy::canMove() const {
    return _canMove && !isDead();
}

bool Enemy::canAttack() const {
    return _canAttack && !isDead();
}

bool Enemy::isDead() const {
    // 直接查询HealthComponent
    return _health ? _health->isDead() : false;
}

Enemy::EnemyType Enemy::getEnemyType() const {
    return _enemyType;
}

void Enemy::setEnemyType(EnemyType type) {
    _enemyType = type;
    
    // 根据敌人类型调整基础移动属性
    switch (type) {
        case EnemyType::NORMAL:
            _moveSpeed = 50.0f;
            _rotateSpeed = 180.0f;
            _viewRange = 200.0f;
            break;
        case EnemyType::BOSS:
            _moveSpeed = 40.0f;
            _rotateSpeed = 120.0f;
            _viewRange = 300.0f;
            break;
    }
}

void Enemy::setPosition3D(const Vec3& position) {
    Node::setPosition3D(position);
}


Vec3 Enemy::getPosition3D() const {
    return Node::getPosition3D();
}

StateMachine<Enemy>* Enemy::getStateMachine() const {
    return _stateMachine;
}

Sprite3D* Enemy::getSprite() const {
    return _sprite;
}

// 设置出生点
void Enemy::setBirthPosition(const Vec3& pos) {
    _birthPosition = pos;
}

// 获取出生点
const Vec3& Enemy::getBirthPosition() const {
    return _birthPosition;
}

// 获取最大追击距离
float Enemy::getMaxChaseRange() const {
    return _maxChaseRange;
}
void Enemy::initStateMachine() {
    // 创建状态机实例
    _stateMachine = new StateMachine<Enemy>(this);
    
    // 注册所有状态
    _stateMachine->registerState(new EnemyIdleState());
    _stateMachine->registerState(new EnemyPatrolState());
    _stateMachine->registerState(new EnemyChaseState());
    _stateMachine->registerState(new EnemyAttackState());
    _stateMachine->registerState(new EnemyHitState());
    _stateMachine->registerState(new EnemyDeadState());
    _stateMachine->registerState(new ReturnState());

    // 初始化为待机状态（使用已注册的状态）
    _stateMachine->changeState("Idle");
}

void Enemy::initHealthComponent() {
    // 创建生命值组件
    _health = HealthComponent::create(1.0f);
    if (_health) {
        this->addComponent(_health);
        
        // 设置受伤和死亡回调
        _health->setOnHurtCallback(std::bind(&Enemy::onHurtCallback, this, std::placeholders::_1, std::placeholders::_2));
        _health->setOnDeadCallback(std::bind(&Enemy::onDeadCallback, this, std::placeholders::_1));
    }
}

void Enemy::initCombatComponent() {
    // 创建战斗组件
    _combat = CombatComponent::create();
    if (_combat) {
        //this->addComponent(_combat);
    }
}

void Enemy::onHurtCallback(float damage, Node* attacker) {
    // 受击反馈：闪烁效果
    if (_sprite) {
        _sprite->runAction(Blink::create(0.5f, 5));
    }
    
    // 临时禁用移动和攻击
    _canMove = false;
    _canAttack = false;
    
    // 切换到受击状态
    if (_stateMachine) {
        _stateMachine->changeState("Hit");
    }
    
    // 一段时间后恢复移动和攻击能力
    this->runAction(Sequence::create(
        DelayTime::create(0.5f),
        CallFunc::create([this]() {
            // 只有在未死亡时才恢复移动和攻击能力
            if (!isDead()) {
                _canMove = true;
                _canAttack = true;
            }
        }),
        nullptr
    ));
}

void Enemy::onDeadCallback(Node* attacker) {
    // 当HealthComponent检测到死亡时，只做行为与状态切换
    _canMove = false;
    _canAttack = false;
    
    if (_stateMachine) {
        _stateMachine->changeState("Dead");
    }
}

bool Enemy::isLowHealth() const {
    if (!_health) return false;
    return _health->getHealthPercentage() <= 0.3f;
}

float Enemy::getHealthRatio() const {
    return _health ? _health->getHealthPercentage() : 1.0f;
}

void Enemy::setTarget(Wukong* w) { _target = w; }
Wukong* Enemy::getTarget() const { return _target; }

cocos2d::Vec3 Enemy::getTargetWorldPos() const {
    return _target ? _target->getWorldPosition3D() : cocos2d::Vec3::ZERO;
}

cocos2d::Vec3 Enemy::getWorldPosition3D() const {
    cocos2d::Vec3 out = cocos2d::Vec3::ZERO;
    cocos2d::Mat4 m = this->getNodeToWorldTransform();
    m.transformPoint(cocos2d::Vec3::ZERO, &out);
    return out;
}

//加载模型
bool Enemy::initWithResRoot(const std::string& resRoot, const std::string& modelFile) {
    _resRoot = resRoot;
    _modelFile = modelFile;

    if (!Node::init()) return false; // 不再调用 this->init()，避免重复逻辑

    // 初始化组件（保持一致）
    initStateMachine();
    initHealthComponent();
    initCombatComponent();

    // 加载模型
    std::string modelPath = _resRoot + "/" + _modelFile;
    _sprite = cocos2d::Sprite3D::create(modelPath);
    if (!_sprite) {
        CCLOG("错误: 无法加载敌人模型: %s", modelPath.c_str());
        return false;
    }

    _sprite->setScale(0.25f);
    _sprite->setRotation3D(Vec3(0, 0, 0));
    _sprite->setCameraMask((unsigned short)cocos2d::CameraFlag::USER1);
    _sprite->setForceDepthWrite(true);
    _sprite->setCullFaceEnabled(false);
    this->addChild(_sprite);

    // 修正模型位置，确保脚底在地面（y=0）
    auto aabb = _sprite->getAABB();
    _sprite->setPosition3D(Vec3(0, -aabb._min.y, 0));

    // 初始化 AABB 碰撞器，收缩 XZ 轴到 40%
    _collider.calculateBoundingBox(_sprite, 0.4f);

    // 记录出生点
    _birthPosition = this->getPosition3D();

    // 开启 update
    this->scheduleUpdate();
    return true;
}

Enemy* Enemy::createWithResRoot(const std::string& resRoot,
    const std::string& modelFile) {
    auto enemy = new (std::nothrow) Enemy();
    if (enemy && enemy->initWithResRoot(resRoot, modelFile)) {
        enemy->autorelease();
        return enemy;
    }
    CC_SAFE_DELETE(enemy);
    return nullptr;
}

//加载动画
void Enemy::playAnim(const std::string& name, bool loop) {
    if (!_sprite) return;
    _sprite->stopAllActions();

    std::string file = _resRoot + "/" + name + ".c3b";
    auto anim = cocos2d::Animation3D::create(file);
    if (!anim) { CCLOG("Anim load failed: %s", file.c_str()); return; }

    auto act = cocos2d::Animate3D::create(anim);
    if (loop) _sprite->runAction(cocos2d::RepeatForever::create(act));
    else _sprite->runAction(act);
}
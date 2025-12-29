// 功能描述：
// Enemy类的实现文件，包含所有敌人通用的行为逻辑、状态管理、
// 物理系统交互、碰撞检测和战斗系统实现。

#include "Enemy.h"
#include "EnemyStates.h"
#include "combat/HealthComponent.h"
#include "combat/CombatComponent.h"
#include "combat/Collider.h"
#include "player/Wukong.h"

// 创建Enemy实例的静态工厂方法
// @return Enemy* 创建成功返回敌人指针，失败返回nullptr
Enemy* Enemy::create() {
    auto enemy = new (std::nothrow) Enemy();
    if (enemy && enemy->init()) {
        enemy->autorelease();
        return enemy;
    }
    CC_SAFE_DELETE(enemy);
    return nullptr;
}

// Enemy构造函数，初始化所有成员变量的默认值
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

// Enemy析构函数，负责释放状态机资源
Enemy::~Enemy() {
    if (_stateMachine) {
        delete _stateMachine;
        _stateMachine = nullptr;
    }
}

// 初始化Enemy
// 初始化父类Node，创建状态机、生命值组件和战斗组件
// @return bool 初始化成功返回true，失败返回false
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

    // 初始化 AABB 碰撞器，收缩 XZ 轴到 40%
    _collider.calculateBoundingBox(_sprite, 0.4f);

    // 开启更新循环
    this->scheduleUpdate();
    
    return true;
}

// 每帧更新函数
// 处理状态机更新、重力应用和移动应用
// @param deltaTime 帧间隔时间
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

// 应用重力效果
// 根据重力加速度更新敌人的垂直速度
// @param dt 帧间隔时间
void Enemy::applyGravity(float dt) {
    if (_onGround && _terrainCollider) {
        return;
    }
    _velocity.y -= _gravity * dt;
}

// 应用移动效果
// 处理敌人的位置更新和地形碰撞检测
// @param dt 帧间隔时间
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

// 获取移动速度
// @return float 当前移动速度
float Enemy::getMoveSpeed() const {
    return _moveSpeed;
}

// 获取旋转速度
// @return float 当前旋转速度
float Enemy::getRotateSpeed() const {
    return _rotateSpeed;
}

// 获取视野范围
// @return float 当前视野范围
float Enemy::getViewRange() const {
    return _viewRange;
}

// 获取是否可以移动
// @return bool 是否允许移动且未死亡
bool Enemy::canMove() const {
    return _canMove && !isDead();
}

// 获取是否可以攻击
// @return bool 是否允许攻击且未死亡
bool Enemy::canAttack() const {
    return _canAttack && !isDead();
}

// 获取是否已死亡
// @return bool 是否处于死亡状态
bool Enemy::isDead() const {
    // 直接查询HealthComponent
    return _health ? _health->isDead() : false;
}

// 获取敌人类型
// @return EnemyType 当前敌人类型
Enemy::EnemyType Enemy::getEnemyType() const {
    return _enemyType;
}

// 设置敌人类型
// @param type 要设置的敌人类型
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

// 设置3D位置
// @param position 要设置的3D位置
void Enemy::setPosition3D(const Vec3& position) {
    Node::setPosition3D(position);
}

// 获取3D位置
// @return Vec3 当前3D位置
Vec3 Enemy::getPosition3D() const {
    return Node::getPosition3D();
}

// 获取状态机指针
// @return StateMachine<Enemy>* 状态机指针
StateMachine<Enemy>* Enemy::getStateMachine() const {
    return _stateMachine;
}

// 获取精灵指针
// @return Sprite3D* 3D精灵指针
Sprite3D* Enemy::getSprite() const {
    return _sprite;
}

// 设置出生点位置
// @param pos 出生点的3D位置
void Enemy::setBirthPosition(const Vec3& pos) {
    _birthPosition = pos;
    setPosition3D(pos);
}

// 获取出生点位置
// @return const Vec3& 出生点的3D位置引用
const Vec3& Enemy::getBirthPosition() const {
    return _birthPosition;
}

// 获取最大追击距离
// @return float 最大追击距离
float Enemy::getMaxChaseRange() const {
    return _maxChaseRange;
}
// 初始化状态机
// 创建并配置敌人的状态机，注册所有必要的状态并设置初始状态为Idle
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

// 初始化生命值组件
// 创建健康组件并注册受伤和死亡回调函数，设置默认生命值为100
void Enemy::initHealthComponent() {
    // 创建生命值组件，设置默认生命值为 100
    _health = HealthComponent::create(100.0f);
    if (_health) {
        this->addComponent(_health);
        
        // 设置受伤和死亡回调
        _health->setOnHurtCallback(std::bind(&Enemy::onHurtCallback, this, std::placeholders::_1, std::placeholders::_2));
        _health->setOnDeadCallback(std::bind(&Enemy::onDeadCallback, this, std::placeholders::_1));
    }
}

// 初始化战斗组件
// 创建战斗组件并设置各种战斗属性，包括攻击力、防御力、暴击率和暴击伤害
void Enemy::initCombatComponent() {
    // 创建战斗组件
    _combat = CombatComponent::create();
    if (_combat) {
        // 设置默认战斗属性
        _combat->setAttackPower(10.0f);
        _combat->setDefense(5.0f);
        _combat->setCritRate(0.05f);
        _combat->setCritDamage(1.5f);
        this->addComponent(_combat);
    }
}

/// @brief 受伤回调函数
/// @param damage 受伤数值
/// @param attacker 攻击者节点指针
void Enemy::onHurtCallback(float damage, Node* attacker) {
    // 对于Boss类型的敌人，直接切换到Hit状态以播放受击动画
    if (_enemyType == EnemyType::BOSS && _stateMachine && !isDead()) {
        _stateMachine->changeState("Hit");
        return;
    }

    // 普通敌人的受击处理
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

/// @brief 死亡回调函数
/// @param attacker 攻击者节点指针
void Enemy::onDeadCallback(Node* attacker) {
    // 当HealthComponent检测到死亡时，只做行为与状态切换
    _canMove = false;
    _canAttack = false;

    CCLOG("Enemy onDeadCallback triggered, changing state to Dead");

    if (_stateMachine) {
        _stateMachine->changeState("Dead");
    }
    else {
        CCLOG("WARNING: Enemy state machine is null, cannot change to Dead state!");
    }
}

// 检查是否低生命值状态
// 当生命值百分比低于30%时返回true
// @return bool 是否处于低生命值状态
bool Enemy::isLowHealth() const {
    if (!_health) return false;
    return _health->getHealthPercentage() <= 0.3f;
}

// 获取生命值比例
// 返回当前生命值百分比（0-1之间），如果没有健康组件则返回1.0f
// @return float 生命值比例
float Enemy::getHealthRatio() const {
    return _health ? _health->getHealthPercentage() : 1.0f;
}

// 设置目标
// @param w 悟空角色指针
void Enemy::setTarget(Wukong* w) { _target = w; }

// 获取目标
// @return Wukong* 悟空角色指针
Wukong* Enemy::getTarget() const { return _target; }

// 获取目标的世界坐标
// 获取悟空的世界坐标位置，如果没有目标则返回零向量
// @return Vec3 目标的世界坐标
cocos2d::Vec3 Enemy::getTargetWorldPos() const {
    return _target ? _target->getWorldPosition3D() : cocos2d::Vec3::ZERO;
}

// 获取敌人自身的世界坐标
// 通过矩阵变换获取敌人在世界空间中的坐标位置
// @return Vec3 敌人自身的世界坐标
cocos2d::Vec3 Enemy::getWorldPosition3D() const {
    cocos2d::Vec3 out = cocos2d::Vec3::ZERO;
    cocos2d::Mat4 m = this->getNodeToWorldTransform();
    m.transformPoint(cocos2d::Vec3::ZERO, &out);
    return out;
}

// 更新精灵位置
// 处理精灵的位置更新，确保模型底部正确对齐地面
void Enemy::updateSpritePosition() {
    if (!_sprite) return;
    
    _sprite->updateTransform();
    auto aabb = _sprite->getAABB();
    if (aabb.isEmpty()) {
        _sprite->setPosition3D(Vec3(0, _spriteOffsetY, 0));
        CCLOG("Enemy sprite AABB is empty, using offset: %f", _spriteOffsetY);
    } else {
        // 修正模型位置，确保脚底在地面（y=0），并加上微调偏移
        _sprite->setPosition3D(Vec3(0, -aabb._min.y + _spriteOffsetY, 0));
        CCLOG("Enemy sprite AABB: min.y=%f, max.y=%f, offset=%f, final.y=%f", 
            aabb._min.y, aabb._max.y, _spriteOffsetY, -aabb._min.y + _spriteOffsetY);
    }
}

// 使用指定资源根目录初始化敌人
// @param resRoot 资源根目录，如 "Enemy/enemy1" 或 "Enemy/boss"
// @param modelFile 模型文件，如 "enemy1.c3b" 或 "boss.c3b"
// @return bool 初始化成功返回true，失败返回false
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

    // 更新精灵位置（包含 AABB 修正和初始偏移）
    updateSpritePosition();

    // 初始化 AABB 碰撞器，收缩 XZ 轴到 40%
    _collider.calculateBoundingBox(_sprite, 0.4f);

    // 记录出生点
    _birthPosition = this->getPosition3D();

    // 开启 update
    this->scheduleUpdate();
    return true;
}

// 使用指定资源根目录创建敌人实例的静态工厂方法
// @param resRoot 资源根目录，如 "Enemy/enemy1" 或 "Enemy/boss"
// @param modelFile 模型文件，如 "enemy1.c3b" 或 "boss.c3b"
// @return Enemy* 创建成功返回敌人指针，失败返回nullptr
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

// 播放指定名称的动画
// @param name 动画名称，如 "idle", "chase", "attack" 等
// @param loop 是否循环播放动画
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

// 重置敌人状态
// 将敌人恢复到初始状态，包括生命值和位置
void Enemy::resetEnemy() {
    if (_health) {
        _health->reset();
    }
    this->setPosition3D(_birthPosition);
    if (_stateMachine) {
        _stateMachine->changeState("Idle");
    }
    CCLOG("Enemy %p reset to birth position.", this);
}
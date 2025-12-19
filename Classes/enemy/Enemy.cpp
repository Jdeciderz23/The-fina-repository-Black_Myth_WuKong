#include "Enemy.h"
#include "EnemyStates.h"
#include "combat/HealthComponent.h"
#include "combat/CombatComponent.h"


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
    , _targetPosition(Vec3::ZERO) {
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
    
    // 创建3D精灵（使用Cocos2d-x内置的立方体作为占位符）
    _sprite = Sprite3D::create("cc3d:cube.obj");
    if (_sprite) {
        _sprite->setColor(Color3B::RED);
        this->addChild(_sprite);
    }
    
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
        case EnemyType::ELITE:
            _moveSpeed = 70.0f;
            _rotateSpeed = 240.0f;
            _viewRange = 250.0f;
            break;
        case EnemyType::BOSS:
            _moveSpeed = 40.0f;
            _rotateSpeed = 120.0f;
            _viewRange = 300.0f;
            break;
    }
}

void Enemy::setPosition(const Vec3& position) {
    Node::setPosition3D(position);
    if (_sprite) {
        _sprite->setPosition3D(position);
    }
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
    
    // 初始化为待机状态
    _stateMachine->init(new EnemyIdleState());
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
        this->addComponent(_combat);
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
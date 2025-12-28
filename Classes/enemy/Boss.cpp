#include "Boss.h"
#include "BossAI.h"
// 后面写 BossAI 时再在 Boss.cpp 里 include 并 new BossAI(this) 也行
#include "BossStates.h"
#include "cocos2d.h"

USING_NS_CC;

Boss* Boss::createBoss(const std::string& resRoot, const std::string& modelFile) {
    auto b = new (std::nothrow) Boss();
    if (b && b->initBoss(resRoot, modelFile)) {
        b->autorelease();
        return b;
    }
    CC_SAFE_DELETE(b);
    return nullptr;
}

Boss::Boss() = default;

Boss::~Boss() {
    if (_ai) {
        delete _ai;
        _ai = nullptr;
    }
}

bool Boss::initBoss(const std::string& resRoot, const std::string& modelFile) {
    // 复用 Enemy 的资源加载（你已经实现了 initWithResRoot）
    if (!this->initWithResRoot(resRoot, modelFile)) {
        return false;
    }

    // 设为 Boss 类型（会调基础速度/视野等）
    setEnemyType(EnemyType::BOSS);

    // 你可以再覆盖 Boss 的数值（按你手感调）
    // 注意：这些成员在 Enemy 里是 protected，Boss 继承可直接改
    _viewRange = 500.0f;      // 约 15m（假设 1m=100）
    _maxChaseRange = 500.f; // Boss 基本不回家

    _phase = 1;
    _moveMul = 1.0f;
    _dmgMul = 1.0f;
    _busy = false;
    _pendingSkill.clear();

    // AI 先不在这里 new，避免你还没写 BossAI 就编译炸
    // 等你写 BossAI.cpp 时，再在 BaseScene 创建 boss 后 boss->setAI(new BossAI(boss));
    return true;
}

void Boss::update(float dt) {
    // 先走 Enemy（更新状态机）
    Enemy::update(dt);

    // AI 决策（可选）
    if (_ai) {
        _ai->update(dt);  // 等你写 BossAI.h/.cpp 后再打开这行
    }
}

void Boss::initStateMachine() {
    _stateMachine = new StateMachine<Enemy>(this);

    _stateMachine->registerState(new BossIdleState());
    _stateMachine->registerState(new BossChaseState());
    _stateMachine->registerState(new BossAttackState());
    _stateMachine->registerState(new BossPhaseChangeState());
    _stateMachine->registerState(new BossHitState());
    _stateMachine->registerState(new BossDeadState());

    _stateMachine->changeState("Chase");
}

float Boss::distanceToPlayer() const {
    if (!getTarget()) return 1e9f;
    return (getTargetWorldPos() - getWorldPosition3D()).length();
}

void Boss::setAI(BossAI* ai) {
    if (_ai == ai) return;
    if (_ai) delete _ai;
    _ai = ai;
}

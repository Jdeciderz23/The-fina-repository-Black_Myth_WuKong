#include "Boss.h"
#include "BossAI.h"
#include "BossStates.h"
#include "cocos2d.h"
#include "scene_ui/UIManager.h"
#include "combat/HealthComponent.h"

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
    if (!this->initWithResRoot(resRoot, modelFile)) {
        return false;
    }

    setEnemyType(EnemyType::BOSS);

    _viewRange = 500.0f;
    _maxChaseRange = 500.f;

    _phase = 1;
    _moveMul = 1.0f;
    _dmgMul = 1.0f;
    _busy = false;
    _hasHealed = false;
    _pendingSkill.clear();

    if (_health) {
        _health->setOnHealthChangeCallback([this](float current, float max) {
            float percent = current / max;
            UIManager::getInstance()->updateBossHP(percent);

            if (!_hasHealed && percent <= 0.5f && !isDead()) {
                _hasHealed = true;
                _phase = 2; // 设置为第二阶段
                _health->fullHeal();
                CCLOG("Boss: Phase 2 triggered! HP restored to 100%%");
                
                if (_stateMachine) {
                    _stateMachine->changeState("PhaseChange");
                }
            }
        });
    }

    return true;
}

void Boss::resetEnemy() {
    Enemy::resetEnemy();
    _phase = 1;
    _hasHealed = false;
    _busy = false;
    _pendingSkill.clear();
    
    // 重置 Boss 血条 UI
    UIManager::getInstance()->updateBossHP(1.0f);
    UIManager::getInstance()->showBossHPBar(false);
    
    CCLOG("Boss: Reset to initial state");
}

void Boss::update(float dt) {
    Enemy::update(dt);

    if (_ai) {
        _ai->update(dt);
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

#include "Boss.h"
#include "BossAI.h"
#include "BossStates.h"
#include "cocos2d.h"
#include "scene_ui/UIManager.h"
#include "combat/HealthComponent.h"

USING_NS_CC;

// 创建Boss实例的工厂方法
// @param resRoot 资源根目录路径
// @param modelFile 模型文件路径
// @return 返回初始化成功的Boss指针，如果失败则返回nullptr
Boss* Boss::createBoss(const std::string& resRoot, const std::string& modelFile) {
  auto b = new (std::nothrow) Boss();
  if (b && b->initBoss(resRoot, modelFile)) {
    b->autorelease();
    return b;
  }
  CC_SAFE_DELETE(b);
  return nullptr;
}

// 默认构造函数
Boss::Boss() = default;

// 析构函数，清理AI资源
Boss::~Boss() {
  if (_ai) {
    delete _ai;
    _ai = nullptr;
  }
}

// 初始化Boss
// 设置Boss基本属性、状态和监听回调
// @param resRoot 资源根目录路径
// @param modelFile 模型文件路径
// @return 初始化是否成功
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

      // 触发第二阶段逻辑
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

// 重置Boss到初始状态
// 恢复血量UI、重置内部状态变量
void Boss::resetEnemy() {
  Enemy::resetEnemy();
  _phase = 1;
  _hasHealed = false;
  _busy = false;
  _pendingSkill.clear();
  
  // 重置Boss血条UI
  UIManager::getInstance()->updateBossHP(1.0f);
  UIManager::getInstance()->showBossHPBar(false);
  
  CCLOG("Boss: Reset to initial state");
}

// 每帧更新Boss状态和AI逻辑
// 调用父类更新并执行AI决策
// @param dt 帧间隔时间
void Boss::update(float dt) {
  Enemy::update(dt);

  if (_ai) {
    _ai->update(dt);
  }
}

// 初始化Boss状态机
// 注册Boss特有的状态
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

// 计算Boss到玩家的距离
// @return Boss和玩家之间的距离
float Boss::distanceToPlayer() const {
  if (!getTarget()) return 1e9f;
  return (getTargetWorldPos() - getWorldPosition3D()).length();
}

// 设置Boss的AI控制器
// 管理AI的生命周期
// @param ai BossAI控制器指针
void Boss::setAI(BossAI* ai) {
  if (_ai == ai) return;
  if (_ai) delete _ai;
  _ai = ai;
}

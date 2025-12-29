// BossStates.cpp
// 实现Boss实体的所有状态类，包括Idle、Chase、Attack等状态
#include "BossStates.h"
#include "Boss.h"
#include "Wukong.h"
#include "cocos2d.h"
#include <algorithm>
#include <cmath>

USING_NS_CC;

// 定义π常量
static constexpr float PI_F = 3.1415926f;

// 米到世界单位的转换辅助函数
// @param meters 米数
// @return 对应的世界单位
static inline float M(float meters) { return meters * 100.0f; }

// 世界坐标系到父节点坐标系的转换
// @param node 节点对象
// @param worldPos 世界坐标
// @return 父节点空间中的坐标
static Vec3 worldToParentSpace(const Node* node, const Vec3& worldPos) {
  auto p = node->getParent();
  if (!p) return worldPos;
  Vec3 out = Vec3::ZERO;
  Mat4 inv = p->getWorldToNodeTransform();
  inv.transformPoint(worldPos, &out);
  return out;
}

// 使敌人面向指定世界方向
// @param e 敌人对象
// @param dirW 世界坐标系下的方向向量
// @param yOffsetDeg Y轴旋转偏移角度
static void faceToWorldDir(Enemy* e, Vec3 dirW, float yOffsetDeg = 45.0f) {
  if (!e || !e->getSprite()) return;
  dirW.y = 0;
  if (dirW.lengthSquared() < 1e-6f) return;
  dirW.normalize();

  float yaw = atan2f(dirW.x, dirW.z) * 180.0f / PI_F + yOffsetDeg;
  e->getSprite()->setRotation3D(Vec3(0, yaw, 0));
}

// 获取指定技能的配置
// @param skill 技能名称
// @return 对应的技能配置结构体
static BossSkillConfig getCfg(const std::string& skill) {
  if (skill == "Combo3") {
    return BossSkillConfig{
      "Combo3", "combo3",
      0.35f, 0.0f, 0.50f, 0.65f,  // 增加所有时间参数以延长动画播放时间
      0.f, M(1.2f), 12.f, false
    };
  }
  if (skill == "DashSlash") {
    return BossSkillConfig{
      "DashSlash", "rush",
      0.30f, 0.25f, 0.15f, 0.50f,
      M(2.0f), M(1.4f), 16.f, true
    };
  }
  if (skill == "GroundSlam") {
    return BossSkillConfig{
      "GroundSlam", "groundslam",
      0.60f, 0.0f, 0.20f, 0.80f,
      0.f, M(1.7f), 20.f, false
    };
  }
  if (skill == "Roar") {
    return BossSkillConfig{
      "Roar", "roar",
      1.00f, 0.0f, 0.0f, 0.0f,
      0.f, 0.f, 0.f, false
    };
  }
  if (skill == "LeapSlam") {
    return BossSkillConfig{
      "LeapSlam", "rush",  // 首先播放rush动画
      0.35f, 0.35f, 0.15f, 1.30f,  // 延长recovery时间以容纳第二个动画
      M(2.0f), M(3.0f), 26.f, true
    };
  }

  return getCfg("Combo3");  // 默认返回Combo3配置
}

// 应用一次伤害判定
// @param enemy 敌人对象
// @param cfg 技能配置
// @param dmgMul 伤害倍率
static void applyHitOnce(Enemy* enemy, const BossSkillConfig& cfg, float dmgMul) {
  if (!enemy) return;

  Vec3 pW = enemy->getTargetWorldPos();
  Vec3 eW = enemy->getWorldPosition3D();
  float dist = (pW - eW).length();

  if (dist <= cfg.hitRadius) {
    float dmg = cfg.damage * dmgMul;
    CCLOG("[BossAttack] %s HIT dmg=%.2f dist=%.1f", cfg.skill.c_str(), dmg, dist);

    // 使用与普通敌人相同的减血逻辑
    auto target = enemy->getTarget();
    if (target && target->getHealth()) {
      target->getHealth()->takeDamage(dmg, enemy);
      CCLOG("Boss dealt %.2f damage to player", dmg);
    }
  }
  else {
    CCLOG("[BossAttack] %s miss dist=%.1f", cfg.skill.c_str(), dist);
  }
}

// ================= Idle =================
// 进入BossIdleState状态
// @param enemy 敌人对象，这里是Boss实例
void BossIdleState::onEnter(Enemy* enemy) {
  if (!enemy) return;
  enemy->playAnim("idle", true);  // 播放idle动画，循环播放

  auto boss = static_cast<Boss*>(enemy);
  boss->setBusy(false);  // 设置Boss为非忙碌状态
}

// 更新BossIdleState状态
// @param enemy 敌人对象，这里是Boss实例
// @param dt 帧间隔时间
void BossIdleState::onUpdate(Enemy* enemy, float) {
  if (!enemy) return;
  if (enemy->isDead()) {
    enemy->getStateMachine()->changeState("Dead");  // 如果Boss死亡，切换到Dead状态
    return;
  }
}

// 退出BossIdleState状态
// @param enemy 敌人对象，这里是Boss实例
void BossIdleState::onExit(Enemy*) {}

// ================= Chase =================
// 进入BossChaseState状态
// @param enemy 敌人对象，这里是Boss实例
void BossChaseState::onEnter(Enemy* enemy) {
  if (!enemy) return;
  enemy->playAnim("chase", true);  // 播放chase动画，循环播放

  auto boss = static_cast<Boss*>(enemy);
  boss->setBusy(false);  // 设置Boss为非忙碌状态
}

// 更新BossChaseState状态
// @param enemy 敌人对象，这里是Boss实例
// @param dt 帧间隔时间
void BossChaseState::onUpdate(Enemy* enemy, float dt) {
  if (!enemy) return;

  if (enemy->isDead()) {
    enemy->getStateMachine()->changeState("Dead");  // 如果Boss死亡，切换到Dead状态
    return;
  }

  Vec3 pW = enemy->getTargetWorldPos();
  if (pW == Vec3::ZERO) {
    enemy->getStateMachine()->changeState("Idle");  // 如果没有目标，切换到Idle状态
    return;
  }

  auto boss = static_cast<Boss*>(enemy);
  Vec3 eW = enemy->getWorldPosition3D();

  Vec3 dir = pW - eW;
  dir.y = 0;
  if (dir.lengthSquared() < 1e-6f) return;
  dir.normalize();

  faceToWorldDir(enemy, dir);  // 让Boss面向玩家方向

  float speed = enemy->getMoveSpeed() * boss->getMoveMul();  // 计算实际移动速度
  Vec3 newW = eW + dir * speed * dt;  // 计算新位置
  newW.y = eW.y;  // 保持Y轴位置不变

  enemy->setPosition3D(worldToParentSpace(enemy, newW));  // 设置新位置
}

// 退出BossChaseState状态
// @param enemy 敌人对象，这里是Boss实例
void BossChaseState::onExit(Enemy*) {}

// ================= PhaseChange =================
// 进入BossPhaseChangeState状态
// @param enemy 敌人对象，这里是Boss实例
void BossPhaseChangeState::onEnter(Enemy* enemy) {
  if (!enemy) return;

  _timer = 0.f;  // 重置计时器
  CCLOG("Boss phase change triggered, playing roar animation");

  // 播放roar.c3b动画，这是BOSS血量降到50%以下时的特殊动画
  enemy->playAnim("roar", false);

  auto boss = static_cast<Boss*>(enemy);
  boss->setBusy(true);  // 设置Boss为忙碌状态
}

// 更新BossPhaseChangeState状态
// @param enemy 敌人对象，这里是Boss实例
// @param dt 帧间隔时间
void BossPhaseChangeState::onUpdate(Enemy* enemy, float dt) {
  if (!enemy) return;

  if (enemy->isDead()) {
    enemy->getStateMachine()->changeState("Dead");  // 如果Boss死亡，切换到Dead状态
    return;
  }

  _timer += dt;
  // 延长时间从1.0秒到1.5秒，确保roar.c3b动画完整播放
  if (_timer >= 3.5f) {
    auto boss = static_cast<Boss*>(enemy);
    boss->applyPhase2Buff(1.2f, 1.15f);  // 应用第二阶段的属性提升
    boss->setBusy(false);  // 设置Boss为非忙碌状态

    enemy->getStateMachine()->changeState("Chase");  // 阶段转换完成后切换到Chase状态
  }
}

// 退出BossPhaseChangeState状态
// @param enemy 敌人对象，这里是Boss实例
void BossPhaseChangeState::onExit(Enemy*) {}

// ================= Attack =================
// 进入BossAttackState状态
// @param enemy 敌人对象，这里是Boss实例
void BossAttackState::onEnter(Enemy* enemy) {
  if (!enemy) return;

  _timer = 0.f;  // 重置计时器
  _didHit = false;  // 重置伤害判定标志
  _stage = Stage::Windup;  // 设置初始阶段为前摇阶段

  auto boss = static_cast<Boss*>(enemy);
  boss->setBusy(true);  // 设置Boss为忙碌状态

  std::string skill = boss->hasPendingSkill() ? boss->consumePendingSkill() : "Combo3";  // 获取要使用的技能
  _cfg = getCfg(skill);  // 获取技能配置

  enemy->playAnim(_cfg.anim, false);  // 播放技能动画

  _startW = enemy->getWorldPosition3D();  // 记录起始位置

  _targetW = enemy->getTargetWorldPos();  // 获取目标位置
  if (_cfg.moveTime > 0.f && _cfg.lockTarget) {
    // 如果是移动类技能且需要锁定目标，则计算跳跃目标位置
    Vec3 toP = _targetW - _startW;
    toP.y = 0;
    if (toP.lengthSquared() > 1e-6f) {
      float len = toP.length();
      toP.normalize();
      float want = std::max(0.0f, len - _cfg.dashDistance);
      _targetW = _startW + toP * want;
    }
  }
}

// 更新BossAttackState状态
// @param enemy 敌人对象，这里是Boss实例
// @param dt 帧间隔时间
void BossAttackState::onUpdate(Enemy* enemy, float dt) {
  if (!enemy) return;

  if (enemy->isDead()) {
    enemy->getStateMachine()->changeState("Dead");  // 如果Boss死亡，切换到Dead状态
    return;
  }

  auto boss = static_cast<Boss*>(enemy);

  _timer += dt;

  // 阶段切换辅助函数
  auto gotoStage = [&](Stage s) {
    _stage = s;
    _timer = 0.f;
    if (s == Stage::Active) _didHit = false;  // 进入伤害判定阶段时重置伤害标志
    };

  // 1) 前摇阶段
  if (_stage == Stage::Windup) {
    if (_timer >= _cfg.windup) {
      if (_cfg.moveTime > 0.f) gotoStage(Stage::Move);  // 如果是移动类技能，进入移动阶段
      else gotoStage(Stage::Active);  // 否则直接进入伤害判定阶段
    }
    return;
  }

  // 2) 移动阶段
  if (_stage == Stage::Move) {
    float denom = std::max(0.0001f, _cfg.moveTime);
    float t01 = std::min(1.0f, _timer / denom);  // 计算移动进度

    // 计算当前位置
    Vec3 newW = _startW + (_targetW - _startW) * t01;
    newW.y = enemy->getWorldPosition3D().y;

    faceToWorldDir(enemy, _targetW - _startW);  // 让Boss面向目标方向
    enemy->setPosition3D(worldToParentSpace(enemy, newW));  // 设置新位置

    if (_timer >= _cfg.moveTime) {
      gotoStage(Stage::Active);  // 移动完成后进入伤害判定阶段
    }
    return;
  }

  // 3) 伤害判定阶段
  if (_stage == Stage::Active) {
    if (!_didHit) {
      applyHitOnce(enemy, _cfg, boss->getDmgMul());  // 应用伤害判定
      _didHit = true;  // 设置伤害判定标志
    }

    if (_timer >= _cfg.active) {
      gotoStage(Stage::Recovery);  // 伤害判定完成后进入后摇阶段

      // 如果是LeapSlam技能，播放groundslam动画作为第二个动画
      if (_cfg.skill == "LeapSlam") {
        enemy->playAnim("groundslam", false);
      }
    }
    return;
  }

  // 4) 后摇阶段
  if (_stage == Stage::Recovery) {
    if (_timer >= _cfg.recovery) {
      boss->setBusy(false);  // 设置Boss为非忙碌状态
      enemy->getStateMachine()->changeState("Chase");  // 后摇完成后切换到Chase状态
    }
    return;
  }
}

// 退出BossAttackState状态
// @param enemy 敌人对象，这里是Boss实例
void BossAttackState::onExit(Enemy* enemy) {
  if (!enemy) return;
  auto boss = static_cast<Boss*>(enemy);
  boss->setBusy(false);  // 设置Boss为非忙碌状态
}

// ================= Hit =================
// 进入BossHitState状态
// @param enemy 敌人对象，这里是Boss实例
void BossHitState::onEnter(Enemy* enemy) {
    if (!enemy) return;
    _timer = 0.f;  // 重置计时器

    auto boss = static_cast<Boss*>(enemy);
    boss->setBusy(true);  // 设置Boss为忙碌状态

    // 播放受击动画，确保使用正确的文件名
    enemy->playAnim("hited", false);
    CCLOG("Boss hit state triggered, playing hited animation");
}

// 更新BossHitState状态
// @param enemy 敌人对象，这里是Boss实例
// @param dt 帧间隔时间
void BossHitState::onUpdate(Enemy* enemy, float dt) {
    if (!enemy) return;

    if (enemy->isDead()) {
        enemy->getStateMachine()->changeState("Dead");  // 如果Boss死亡，切换到Dead状态
        return;
    }

    _timer += dt;
    // 延长受击状态时间从0.5秒到0.8秒，确保hited.c3b动画能够完整播放
    if (_timer >= 0.8f) {
        auto boss = static_cast<Boss*>(enemy);
        boss->setBusy(false);  // 设置Boss为非忙碌状态
        enemy->getStateMachine()->changeState("Chase");  // 切换到Chase状态
    }
}

// 退出BossHitState状态
// @param enemy 敌人对象，这里是Boss实例
void BossHitState::onExit(Enemy* enemy) {
    if (!enemy) return;
    auto boss = static_cast<Boss*>(enemy);
    boss->setBusy(false);  // 设置Boss为非忙碌状态
}

// ================= Dead =================
/// 进入BossDeadState状态
/// @param enemy 敌人对象，这里是Boss实例
void BossDeadState::onEnter(Enemy* enemy) {
    if (!enemy) return;

    auto boss = static_cast<Boss*>(enemy);
    boss->setBusy(true);

    CCLOG("Boss entering death state, playing dying animation");

    // 播放死亡动画 dying.c3b
    enemy->playAnim("dying", false);

    // 与普通敌人一样的死亡处理流程：发送事件 + 延迟移除
    // 延长延迟时间至3秒以确保死亡动画完整播放
    enemy->runAction(Sequence::create(
        DelayTime::create(3.0f),
        CallFunc::create([enemy]() {
            CCLOG("Boss is being removed after death animation");

            // 通知游戏管理器从敌人列表中移除此敌人
            // 通过一个全局事件通知系统
            cocos2d::EventCustom event("enemy_died");
            event.setUserData(enemy);
            cocos2d::Director::getInstance()->getEventDispatcher()->dispatchEvent(&event);

            // 执行实际的移除操作
            enemy->removeFromParent();
            }),
        nullptr
    ));
}

/// 更新BossDeadState状态
/// @param enemy 敌人对象，这里是Boss实例
/// @param dt 帧间隔时间
void BossDeadState::onUpdate(Enemy*, float) {}

/// 退出BossDeadState状态
/// @param enemy 敌人对象，这里是Boss实例
void BossDeadState::onExit(Enemy*) {}

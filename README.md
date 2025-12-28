# 黑神话：悟空（Cocos2d-x 4.0 引擎实现）

> 第三人称动作冒险 + 敌人 AI + Boss 战（含二阶段）  
> 玩家操控悟空在 3D 场景中移动、翻滚闪避、三段连招与技能释放，对抗敌人/Boss，并接入地形碰撞与基础 UI 框架。

---

## 0. 项目目标与说明

本项目选题：**黑神话：悟空**。  
核心目标：完成第三人称动作战斗的最小可玩闭环，并体现 C++ 面向对象设计与工程组织能力。

---

## 1. 功能对照表（项目要求）

### 1.1 基础功能

- **角色系统**
  - ✅ 支持主角模型（悟空 `WuKong/wukong.c3b`）
  - ✅ 基础动作：行走 / 奔跑 / 跳跃 / 翻滚（闪避）
  - ✅ 基础攻击：至少 3 段连招（Attack1→Attack2→Attack3）
  - ✅ 基础状态切换：Idle / 攻击 / 受击 / 死亡（并含 Move/Roll/Skill 等）

- **战斗系统**
  - ✅ 悟空生命值 / 敌人生命值（`HealthComponent`）
  - ✅ 受击硬直反馈（悟空 `HurtState`，敌人 `EnemyHitState`）
  - ✅ 至少一种技能（悟空 `SkillState` + `castSkill()`）
  - ✅ 战斗判定：近战 AABB 判定（`CharacterCollider` + `CombatComponent::executeMeleeAttack`）
  - ✅ Boss 对玩家的伤害： **BossAttack**（`BossStates.cpp` 中），接入 `HealthComponent/CombatComponent`

- **敌人 AI**
  - ✅ 敌人 AI（见 `EnemyStates.cpp`）：Idle / Patrol / Chase / Attack / Hit / Dead / Return
  - ✅ Boss 二阶段/怒气思路：Phase + Buff + BossAI（技能权重、CD、距离分层）

- **场景功能**
  - ✅ 支持 3D 场景移动 + 第三人称相机（轨道相机 Orbit Camera）
  - ✅ 地形碰撞/落地贴地：`TerrainCollider`（射线 + 三角网格 + 网格加速）
  - ✅ 场景切换点 / 动态音乐：主场景接入 ， 场地中有两处传送点可自由传送

- **UI 功能（要求：标题菜单/血条/技能冷却等）**
  - ✅ 工程已引用 `scene_ui/UIManager.h`（`EnemyStates.cpp`）
  - ✅ UI 展示完整度以仓库 `UIManager.*` 

- **死亡重开 / 暂停继续**
  - ✅ 死亡后UI界面可重生 ， UI界面可暂停再继续

---

## 2. 操作说明（键位）

控制逻辑：`PlayerController / InputController`

### 2.1 移动与战斗
- **W/A/S/D**：移动
- **Shift**：奔跑
- **Space**：跳跃
- **C**：翻滚/闪避（Roll）
- **鼠标左键**：普通攻击（支持三段连招输入缓冲）
- **数字键 1**：释放技能（进入 SkillState）
- **R**：重置相机参数（yaw/pitch/distance）

### 2.2 相机（第三人称轨道相机）
- **按住鼠标右键**：旋转镜头（隐藏鼠标并锁定旋转）

---

## 3. 工程结构（按模块）

### 结构设计要点
- **core**：负责“程序骨架”（应用入口、场景切换、事件系统、状态机模板），保证逻辑正常。
- **player / enemy**：只关心各自实体与状态逻辑。
- **combat**：把“伤害、碰撞、生命值”做成通用组件，玩家和敌人统一复用，减少耦合。
- **scene_ui**：负责“展示与承载”（场景组织、UI与音频），与战斗/AI通过事件或接口连接。

> 以下结构来自当前工程 `Classes/` 目录（与 VS 工程视图一致），并按模块说明每个文件职责。

```text
Classes/
├─ combat/                         # 战斗与碰撞、生命值、技能
│  ├─ CharacterCollider.h          # 角色/敌人的AABB碰撞体（用于近战判定、推开等）
│  ├─ Collider.h / Collider.cpp    # TerrainCollider：地形三角网格 + 网格加速 + 射线求交
│  ├─ CombatComponent.h/.cpp       # 战斗组件：近战命中检测、伤害结算入口
│  ├─ HealthComponent.h/.cpp       # 生命值组件：扣血/死亡/无敌时间/回调等
│  └─ Skill.h/.cpp                 # 技能数据与执行逻辑（与SkillState/释放接口对接）
│
├─ core/                           # 核心框架：应用入口、场景管理、事件、状态机
│  ├─ StateMachine.h               # 模板状态机：StateMachine<T> / 切换/更新
│  ├─ BaseState.h                  # 状态基类：enter/update/exit 等接口
│  ├─ SceneManager.h/.cpp          # 场景管理：切场景、维护当前场景引用等
│  ├─ GameApp.h/.cpp               # 游戏主入口封装：初始化/资源/管理器挂载等
│  ├─ EventManager.h/.cpp          # 事件系统：广播/订阅/触发（用于解耦模块）
│  └─ AreaManager.h/.cpp           # 区域/触发管理：区域进入/离开（可用于关卡逻辑/刷怪点）
│
├─ enemy/                          # 敌人与Boss：实体类 + AI状态 + BossAI决策
│  ├─ Enemy.h/.cpp                 # 敌人基类：移动/感知/追击距离/出生点/组件引用
│  ├─ EnemyStates.h/.cpp           # 普通敌人状态机：Idle/Patrol/Chase/Attack/Hit/Dead/Return
│  ├─ Boss.h/.cpp                  # Boss：继承Enemy，含phase、倍率buff、pendingSkill等
│  ├─ BossAI.h/.cpp                # Boss决策：距离区间+权重随机+冷却+阶段过滤
│  └─ BossStates.h/.cpp            # Boss状态：Idle/Chase/PhaseChange/Attack/Hit/Dead等
│
├─ player/                         # 玩家（悟空）：角色基类、输入、状态与动画
│  ├─ Character.h/.cpp             # 角色基类：位移/重力/贴地/与敌人推开/状态机驱动
│  ├─ Wukong.h/.cpp                # 悟空实体：模型动画加载、状态切换入口、技能释放等
│  ├─ WukongStates.h               # 悟空状态集合：Idle/Move/Jump/Roll/Attack1-3/Hurt/Dead/Skill
│  └─ InputController.h/.cpp       # 键鼠输入 + 第三人称轨道相机（右键旋转、滚轮缩放）
│
├─ scene_ui/                       # 场景与UI：基础场景、UI管理、音频
│  ├─ BaseScene.h/.cpp             # 场景基类：初始化玩家/敌人/Boss/地形/更新循环等
│  ├─ UIManager.h/.cpp             # UI管理：血条/提示/菜单（以当前实现为准）
│  └─ AudioManager.h/.cpp          # 音频管理：BGM/音效播放封装（以当前实现为准）
│
└─ AppDelegate.h/.cpp              # Cocos2d-x 应用委托：窗口/导演/GLView/主场景启动
```

---

## 4. 核心系统设计

### 4.1 状态机（StateMachine<T>）
- 悟空与敌人均使用状态机组织逻辑，状态名字符串切换：`"Idle" / "Move" / "Attack1" ...`
- 优点
  - **输入/AI** 只产出“意图”，状态负责“动作执行细节”
  - 新增动作/技能只需新增 State 并注册

### 4.2 角色移动、重力与地形贴地（Character / TerrainCollider）
- `Character::applyGravity()`：重力加速度（y 轴）
- `Character::applyMovement(dt)`：
  - 先水平位移，再做敌人 AABB 碰撞的推开修正（避免穿模/卡住）
  - 再做向下射线检测，贴合地形高度（`TerrainCollider::rayIntersects`）
  - 支持台阶高度阈值（`maxStepHeight`），避免小落差抖动
  - 当坡度过陡会阻止水平移动（防止“爬墙”）

> 地形碰撞数据来源：  
> `TerrainCollider::create(terrainSprite3D, "xxx.obj")` 优先读取 obj 三角形；失败则用 AABB 底面做保底平面。

### 4.3 战斗系统（CombatComponent + Collider）
- **近战命中**：攻击者 AABB vs 目标 AABB（由 `CombatComponent::executeMeleeAttack` 统一处理）
- **悟空连招**：`AttackState(step)` 内部按动画时长比例设定“输入窗口”和“伤害窗口”
- **敌人攻击**：`EnemyAttackState` 在动画约 0.3s 时做一次近战判定

### 4.4 Boss 系统（二阶段 + 技能决策）
- `BossAI` 每 **0.1s** 决策一次（有冷却/距离/阶段过滤）
- 二阶段触发条件：`HP <= 50%` → 切换 `PhaseChange`（怒吼 1s）→ 上 Buff
  - 移速倍率：`1.2`
  - 伤害倍率：`1.15`

#### Boss 技能列表
| 技能名 | 距离区间 | CD | 权重 | 阶段 |
|---|---:|---:|---:|---|
| Combo3 | 0 ~ 2.5m | 2s | 1.0 | Phase1/2 |
| DashSlash | 2.5m ~ 6m | 4s | 0.9 | Phase1/2 |
| GroundSlam | 0 ~ 6m | 6s | 0.7 | Phase1/2 |
| LeapSlam | 2.5m ~ 远 | 8s | 1.2 | Phase2 |

> 距离单位：代码默认 `1m ≈ 100 world units`（ `BossAI.cpp / BossStates.cpp` 中 `M(meters)`）。

#### Boss 攻击实现说明
- `BossStates.cpp` 的 `BossAttackState` 已实现：
  - Windup → Move（Dash/Leap）→ Active（命中判定一次）→ Recovery
- 调用玩家 `HealthComponent::takeDamage`  
  接入伤害接口，实现 Boss 扣血。

---

## 5. 编译与运行

### 5.1 环境依赖
- Windows 10/11
- Visual Studio 2022（MSVC v143）
- CMake（生成 VS 工程）
- Cocos2d-x 4.0

### 5.2 构建流程
1. CMake 生成 VS 工程（x64）
2. 选择启动项目（Game / MyGame，以仓库为准）编译运行
3. 确保 `Resources/` 在运行目录可被正确访问（否则模型/动画会加载失败）

### 5.3 资源路径自检
- 悟空模型与动画硬编码路径示例：
  - `WuKong/wukong.c3b`
  - `WuKong/Idle.c3b`
  - `WuKong/attack1.c3b` / `attack2.c3b` / `attack3.c3b`
  - `WuKong/Roll.c3b` / `WuKong/Skills.c3b` / `WuKong/Hurt.c3b` / `WuKong/Death.c3b`

若资源目录命名不同，需要同步修改 `Wukong.cpp` 的加载路径。

---

## 6. C++ 特性符合性

本项目明确使用：
- ✅ STL 容器：`std::vector / std::unordered_map / std::string`
- ✅ 类、继承与多态：`Character -> Wukong`，`Enemy -> Boss`，大量 `virtual/override`
- ✅ 模板：`BaseState<T>`、`StateMachine<T>`
- ✅ C++11+：`enum class`、`nullptr`、lambda、`override`、智能指针（状态机内）
- ✅ C++ 风格类型转换：`static_cast / dynamic_cast`
- ✅ const 合理使用：大量 getter / helper `const`

---

## 7. 已知问题 / TODO

### 7.1 已知问题
- JumpState 当前主要依赖动画播放结束回调切换（落地判定与物理联动仍可完善）
- Wukong 的动作动画必须执行完一个动画才能进行下一个动画，动画不够连续
- AttackState 中3段攻击是通过缓冲区进行输入，如果连点三次攻击就会完成三段并不能随时选择是否要进行攻击
- 由于 cocos2d 引擎的限制，我们的模型不能选择骨骼数多、节点数多的模型；场景内生成的模型也不能拥有太多不同套的骨骼，否则以上问题都会导致导入的模型渲染错乱无法正常显示
- 会产生位移的动画加载过程中，节点无法跟随移动，导致节点计算出现偏差

### 7.2 推荐的下一步完善点
1. **技能/攻击命中反馈**：命中特效、震屏、音效、短暂停顿（hit stop）
2、**boss/小怪的动作逻辑**：根据距离、血量等多个参数共同决策下一步做什么

---

## 8. 协作规范（PR/分支）

- feature 分支开发 → PR → 合并到 main
- 风格：Google C++ Style（命名、const、注释、cast）

---

## 9. 参考资料
```text
Cocos2d-x Manual : https://docs.cocos.com/cocos2d-x/manual/zh/
Git Book : https://git-scm.com/book/zh/v2
GitHub PR 简介: https://www.jianshu.com/p/be9f0484af9d


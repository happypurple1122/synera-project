# Synera: Synergy Auto-Arena

> 高级程序设计 2026 春 · 自走棋项目
>


---

## 一、项目阶段完成度

| 阶段 | 内容 | 状态 |
|------|------|------|
| Phase 1 | 棋盘、备战区、Unit 基类、拖拽交互 | 完成 |
| Phase 2 | 战斗系统（A* 寻路、帧驱动循环、技能、胜负结算） | 完成 |
| Phase 3 | 商店、羁绊、升星、装备、存档、人口/等级 | 完成 |
| Phase 4 | 装备合成树、高级装备、回复/复活机制 | 完成 |

---

## 二、文件树结构

```
Synera/
├── CMakeLists.txt
├── CMakePresets.json
├── .clangd
├── README.md
│
├── src/
│   ├── main.cpp
│   │
│   ├── core/
│   │   ├── Unit.h / .cpp
│   │   ├── Hero_Warrior.h/.cpp
│   │   ├── Hero_Mage.h/.cpp
│   │   ├── Hero_Archer.h/.cpp
│   │   ├── Hero_Assassin.h/.cpp
│   │   ├── Player.h / .cpp
│   │   ├── Equipment.h / .cpp
│   │   ├── BattleSystem.h/.cpp
│   │   ├── GameManager.h/.cpp
│   │   ├── Shop.h / .cpp
│   │   ├── SynergySystem.h/.cpp
│   │   ├── StarUpSystem.h/.cpp
│   │   ├── SaveLoadManager.h/.cpp
│   │   ├── EnemyWaveGenerator.h/.cpp
│   │   └── PathFinder.h / .cpp
│   │
│   └── gui/
│       ├── MainWindow.h / .cpp
│       ├── BoardWidget.h / .cpp
│       ├── BenchWidget.h / .cpp
│       ├── DragDropMgr.h / .cpp
│       ├── ShopWidget.h / .cpp
│       ├── EquipmentBarWidget.h/.cpp
│       └── SynergyPanelWidget.h/.cpp
```

---

## 三、核心类与数据结构

### 3.1 继承体系

```
Unit (抽象基类)
├── Hero_Warrior  (战士)  — 高HP, 近战, 技能：范围眩晕
├── Hero_Mage     (法师)  — 高ATK, 远程, 技能：AOE火球
├── Hero_Archer   (游侠)  — 中HP, 远程, 技能：多重箭
└── Hero_Assassin (刺客)  — 中高ATK, 近战, 技能：背刺暴击
```

### 3.2 核心类

| 类名 | 文件 | 功能 |
|------|------|------|
| `Unit` | `core/Unit.h` | 单位基类：HP/ATK/Mana/护甲/星级/装备槽/状态机 |
| `Hero_Warrior` | `core/Hero_Warrior.h` | 战士：范围眩晕 |
| `Hero_Mage` | `core/Hero_Mage.h` | 法师：AOE火球 |
| `Hero_Archer` | `core/Hero_Archer.h` | 游侠：多重箭 |
| `Hero_Assassin` | `core/Hero_Assassin.h` | 刺客：背刺暴击 |
| `Player` | `core/Player.h` | 玩家数据：金币/血量/等级/经验/人口 |
| `BoardWidget` | `gui/BoardWidget.h` | 8×8棋盘：绘制单位/血条蓝条/星级/装备图标/攻击特效 |
| `BenchWidget` | `gui/BenchWidget.h` | 备战区：8格槽位 |
| `BattleSystem` | `core/BattleSystem.h` | 战斗系统：帧驱动循环/技能触发/HP回复/复活检测/胜负判定 |
| `GameManager` | `core/GameManager.h` | 三阶段状态机：准备→战斗→结算 |
| `Shop` | `core/Shop.h` | 商店：英雄池刷新/购买 |
| `SynergySystem` | `core/SynergySystem.h` | 羁绊：6种职业标签统计/阈值判定/Buff叠加 |
| `StarUpSystem` | `core/StarUpSystem.h` | 升星：3合1自动合成 |
| `Equipment` | `core/Equipment.h` | 装备：9种类型/属性加成/合成配方/掉落随机 |
| `SaveLoadManager` | `core/SaveLoadManager.h` | 存档/读档：JSON序列化 |
| `EnemyWaveGenerator` | `core/EnemyWaveGenerator.h` | 敌方波次生成与部署 |
| `PathFinder` | `core/PathFinder.h` | A*寻路（4方向，曼哈顿距离） |
| `DragDropMgr` | `gui/DragDropMgr.h` | 拖拽管理/装备穿戴/合成触发 |
| `ShopWidget` | `gui/ShopWidget.h` | 商店UI |
| `EquipmentBarWidget` | `gui/EquipmentBarWidget.h` | 装备栏UI |
| `SynergyPanelWidget` | `gui/SynergyPanelWidget.h` | 羁绊面板UI |

### 3.3 关键枚举

```cpp
enum class Owner     { PlayerCtrl, EnemyCtrl };
enum class UnitState { Idle, Moving, Attacking, Casting, Stunned, Dead };
enum class GamePhase { Preparation, Battle, Settlement };
enum class AttackEffectType { Arrow, Fireball, Slash, Backstab };
enum class EquipmentType { None, IronSword, ChainMail, SwiftGlove, BlueCrystal,
                           RevivalArmor, VampireBlade, TimeStaff, FireCannon };
```

---

## 四、算法描述

### 4.1 A* 寻路（PathFinder.cpp）

在 8×8 棋盘上找最短路径，用曼哈顿距离做启发函数。每个节点维护 gScore（实际距离）和 fScore（g + 启发值），从 open 列表中选 f 最小的节点扩展，4 方向移动。终点被占时允许寻路到相邻格。

### 4.2 目标锁定（Unit::findTarget）

索敌优先级：距离最近 → HP 最低 → 列最小 → 行最大。每帧 tick：先索敌，在攻击范围内就攻击，否则朝目标移动一格。

### 4.3 羁绊计算（SynergySystem.cpp）

统计棋盘和备战区所有玩家单位的职业标签，按阈值判定激活等级。属性光环只对同职业标签生效，全局光环对所有单位生效。

---

## 五、辅助函数

| 函数 | 文件 | 作用 |
|------|------|------|
| `drawStar()` | `BoardWidget.cpp` | 绘制五角星，用于星级显示 |
| `Equipment::checkRecipe(a,b)` | `Equipment.cpp` | 合成配方检测，两个基础装备归一化后匹配 4 种配方 |
| `Equipment::rollDrop()` | `Equipment.cpp` | 装备掉落加权随机 |
| `manhattan()` | `PathFinder.cpp` | 曼哈顿距离，A* 启发式函数 |

---

## 六、AI 使用说明

### 6.1 项目规划

本项目借助 Trae IDE 内置 AI 助手进行辅助开发。我把每个阶段拆成独立的需求，让 AI 生成代码框架，然后自己审查、修改、编译验证。

- Phase 1：棋盘、Unit 基类、拖拽交互，交给 AI 生成初始代码，我调整布局和事件处理
- Phase 2：战斗系统核心——A* 寻路、帧循环、技能触发，AI 实现后我重点验证了边界情况
- Phase 3：商店、羁绊、升星、装备、存档，子系统较多，逐个让 AI 实现再集成
- Phase 4：装备合成树、高级装备效果（复活甲/吸血/回复/攻速），AI 补充实现

整个过程我保持了完全掌控：每个文件都逐行读过，编译运行测试过，对不合理的地方做了修改。

### 6.2 核心代码解析：GameManager 三阶段状态机

**文件**：[`GameManager.h`](src/core/GameManager.h) / [`GameManager.cpp`](src/core/GameManager.cpp)

GameManager 是整个游戏流程的控制器，管理三个阶段的切换。

**准备阶段**：玩家可以自由拖拽英雄、从商店购买、穿戴装备。点击"开始作战"后，startNewRound() 清空敌方单位、生成新一轮敌人、部署到敌方半场，然后进入战斗。

**战斗阶段**：enterBattlePhase() 禁用拖拽、重置单位战斗状态，启动 m_battleTimer（50ms 间隔）驱动 BattleSystem::onTick()。每帧收集双方存活单位 → 执行 tick（索敌/移动/攻击）→ HP 回复检测 → 复活检测 → 触发技能 → 清理死亡单位 → 检查胜负。一方全灭时发射 battleFinished 信号。

**结算阶段**：根据胜负结果增减金币/经验/HP，胜利 +5 金 +2 经验，失败 +2 金 +1 经验 -3 HP。2 秒后自动回到准备阶段。HP 归零则游戏结束，通过全部 10 轮则通关。

```
准备阶段 ──[开始作战]──→ 战斗阶段 ──[一方全灭]──→ 结算阶段
   ↑                                                    │
   └──────────────[2秒后自动返回]────────────────────────┘
```

### 6.3 核心代码解析：BoardWidget::paintEvent 绘制

**文件**：[`BoardWidget.cpp`](src/gui/BoardWidget.cpp)

paintEvent 是 Qt 的绘制入口，按从底层到上层的顺序绘制：

1. **背景**：深灰色填充整个区域
2. **半场底色**：玩家半场（row ≥ 4）浅蓝色，敌方半场（row < 4）浅红色
3. **分隔线**：3px 灰色虚线区分上下半场
4. **网格线**：1px 灰色实线画 8×8 格子
5. **单位绘制**：每个非空格子画圆形（玩家蓝/敌方红/死亡灰+叉号）、血条（红色，按比例）、蓝条（蓝色）、名称、星级（金色五角星）、装备图标（橙色方块）
6. **攻击特效**：根据 EffectType 绘制不同效果——弓箭手黄线、法师火球、战士斩击、刺客背刺，带透明度渐变（10 帧内从 255→0）

---

## 七、构建与运行

```powershell
cd Synera\build
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/mingw_64" -DCMAKE_BUILD_TYPE=Debug
mingw32-make -j4
.\Synera.exe
```

- **Qt 版本**：6.11.1 (MinGW 64)
- **C++ 标准**：C++17
- **构建系统**：CMake + MinGW Makefiles

#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QList>

// 前向声明：Unit 完整类在任务 3 实现，本阶段只需指针
class Unit;

// 攻击特效类型枚举
enum class AttackEffectType {
    Arrow,      // 弓箭手箭�?    Fireball,   // 法师火球
    Slash,      // 战士斩击
    Backstab    // 刺客背刺（紫色刀刃特效）
};

// 攻击特效数据结构
struct AttackEffect {
    int fromRow, fromCol;   // 来源格子
    int toRow, toCol;       // 目标格子
    AttackEffectType type;  // 特效类型
    int timer;              // 剩余显示帧数（约 10 �?= 500ms�?};

// 8×8 网格棋盘组件，管理单位放�?移除/交换，不拥有 Unit 所有权
class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    // 所有魔法数字用 constexpr 常量替代，符合代码规范第 4 �?    static constexpr int GRID_ROWS = 8;   // 棋盘行数（PA 文档建议 M × N，此�?N=8�?    static constexpr int GRID_COLS = 8;   // 棋盘列数（PA 文档建议 M × N，此�?M=8�?    static constexpr int CELL_SIZE = 60;  // 每个格子像素宽度，棋盘总宽 = 8*60 = 480px

    // 构造与析构

    // 构造函数，固定尺寸 480×480
    explicit BoardWidget(QWidget *parent = nullptr);

    ~BoardWidget() override = default;


    // 判断 row 是否属于玩家半场（row >= 4�?    bool isPlayerHalf(int row) const;

    // 判断格子是否为空（越界也返回 false�?    bool isCellEmpty(int row, int col) const;

    // 放置单位到指定格子（仅允许同半场：玩家放 row>=4，敌方放 row<4�?    bool placeUnit(Unit *unit, int row, int col);

    // 移除单位，返回指针（调用者获得所有权），格子为空返回 nullptr
    Unit *removeUnit(int row, int col);

    // 交换两个格子的单�?    bool swapUnit(int r1, int c1, int r2, int c2);

    // 清空棋盘所有引用（�?delete 对象�?    void clearBoard();

    // 清空攻击特效（进入准备阶段时调用�?    void clearAttackEffects();

    // 强制放置（跳过半场检查，战斗阶段使用�?    bool forcePlaceUnit(Unit *unit, int row, int col);

    // 获取指定位置单位指针
    Unit *getUnitAt(int row, int col) const;

    // 添加攻击特效
    void addAttackEffect(int fromRow, int fromCol, int toRow, int toCol,
                         AttackEffectType type);

    // 每帧更新特效计时器（�?BattleSystem 驱动�?    void tickEffects();

protected:

    // 绘制棋盘：网格、半场底色、单位圆形、血条蓝条、攻击特�?    void paintEvent(QPaintEvent *event) override;

    // 鼠标按下事件（Phase 1 仅打印日志，拖拽�?DragDropMgr 接管�?    void mousePressEvent(QMouseEvent *event) override;

private:
    // 8×8 单位指针数组，nullptr 表示空，row=0 顶部，row=7 底部
    Unit *m_grid[GRID_ROWS][GRID_COLS] = {};

    // 当前活跃的攻击特效列�?    QList<AttackEffect> m_attackEffects;
};

#endif // BOARDWIDGET_H
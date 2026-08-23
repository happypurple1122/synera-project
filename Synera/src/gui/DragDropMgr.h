#ifndef DRAGDROPMGR_H
#define DRAGDROPMGR_H

#include <QObject>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>

class BoardWidget;
class BenchWidget;
class Unit;
class Player;
class EquipmentBarWidget;

// 拖拽来源枚举
enum class DragSource {
    None,       // 无拖拽
    FromBench,  // 从备战区拖出
    FromBoard   // 从棋盘拖出
};

// 拖拽管理器，通过事件过滤器实现备战区↔棋盘的单位拖拽交互
class DragDropMgr : public QObject
{
    Q_OBJECT

public:
    explicit DragDropMgr(BoardWidget *boardWidget,
                         BenchWidget *benchWidget,
                         Player *player,
                         EquipmentBarWidget *equipmentBar,
                         QObject *parent = nullptr);

    ~DragDropMgr() override = default;

    // 启用/禁用拖拽（战斗阶段禁用）
    void setEnabled(bool enabled) { m_enabled = enabled; }

signals:
    // 拖拽完成信号（放置/交换/装备后发射）
    void dragCompleted();

    // 装备合成成功信号
    void equipmentCrafted(const QString &heroName, const QString &equipName);

protected:
    // Qt 事件过滤器，拦截棋盘和备战区的鼠标事件
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    // 备战区鼠标按下：开始从备战区拖拽
    void handleBenchPress(int index);

    // 棋盘鼠标按下：开始从棋盘拖拽
    void handleBoardPress(int row, int col);

    // 棋盘鼠标释放：放置到目标格
    void handleBoardRelease(int row, int col);

    // 备战区鼠标释放：放置回备战区
    void handleBenchRelease(int index);

    // 取消拖拽：弹回原位
    void cancelDrag();

    BoardWidget *m_boardWidget;
    BenchWidget *m_benchWidget;
    Player *m_player;
    EquipmentBarWidget *m_equipmentBar;

    DragSource m_dragSource = DragSource::None;  // 当前拖拽来源
    Unit *m_draggedUnit = nullptr;               // 被拖拽的单位指针
    bool m_enabled = true;                        // 拖拽是否启用

    int m_srcBenchIndex = -1;   // 来源备战区槽位
    int m_srcBoardRow = -1;     // 来源棋盘行
    int m_srcBoardCol = -1;     // 来源棋盘列
};

#endif // DRAGDROPMGR_H
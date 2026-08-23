#ifndef BENCHWIDGET_H
#define BENCHWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

// 前向声明：完整 Unit 类在任务 3 实现，本阶段只需指针
class Unit;

// 一维 8 格备战区组件，管理单位槽位，不拥有 Unit 所有权
class BenchWidget : public QWidget
{
    Q_OBJECT

public:
    // 常量定义
    // PA 文档要求"一维的备战区，例如 8 个格子"
    static constexpr int BENCH_SIZE = 8;  // 备战区槽位总数
    static constexpr int SLOT_SIZE = 60;  // 每个槽位的像素尺寸

    // 构造与析构

    // 构造函数，固定尺寸 BENCH_SIZE * SLOT_SIZE × SLOT_SIZE
    explicit BenchWidget(QWidget *parent = nullptr);

    ~BenchWidget() override = default;

    // 数据操作接口

    // 将单位放入第一个空槽位
    bool addUnit(Unit *unit);

    // 移除指定槽位的单位，返回指针
    Unit *removeUnit(int index);

    // 交换两个槽位的单位
    bool swapSlot(int i, int j);

    // 直接设置指定槽位（不检查该槽位是否已被占）
    void setUnitAt(int index, Unit *unit);

    // 获取指定槽位的单位指针
    Unit *getUnitAt(int index) const;

    // 查找第一个空槽位索引，全满返回 -1
    int findEmptySlot() const;

    // 判断备战区是否已满
    bool isFull() const;

protected:
    // Qt 事件重写

    // 绘制备战区：8 格水平槽位、单位圆形、血条蓝条、星级
    void paintEvent(QPaintEvent *event) override;

    // 鼠标按下事件（Phase 1 仅打印日志）
    void mousePressEvent(QMouseEvent *event) override;

private:
    // 8 格单位槽位数组，nullptr 表示空
    Unit *m_slots[BENCH_SIZE] = {};
};

#endif // BENCHWIDGET_H
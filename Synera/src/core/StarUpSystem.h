#ifndef STARUPSYSTEM_H
#define STARUPSYSTEM_H

#include <QList>
#include <QMap>
#include <QPair>
#include <QDebug>

class Unit;
class BenchWidget;
class BoardWidget;

// 升星系统，三合一晋升机制：3个同名称同星级 → 合并为更高星级
class StarUpSystem
{
public:
    explicit StarUpSystem() = default;
    ~StarUpSystem() = default;

    // 检查并执行升星合并，返回是否发生了合并
    bool checkAndMerge(BenchWidget *bench, BoardWidget *board);

private:
    // 合并 3 个单位，保留 sameUnits[0]，提升星级，销毁其余两个
    void mergeUnits(QList<Unit*> &sameUnits, BenchWidget *bench, BoardWidget *board);

    // 升星公式：1星→2星：maxHp×1.6, atk×1.6, attackSpeed×0.85（向下取整），满血恢复
    void applyStarUpBonus(Unit *unit);
};

#endif // STARUPSYSTEM_H
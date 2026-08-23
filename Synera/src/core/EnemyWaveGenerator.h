#ifndef ENEMYWAVEGENERATOR_H
#define ENEMYWAVEGENERATOR_H

#include <QList>
#include <QRandomGenerator>
#include <QDebug>

class Unit;
class BoardWidget;

// 敌方轮次生成器，根据当前轮次生成敌方单位集合 Er 并部署到敌方半场（row 0~3）
class EnemyWaveGenerator
{
public:
    // 生成第 r 轮敌方单位集合，调用者获得所有权
    static QList<Unit *> generateWave(int round);

    // 将敌方单位部署到棋盘敌方半场，优先从 row=3 靠近中线处开始填充
    static void deployWave(BoardWidget *board, const QList<Unit *> &units);

    // 清空并销毁敌方单位（轮次结算后调用）
    static void clearWave(QList<Unit *> &units);

private:
    static int calculateEnemyCount(int round);

    // 随机选择英雄类型：1=战士, 2=弓箭手, 3=法师, 4=刺客
    static int pickHeroType(int round);

    // 应用轮次难度倍率：roundMult = 1.0 + ((round-1)/3) × 0.2
    // 精英单位 ×1.5，Boss 单位 ×2.0
    static void applyStatScaling(Unit *unit, int round, bool isElite, bool isBoss);
};

#endif // ENEMYWAVEGENERATOR_H
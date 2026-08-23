#ifndef HERO_MAGE_H
#define HERO_MAGE_H

#include "core/Unit.h"

// 法师英雄：远程法术输出型，HP=80, ATK=25, Range=3
// 技能"火球"：对直线方向 3 格内所有敌人造成 1.5×ATK 的 AOE 伤害
class Hero_Mage : public Unit
{
public:
    explicit Hero_Mage(int id, Owner owner);

    void castSkill(QList<Unit *> &targets) override;
};

#endif // HERO_MAGE_H
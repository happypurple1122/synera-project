#ifndef HERO_ARCHER_H
#define HERO_ARCHER_H

#include "core/Unit.h"

// 弓箭手英雄：远程敏捷输出型，HP=90, ATK=18, Range=2
// 技能"精准射击"：攻击血量比例最低的敌人，造成 3×ATK 伤害
class Hero_Archer : public Unit
{
public:
    explicit Hero_Archer(int id, Owner owner);

    void castSkill(QList<Unit *> &targets) override;
};

#endif // HERO_ARCHER_H
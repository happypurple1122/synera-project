#ifndef HERO_WARRIOR_H
#define HERO_WARRIOR_H

#include "core/Unit.h"

// 战士英雄：近战坦克型，HP=150, ATK=15, Range=1
// 技能"猛击"：对单体造成 2×ATK 伤害并眩晕 1.5 秒
class Hero_Warrior : public Unit
{
public:
    explicit Hero_Warrior(int id, Owner owner);

    void castSkill(QList<Unit *> &targets) override;
};

#endif // HERO_WARRIOR_H
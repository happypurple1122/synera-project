#ifndef HERO_ASSASSIN_H
#define HERO_ASSASSIN_H

#include "core/Unit.h"

// 刺客英雄：近战爆发型，HP=100, ATK=25, Range=1
// 技能"暗杀"：对单体造成 4×ATK 伤害
class Hero_Assassin : public Unit
{
public:
    explicit Hero_Assassin(int id, Owner owner);

    void castSkill(QList<Unit *> &targets) override;
};

#endif // HERO_ASSASSIN_H
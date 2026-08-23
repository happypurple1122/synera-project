#include "core/Hero_Assassin.h"

// 刺客英雄：近战爆发，技能为全英雄最高单体伤害（4 倍攻击力）
Hero_Assassin::Hero_Assassin(int id, Owner owner)
    : Unit(id, QStringLiteral("刺客"), owner)
{
    setMaxHp(100);
    setHp(100);

    setAtk(25);
    setRange(1);

    setMaxMana(70);
    setMana(0);

    setTraits(QStringLiteral("刺客"));
    addTraitTag(QStringLiteral("刺客"));
}

// 技能"暗杀"：对首个目标造成 4 倍攻击力伤害
void Hero_Assassin::castSkill(QList<Unit *> &targets)
{
    if (!targets.isEmpty() && targets[0] != nullptr && targets[0]->isAlive()) {
        int damage = getAtk() * 4;
        targets[0]->takeDamage(damage);
        qDebug().noquote() << QString("[刺客] 释放技能【暗杀】对 %1 造成 %2 伤害")
                                  .arg(targets[0]->getName())
                                  .arg(damage);
    }
}
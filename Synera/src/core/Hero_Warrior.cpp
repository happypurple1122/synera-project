#include "core/Hero_Warrior.h"

// 战士英雄：前排坦克，高血量、近战、带眩晕控制
Hero_Warrior::Hero_Warrior(int id, Owner owner)
    : Unit(id, QStringLiteral("战士"), owner)
{
    setMaxHp(150);
    setHp(150);

    setAtk(15);
    setRange(1);

    setMaxMana(80);
    setMana(0);

    setTraits(QStringLiteral("战士"));
    addTraitTag(QStringLiteral("战士"));
    addTraitTag(QStringLiteral("守护者"));
}

// 技能"猛击"：对首个目标造成 2 倍攻击力伤害并眩晕 30 帧（1.5 秒）
void Hero_Warrior::castSkill(QList<Unit *> &targets)
{
    if (!targets.isEmpty() && targets[0] != nullptr && targets[0]->isAlive()) {
        int damage = getAtk() * 2;
        targets[0]->takeDamage(damage);
        targets[0]->setStunTimer(30);
        qDebug().noquote() << QString("[战士] 释放技能【猛击】对 %1 造成 %2 伤害并眩晕 1.5 秒")
                                  .arg(targets[0]->getName())
                                  .arg(damage);
    }
}
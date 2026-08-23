#include "core/Hero_Archer.h"

// 弓箭手英雄：中程持续输出，技能锁定血量最低的敌人进行收割
Hero_Archer::Hero_Archer(int id, Owner owner)
    : Unit(id, QStringLiteral("弓箭手"), owner)
{
    setMaxHp(90);
    setHp(90);

    setAtk(18);
    setRange(2);

    setMaxMana(90);
    setMana(0);

    setTraits(QStringLiteral("弓箭手"));
    addTraitTag(QStringLiteral("游侠"));
    addTraitTag(QStringLiteral("圣职者"));
}

// 技能"精准射击"：选择血量比例最低的敌人，造成 3 倍攻击力伤害
void Hero_Archer::castSkill(QList<Unit *> &targets)
{
    Unit *lowestHpTarget = nullptr;
    double minRatio = 1.0;
    for (Unit *target : targets) {
        if (target != nullptr && target->isAlive()) {
            double ratio = target->getHpRatio();
            if (ratio < minRatio) {
                minRatio = ratio;
                lowestHpTarget = target;
            }
        }
    }
    if (lowestHpTarget != nullptr) {
        int damage = static_cast<int>(getAtk() * 3);
        lowestHpTarget->takeDamage(damage);
        qDebug().noquote() << QString("[弓箭手] 释放技能【精准射击】对 %1 造成 %2 伤害")
                                  .arg(lowestHpTarget->getName())
                                  .arg(damage);
    }
}
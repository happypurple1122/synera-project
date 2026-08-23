#include "core/Hero_Mage.h"
#include <cstdlib>

// 法师英雄：后排远程，高攻击力、低血量、AOE 技能
Hero_Mage::Hero_Mage(int id, Owner owner)
    : Unit(id, QStringLiteral("法师"), owner)
{
    setMaxHp(80);
    setHp(80);

    setAtk(25);
    setRange(3);

    setMaxMana(120);
    setMana(0);

    setTraits(QStringLiteral("法师"));
    addTraitTag(QStringLiteral("法师"));
}

// 技能"火球"：朝主目标方向发射直线 AOE，3 格内同行/同列敌人受 1.5 倍伤害
void Hero_Mage::castSkill(QList<Unit *> &targets)
{
    Unit *primaryTarget = nullptr;
    for (Unit *t : targets) {
        if (t != nullptr && t->isAlive()) {
            primaryTarget = t;
            break;
        }
    }
    if (primaryTarget == nullptr) return;

    int dx = primaryTarget->getPosX() - getPosX();
    int dy = primaryTarget->getPosY() - getPosY();

    bool isHorizontal = (std::abs(dx) >= std::abs(dy));

    int dirSign = 0;
    if (isHorizontal) {
        dirSign = (dx > 0) ? 1 : -1;
    } else {
        dirSign = (dy > 0) ? 1 : -1;
    }

    int damage = static_cast<int>(getAtk() * 1.5);
    int targetsHit = 0;

    for (Unit *target : targets) {
        if (target == nullptr || !target->isAlive()) continue;

        int tx = target->getPosX();
        int ty = target->getPosY();

        bool inLine = false;
        if (isHorizontal) {
            if (ty == getPosY() &&
                (tx - getPosX()) * dirSign > 0 &&
                std::abs(tx - getPosX()) <= 3) {
                inLine = true;
            }
        } else {
            if (tx == getPosX() &&
                (ty - getPosY()) * dirSign > 0 &&
                std::abs(ty - getPosY()) <= 3) {
                inLine = true;
            }
        }

        if (inLine) {
            target->takeDamage(damage);
            targetsHit++;
            qDebug().noquote() << QString("[法师] 火球击中 %1（位置 %2,%3），造成 %4 伤害")
                                      .arg(target->getName())
                                      .arg(target->getPosY())
                                      .arg(target->getPosX())
                                      .arg(damage);
        }
    }

    qDebug().noquote() << QString("[法师] 释放技能【火球】，击中 %1 个目标").arg(targetsHit);
}
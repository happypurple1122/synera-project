#include "core/Equipment.h"
#include "core/Unit.h"

// 获取装备信息

EquipmentInfo Equipment::getInfo(EquipmentType type)
{
    switch (type) {
    case EquipmentType::IronSword:
        return { EquipmentType::IronSword,
                 QStringLiteral("铁剑"),
                 QStringLiteral("攻击力 +5"),
                 30,
                 "#FF7043" };
    case EquipmentType::ChainMail:
        return { EquipmentType::ChainMail,
                 QStringLiteral("锁子甲"),
                 QStringLiteral("生命值 +100"),
                 30,
                 "#42A5F5" };
    case EquipmentType::SwiftGlove:
        return { EquipmentType::SwiftGlove,
                 QStringLiteral("迅捷手套"),
                 QStringLiteral("攻击距离 +1"),
                 20,
                 "#66BB6A" };
    case EquipmentType::BlueCrystal:
        return { EquipmentType::BlueCrystal,
                 QStringLiteral("蓝水晶"),
                 QStringLiteral("最大法力值 +30"),
                 20,
                 "#AB47BC" };
    case EquipmentType::RevivalArmor:
        return { EquipmentType::RevivalArmor,
                 QStringLiteral("复活甲"),
                 QStringLiteral("死亡后 1.5s 以 50% HP 复活"),
                 0,
                 "#FFD700" };
    case EquipmentType::VampireBlade:
        return { EquipmentType::VampireBlade,
                 QStringLiteral("吸血剑"),
                 QStringLiteral("攻击回复 30% 伤害的 HP"),
                 0,
                 "#FF4500" };
    case EquipmentType::TimeStaff:
        return { EquipmentType::TimeStaff,
                 QStringLiteral("时光杖"),
                 QStringLiteral("每秒回复 5 HP"),
                 0,
                 "#00CED1" };
    case EquipmentType::FireCannon:
        return { EquipmentType::FireCannon,
                 QStringLiteral("疾射火炮"),
                 QStringLiteral("攻击距离 +2，攻速 +33%"),
                 0,
                 "#FF6347" };
    default:
        return { EquipmentType::None,
                 QStringLiteral("空"),
                 QStringLiteral(""),
                 0,
                 "#888888" };
    }
}

// 应用装备效果

void Equipment::applyEffect(Unit *unit, EquipmentType type)
{
    if (unit == nullptr || type == EquipmentType::None) return;

    switch (type) {
    case EquipmentType::IronSword:
        unit->setAtk(unit->getAtk() + 5);
        qDebug().noquote() << "[Equipment] 铁剑 →" << unit->getName()
                           << "ATK +5 (当前:" << unit->getAtk() << ")";
        break;

    case EquipmentType::ChainMail:
        unit->setMaxHp(unit->getMaxHp() + 100);
        unit->setHp(unit->getHp() + 100);
        qDebug().noquote() << "[Equipment] 锁子甲 →" << unit->getName()
                           << "HP +100 (当前:" << unit->getHp() << "/" << unit->getMaxHp() << ")";
        break;

    case EquipmentType::SwiftGlove:
        unit->setRange(unit->getRange() + 1);
        qDebug().noquote() << "[Equipment] 迅捷手套 →" << unit->getName()
                           << "Range +1 (当前:" << unit->getRange() << ")";
        break;

    case EquipmentType::BlueCrystal:
        unit->setMaxMana(unit->getMaxMana() + 30);
        qDebug().noquote() << "[Equipment] 蓝水晶 →" << unit->getName()
                           << "MaxMana +30 (当前:" << unit->getMaxMana() << ")";
        break;

    case EquipmentType::RevivalArmor:
        unit->setReviveEnabled(true);
        unit->setReviveTimer(1.5f);  // 1.5 秒后复活
        unit->setMaxHp(unit->getMaxHp() + 100);
        unit->setHp(unit->getHp() + 100);
        qDebug().noquote() << "[Equipment] 复活甲 →" << unit->getName()
                           << "获得复活能力，HP +100";
        break;

    case EquipmentType::VampireBlade:
        unit->setLifeStealPercent(0.3f);
        unit->setAtk(unit->getAtk() + 10);
        qDebug().noquote() << "[Equipment] 吸血剑 →" << unit->getName()
                           << "获得 30% 吸血，ATK +10";
        break;

    case EquipmentType::TimeStaff:
        unit->setHpRegen(5);
        unit->setMaxHp(unit->getMaxHp() + 100);
        unit->setHp(unit->getHp() + 100);
        qDebug().noquote() << "[Equipment] 时光杖 →" << unit->getName()
                           << "获得每秒回复 5 HP，HP +100";
        break;

    case EquipmentType::FireCannon:
        {
            int newSpeed = qMax(1, static_cast<int>(unit->getAttackSpeed() * 0.75f));
            unit->setRange(unit->getRange() + 2);
            unit->setAttackSpeed(newSpeed);
            qDebug().noquote() << "[Equipment] 疾射火炮 →" << unit->getName()
                               << "Range+2，攻速+33%";
        }
        break;

    default:
        break;
    }

    unit->setEquipmentCount(unit->getEquipmentCount() + 1);
    unit->setEquippedType(type);
}

// 移除装备效果

// HP 移除时不超过当前 MaxHp，防止 HP > MaxHp
void Equipment::removeEffect(Unit *unit, EquipmentType type)
{
    if (unit == nullptr || type == EquipmentType::None) return;

    switch (type) {
    case EquipmentType::IronSword:
        unit->setAtk(unit->getAtk() - 5);
        break;

    case EquipmentType::ChainMail:
        unit->setMaxHp(unit->getMaxHp() - 100);
        if (unit->getHp() > unit->getMaxHp()) {
            unit->setHp(unit->getMaxHp());
        }
        break;

    case EquipmentType::SwiftGlove:
        unit->setRange(unit->getRange() - 1);
        break;

    case EquipmentType::BlueCrystal:
        unit->setMaxMana(unit->getMaxMana() - 30);
        if (unit->getMana() > unit->getMaxMana()) {
            unit->setMana(unit->getMaxMana());
        }
        break;

    case EquipmentType::RevivalArmor:
        unit->setReviveEnabled(false);
        unit->setRevivedThisBattle(false);
        unit->setMaxHp(unit->getMaxHp() - 100);
        if (unit->getHp() > unit->getMaxHp()) {
            unit->setHp(unit->getMaxHp());
        }
        break;

    case EquipmentType::VampireBlade:
        unit->setLifeStealPercent(0.0f);
        unit->setAtk(unit->getAtk() - 10);
        break;

    case EquipmentType::TimeStaff:
        unit->setHpRegen(0);
        unit->setMaxHp(unit->getMaxHp() - 100);
        if (unit->getHp() > unit->getMaxHp()) {
            unit->setHp(unit->getMaxHp());
        }
        break;

    case EquipmentType::FireCannon:
        {
            int oldSpeed = static_cast<int>(unit->getAttackSpeed() / 0.75f);
            unit->setRange(unit->getRange() - 2);
            unit->setAttackSpeed(oldSpeed);
        }
        break;

    default:
        break;
    }

    unit->setEquipmentCount(unit->getEquipmentCount() - 1);
    unit->setEquippedType(EquipmentType::None);
}

// 装备掉落随机

// 每个敌人死亡必定掉落一件基础装备，加权随机：IronSword 30%, ChainMail 30%, SwiftGlove 20%, BlueCrystal 20%
EquipmentType Equipment::rollDrop()
{
    int roll = QRandomGenerator::global()->bounded(100);
    if (roll < 30) {
        return EquipmentType::IronSword;
    } else if (roll < 60) {
        return EquipmentType::ChainMail;
    } else if (roll < 80) {
        return EquipmentType::SwiftGlove;
    } else {
        return EquipmentType::BlueCrystal;
    }
}

// 合成配方检测

EquipmentType Equipment::checkRecipe(EquipmentType a, EquipmentType b)
{
    if (a > b) {
        EquipmentType t = a; a = b; b = t;
    }

    if (a == EquipmentType::IronSword && b == EquipmentType::SwiftGlove)
        return EquipmentType::VampireBlade;
    if (a == EquipmentType::IronSword && b == EquipmentType::BlueCrystal)
        return EquipmentType::FireCannon;
    if (a == EquipmentType::ChainMail && b == EquipmentType::SwiftGlove)
        return EquipmentType::TimeStaff;
    if (a == EquipmentType::ChainMail && b == EquipmentType::BlueCrystal)
        return EquipmentType::RevivalArmor;

    return EquipmentType::None;
}

// 高级装备判定

bool Equipment::isAdvanced(EquipmentType type)
{
    switch (type) {
    case EquipmentType::RevivalArmor:
    case EquipmentType::VampireBlade:
    case EquipmentType::TimeStaff:
    case EquipmentType::FireCannon:
        return true;
    default:
        return false;
    }
}
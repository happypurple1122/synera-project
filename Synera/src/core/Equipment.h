#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <QString>
#include <QDebug>
#include <QRandomGenerator>

class Unit;

// 装备类型枚举

// 基础装备：IronSword(+5 ATK), ChainMail(+100 HP), SwiftGlove(+1 Range), BlueCrystal(+30 MaxMana)
// 高级装备（合成获得）：RevivalArmor, VampireBlade, TimeStaff, FireCannon
enum class EquipmentType {
    None,
    IronSword,
    ChainMail,
    SwiftGlove,
    BlueCrystal,
    RevivalArmor,  // 铁剑+锁子甲：死亡后 1.5s 以 50% HP 复活
    VampireBlade,  // 铁剑+迅捷手套：攻击回复 30% HP
    TimeStaff,     // 锁子甲+蓝水晶：每秒回复 5 HP
    FireCannon     // 迅捷手套+蓝水晶：Range+2，攻速+33%
};

// 装备信息结构体

struct EquipmentInfo {
    EquipmentType type;
    QString name;
    QString description;
    int dropWeight;      // 掉落权重（加权随机用）
    QString colorStyle;  // UI 颜色标记
};

// Equipment 静态工具类

// 装备系统静态工具类，提供装备信息查询、效果应用/移除、掉落随机
class Equipment
{
public:
    static EquipmentInfo getInfo(EquipmentType type);

    static void applyEffect(Unit *unit, EquipmentType type);

    static void removeEffect(Unit *unit, EquipmentType type);

    // 每个敌人死亡必定掉落一件基础装备，加权随机决定类型
    // IronSword 30%, ChainMail 30%, SwiftGlove 20%, BlueCrystal 20%
    static EquipmentType rollDrop();

    // 检测两件装备是否匹配合成配方（顺序无关）
    static EquipmentType checkRecipe(EquipmentType a, EquipmentType b);

    static bool isAdvanced(EquipmentType type);
};

#endif // EQUIPMENT_H
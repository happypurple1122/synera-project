#ifndef SHOP_H
#define SHOP_H

#include <QString>
#include <QList>
#include <QDebug>
#include <optional>

#include "core/Unit.h"
#include "core/Equipment.h"

class Unit;
class Player;
class BenchWidget;

// 商店单个槽位的数据
struct ShopSlot {
    QString heroName;
    int price;
};

// 商店装备槽位的数据
struct EquipmentShopSlot {
    EquipmentType type = EquipmentType::None;
    int price = 0;
};

// 商店核心数据类，管理 5 个刷新槽位的随机生成和购买
class Shop
{
public:
    static constexpr int SLOT_COUNT = 5;
    static constexpr int EQUIP_SLOT_COUNT = 2;
    static constexpr int EQUIP_PRICE = 4;
    static constexpr int REFRESH_COST = 2;

    explicit Shop();

    // 根据当前轮次刷新 5 个随机英雄
    void refresh(int currentStage);

    // 购买指定槽位的英雄，失败返回 nullptr
    Unit *buy(int index, Player &player, BenchWidget &bench);

    // 刷新装备槽（2 个随机基础装备）
    void refreshEquipment(int currentStage);

    // 购买指定装备槽位的装备，失败返回 None
    EquipmentType buyEquipment(int index, Player &player);

    std::optional<EquipmentShopSlot> getEquipSlot(int index) const;

    std::optional<ShopSlot> getSlot(int index) const;

    QList<ShopSlot> getSlots() const;

    int getRefreshCost() const;

    static QStringList getPoolNames();

    // 直接设置指定槽位（读档恢复用）
    void restoreSlot(int index, const QString &heroName, int price);

    // 根据名称创建英雄实例（静态工厂方法）
    static Unit *createUnit(int id, const QString &name, Owner owner);

private:
    ShopSlot m_slots[SLOT_COUNT];
    EquipmentShopSlot m_equipSlots[EQUIP_SLOT_COUNT];
    int m_nextUnitId;

    // 根据轮次和随机数决定费用等级
    // 概率表：轮次1~2:100%1费, 3~4:80%1费+20%2费, 5~6:60%1费+30%2费+10%3费, 7~8:50%+35%+15%, 9+:40%+35%+25%
    int determineTier(int roll, int currentStage) const;

    // 从指定费用等级的英雄池中随机选一个
    QString randomHeroFromTier(int tier) const;

    // 费用等级对应价格：Tier1=2, Tier2=4, Tier3=6
    int getPriceForTier(int tier) const;
};

#endif // SHOP_H
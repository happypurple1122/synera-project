#include "core/Shop.h"
#include "core/Player.h"
#include "core/Hero_Warrior.h"
#include "core/Hero_Mage.h"
#include "core/Hero_Archer.h"
#include "core/Hero_Assassin.h"
#include "gui/BenchWidget.h"

#include <QRandomGenerator>
#include <optional>

// 构造函数

Shop::Shop()
    : m_nextUnitId(1000)
{
    try {
        for (int i = 0; i < SLOT_COUNT; ++i) {
            m_slots[i].heroName = QString();
            m_slots[i].price = 0;
        }
        for (int i = 0; i < EQUIP_SLOT_COUNT; ++i) {
            m_equipSlots[i].type = EquipmentType::None;
            m_equipSlots[i].price = 0;
        }
        qDebug() << "[Shop] 商店初始化完成，下一个 ID：" << m_nextUnitId;
    } catch (const std::exception &e) {
        qDebug() << "[Shop] 构造函数异常：" << e.what();
    }
}

// 刷新商店

// 每个槽位独立：随机决定费用等级 → 从对应池中随机选英雄 → 填入槽位
void Shop::refresh(int currentStage)
{
    try {
        qDebug() << "[Shop] 刷新商店（当前轮次：" << currentStage << "）";

        for (int i = 0; i < SLOT_COUNT; ++i) {
            try {
                int roll = QRandomGenerator::global()->bounded(100);
                int tier = determineTier(roll, currentStage);
                QString heroName = randomHeroFromTier(tier);
                int price = getPriceForTier(tier);

                m_slots[i].heroName = heroName;
                m_slots[i].price = price;

                qDebug() << "[Shop] 槽位" << i << ":" << heroName << " 价格:" << price
                         << " (roll=" << roll << ", tier=" << tier << ")";
            } catch (const std::exception &e) {
                qDebug() << "[Shop] 刷新槽位" << i << "异常：" << e.what();
                m_slots[i].heroName = QString();
                m_slots[i].price = 0;
            }
        }

        qDebug() << "[Shop] 商店刷新完成";
    } catch (const std::exception &e) {
        qDebug() << "[Shop] refresh 整体异常：" << e.what();
    } catch (...) {
        qDebug() << "[Shop] refresh 未知异常";
    }
}

// 购买英雄

// 边界检查 → 有效性检查 → 备战区检查 → 金币检查 → 创建英雄 → 放入备战区 → 清空槽位
// 任何步骤失败都会回滚（退还金币/删除英雄）
Unit *Shop::buy(int index, Player &player, BenchWidget &bench)
{
    try {
        if (index < 0 || index >= SLOT_COUNT) {
            qDebug() << "[Shop] buy 失败：索引越界" << index;
            return nullptr;
        }

        if (m_slots[index].heroName.isEmpty()) {
            qDebug() << "[Shop] buy 失败：槽位" << index << "为空";
            return nullptr;
        }

        const QString &heroName = m_slots[index].heroName;
        int price = m_slots[index].price;

        if (bench.isFull()) {
            qDebug() << "[Shop] buy 失败：备战区已满";
            return nullptr;
        }

        if (!player.spendGold(price)) {
            qDebug() << "[Shop] buy 失败：金币不足，需要" << price
                     << "，当前" << player.getGold();
            return nullptr;
        }

        int newId = m_nextUnitId++;
        Unit *unit = createUnit(newId, heroName, Owner::PlayerCtrl);
        if (unit == nullptr) {
            qDebug() << "[Shop] buy 失败：无法创建英雄" << heroName;
            player.addGold(price);
            return nullptr;
        }

        if (!bench.addUnit(unit)) {
            qDebug() << "[Shop] buy 失败：addUnit 返回 false";
            delete unit;
            player.addGold(price);
            return nullptr;
        }

        m_slots[index].heroName = QString();
        m_slots[index].price = 0;

        qDebug() << "[Shop] 购买成功：" << heroName << "（ID:" << newId
                 << "，价格:" << price << "），剩余金币：" << player.getGold();
        return unit;

    } catch (const std::exception &e) {
        qDebug() << "[Shop] buy 异常：" << e.what();
        return nullptr;
    } catch (...) {
        qDebug() << "[Shop] buy 未知异常";
        return nullptr;
    }
}

// 存档恢复方法

void Shop::restoreSlot(int index, const QString &heroName, int price)
{
    if (index < 0 || index >= SLOT_COUNT) {
        qDebug() << "[Shop] restoreSlot 失败：索引越界" << index;
        return;
    }
    m_slots[index].heroName = heroName;
    m_slots[index].price = price;
}

// 装备商店

void Shop::refreshEquipment(int currentStage)
{
    try {
        Q_UNUSED(currentStage);
        qDebug() << "[Shop] 刷新装备槽";

        for (int i = 0; i < EQUIP_SLOT_COUNT; ++i) {
            try {
                EquipmentType types[] = {
                    EquipmentType::IronSword,
                    EquipmentType::ChainMail,
                    EquipmentType::SwiftGlove,
                    EquipmentType::BlueCrystal
                };
                int idx = QRandomGenerator::global()->bounded(4);
                m_equipSlots[i].type = types[idx];
                m_equipSlots[i].price = EQUIP_PRICE;

                qDebug() << "[Shop] 装备槽" << i << ":"
                         << Equipment::getInfo(m_equipSlots[i].type).name
                         << " 价格:" << m_equipSlots[i].price;
            } catch (const std::exception &e) {
                qDebug() << "[Shop] 刷新装备槽" << i << "异常：" << e.what();
                m_equipSlots[i].type = EquipmentType::None;
                m_equipSlots[i].price = 0;
            }
        }

        qDebug() << "[Shop] 装备槽刷新完成";
    } catch (const std::exception &e) {
        qDebug() << "[Shop] refreshEquipment 异常：" << e.what();
    }
}

EquipmentType Shop::buyEquipment(int index, Player &player)
{
    try {
        if (index < 0 || index >= EQUIP_SLOT_COUNT) {
            qDebug() << "[Shop] buyEquipment 失败：索引越界" << index;
            return EquipmentType::None;
        }

        if (m_equipSlots[index].type == EquipmentType::None) {
            qDebug() << "[Shop] buyEquipment 失败：装备槽" << index << "为空";
            return EquipmentType::None;
        }

        int price = m_equipSlots[index].price;

        if (!player.spendGold(price)) {
            qDebug() << "[Shop] buyEquipment 失败：金币不足，需要" << price
                     << "，当前" << player.getGold();
            return EquipmentType::None;
        }

        EquipmentType boughtType = m_equipSlots[index].type;

        m_equipSlots[index].type = EquipmentType::None;
        m_equipSlots[index].price = 0;

        qDebug() << "[Shop] 装备购买成功：" << Equipment::getInfo(boughtType).name
                 << "，价格：" << price << "，剩余金币：" << player.getGold();
        return boughtType;

    } catch (const std::exception &e) {
        qDebug() << "[Shop] buyEquipment 异常：" << e.what();
        return EquipmentType::None;
    }
}

std::optional<EquipmentShopSlot> Shop::getEquipSlot(int index) const
{
    if (index < 0 || index >= EQUIP_SLOT_COUNT) {
        return std::nullopt;
    }
    if (m_equipSlots[index].type == EquipmentType::None) {
        return std::nullopt;
    }
    return m_equipSlots[index];
}

// 查询方法

std::optional<ShopSlot> Shop::getSlot(int index) const
{
    if (index < 0 || index >= SLOT_COUNT) {
        return std::nullopt;
    }
    if (m_slots[index].heroName.isEmpty()) {
        return std::nullopt;
    }
    return m_slots[index];
}

QList<ShopSlot> Shop::getSlots() const
{
    QList<ShopSlot> result;
    result.reserve(SLOT_COUNT);
    for (int i = 0; i < SLOT_COUNT; ++i) {
        if (!m_slots[i].heroName.isEmpty()) {
            result.append(m_slots[i]);
        }
    }
    return result;
}

int Shop::getRefreshCost() const
{
    return REFRESH_COST;
}

// 英雄池

QStringList Shop::getPoolNames()
{
    return {
        QStringLiteral("战士"),
        QStringLiteral("法师"),
        QStringLiteral("弓箭手"),
        QStringLiteral("刺客")
    };
}

// 英雄工厂

Unit *Shop::createUnit(int id, const QString &name, Owner owner)
{
    try {
        if (name == QStringLiteral("战士")) {
            return new Hero_Warrior(id, owner);
        } else if (name == QStringLiteral("法师")) {
            return new Hero_Mage(id, owner);
        } else if (name == QStringLiteral("弓箭手")) {
            return new Hero_Archer(id, owner);
        } else if (name == QStringLiteral("刺客")) {
            return new Hero_Assassin(id, owner);
        } else {
            qDebug() << "[Shop] createUnit 不认识的名字：" << name;
            return nullptr;
        }
    } catch (const std::exception &e) {
        qDebug() << "[Shop] createUnit 异常：" << e.what();
        return nullptr;
    }
}

// 私有方法

// 概率表：
//   轮次 1~2：100% 1费
//   轮次 3~4：1费80%, 2费20%
//   轮次 5~6：1费60%, 2费30%, 3费10%
//   轮次 7~8：1费50%, 2费35%, 3费15%
//   轮次 9+：1费40%, 2费35%, 3费25%
int Shop::determineTier(int roll, int currentStage) const
{
    try {
        if (currentStage <= 0) currentStage = 1;

        if (currentStage <= 2) {
            return 1;
        } else if (currentStage <= 4) {
            if (roll < 80) return 1;
            return 2;
        } else if (currentStage <= 6) {
            if (roll < 60) return 1;
            if (roll < 90) return 2;
            return 3;
        } else if (currentStage <= 8) {
            if (roll < 50) return 1;
            if (roll < 85) return 2;
            return 3;
        } else {
            if (roll < 40) return 1;
            if (roll < 75) return 2;
            return 3;
        }
    } catch (const std::exception &e) {
        qDebug() << "[Shop] determineTier 异常：" << e.what();
        return 1;
    }
}

QString Shop::randomHeroFromTier(int tier) const
{
    try {
        Q_UNUSED(tier);

        QStringList pool = getPoolNames();
        if (pool.isEmpty()) {
            qDebug() << "[Shop] randomHeroFromTier 英雄池为空";
            return QString();
        }

        int index = QRandomGenerator::global()->bounded(pool.size());
        return pool[index];
    } catch (const std::exception &e) {
        qDebug() << "[Shop] randomHeroFromTier 异常：" << e.what();
        return QStringLiteral("战士");
    }
}

// Tier1=2, Tier2=4, Tier3=6
int Shop::getPriceForTier(int tier) const
{
    switch (tier) {
    case 1: return 2;
    case 2: return 4;
    case 3: return 6;
    default:
        qDebug() << "[Shop] getPriceForTier 未知 tier：" << tier;
        return 2;
    }
}
#include "core/SynergySystem.h"
#include "core/Unit.h"

#include <QDebug>

SynergySystem::SynergySystem()
{
    try {
        initTraits();
        qDebug().noquote() << "[SynergySystem] 羁绊系统初始化完成，共" << m_traits.size() << "个羁绊";
    } catch (const std::exception &e) {
        qDebug() << "[SynergySystem] 构造函数异常：" << e.what();
    }
}

// 初始化 6 个羁绊定义，每个羁绊包含名称、标签、阈值和增益列表
void SynergySystem::initTraits()
{
    try {
        
        // 1. 钢铁战魂（战士） — 属性光环
        //    标签: "战士", 阈值: 2/4
        //    效果: +200 HP / +200 HP +15 ATK
        {
            SynergyTrait trait;
            trait.name = QStringLiteral("钢铁战魂");
            trait.tag  = QStringLiteral("战士");
            trait.type = SynergyType::AttributeAura;

            SynergyBuff buff2;
            buff2.threshold = 2;
            buff2.description = QStringLiteral("+200 HP");
            buff2.bonusHp = 200;

            SynergyBuff buff4;
            buff4.threshold = 4;
            buff4.description = QStringLiteral("+200 HP, +15 ATK");
            buff4.bonusHp = 200;
            buff4.bonusAtk = 15;

            trait.buffs.append(buff2);
            trait.buffs.append(buff4);
            m_traits.append(trait);
        }

        
        // 2. 奥术智慧（法师） — 属性光环
        //    标签: "法师", 阈值: 2/4
        //    效果: +30 ATK / +30 ATK 技能伤害x1.5
        
        {
            SynergyTrait trait;
            trait.name = QStringLiteral("奥术智慧");
            trait.tag  = QStringLiteral("法师");
            trait.type = SynergyType::AttributeAura;

            SynergyBuff buff2;
            buff2.threshold = 2;
            buff2.description = QStringLiteral("+30 ATK");
            buff2.bonusAtk = 30;

            SynergyBuff buff4;
            buff4.threshold = 4;
            buff4.description = QStringLiteral("+30 ATK, 法力获取x1.5");
            buff4.bonusAtk = 30;
            buff4.manaMultiplier = 1.5f;

            trait.buffs.append(buff2);
            trait.buffs.append(buff4);
            m_traits.append(trait);
        }

        
        // 3. 疾风猎手（游侠） — 机制变更
        //    标签: "游侠", 阈值: 2/3
        //    效果: 20%连击 / 40%连击
        
        {
            SynergyTrait trait;
            trait.name = QStringLiteral("疾风猎手");
            trait.tag  = QStringLiteral("游侠");
            trait.type = SynergyType::MechanicChange;

            SynergyBuff buff2;
            buff2.threshold = 2;
            buff2.description = QStringLiteral("20%连击");
            buff2.doubleAttack = true;

            SynergyBuff buff3;
            buff3.threshold = 3;
            buff3.description = QStringLiteral("40%连击");
            buff3.doubleAttack = true;

            trait.buffs.append(buff2);
            trait.buffs.append(buff3);
            m_traits.append(trait);
        }

        
        // 4. 暗影突袭（刺客） — 机制变更
        //    标签: "刺客", 阈值: 2/3
        //    效果: 无视50%护甲 / 无视全部护甲
        
        {
            SynergyTrait trait;
            trait.name = QStringLiteral("暗影突袭");
            trait.tag  = QStringLiteral("刺客");
            trait.type = SynergyType::MechanicChange;

            SynergyBuff buff2;
            buff2.threshold = 2;
            buff2.description = QStringLiteral("无视50%护甲");
            buff2.ignoreArmor = true;

            SynergyBuff buff3;
            buff3.threshold = 3;
            buff3.description = QStringLiteral("无视全部护甲");
            buff3.ignoreArmor = true;

            trait.buffs.append(buff2);
            trait.buffs.append(buff3);
            m_traits.append(trait);
        }

        
        // 5. 自然守护（守护者） — 属性光环（全局光环）
        //    标签: "守护者", 阈值: 2/3
        //    效果: +10护甲 / +10护甲（全局生效于所有单位）
        
        {
            SynergyTrait trait;
            trait.name = QStringLiteral("自然守护");
            trait.tag  = QStringLiteral("守护者");
            trait.type = SynergyType::AttributeAura;

            SynergyBuff buff2;
            buff2.threshold = 2;
            buff2.description = QStringLiteral("所有单位+10护甲");
            buff2.bonusArmor = 10;

            SynergyBuff buff3;
            buff3.threshold = 3;
            buff3.description = QStringLiteral("所有单位+10护甲");
            buff3.bonusArmor = 10;

            trait.buffs.append(buff2);
            trait.buffs.append(buff3);
            m_traits.append(trait);
        }

        
        // 6. 神圣祝福（圣职者） — 属性光环（全局光环）
        //    标签: "圣职者", 阈值: 2
        //    效果: +20%法力获取（全局生效于所有单位）
        
        {
            SynergyTrait trait;
            trait.name = QStringLiteral("神圣祝福");
            trait.tag  = QStringLiteral("圣职者");
            trait.type = SynergyType::AttributeAura;

            SynergyBuff buff2;
            buff2.threshold = 2;
            buff2.description = QStringLiteral("所有单位法力获取+20%");
            buff2.manaMultiplier = 1.2f;

            trait.buffs.append(buff2);
            m_traits.append(trait);
        }

    } catch (const std::exception &e) {
        qDebug() << "[SynergySystem] initTraits 异常：" << e.what();
    }
}

// 全局光环（守护者/圣职者）作用于所有单位，非全局只作用于拥有该标签的单位
bool SynergySystem::isGlobalAura(const QString &tag) const
{
    return (tag == QStringLiteral("守护者") ||
            tag == QStringLiteral("圣职者"));
}

// 主入口：清旧增益 → 统计标签 → 确定阈值 → 应用增益
void SynergySystem::calculate(QList<Unit*> &allPlayerUnits)
{
    try {
        clearBuffs(allPlayerUnits);

        m_traitCounts.clear();
        m_activeThresholds.clear();

        for (Unit *unit : allPlayerUnits) {
            if (unit == nullptr || !unit->isAlive()) continue;

            const QStringList &tags = unit->getTraitTags();
            for (const QString &tag : tags) {
                m_traitCounts[tag]++;
            }
        }

        for (const SynergyTrait &trait : m_traits) {
            int count = m_traitCounts.value(trait.tag, 0);
            int highestThreshold = 0;

            for (const SynergyBuff &buff : trait.buffs) {
                if (count >= buff.threshold) {
                    highestThreshold = buff.threshold;
                }
            }

            if (highestThreshold > 0) {
                m_activeThresholds[trait.tag] = highestThreshold;
                qDebug().noquote() << QString("[羁绊] %1(%2) 激活阈值=%3 (场上%4个)")
                                          .arg(trait.name).arg(trait.tag)
                                          .arg(highestThreshold).arg(count);
            } else {
                qDebug().noquote() << QString("[羁绊] %1(%2) 未激活 (场上%3个)")
                                          .arg(trait.name).arg(trait.tag).arg(count);
            }
        }

        applyBuffs(allPlayerUnits);

    } catch (const std::exception &e) {
        qDebug() << "[SynergySystem] calculate 异常：" << e.what();
    } catch (...) {
        qDebug() << "[SynergySystem] calculate 未知异常";
    }
}

// 重置所有单位的羁绊增益为默认值
void SynergySystem::clearBuffs(QList<Unit*> &allPlayerUnits)
{
    try {
        for (Unit *unit : allPlayerUnits) {
            if (unit == nullptr) continue;
            unit->setDoubleAttackEnabled(false);
            unit->setIgnoreArmorEnabled(false);
            unit->setBonusHp(0);
            unit->setBonusAtk(0);
            unit->setBonusArmor(0);
            unit->setManaMultiplier(1.0f);
        }
    } catch (const std::exception &e) {
        qDebug() << "[SynergySystem] clearBuffs 异常：" << e.what();
    }
}

// 根据活跃阈值，叠加所有 ≤ 阈值的 buff，全局光环应用于所有单位
void SynergySystem::applyBuffs(QList<Unit*> &allPlayerUnits)
{
    try {
        // 遍历每个羁绊
        for (const SynergyTrait &trait : m_traits) {
            const QString &tag = trait.tag;
            int activeThreshold = m_activeThresholds.value(tag, 0);
            if (activeThreshold <= 0) continue;

            // 计算该羁绊的所有叠加增益（累加 ≤ 活跃阈值的所有 buff）
            int totalBonusHp = 0;
            int totalBonusAtk = 0;
            int totalBonusArmor = 0;
            bool hasDoubleAttack = false;
            bool hasIgnoreArmor = false;
            float maxManaMultiplier = 1.0f;

            for (const SynergyBuff &buff : trait.buffs) {
                if (buff.threshold > activeThreshold) continue;
                totalBonusHp += buff.bonusHp;
                totalBonusAtk += buff.bonusAtk;
                totalBonusArmor += buff.bonusArmor;
                if (buff.doubleAttack) hasDoubleAttack = true;
                if (buff.ignoreArmor) hasIgnoreArmor = true;
                if (buff.manaMultiplier > maxManaMultiplier) {
                    maxManaMultiplier = buff.manaMultiplier;
                }
            }

            bool global = isGlobalAura(tag);

            // 应用到所有单位
            for (Unit *unit : allPlayerUnits) {
                if (unit == nullptr) continue;

                bool shouldApply = global || unit->hasTraitTag(tag);
                if (!shouldApply) continue;

                // 叠加属性增益
                if (totalBonusHp > 0) {
                    unit->setBonusHp(unit->getBonusHp() + totalBonusHp);
                }
                if (totalBonusAtk > 0) {
                    unit->setBonusAtk(unit->getBonusAtk() + totalBonusAtk);
                }
                if (totalBonusArmor > 0) {
                    unit->setBonusArmor(unit->getBonusArmor() + totalBonusArmor);
                }

                // 设置标志（某些羁绊同时触发多个 flag）
                if (hasDoubleAttack) {
                    unit->setDoubleAttackEnabled(true);
                }
                if (hasIgnoreArmor) {
                    unit->setIgnoreArmorEnabled(true);
                }
                if (maxManaMultiplier > 1.0f) {
                    unit->setManaMultiplier(maxManaMultiplier);
                }

                qDebug().noquote() << QString("  [增益] %1 → %2 (HP+%3 ATK+%4 护甲+%5 连击=%6 无视护甲=%7 法力倍率=%8)")
                                          .arg(trait.name)
                                          .arg(unit->getName())
                                          .arg(totalBonusHp)
                                          .arg(totalBonusAtk)
                                          .arg(totalBonusArmor)
                                          .arg(hasDoubleAttack)
                                          .arg(hasIgnoreArmor)
                                          .arg(maxManaMultiplier);
            }
        }

    } catch (const std::exception &e) {
        qDebug() << "[SynergySystem] applyBuffs 异常：" << e.what();
    } catch (...) {
        qDebug() << "[SynergySystem] applyBuffs 未知异常";
    }
}

const QList<SynergyTrait> &SynergySystem::getAllTraits() const
{
    return m_traits;
}

QMap<QString, int> SynergySystem::getTraitCounts() const
{
    return m_traitCounts;
}

QMap<QString, int> SynergySystem::getActiveThresholds() const
{
    return m_activeThresholds;
}
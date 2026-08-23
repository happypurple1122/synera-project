#ifndef SYNERGYSYSTEM_H
#define SYNERGYSYSTEM_H

#include <QString>
#include <QList>
#include <QMap>
#include <QDebug>

class Unit;

// 羁绊类型：属性光环（简单属性增减）或机制变更（连击/无视护甲等）
enum class SynergyType {
    AttributeAura,
    MechanicChange
};

// 单个阈值对应的羁绊增益数据
struct SynergyBuff {
    int threshold;
    QString description;
    int bonusHp = 0;
    int bonusAtk = 0;
    int bonusArmor = 0;
    bool doubleAttack = false;
    bool ignoreArmor = false;
    float manaMultiplier = 1.0f;
};

// 一个完整羁绊的定义
struct SynergyTrait {
    QString name;
    QString tag;
    SynergyType type;
    QList<SynergyBuff> buffs;  // 按阈值排序
};

// 羁绊系统核心类：统计场上玩家单位的羁绊标签，计算并应用增益效果
class SynergySystem
{
public:
    // 初始化 6 个羁绊定义
    explicit SynergySystem();

    // 主入口：clearBuffs → 统计标签 → 确定阈值 → applyBuffs
    void calculate(QList<Unit*> &allPlayerUnits);

    void clearBuffs(QList<Unit*> &allPlayerUnits);

    void applyBuffs(QList<Unit*> &allPlayerUnits);

    const QList<SynergyTrait> &getAllTraits() const;

    QMap<QString, int> getTraitCounts() const;

    QMap<QString, int> getActiveThresholds() const;

private:
    QList<SynergyTrait> m_traits;
    QMap<QString, int> m_traitCounts;
    QMap<QString, int> m_activeThresholds;

    void initTraits();

    // 全局光环（守护者/圣职者）的增益作用于所有单位
    bool isGlobalAura(const QString &tag) const;
};

#endif // SYNERGYSYSTEM_H
#ifndef UNIT_H
#define UNIT_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QDebug>

#include "core/Equipment.h"

// 前向声明（避免循环 include）
class BoardWidget;

// 枚举定义

// 区分单位归属权：PlayerCtrl=玩家方, EnemyCtrl=敌方
enum class Owner {
    PlayerCtrl,
    EnemyCtrl
};

// 单位行为阶段状态机
enum class UnitState {
    Idle,
    Moving,
    Attacking,
    Casting,
    Stunned,    // 眩晕状态（被战士猛击击中后，30 帧 ≈ 1.5 秒无法行动）
    Dead
};

// Unit 抽象基类

// 游戏单位的抽象基类，定义所有单位的公共属性与行为
class Unit
{
public:
    // 构造与析构

    // 构造时设置基础默认值，派生类可覆盖
    explicit Unit(int id, const QString &name, Owner owner);
    virtual ~Unit() = default;

    // 纯虚函数（多态技能接口）

    // 释放技能，各英雄子类实现不同效果
    virtual void castSkill(QList<Unit *> &targets) = 0;

    // 战斗阶段方法

    // 每帧更新（由 BattleSystem 驱动），stun 时不行动
    void tick(BoardWidget *board, const QList<Unit*> &allEnemies);

    // 从敌方列表中按规则（距离→HP→列→行）选出最优目标
    Unit *findTarget(const QList<Unit*> &enemies) const;

    // 向目标位置移动一步（A* 寻路，不穿越单位）
    bool moveToward(int targetX, int targetY, BoardWidget *board);

    // 对目标发动普攻，造成 m_atk 伤害
    void attack(Unit *target, BoardWidget *board);

    // 增加法力值（不超过上限），受羁绊倍率影响
    void addMana(int amount);

    // 核心方法

    // 受到伤害：finalDmg = dmg - armor，最低 1 点
    void takeDamage(int dmg);

    // 治疗：不超过 maxHp
    void heal(int amount);

    // 是否存活
    bool isAlive() const;

    // 生命值比例 0.0~1.0（用于血条绘制）
    double getHpRatio() const;

    // 法力值比例 0.0~1.0（用于蓝条绘制）
    double getManaRatio() const;

    // Getter / Setter

    int getId() const;
    void setId(int id);

    const QString &getName() const;
    void setName(const QString &name);

    int getHp() const;
    void setHp(int hp);
    int getMaxHp() const;
    void setMaxHp(int maxHp);

    int getAtk() const;
    void setAtk(int atk);

    int getRange() const;
    void setRange(int range);

    int getMana() const;
    void setMana(int mana);
    int getMaxMana() const;
    void setMaxMana(int maxMana);

    Owner getOwner() const;
    void setOwner(Owner owner);

    int getPosX() const;
    void setPosX(int x);
    int getPosY() const;
    void setPosY(int y);

    UnitState getState() const;
    void setState(UnitState state);

    int getStarLevel() const;
    void setStarLevel(int level);

    const QString &getTraits() const;
    void setTraits(const QString &traits);

    // Phase 3 扩展方法

    const QStringList &getTraitTags() const;
    void addTraitTag(const QString &tag);
    bool hasTraitTag(const QString &tag) const;

    int getEquipmentCount() const;
    void setEquipmentCount(int count);
    // 1星最多1件装备，2星及以上最多2件
    bool canEquip() const;

    // Synergy Flags

bool isDoubleAttackEnabled() const;
    void setDoubleAttackEnabled(bool enabled);
    bool isIgnoreArmorEnabled() const;
    void setIgnoreArmorEnabled(bool enabled);
    int getBonusHp() const;
    void setBonusHp(int hp);
    int getBonusAtk() const;
    void setBonusAtk(int atk);
    int getBonusArmor() const;
    void setBonusArmor(int armor);
    float getManaMultiplier() const;
    void setManaMultiplier(float multiplier);

    // Phase 4 高级装备

    EquipmentType getEquippedType() const;
    void setEquippedType(EquipmentType type);
    bool isReviveEnabled() const;
    void setReviveEnabled(bool enabled);
    bool isRevivedThisBattle() const;
    void setRevivedThisBattle(bool revived);
    float getReviveTimer() const;
    void setReviveTimer(float timer);
    float getLifeStealPercent() const;
    void setLifeStealPercent(float percent);
    int getHpRegen() const;
    void setHpRegen(int regen);

    // 将羁绊增益应用到战斗属性上（m_maxHp += bonusHp, m_atk += bonusAtk，满血）
    void applySynergyBuffs();

    // 移除已应用的羁绊增益，恢复基础属性
    void removeSynergyBuffs();

    // 战斗字段 Getter/Setter

    Unit *getCurrentTarget() const;
    void setCurrentTarget(Unit *target);
    int getAttackSpeed() const;
    void setAttackSpeed(int speed);
    int getMoveSpeed() const;
    void setMoveSpeed(int speed);
    int getStunTimer() const;
    void setStunTimer(int timer);

    // 重置所有战斗状态（新战斗开始前调用）
    void resetBattleState();

    void debugPrint() const;

private:
    // 私有属性

    int m_id;
    QString m_name;
    int m_hp;
    int m_maxHp;
    int m_atk;
    int m_range;
    int m_mana;
    int m_maxMana;

    Owner m_owner;
    int m_posX;                 // -1=未上阵
    int m_posY;                 // -1=未上阵
    UnitState m_state;
    int m_starLevel;
    QString m_traits;
    QStringList m_traitTags;
    int m_equipmentCount = 0;

    bool m_doubleAttackFlag = false;   // 连击标志（游侠羁绊）
    bool m_ignoreArmorFlag = false;    // 无视护甲标志（刺客羁绊）
    int m_bonusHp = 0;
    int m_bonusAtk = 0;
    int m_bonusArmor = 0;
    float m_manaMultiplier = 1.0f;

    EquipmentType m_equippedType = EquipmentType::None;
    bool m_reviveEnabled = false;
    bool m_revivedThisBattle = false;
    float m_reviveTimer = 0.0f;       // 复活倒计时（秒）
    float m_lifeStealPercent = 0.0f;  // 吸血比例
    int m_hpRegen = 0;                // 每秒回复 HP

    Unit *m_currentTarget;
    int m_attackTimer;
    int m_attackSpeed;          // 攻击间隔（帧数，20 帧≈1 秒）
    int m_moveTimer;
    int m_moveSpeed;            // 移动间隔（帧数，10 帧≈0.5 秒走一格）
    int m_stunTimer;            // 眩晕倒计时（0=无眩晕）
};

#endif // UNIT_H
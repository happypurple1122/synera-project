#include "core/Unit.h"
#include "core/PathFinder.h"
#include "gui/BoardWidget.h"

#include <cmath>
#include <algorithm>
#include <QRandomGenerator>

// 构造函数

Unit::Unit(int id, const QString &name, Owner owner)
    : m_id(id)
    , m_name(name)
    , m_owner(owner)
{
    m_hp = 100;
    m_maxHp = 100;

    m_atk = 10;
    m_range = 1;

    m_mana = 0;
    m_maxMana = 100;

    m_posX = -1;
    m_posY = -1;

    m_state = UnitState::Idle;
    m_starLevel = 1;
    m_traits = QStringLiteral("无");

    m_currentTarget = nullptr;
    m_attackTimer = 0;
    m_attackSpeed = 20;    // 20 帧 × 50ms = 1s
    m_moveTimer = 0;
    m_moveSpeed = 10;      // 10 帧 × 50ms = 0.5s 走一格
    m_stunTimer = 0;
}

// 核心方法

// 最终伤害 = dmg - armor，最低 1 点；HP 归零标记为 Dead
void Unit::takeDamage(int dmg)
{
    try {
        int armor;
        if (m_ignoreArmorFlag) {
            armor = 0;  // 暗影突袭羁绊：无视全部护甲
        } else {
            armor = (m_atk / 10) + m_bonusArmor;
        }

        int actualDmg = dmg - armor;
        if (actualDmg < 1) {
            actualDmg = 1;
        }

        m_hp -= actualDmg;
        if (m_hp < 0) {
            m_hp = 0;
        }

        if (m_hp <= 0 && m_state != UnitState::Dead) {
            m_state = UnitState::Dead;
        }
    } catch (const std::exception &e) {
        qDebug() << "[Unit] takeDamage 异常：" << e.what();
    } catch (...) {
        qDebug() << "[Unit] takeDamage 未知异常";
    }
}

// 治疗：不超过 maxHp
void Unit::heal(int amount)
{
    try {
        m_hp += amount;
        if (m_hp > m_maxHp) {
            m_hp = m_maxHp;
        }
    } catch (const std::exception &e) {
        qDebug() << "[Unit] heal 异常：" << e.what();
    } catch (...) {
        qDebug() << "[Unit] heal 未知异常";
    }
}

bool Unit::isAlive() const
{
    return m_hp > 0;
}

// 用于血条绘制，处理 maxHp=0 的除零保护
double Unit::getHpRatio() const
{
    if (m_maxHp <= 0) {
        return 0.0;
    }
    return static_cast<double>(m_hp) / m_maxHp;
}

// 用于蓝条绘制，处理 maxMana=0 的除零保护
double Unit::getManaRatio() const
{
    if (m_maxMana <= 0) {
        return 0.0;
    }
    return static_cast<double>(m_mana) / m_maxMana;
}

// Getter / Setter

int Unit::getId() const { return m_id; }
void Unit::setId(int id) { m_id = id; }

const QString &Unit::getName() const { return m_name; }
void Unit::setName(const QString &name) { m_name = name; }

int Unit::getHp() const { return m_hp; }
void Unit::setHp(int hp) { m_hp = hp; }
int Unit::getMaxHp() const { return m_maxHp; }
void Unit::setMaxHp(int maxHp) { m_maxHp = maxHp; }

int Unit::getAtk() const { return m_atk; }
void Unit::setAtk(int atk) { m_atk = atk; }

int Unit::getRange() const { return m_range; }
void Unit::setRange(int range) { m_range = range; }

int Unit::getMana() const { return m_mana; }
void Unit::setMana(int mana) { m_mana = mana; }
int Unit::getMaxMana() const { return m_maxMana; }
void Unit::setMaxMana(int maxMana) { m_maxMana = maxMana; }

Owner Unit::getOwner() const { return m_owner; }
void Unit::setOwner(Owner owner) { m_owner = owner; }

int Unit::getPosX() const { return m_posX; }
void Unit::setPosX(int x) { m_posX = x; }
int Unit::getPosY() const { return m_posY; }
void Unit::setPosY(int y) { m_posY = y; }

UnitState Unit::getState() const { return m_state; }
void Unit::setState(UnitState state) { m_state = state; }

int Unit::getStarLevel() const { return m_starLevel; }
void Unit::setStarLevel(int level) { m_starLevel = level; }

const QString &Unit::getTraits() const { return m_traits; }
void Unit::setTraits(const QString &traits) { m_traits = traits; }

// Phase 3 扩展方法

const QStringList &Unit::getTraitTags() const
{
    return m_traitTags;
}

void Unit::addTraitTag(const QString &tag)
{
    if (!m_traitTags.contains(tag)) {
        m_traitTags.append(tag);
    }
}

bool Unit::hasTraitTag(const QString &tag) const
{
    return m_traitTags.contains(tag);
}

int Unit::getEquipmentCount() const
{
    return m_equipmentCount;
}

void Unit::setEquipmentCount(int count)
{
    m_equipmentCount = count;
}

// 1 星最多 1 件装备，2 星及以上最多 2 件
bool Unit::canEquip() const
{
    int maxEquip = (m_starLevel >= 2) ? 2 : 1;
    return m_equipmentCount < maxEquip;
}

// Synergy Flags Getter/Setter

bool Unit::isDoubleAttackEnabled() const { return m_doubleAttackFlag; }
void Unit::setDoubleAttackEnabled(bool enabled) { m_doubleAttackFlag = enabled; }

bool Unit::isIgnoreArmorEnabled() const { return m_ignoreArmorFlag; }
void Unit::setIgnoreArmorEnabled(bool enabled) { m_ignoreArmorFlag = enabled; }

int Unit::getBonusHp() const { return m_bonusHp; }
void Unit::setBonusHp(int hp) { m_bonusHp = hp; }

int Unit::getBonusAtk() const { return m_bonusAtk; }
void Unit::setBonusAtk(int atk) { m_bonusAtk = atk; }

int Unit::getBonusArmor() const { return m_bonusArmor; }
void Unit::setBonusArmor(int armor) { m_bonusArmor = armor; }

float Unit::getManaMultiplier() const { return m_manaMultiplier; }
void Unit::setManaMultiplier(float multiplier) { m_manaMultiplier = multiplier; }

// Phase 4 高级装备 Getter/Setter

EquipmentType Unit::getEquippedType() const { return m_equippedType; }
void Unit::setEquippedType(EquipmentType type) { m_equippedType = type; }
bool Unit::isReviveEnabled() const { return m_reviveEnabled; }
void Unit::setReviveEnabled(bool enabled) { m_reviveEnabled = enabled; }
bool Unit::isRevivedThisBattle() const { return m_revivedThisBattle; }
void Unit::setRevivedThisBattle(bool revived) { m_revivedThisBattle = revived; }
float Unit::getReviveTimer() const { return m_reviveTimer; }
void Unit::setReviveTimer(float timer) { m_reviveTimer = timer; }
float Unit::getLifeStealPercent() const { return m_lifeStealPercent; }
void Unit::setLifeStealPercent(float percent) { m_lifeStealPercent = percent; }
int Unit::getHpRegen() const { return m_hpRegen; }
void Unit::setHpRegen(int regen) { m_hpRegen = regen; }

// 羁绊增益应用/移除

void Unit::applySynergyBuffs()
{
    m_maxHp += m_bonusHp;
    m_hp = m_maxHp;       // 满血进入战斗
    m_atk += m_bonusAtk;
    qDebug() << "[Unit]" << m_name << "应用羁绊增益：HP +" << m_bonusHp
             << "→" << m_maxHp << "ATK +" << m_bonusAtk << "→" << m_atk;
}

void Unit::removeSynergyBuffs()
{
    m_maxHp -= m_bonusHp;
    m_atk -= m_bonusAtk;
    if (m_hp > m_maxHp) m_hp = m_maxHp;
    qDebug() << "[Unit]" << m_name << "移除羁绊增益：HP -" << m_bonusHp
             << "→" << m_maxHp << "ATK -" << m_bonusAtk << "→" << m_atk;
}

// 战斗字段 Getter/Setter

Unit *Unit::getCurrentTarget() const { return m_currentTarget; }
void Unit::setCurrentTarget(Unit *target) { m_currentTarget = target; }
int Unit::getAttackSpeed() const { return m_attackSpeed; }
void Unit::setAttackSpeed(int speed) { m_attackSpeed = speed; }
int Unit::getMoveSpeed() const { return m_moveSpeed; }
void Unit::setMoveSpeed(int speed) { m_moveSpeed = speed; }
int Unit::getStunTimer() const { return m_stunTimer; }
void Unit::setStunTimer(int timer) { m_stunTimer = timer; }

// 战斗方法

// 每帧状态机：stun→跳过 → 无目标→索敌 → 在攻击范围内→攻击 → 否则→移动
void Unit::tick(BoardWidget *board, const QList<Unit*> &allEnemies)
{
    try {
        if (m_state == UnitState::Dead) return;

        if (m_stunTimer > 0) {
            m_stunTimer--;
            setState(UnitState::Stunned);
            return;
        }

        if (m_currentTarget == nullptr || !m_currentTarget->isAlive()) {
            m_currentTarget = findTarget(allEnemies);
            if (m_currentTarget == nullptr) return;
        }

        double dist = std::sqrt(
            std::pow(m_posX - m_currentTarget->getPosX(), 2) +
            std::pow(m_posY - m_currentTarget->getPosY(), 2));

        if (dist <= m_range) {
            m_state = UnitState::Attacking;
            m_attackTimer++;
            if (m_attackTimer >= m_attackSpeed) {
                attack(m_currentTarget, board);
                m_attackTimer = 0;
            }
        } else {
            m_state = UnitState::Moving;
            m_moveTimer++;
            if (m_moveTimer >= m_moveSpeed) {
                moveToward(m_currentTarget->getPosX(),
                           m_currentTarget->getPosY(), board);
                m_moveTimer = 0;
            }
        }

    } catch (const std::exception &e) {
        qDebug() << "[Unit] tick 异常：" << e.what();
    } catch (...) {
        qDebug() << "[Unit] tick 未知异常";
    }
}

// 索敌规则：1.欧氏距离最小 2.HP最低 3.列最小(靠左) 4.行最大(靠下)
Unit *Unit::findTarget(const QList<Unit*> &enemies) const
{
    try {
        Unit *best = nullptr;
        double bestDist = 1e9;
        int bestHp = 1e9;
        int bestCol = 999;
        int bestRow = -1;

        for (Unit *e : enemies) {
            if (e == nullptr || !e->isAlive()) continue;

            double dist = std::sqrt(
                std::pow(m_posX - e->getPosX(), 2) +
                std::pow(m_posY - e->getPosY(), 2));
            int hp = e->getHp();
            int col = e->getPosX();
            int row = e->getPosY();

            if (best == nullptr) {
                best = e;
                bestDist = dist;
                bestHp = hp;
                bestCol = col;
                bestRow = row;
                continue;
            }

            bool better = false;
            if (dist < bestDist - 0.001) {
                better = true;
            } else if (std::abs(dist - bestDist) < 0.001) {
                if (hp < bestHp) {
                    better = true;
                } else if (hp == bestHp) {
                    if (col < bestCol) {
                        better = true;
                    } else if (col == bestCol && row > bestRow) {
                        better = true;
                    }
                }
            }

            if (better) {
                best = e;
                bestDist = dist;
                bestHp = hp;
                bestCol = col;
                bestRow = row;
            }
        }
        return best;

    } catch (const std::exception &e) {
        qDebug() << "[Unit] findTarget 异常：" << e.what();
        return nullptr;
    }
}

// 向目标位置移动一步：优先 A* 寻路，A* 无路则 4 方向贪心回退
bool Unit::moveToward(int targetX, int targetY, BoardWidget *board)
{
    try {
        if (m_posX < 0 || m_posY < 0) return false;

        int dx = targetX - m_posX;
        int dy = targetY - m_posY;
        if (dx == 0 && dy == 0) return false;

        int newCol = m_posX;
        int newRow = m_posY;

        auto path = PathFinder::findPath(m_posY, m_posX, targetY, targetX, board, true);
        if (!path.isEmpty()) {
            newRow = path.first().first;
            newCol = path.first().second;
        } else {
            // A* 无路 → 4 方向贪心回退：按靠近目标的优先级尝试
            int dirs[4];
            if (std::abs(dx) >= std::abs(dy)) {
                dirs[0] = (dx > 0) ? 1 : 3;
                dirs[1] = (dy > 0) ? 2 : 0;
                dirs[2] = (dx > 0) ? 3 : 1;
                dirs[3] = (dy > 0) ? 0 : 2;
            } else {
                dirs[0] = (dy > 0) ? 2 : 0;
                dirs[1] = (dx > 0) ? 1 : 3;
                dirs[2] = (dy > 0) ? 0 : 2;
                dirs[3] = (dx > 0) ? 3 : 1;
            }

            for (int d = 0; d < 4; d++) {
                int tryRow = m_posY;
                int tryCol = m_posX;
                if (dirs[d] == 0) tryRow--;
                else if (dirs[d] == 1) tryCol++;
                else if (dirs[d] == 2) tryRow++;
                else if (dirs[d] == 3) tryCol--;

                if (tryRow < 0 || tryRow >= 8 || tryCol < 0 || tryCol >= 8) continue;
                if (!board->isCellEmpty(tryRow, tryCol)) continue;

                newRow = tryRow;
                newCol = tryCol;
                break;
            }
        }

        if (newCol == m_posX && newRow == m_posY) return false;

        board->removeUnit(m_posY, m_posX);
        m_posX = newCol;
        m_posY = newRow;
        board->forcePlaceUnit(this, m_posY, m_posX);
        return true;

    } catch (const std::exception &e) {
        qDebug() << "[Unit] moveToward 异常：" << e.what();
        return false;
    } catch (...) {
        qDebug() << "[Unit] moveToward 未知异常";
        return false;
    }
}

// 普攻：造成 m_atk 伤害，获得 10 法力值，显示攻击特效
void Unit::attack(Unit *target, BoardWidget *board)
{
    try {
        if (target == nullptr || !target->isAlive()) return;
        target->takeDamage(m_atk);
        addMana(10);

        // 吸血剑：回复 30% 伤害的 HP
        if (m_lifeStealPercent > 0.0f) {
            int healing = static_cast<int>(m_atk * m_lifeStealPercent);
            if (healing > 0) {
                heal(healing);
                qDebug().noquote() << "[吸血剑]" << m_name << "回复" << healing << "HP";
            }
        }

        // 疾风猎手羁绊：40% 概率连击
        if (m_doubleAttackFlag && target->isAlive()) {
            int roll = QRandomGenerator::global()->bounded(100);
            if (roll < 40) {
                target->takeDamage(m_atk);
                addMana(10);
                qDebug().noquote() << QString("[%1] 疾风猎手连击！再次攻击 %2，造成 %3 伤害")
                                          .arg(m_name, target->getName())
                                          .arg(m_atk);
            }
        }

        if (board != nullptr) {
            AttackEffectType effectType = AttackEffectType::Slash;
            if (m_name == QStringLiteral("弓箭手")) {
                effectType = AttackEffectType::Arrow;
            } else if (m_name == QStringLiteral("法师")) {
                effectType = AttackEffectType::Fireball;
            }
            board->addAttackEffect(m_posY, m_posX,
                                   target->getPosY(), target->getPosX(),
                                   effectType);
        }

        qDebug().noquote() << QString("[%1] 攻击 %2，造成 %3 伤害，法力 %4/%5")
                                  .arg(m_name, target->getName())
                                  .arg(m_atk)
                                  .arg(m_mana)
                                  .arg(m_maxMana);
    } catch (const std::exception &e) {
        qDebug() << "[Unit] attack 异常：" << e.what();
    }
}

// 增加法力值，受羁绊法力获取倍率影响（神圣祝福 +20%，奥术智慧 ×1.5）
void Unit::addMana(int amount)
{
    try {
        int actualAmount = static_cast<int>(amount * m_manaMultiplier);
        if (actualAmount < 0) actualAmount = 0;
        m_mana += actualAmount;
        if (m_mana > m_maxMana) m_mana = m_maxMana;
    } catch (const std::exception &e) {
        qDebug() << "[Unit] addMana 异常：" << e.what();
    }
}

// 辅助方法

void Unit::resetBattleState()
{
    m_state = UnitState::Idle;
    m_currentTarget = nullptr;
    m_attackTimer = 0;
    m_moveTimer = 0;
    m_stunTimer = 0;

    m_revivedThisBattle = false;
    m_reviveTimer = 0.0f;
}

void Unit::debugPrint() const
{
    qDebug() << "Unit[id=" << m_id
             << ", name=" << m_name
             << ", hp=" << m_hp << "/" << m_maxHp
             << ", atk=" << m_atk
             << ", range=" << m_range
             << ", man=" << m_mana << "/" << m_maxMana
             << ", owner=" << (m_owner == Owner::PlayerCtrl ? "Player" : "Enemy")
             << ", pos=(" << m_posX << "," << m_posY << ")"
             << ", state=" << static_cast<int>(m_state)
             << ", star=" << m_starLevel
             << ", traits=" << m_traits
             << "]";
}
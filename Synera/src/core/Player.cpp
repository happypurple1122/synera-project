#include "core/Player.h"



Player::Player()
    : m_gold(10)
    , m_hp(100)
    , m_level(1)
    , m_currentStage(0)    // startNewRound 会先 +1 变成第 1 轮
    , m_currentExp(0)
    , m_expToLevel(2)
{
}



void Player::addGold(int amount)
{
    try {
        if (amount <= 0) {
            qDebug() << "[Player] addGold：amount 必须为正数，收到" << amount;
            return;
        }
        m_gold += amount;
        qDebug() << "[Player] 获得" << amount << "金币，当前：" << m_gold;
    } catch (const std::exception &e) {
        qDebug() << "[Player] addGold 异常：" << e.what();
    }
}

bool Player::spendGold(int amount)
{
    try {
        if (amount <= 0) {
            qDebug() << "[Player] spendGold：amount 必须为正数，收到" << amount;
            return false;
        }
        if (m_gold < amount) {
            qDebug() << "[Player] 金币不足：需要" << amount << "，当前" << m_gold;
            return false;
        }
        m_gold -= amount;
        qDebug() << "[Player] 花费" << amount << "金币，剩余：" << m_gold;
        return true;
    } catch (const std::exception &e) {
        qDebug() << "[Player] spendGold 异常：" << e.what();
        return false;
    }
}

// 经验与升级

// 经验累计到阈值自动升级，超出部分保留到下一级
void Player::addExp(int amount)
{
    try {
        if (amount <= 0) {
            qDebug() << "[Player] addExp：amount 必须为正数，收到" << amount;
            return;
        }
        m_currentExp += amount;
        qDebug() << "[Player] 获得" << amount << "经验，当前：" << m_currentExp << "/" << m_expToLevel;

        while (m_currentExp >= m_expToLevel) {
            m_currentExp -= m_expToLevel;
            levelUp();
        }
    } catch (const std::exception &e) {
        qDebug() << "[Player] addExp 异常：" << e.what();
    }
}

// 等级+1，升级所需经验+2（线性增长）
void Player::levelUp()
{
    try {
        m_level++;
        m_expToLevel += 2;
        qDebug() << "[Player] 升级！当前等级：" << m_level
                 << "，下一级需经验：" << m_expToLevel
                 << "，人口上限：" << getMaxPopulation();
    } catch (const std::exception &e) {
        qDebug() << "[Player] levelUp 异常：" << e.what();
    }
}



bool Player::isAlive() const
{
    return m_hp > 0;
}

// Getter / Setter

int Player::getGold() const { return m_gold; }
void Player::setGold(int gold) { m_gold = gold; }

int Player::getHp() const { return m_hp; }
void Player::setHp(int hp) { if (hp < 0) hp = 0; m_hp = hp; }

int Player::getLevel() const { return m_level; }
void Player::setLevel(int level) { m_level = level; }

// 人口上限 = level + 2 + bonusPopulation
int Player::getMaxPopulation() const
{
    return m_level + 2 + m_bonusPopulation;
}

int Player::getCurrentStage() const { return m_currentStage; }
void Player::setCurrentStage(int stage) { m_currentStage = stage; }

int Player::getCurrentExp() const { return m_currentExp; }
int Player::getExpToLevel() const { return m_expToLevel; }

void Player::setCurrentExp(int exp) { m_currentExp = exp; }
void Player::setExpToLevel(int exp) { m_expToLevel = exp; }



// 花金币升等级，费用 = level × 4
bool Player::buyLevelUp()
{
    try {
        if (m_level >= MAX_LEVEL) {
            qDebug() << "[Player] 已达最高等级" << MAX_LEVEL << "，无法继续升级";
            return false;
        }
        int cost = getLevelUpCost();
        if (!spendGold(cost)) {
            qDebug() << "[Player] 金币不足，升级需要" << cost << "金币，当前" << m_gold;
            return false;
        }
        levelUp();
        qDebug() << "[Player] 花费" << cost << "金币升级成功，当前等级" << m_level;
        return true;
    } catch (const std::exception &e) {
        qDebug() << "[Player] buyLevelUp 异常：" << e.what();
        return false;
    } catch (...) {
        qDebug() << "[Player] buyLevelUp 未知异常";
        return false;
    }
}

int Player::getLevelUpCost() const
{
    if (m_level >= MAX_LEVEL) return -1;
    return m_level * 4;
}



bool Player::buyBonusPopulation()
{
    try {
        if (!spendGold(1)) {
            qDebug() << "[Player] 金币不足，无法购买额外人口";
            return false;
        }
        m_bonusPopulation++;
        qDebug() << "[Player] 花费1金币购买额外人口，当前额外+"
                 << m_bonusPopulation << "，总人口上限=" << getMaxPopulation();
        return true;
    } catch (const std::exception &e) {
        qDebug() << "[Player] buyBonusPopulation 异常：" << e.what();
        return false;
    } catch (...) {
        qDebug() << "[Player] buyBonusPopulation 未知异常";
        return false;
    }
}



void Player::debugPrint() const
{
    qDebug() << "Player[金币=" << m_gold
             << ", HP=" << m_hp
             << ", 等级=" << m_level
             << ", 人口上限=" << getMaxPopulation()
             << ", 关卡=" << m_currentStage
             << ", 经验=" << m_currentExp << "/" << m_expToLevel
             << "]";
}
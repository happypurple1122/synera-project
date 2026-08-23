#include "core/BattleSystem.h"
#include "core/Unit.h"
#include "core/Player.h"
#include "gui/BoardWidget.h"

#include <QDebug>



BattleSystem::BattleSystem(BoardWidget *board, Player *player,
                           QObject *parent)
    : QObject(parent)
    , m_board(board)
    , m_player(player)
    , m_battleInProgress(false)
{
}



void BattleSystem::startBattle()
{
    qDebug() << "[BattleSystem] 战斗开始";
    m_battleInProgress = true;
    m_enemiesKilledThisRound = 0;
    m_hpRegenAccumulator.clear();

    for (int r = 0; r < BoardWidget::GRID_ROWS; ++r) {
        for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
            Unit *unit = m_board->getUnitAt(r, c);
            if (unit != nullptr) {
                unit->resetBattleState();
            }
        }
    }
}

void BattleSystem::stopBattle()
{
    m_battleInProgress = false;
    qDebug() << "[BattleSystem] 战斗结束";
}

bool BattleSystem::isBattleInProgress() const
{
    return m_battleInProgress;
}

int BattleSystem::getEnemiesKilledThisRound() const
{
    return m_enemiesKilledThisRound;
}




void BattleSystem::onTick()
{
    if (!m_battleInProgress) return;

    try {
        
        QList<Unit*> playerUnits = getAlivePlayerUnits();
        QList<Unit*> enemyUnits  = getAliveEnemyUnits();

        if (playerUnits.isEmpty() || enemyUnits.isEmpty()) {
            checkWinCondition();
            return;
        }

        
        for (Unit *unit : playerUnits) {
            if (unit != nullptr && unit->isAlive()) {
                unit->tick(m_board, enemyUnits);
            }
        }

        for (Unit *unit : enemyUnits) {
            if (unit != nullptr && unit->isAlive()) {
                unit->tick(m_board, playerUnits);
            }
        }

        // 使用浮点累加器避免整数截断
        for (int r = 0; r < BoardWidget::GRID_ROWS; ++r) {
            for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
                Unit *unit = m_board->getUnitAt(r, c);
                if (unit != nullptr && unit->isAlive() && unit->getHpRegen() > 0) {
                    auto key = std::make_pair(r, c);
                    float &acc = m_hpRegenAccumulator[key];
                    acc += unit->getHpRegen() * 0.05f;
                    if (acc >= 1.0f) {
                        int healAmount = static_cast<int>(acc);
                        unit->heal(healAmount);
                        acc -= healAmount;
                    }
                }
            }
        }
        
        for (int r = 0; r < BoardWidget::GRID_ROWS; ++r) {
            for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
                Unit *unit = m_board->getUnitAt(r, c);
                if (unit != nullptr && !unit->isAlive()
                    && unit->isReviveEnabled() && !unit->isRevivedThisBattle()) {
                    float timer = unit->getReviveTimer();
                    timer -= 0.05f;
                    unit->setReviveTimer(timer);
                    if (timer <= 0.0f) {
                        unit->setHp(unit->getMaxHp() / 2);
                        unit->setRevivedThisBattle(true);
                        unit->setState(UnitState::Idle);
                        qDebug().noquote() << "[复活甲]" << unit->getName() << "复活！HP=" << unit->getHp();
                    }
                }
            }
        }

        
        triggerSkills();

        m_board->update();

        m_board->tickEffects();

        removeDeadUnits();

        checkWinCondition();

    } catch (const std::exception &e) {
        qDebug() << "[BattleSystem] onTick 异常：" << e.what();
    } catch (...) {
        qDebug() << "[BattleSystem] onTick 未知异常";
    }
}



QList<Unit*> BattleSystem::getAlivePlayerUnits() const
{
    QList<Unit*> result;
    for (int r = 0; r < BoardWidget::GRID_ROWS; ++r) {
        for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
            Unit *unit = m_board->getUnitAt(r, c);
            if (unit != nullptr &&
                unit->getOwner() == Owner::PlayerCtrl &&
                unit->isAlive()) {
                result.append(unit);
            }
        }
    }
    return result;
}

QList<Unit*> BattleSystem::getAliveEnemyUnits() const
{
    QList<Unit*> result;
    for (int r = 0; r < BoardWidget::GRID_ROWS; ++r) {
        for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
            Unit *unit = m_board->getUnitAt(r, c);
            if (unit != nullptr &&
                unit->getOwner() == Owner::EnemyCtrl &&
                unit->isAlive()) {
                result.append(unit);
            }
        }
    }
    return result;
}



// 玩家先放技能，敌方后放；玩家技能先打死敌人，敌人就放不出技能
void BattleSystem::triggerSkills()
{
    try {
        QList<Unit*> playerUnits = getAlivePlayerUnits();
        QList<Unit*> enemyUnits  = getAliveEnemyUnits();

        // 玩家单位释放技能
        for (Unit *unit : playerUnits) {
            if (unit == nullptr || !unit->isAlive()) continue;
            if (unit->getMana() >= unit->getMaxMana()) {
                QList<Unit*> currentEnemies = getAliveEnemyUnits();
                if (!currentEnemies.isEmpty()) {
                    unit->castSkill(currentEnemies);
                    unit->setMana(0);
                    qDebug().noquote() << QString("[BattleSystem] %1 释放技能完毕，法力已清空")
                                              .arg(unit->getName());

                    if (!currentEnemies.isEmpty()) {
                        Unit *firstTarget = currentEnemies.first();
                        AttackEffectType skillEffect = AttackEffectType::Slash;
                        if (unit->getName() == QStringLiteral("法师")) {
                            skillEffect = AttackEffectType::Fireball;
                        } else if (unit->getName() == QStringLiteral("弓箭手")) {
                            skillEffect = AttackEffectType::Arrow;
                        } else if (unit->getName() == QStringLiteral("刺客")) {
                            skillEffect = AttackEffectType::Backstab;
                        }
                        m_board->addAttackEffect(
                            unit->getPosY(), unit->getPosX(),
                            firstTarget->getPosY(), firstTarget->getPosX(),
                            skillEffect);
                    }
                }
            }
        }

        // 敌方单位释放技能
        for (Unit *unit : enemyUnits) {
            if (unit == nullptr || !unit->isAlive()) continue;
            if (unit->getMana() >= unit->getMaxMana()) {
                QList<Unit*> currentPlayers = getAlivePlayerUnits();
                if (!currentPlayers.isEmpty()) {
                    unit->castSkill(currentPlayers);
                    unit->setMana(0);
                    qDebug().noquote() << QString("[BattleSystem] 敌方%1 释放技能完毕，法力已清空")
                                              .arg(unit->getName());

                    if (!currentPlayers.isEmpty()) {
                        Unit *firstTarget = currentPlayers.first();
                        AttackEffectType skillEffect = AttackEffectType::Slash;
                        if (unit->getName() == QStringLiteral("法师")) {
                            skillEffect = AttackEffectType::Fireball;
                        } else if (unit->getName() == QStringLiteral("弓箭手")) {
                            skillEffect = AttackEffectType::Arrow;
                        } else if (unit->getName() == QStringLiteral("刺客")) {
                            skillEffect = AttackEffectType::Backstab;
                        }
                        m_board->addAttackEffect(
                            unit->getPosY(), unit->getPosX(),
                            firstTarget->getPosY(), firstTarget->getPosX(),
                            skillEffect);
                    }
                }
            }
        }

    } catch (const std::exception &e) {
        qDebug() << "[BattleSystem] triggerSkills 异常：" << e.what();
    } catch (...) {
        qDebug() << "[BattleSystem] triggerSkills 未知异常";
    }
}



// 从棋盘移除死亡单位（只移除引用，不 delete），复活甲等待复活中的单位保留
void BattleSystem::removeDeadUnits()
{
    try {
        for (int r = 0; r < BoardWidget::GRID_ROWS; ++r) {
            for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
                Unit *unit = m_board->getUnitAt(r, c);
                if (unit != nullptr && !unit->isAlive()) {
                    // 复活甲：跳过等待复活中的单位
                    if (unit->isReviveEnabled() && !unit->isRevivedThisBattle()) {
                        continue;
                    }
                    if (unit->getOwner() == Owner::EnemyCtrl) {
                        m_enemiesKilledThisRound++;
                    }
                    qDebug().noquote() << QString("[BattleSystem] %1 已阵亡，从棋盘(%2,%3)移除")
                                              .arg(unit->getName())
                                              .arg(r)
                                              .arg(c);
                    m_board->removeUnit(r, c);
                    unit->setPosX(-1);
                    unit->setPosY(-1);
                }
            }
        }
    } catch (const std::exception &e) {
        qDebug() << "[BattleSystem] removeDeadUnits 异常：" << e.what();
    } catch (...) {
        qDebug() << "[BattleSystem] removeDeadUnits 未知异常";
    }
}



// 玩家全灭/同归于尽 → 玩家输；敌方全灭 → 玩家赢
void BattleSystem::checkWinCondition()
{
    try {
        if (!m_battleInProgress) return;

        int playerCount = getAlivePlayerUnits().size();
        int enemyCount  = getAliveEnemyUnits().size();

        if (playerCount == 0 && enemyCount > 0) {
            m_battleInProgress = false;
            qDebug().noquote() << QString("[BattleSystem] 第%1轮 战斗失败")
                                      .arg(m_player->getCurrentStage());
            emit battleFinished(false);
            return;
        }

        if (enemyCount == 0 && playerCount > 0) {
            m_battleInProgress = false;
            qDebug().noquote() << QString("[BattleSystem] 第%1轮 战斗胜利")
                                      .arg(m_player->getCurrentStage());
            emit battleFinished(true);
            return;
        }

        // 同归于尽 → 判定玩家失败
        if (playerCount == 0 && enemyCount == 0) {
            m_battleInProgress = false;
            qDebug().noquote() << QString("[BattleSystem] 第%1轮 同归于尽，判定失败")
                                      .arg(m_player->getCurrentStage());
            emit battleFinished(false);
            return;
        }

    } catch (const std::exception &e) {
        qDebug() << "[BattleSystem] checkWinCondition 异常：" << e.what();
    } catch (...) {
        qDebug() << "[BattleSystem] checkWinCondition 未知异常";
    }
}
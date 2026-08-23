#include "core/GameManager.h"
#include "gui/BoardWidget.h"
#include "gui/BenchWidget.h"
#include "gui/DragDropMgr.h"
#include "core/Hero_Warrior.h"
#include "core/Hero_Mage.h"
#include "core/Hero_Archer.h"
#include "core/Hero_Assassin.h"

#include <QDebug>

GameManager::GameManager(BoardWidget *board, BenchWidget *bench,
                         DragDropMgr *dragDropMgr, Player *player,
                         QList<Unit*> &enemyUnits,
                         QObject *parent)
    : QObject(parent)
    , m_currentPhase(GamePhase::Preparation)
    , m_board(board)
    , m_bench(bench)
    , m_dragDropMgr(dragDropMgr)
    , m_player(player)
    , m_enemyUnits(enemyUnits)
{
    m_battleSystem = new BattleSystem(m_board, m_player, this);

    m_battleTimer = new QTimer(this);
    m_battleTimer->setInterval(50);  // 50ms/帧 ≈ 20FPS
    connect(m_battleTimer, &QTimer::timeout,
            this, &GameManager::onBattleTick);

    m_settlementTimer = new QTimer(this);
    m_settlementTimer->setInterval(2000);
    m_settlementTimer->setSingleShot(true);
    connect(m_settlementTimer, &QTimer::timeout,
            this, &GameManager::onSettlementTimeout);

    m_settlementDelayTimer = new QTimer(this);
    m_settlementDelayTimer->setInterval(500);  // 500ms 延迟让死亡标记可见
    m_settlementDelayTimer->setSingleShot(true);
    connect(m_settlementDelayTimer, &QTimer::timeout,
            this, &GameManager::onSettlementDelayTimeout);

    connect(m_battleSystem, &BattleSystem::battleFinished,
            this, &GameManager::onBattleFinished);
}

void GameManager::startNewRound()
{
    try {
        if (!m_gameActive) {
            qDebug() << "[GameManager] 游戏已结束，无法开始新轮次";
            return;
        }

        qDebug() << "\n========== [GameManager] 开始第" << (m_player->getCurrentStage() + 1) << "轮 ==========";

        // 清空棋盘上的敌方单位
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < BoardWidget::GRID_COLS; ++col) {
                Unit *unit = m_board->getUnitAt(row, col);
                if (unit != nullptr && unit->getOwner() == Owner::EnemyCtrl) {
                    m_board->removeUnit(row, col);
                }
            }
        }

        // 销毁上一轮敌方单位
        if (!m_enemyUnits.isEmpty()) {
            EnemyWaveGenerator::clearWave(m_enemyUnits);
        }

        // 轮次 +1
        m_player->setCurrentStage(m_player->getCurrentStage() + 1);

        // 生成并部署敌方单位
        m_enemyUnits = EnemyWaveGenerator::generateWave(m_player->getCurrentStage());
        EnemyWaveGenerator::deployWave(m_board, m_enemyUnits);

        // 进入战斗阶段
        enterBattlePhase();

        qDebug() << "========== 第" << m_player->getCurrentStage() << "轮准备完毕 ==========\n";

    } catch (const std::exception &e) {
        qDebug() << "[GameManager] startNewRound 异常：" << e.what();
    } catch (...) {
        qDebug() << "[GameManager] startNewRound 未知异常";
    }
}

GamePhase GameManager::getCurrentPhase() const
{
    return m_currentPhase;
}

void GameManager::resetToPreparation()
{
    if (m_battleTimer->isActive()) m_battleTimer->stop();
    if (m_settlementTimer->isActive()) m_settlementTimer->stop();
    if (m_settlementDelayTimer->isActive()) m_settlementDelayTimer->stop();

    if (m_dragDropMgr) m_dragDropMgr->setEnabled(true);

    m_currentPhase = GamePhase::Preparation;
    m_pendingBattleResult = false;

    emit phaseChanged(m_currentPhase);

    qDebug() << "[GameManager] resetToPreparation：已重置到准备阶段";
}

BattleSystem *GameManager::getBattleSystem() const
{
    return m_battleSystem;
}

void GameManager::enterPreparationPhase()
{
    m_currentPhase = GamePhase::Preparation;

    m_board->clearAttackEffects();

    m_dragDropMgr->setEnabled(true);

    m_board->update();

    qDebug() << "[GameManager] → 进入【准备阶段】";
    emit phaseChanged(m_currentPhase);
}

void GameManager::enterBattlePhase()
{
    m_currentPhase = GamePhase::Battle;

    m_dragDropMgr->setEnabled(false);

    m_battleTimer->stop();
    m_settlementTimer->stop();

    m_battleSystem->startBattle();

    // 应用羁绊增益到战斗属性
    for (int r = 0; r < BoardWidget::GRID_ROWS; ++r) {
        for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
            Unit *unit = m_board->getUnitAt(r, c);
            if (unit != nullptr && unit->getOwner() == Owner::PlayerCtrl) {
                unit->applySynergyBuffs();
            }
        }
    }

    m_battleTimer->start();

    qDebug() << "[GameManager] → 进入【战斗阶段】";
    emit phaseChanged(m_currentPhase);
}

void GameManager::enterSettlementPhase(bool playerWon)
{
    m_currentPhase = GamePhase::Settlement;

    m_battleTimer->stop();
    m_settlementTimer->stop();
    m_battleSystem->stopBattle();

    // 移除羁绊增益，恢复基础属性
    for (int r = 0; r < BoardWidget::GRID_ROWS; ++r) {
        for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
            Unit *unit = m_board->getUnitAt(r, c);
            if (unit != nullptr && unit->getOwner() == Owner::PlayerCtrl) {
                unit->removeSynergyBuffs();
            }
        }
    }

    m_dragDropMgr->setEnabled(true);

    int goldChange = 0;
    int expChange = 0;
    int hpChange = 0;

    if (playerWon) {
        goldChange = 5;
        expChange = 2;
        hpChange = 0;
        m_player->addGold(goldChange);
        m_player->addExp(expChange);
        qDebug().noquote() << QString("[GameManager] 第%1轮 胜利，+%2金 +%3经验")
                                  .arg(m_player->getCurrentStage())
                                  .arg(goldChange)
                                  .arg(expChange);
    } else {
        goldChange = 2;
        expChange = 1;
        hpChange = -3;
        m_player->addGold(goldChange);
        m_player->addExp(expChange);
        int newHp = m_player->getHp() + hpChange;
        if (newHp < 0) newHp = 0;
        m_player->setHp(newHp);
        qDebug().noquote() << QString("[GameManager] 第%1轮 失败，+%2金 +%3经验 -%4血")
                                  .arg(m_player->getCurrentStage())
                                  .arg(goldChange)
                                  .arg(expChange)
                                  .arg(-hpChange);
    }

    emit settlementResult(playerWon, goldChange, expChange, hpChange);

    // 通关检测：轮次 >= 10
    if (m_player->getCurrentStage() >= 10) {
        m_gameActive = false;
        qDebug().noquote() << "[GameManager] 通关！全部 10 轮已通过";
        emit victory();
        emit phaseChanged(m_currentPhase);
        return;
    }

    // 游戏结束检测：玩家 HP ≤ 0
    if (!m_player->isAlive()) {
        m_gameActive = false;
        qDebug().noquote() << "[GameManager] 游戏结束，玩家已死亡";
        emit gameOver();
        emit phaseChanged(m_currentPhase);
        return;
    }

    qDebug() << "[GameManager] → 进入【结算阶段】";
    emit phaseChanged(m_currentPhase);

    m_settlementTimer->start();
}

void GameManager::onBattleTick()
{
    try {
        m_battleSystem->onTick();
    } catch (const std::exception &e) {
        qDebug() << "[GameManager] onBattleTick 异常：" << e.what();
    } catch (...) {
        qDebug() << "[GameManager] onBattleTick 未知异常";
    }
}

void GameManager::onBattleFinished(bool playerWon)
{
    try {
        m_battleTimer->stop();
        m_battleSystem->stopBattle();

        m_board->update();

        // 500ms 延迟后进入结算，让死亡标记有渲染时间
        m_pendingBattleResult = playerWon;
        m_settlementDelayTimer->start(500);
    } catch (const std::exception &e) {
        qDebug() << "[GameManager] onBattleFinished 异常：" << e.what();
    } catch (...) {
        qDebug() << "[GameManager] onBattleFinished 未知异常";
    }
}

void GameManager::onSettlementDelayTimeout()
{
    try {
        enterSettlementPhase(m_pendingBattleResult);
    } catch (const std::exception &e) {
        qDebug() << "[GameManager] onSettlementDelayTimeout 异常：" << e.what();
    } catch (...) {
        qDebug() << "[GameManager] onSettlementDelayTimeout 未知异常";
    }
}

void GameManager::onSettlementTimeout()
{
    try {
        enterPreparationPhase();
    } catch (const std::exception &e) {
        qDebug() << "[GameManager] onSettlementTimeout 异常：" << e.what();
    } catch (...) {
        qDebug() << "[GameManager] onSettlementTimeout 未知异常";
    }
}
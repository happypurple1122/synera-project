#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QObject>
#include <QTimer>

#include "core/BattleSystem.h"
#include "core/Player.h"
#include "core/EnemyWaveGenerator.h"

class BoardWidget;
class BenchWidget;
class DragDropMgr;

// 三阶段状态枚举：准备 → 战斗 → 结算 → 准备
enum class GamePhase {
    Preparation,
    Battle,
    Settlement
};

// 游戏状态机管理器，管理三阶段切换、战斗驱动、资源结算
class GameManager : public QObject
{
    Q_OBJECT

public:
    explicit GameManager(BoardWidget *board, BenchWidget *bench,
                         DragDropMgr *dragDropMgr, Player *player,
                         QList<Unit*> &enemyUnits,
                         QObject *parent = nullptr);

    ~GameManager() override = default;

    // 准备阶段点击"开始作战"时调用：清场→生敌→部署→进入战斗
    void startNewRound();

    // 重置到准备阶段（读档时调用）
    void resetToPreparation();

    GamePhase getCurrentPhase() const;

    BattleSystem *getBattleSystem() const;

signals:
    void phaseChanged(GamePhase newPhase);

    // 结算数据信号：playerWon, goldChange, expChange, hpChange
    void settlementResult(bool playerWon, int goldChange, int expChange, int hpChange);

    // 玩家 HP ≤ 0 时发射
    void gameOver();

    // 轮次 > 10 时发射
    void victory();

private slots:
    // 战斗帧更新（由 QTimer 每 50ms 触发）
    void onBattleTick();

    // 接收 BattleSystem 的战斗结束信号
    void onBattleFinished(bool playerWon);

    // 结算阶段超时回调（2 秒后自动回到准备阶段）
    void onSettlementTimeout();

    // 结算延迟超时回调（500ms，让死亡标记有渲染时间）
    void onSettlementDelayTimeout();

private:
    void enterPreparationPhase();
    void enterBattlePhase();
    void enterSettlementPhase(bool playerWon);

    GamePhase m_currentPhase;
    bool m_gameActive = true;
    bool m_pendingBattleResult = false;
    BattleSystem *m_battleSystem;
    QTimer *m_battleTimer;             // 帧循环计时器（50ms）
    QTimer *m_settlementTimer;         // 结算等待计时器（2 秒）
    QTimer *m_settlementDelayTimer;    // 结算延迟计时器（500ms）

    BoardWidget *m_board;
    BenchWidget *m_bench;
    DragDropMgr *m_dragDropMgr;
    Player *m_player;

    QList<Unit*> &m_enemyUnits;
};

#endif // GAMEMANAGER_H
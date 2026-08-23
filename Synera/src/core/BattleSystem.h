#ifndef BATTLESYSTEM_H
#define BATTLESYSTEM_H

#include <QObject>
#include <QList>
#include <QMap>
#include <utility>

class Unit;
class BoardWidget;
class Player;

// 战斗系统核心管理器，驱动所有单位每帧更新、技能触发、胜负判定
class BattleSystem : public QObject
{
    Q_OBJECT

public:
    explicit BattleSystem(BoardWidget *board, Player *player,
                          QObject *parent = nullptr);

    ~BattleSystem() override = default;

    // 开始战斗，重置所有单位战斗状态
    void startBattle();

    void stopBattle();

    bool isBattleInProgress() const;

    int getEnemiesKilledThisRound() const;

public slots:
    // 每帧更新（由外部 QTimer 每 50ms 触发）
    void onTick();

signals:
    // 战斗结束信号，playerWon=true 表示玩家胜利
    void battleFinished(bool playerWon);

private:
    // 收集棋盘上所有存活的玩家方单位
    QList<Unit*> getAlivePlayerUnits() const;

    // 收集棋盘上所有存活的敌方单位
    QList<Unit*> getAliveEnemyUnits() const;

    // 检查胜负：一方全灭则发射 battleFinished
    void checkWinCondition();

    // 触发所有法力值满的单位的技能
    void triggerSkills();

    // 从棋盘上清理所有死亡单位
    void removeDeadUnits();

    BoardWidget *m_board;
    Player *m_player;
    bool m_battleInProgress;
    int m_enemiesKilledThisRound = 0;
    // HP回复累加器（避免整数截断，时光杖每帧累积 5*0.05=0.25）
    QMap<std::pair<int,int>, float> m_hpRegenAccumulator;
};

#endif // BATTLESYSTEM_H
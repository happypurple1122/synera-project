#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>

#include "gui/BoardWidget.h"
#include "gui/BenchWidget.h"
#include "gui/DragDropMgr.h"
#include "gui/ShopWidget.h"
#include "gui/SynergyPanelWidget.h"
#include "gui/EquipmentBarWidget.h"
#include "core/Player.h"
#include "core/Shop.h"
#include "core/SynergySystem.h"
#include "core/SaveLoadManager.h"
#include "core/StarUpSystem.h"
#include "core/EnemyWaveGenerator.h"
#include "core/GameManager.h"

// 游戏主窗口，组合棋盘+备战区+商店+羁绊面板+装备栏
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // 构造函数，创建所有子控件并布局
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override = default;

    BoardWidget *getBoardWidget() const;
    BenchWidget *getBenchWidget() const;
    Player &getPlayer();

    void updateInfoLabel();
    void recalcSynergy();            // 重新计算羁绊增益
    int getPlayerUnitCount() const;  // 统计棋盘上玩家单位数量

private slots:
    // 游戏阶段切换时的 UI 更新
    void onPhaseChanged(GamePhase newPhase);

    // 结算结果处理
    void onSettlementResult(bool playerWon, int goldChange, int expChange, int hpChange);

    void onGameOver();   // 玩家 HP ≤ 0 时调用
    void onVictory();    // 轮次 > 10 时调用

    void onSaveGame();   // 保存游戏
    void onLoadGame();   // 读取存档

    void showStarUpPopup(const QString &heroName); // 升星成功弹窗
    void onBuyPopulation();                        // 花金币买人口
    void onBuyLevelUp();                           // 花金币升级等级

    // 装备合成成功
    void onEquipmentCrafted(const QString &heroName, const QString &equipName);

private:
    
    BoardWidget *m_boardWidget;
    BenchWidget *m_benchWidget;
    DragDropMgr *m_dragDropMgr;
    QLabel *m_infoLabel;
    QPushButton *m_startBtn;
    QPushButton *m_saveBtn;
    QPushButton *m_loadBtn;
    QPushButton *m_buyPopBtn;
    QPushButton *m_buyLevelUpBtn;

    
    GameManager *m_gameManager;

    
    QLabel *m_settlementPopup;
    QTimer *m_popupTimer;

    
    GamePhase m_currentPhase;

    // 商店系统
    Shop *m_shop;
    ShopWidget *m_shopWidget;

    
    SynergySystem *m_synergySystem;
    SynergyPanelWidget *m_synergyPanel;

    
    StarUpSystem *m_starUpSystem;

    
    EquipmentBarWidget *m_equipmentBar;

    
    bool m_isLoading = false;  // 正在读档中（防止 onPhaseChanged 刷新商店覆盖存档数据）

    
    Player m_player;
    QList<Unit *> m_enemyUnits;  // 当前轮敌方单位集合
};

#endif // MAINWINDOW_H
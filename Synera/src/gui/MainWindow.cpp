#include "gui/MainWindow.h"
#include "core/Hero_Warrior.h"
#include "core/Hero_Mage.h"
#include "core/Hero_Archer.h"
#include "core/Hero_Assassin.h"

#include <QApplication>
#include <QScreen>
#include <QFont>
#include <QMessageBox>
#include <QScrollArea>

#include "core/Unit.h"
#include "core/Equipment.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Synera - 自走棋"));
    setFixedSize(1100, 720);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(10);

    // 左侧：棋盘组件
    m_boardWidget = new BoardWidget(this);
    mainLayout->addWidget(m_boardWidget);

    // 右侧面板（垂直布局）
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    // 顶部信息栏
    m_infoLabel = new QLabel(this);
    m_infoLabel->setFixedHeight(40);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setStyleSheet(
        "background-color: #2C2C2C;"
        "color: #FFFFFF;"
        "font-size: 16px;"
        "font-weight: bold;"
        "border-radius: 6px;"
    );
    rightLayout->addWidget(m_infoLabel);

    // QScrollArea：包裹所有可滚动的内容
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(8);

    // 备战区组件
    m_benchWidget = new BenchWidget(scrollContent);
    scrollLayout->addWidget(m_benchWidget, 0, Qt::AlignCenter);

    // 装备栏 + 说明标签
    QVBoxLayout *eqColumn = new QVBoxLayout();
    eqColumn->setSpacing(2);
    QLabel *eqLabel = new QLabel(QStringLiteral("装备栏（点击选中→拖到单位上穿戴）"), scrollContent);
    eqLabel->setStyleSheet(
        "color: #AAAAAA;"
        "font-size: 10px;"
        "padding: 0;"
        "margin: 0;"
    );
    eqLabel->setFixedHeight(14);
    eqLabel->setAlignment(Qt::AlignCenter);
    eqColumn->addWidget(eqLabel);
    m_equipmentBar = new EquipmentBarWidget(scrollContent);
    eqColumn->addWidget(m_equipmentBar, 0, Qt::AlignCenter);
    scrollLayout->addLayout(eqColumn);

    // 花1金币+1人口 和 花金币升级等级 按钮
    m_buyPopBtn = new QPushButton(QStringLiteral("+1 人口 ($1)"), scrollContent);
    m_buyPopBtn->setFixedSize(120, 28);
    m_buyPopBtn->setCursor(Qt::PointingHandCursor);
    m_buyPopBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #FF7043;"
        "  color: #FFFFFF;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "  border-radius: 6px;"
        "  border: none;"
        "}"
        "QPushButton:hover { background-color: #FF8A65; }"
        "QPushButton:pressed { background-color: #E64A19; }"
    );

    m_buyLevelUpBtn = new QPushButton(QStringLiteral("升等级 ($4)"), scrollContent);
    m_buyLevelUpBtn->setFixedSize(120, 28);
    m_buyLevelUpBtn->setCursor(Qt::PointingHandCursor);
    m_buyLevelUpBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #42A5F5;"
        "  color: #FFFFFF;"
        "  font-size: 12px;"
        "  font-weight: bold;"
        "  border-radius: 6px;"
        "  border: none;"
        "}"
        "QPushButton:hover { background-color: #64B5F6; }"
        "QPushButton:pressed { background-color: #1E88E5; }"
    );

    QHBoxLayout *buyPopRow = new QHBoxLayout();
    buyPopRow->addStretch();
    buyPopRow->addWidget(m_buyPopBtn);
    buyPopRow->addSpacing(8);
    buyPopRow->addWidget(m_buyLevelUpBtn);
    buyPopRow->addStretch();
    scrollLayout->addLayout(buyPopRow);

    // 商店系统
    m_shop = new Shop();
    m_shopWidget = new ShopWidget(scrollContent);
    m_shopWidget->setShop(m_shop);
    m_shopWidget->setPlayer(&m_player);
    m_shopWidget->setBench(m_benchWidget);
    m_shop->refresh(1);
    m_shop->refreshEquipment(1);
    m_shopWidget->updateDisplay();
    scrollLayout->addWidget(m_shopWidget, 0, Qt::AlignCenter);

    // 连接商店购买信号 → 更新信息栏 + 升星检查 + 羁绊
    connect(m_shopWidget, &ShopWidget::unitPurchased, this, [this](const QString &heroName) {
        updateInfoLabel();
        bool merged = false;
        if (m_starUpSystem) {
            merged = m_starUpSystem->checkAndMerge(m_benchWidget, m_boardWidget);
            if (merged) {
                showStarUpPopup(heroName);
                m_boardWidget->update();
                m_benchWidget->update();
            }
        }
        recalcSynergy();
        updateInfoLabel();
    });

    // 连接商店装备购买信号 → 装备加入装备栏
    connect(m_shopWidget, &ShopWidget::equipmentPurchased, this, [this](EquipmentType type) {
        if (m_equipmentBar != nullptr) {
            m_equipmentBar->addEquipment(type);
            qDebug().noquote() << "[MainWindow] 商店购买装备:" << Equipment::getInfo(type).name;
        }
        updateInfoLabel();
    });

    // 羁绊面板
    m_synergySystem = new SynergySystem();
    m_synergyPanel = new SynergyPanelWidget(scrollContent);
    scrollLayout->addWidget(m_synergyPanel);

    // 升星系统
    m_starUpSystem = new StarUpSystem();

    scrollLayout->addStretch(1);

    scrollArea->setWidget(scrollContent);
    rightLayout->addWidget(scrollArea, 1);

    // 底部"开始作战"按钮
    m_startBtn = new QPushButton(QStringLiteral("⚔ 开始作战 ⚔"), this);
    m_startBtn->setFixedHeight(60);
    m_startBtn->setCursor(Qt::PointingHandCursor);
    m_startBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #4A7CF7;"
        "  color: #FFFFFF;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "  border-radius: 8px;"
        "  border: none;"
        "}"
        "QPushButton:hover {"
        "  background-color: #5A8CF7;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #3A6CE7;"
        "}"
    );
    rightLayout->addWidget(m_startBtn);

    // 底部按钮行：保存 + 读档
    QHBoxLayout *saveLoadLayout = new QHBoxLayout();
    saveLoadLayout->setSpacing(8);

    m_saveBtn = new QPushButton(QStringLiteral("💾 保存"), this);
    m_saveBtn->setFixedHeight(40);
    m_saveBtn->setCursor(Qt::PointingHandCursor);
    m_saveBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #FF9800;"
        "  color: #FFFFFF;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  border-radius: 6px;"
        "  border: none;"
        "  padding: 0 10px;"
        "}"
        "QPushButton:hover { background-color: #FFB74D; }"
        "QPushButton:pressed { background-color: #F57C00; }"
    );

    m_loadBtn = new QPushButton(QStringLiteral("📂 读档"), this);
    m_loadBtn->setFixedHeight(40);
    m_loadBtn->setCursor(Qt::PointingHandCursor);
    m_loadBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #78909C;"
        "  color: #FFFFFF;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  border-radius: 6px;"
        "  border: none;"
        "  padding: 0 10px;"
        "}"
        "QPushButton:hover { background-color: #90A4AE; }"
        "QPushButton:pressed { background-color: #607D8B; }"
    );

    saveLoadLayout->addWidget(m_saveBtn);
    saveLoadLayout->addWidget(m_loadBtn);
    rightLayout->addLayout(saveLoadLayout);

    mainLayout->addWidget(rightPanel, 1);

    // 创建拖拽管理器
    m_dragDropMgr = new DragDropMgr(m_boardWidget, m_benchWidget,
                                    &m_player, m_equipmentBar, this);

    // 拖拽完成后更新信息栏和羁绊
    connect(m_dragDropMgr, &DragDropMgr::dragCompleted, this, [this]() {
        updateInfoLabel();
        recalcSynergy();
    });

    // 装备合成成功 → 显示合成通知
    connect(m_dragDropMgr, &DragDropMgr::equipmentCrafted,
            this, &MainWindow::onEquipmentCrafted);

    // 装备栏内拖拽合成成功 → 显示合成通知
    connect(m_equipmentBar, &EquipmentBarWidget::synthesized, this, [this](EquipmentType type) {
        QString equipName = Equipment::getInfo(type).name;
        onEquipmentCrafted(QStringLiteral("装备栏"), equipName);
    });

    // 创建游戏状态机管理器
    m_gameManager = new GameManager(m_boardWidget, m_benchWidget,
                                    m_dragDropMgr, &m_player,
                                    m_enemyUnits, this);

    connect(m_startBtn, &QPushButton::clicked,
            m_gameManager, &GameManager::startNewRound);
    connect(m_gameManager, &GameManager::phaseChanged,
            this, &MainWindow::onPhaseChanged);
    connect(m_gameManager, &GameManager::settlementResult,
            this, &MainWindow::onSettlementResult);
    connect(m_gameManager, &GameManager::gameOver,
            this, &MainWindow::onGameOver);
    connect(m_gameManager, &GameManager::victory,
            this, &MainWindow::onVictory);

    connect(m_saveBtn, &QPushButton::clicked,
            this, &MainWindow::onSaveGame);
    connect(m_loadBtn, &QPushButton::clicked,
            this, &MainWindow::onLoadGame);
    connect(m_buyPopBtn, &QPushButton::clicked,
            this, &MainWindow::onBuyPopulation);
    connect(m_buyLevelUpBtn, &QPushButton::clicked,
            this, &MainWindow::onBuyLevelUp);

    // 创建结算弹窗
    m_settlementPopup = new QLabel(centralWidget);
    m_settlementPopup->setAlignment(Qt::AlignCenter);
    m_settlementPopup->setFixedSize(400, 120);
    m_settlementPopup->setStyleSheet(
        "background-color: rgba(0, 0, 0, 200);"
        "color: #FFFFFF;"
        "font-size: 28px;"
        "font-weight: bold;"
        "border-radius: 15px;"
        "border: 2px solid #FFFFFF;"
    );
    int popupX = (480 - 400) / 2;
    int popupY = (480 - 120) / 2 + 40;
    m_settlementPopup->move(popupX, popupY);
    m_settlementPopup->hide();

    m_popupTimer = new QTimer(this);
    m_popupTimer->setInterval(2000);
    m_popupTimer->setSingleShot(true);
    connect(m_popupTimer, &QTimer::timeout, this, [this]() {
        m_settlementPopup->hide();
    });

    // 初始给备战区放几个测试单位（包含4种职业）
    m_benchWidget->addUnit(new Hero_Warrior(1, Owner::PlayerCtrl));
    m_benchWidget->addUnit(new Hero_Mage(2, Owner::PlayerCtrl));
    m_benchWidget->addUnit(new Hero_Archer(3, Owner::PlayerCtrl));
    m_benchWidget->addUnit(new Hero_Assassin(4, Owner::PlayerCtrl));

    updateInfoLabel();
    recalcSynergy();
}



BoardWidget *MainWindow::getBoardWidget() const
{
    return m_boardWidget;
}

BenchWidget *MainWindow::getBenchWidget() const
{
    return m_benchWidget;
}



Player &MainWindow::getPlayer()
{
    return m_player;
}

// 更新信息栏

void MainWindow::updateInfoLabel()
{
    int stage = m_player.getCurrentStage();
    int gold  = m_player.getGold();
    int hp    = m_player.getHp();
    int level = m_player.getLevel();
    int pop   = m_player.getMaxPopulation();
    int exp   = m_player.getCurrentExp();
    int expToLevel = m_player.getExpToLevel();

    QString phaseName;
    switch (m_currentPhase) {
    case GamePhase::Preparation:
        phaseName = QStringLiteral("准备阶段");
        break;
    case GamePhase::Battle:
        phaseName = QStringLiteral("⚔ 战斗阶段 ⚔");
        break;
    case GamePhase::Settlement:
        phaseName = QStringLiteral("结算阶段");
        break;
    default:
        phaseName = QStringLiteral("未知");
        break;
    }

    QString infoText = QStringLiteral(
        "【%1】[第%2轮]    金币: %3  |  血量: %4  |  等级: %5  |  人口: %6/%7  |  经验: %8/%9"
    ).arg(phaseName)
     .arg(stage)
     .arg(gold)
     .arg(hp)
     .arg(level)
     .arg(getPlayerUnitCount())
     .arg(pop)
     .arg(exp)
     .arg(expToLevel);

    m_infoLabel->setText(infoText);

    int levelUpCost = m_player.getLevelUpCost();
    if (levelUpCost < 0) {
        m_buyLevelUpBtn->setText(QStringLiteral("已达最高级"));
        m_buyLevelUpBtn->setEnabled(false);
    } else {
        m_buyLevelUpBtn->setText(QStringLiteral("升等级 ($%1)").arg(levelUpCost));
    }
}



int MainWindow::getPlayerUnitCount() const
{
    int count = 0;
    for (int r = 4; r < BoardWidget::GRID_ROWS; ++r) {
        for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
            Unit *u = m_boardWidget->getUnitAt(r, c);
            if (u != nullptr && u->getOwner() == Owner::PlayerCtrl) {
                count++;
            }
        }
    }
    return count;
}



void MainWindow::recalcSynergy()
{
    try {
        if (m_synergySystem == nullptr || m_synergyPanel == nullptr) return;

        QList<Unit*> allPlayerUnits;

        for (int i = 0; i < BenchWidget::BENCH_SIZE; ++i) {
            Unit *u = m_benchWidget->getUnitAt(i);
            if (u != nullptr && u->getOwner() == Owner::PlayerCtrl) {
                allPlayerUnits.append(u);
            }
        }

        for (int r = 4; r < BoardWidget::GRID_ROWS; ++r) {
            for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
                Unit *u = m_boardWidget->getUnitAt(r, c);
                if (u != nullptr && u->getOwner() == Owner::PlayerCtrl) {
                    allPlayerUnits.append(u);
                }
            }
        }

        m_synergySystem->calculate(allPlayerUnits);

        m_synergyPanel->updateDisplay(
            m_synergySystem->getTraitCounts(),
            m_synergySystem->getActiveThresholds(),
            m_synergySystem->getAllTraits()
        );

    } catch (const std::exception &e) {
        qDebug() << "[MainWindow] recalcSynergy 异常：" << e.what();
    } catch (...) {
        qDebug() << "[MainWindow] recalcSynergy 未知异常";
    }
}



void MainWindow::onPhaseChanged(GamePhase newPhase)
{
    try {
        m_currentPhase = newPhase;

        switch (newPhase) {
        case GamePhase::Preparation:
            m_startBtn->setEnabled(true);
            m_startBtn->setText(QStringLiteral("⚔ 开始作战 ⚔"));
            m_buyPopBtn->setEnabled(true);
            m_buyLevelUpBtn->setEnabled(true);

            if (m_shopWidget) {
                m_shopWidget->setWidgetEnabled(true);
                // 读档时跳过刷新以保留存档中的商店数据
                if (!m_isLoading) {
                    int currentStage = m_player.getCurrentStage();
                    if (currentStage > 0) {
                        m_shop->refresh(currentStage);
                        m_shop->refreshEquipment(currentStage);
                    } else {
                        m_shop->refresh(1);
                        m_shop->refreshEquipment(1);
                    }
                    m_shopWidget->updateDisplay();
                }
            }

            recalcSynergy();
            m_startBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #4A7CF7;"
                "  color: #FFFFFF;"
                "  font-size: 18px;"
                "  font-weight: bold;"
                "  border-radius: 8px;"
                "  border: none;"
                "}"
                "QPushButton:hover {"
                "  background-color: #5A8CF7;"
                "}"
                "QPushButton:pressed {"
                "  background-color: #3A6CE7;"
                "}"
            );
            break;

        case GamePhase::Battle:
            m_startBtn->setEnabled(false);
            m_startBtn->setText(QStringLiteral("⏳ 战斗中 ..."));

            if (m_shopWidget) {
                m_shopWidget->setWidgetEnabled(false);
            }
            m_buyPopBtn->setEnabled(false);
            m_buyLevelUpBtn->setEnabled(false);
            m_startBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #6A6A6A;"
                "  color: #CCCCCC;"
                "  font-size: 18px;"
                "  font-weight: bold;"
                "  border-radius: 8px;"
                "  border: none;"
                "}"
            );
            break;

        case GamePhase::Settlement:
            m_buyPopBtn->setEnabled(false);
            m_buyLevelUpBtn->setEnabled(false);
            if (m_shopWidget) {
                m_shopWidget->setWidgetEnabled(false);
            }
            break;
        }

        updateInfoLabel();

    } catch (const std::exception &e) {
        qDebug() << "[MainWindow] onPhaseChanged 异常：" << e.what();
    } catch (...) {
        qDebug() << "[MainWindow] onPhaseChanged 未知异常";
    }
}

void MainWindow::onSettlementResult(bool playerWon, int goldChange,
                                    int expChange, int hpChange)
{
    try {
        updateInfoLabel();

        QString popupText;
        if (playerWon) {
            popupText = QStringLiteral("🏆 胜利！\n+%1 金币  +%2 经验").arg(goldChange).arg(expChange);
            m_settlementPopup->setStyleSheet(
                "background-color: rgba(76, 175, 80, 200);"
                "color: #FFFFFF;"
                "font-size: 24px;"
                "font-weight: bold;"
                "border-radius: 15px;"
                "border: 2px solid #FFFFFF;"
            );
        } else {
            QString hpText = (hpChange < 0)
                ? QStringLiteral("-%1 血量").arg(-hpChange)
                : QStringLiteral("血量不变");
            popupText = QStringLiteral("💀 失败！\n+%1 金币  +%2 经验  %3")
                            .arg(goldChange).arg(expChange).arg(hpText);
            m_settlementPopup->setStyleSheet(
                "background-color: rgba(244, 67, 54, 200);"
                "color: #FFFFFF;"
                "font-size: 24px;"
                "font-weight: bold;"
                "border-radius: 15px;"
                "border: 2px solid #FFFFFF;"
            );
        }
        m_settlementPopup->setText(popupText);
        m_settlementPopup->show();
        m_settlementPopup->raise();
        m_popupTimer->start();

        if (playerWon) {
            m_startBtn->setText(QStringLiteral("⚔ 进入下一轮 ⚔"));
            m_startBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #4CAF50;"
                "  color: #FFFFFF;"
                "  font-size: 18px;"
                "  font-weight: bold;"
                "  border-radius: 8px;"
                "  border: none;"
                "}"
                "QPushButton:hover {"
                "  background-color: #5CBF60;"
                "}"
                "QPushButton:pressed {"
                "  background-color: #3C9F40;"
                "}"
            );
        } else {
            m_startBtn->setText(QStringLiteral("⚔ 重新挑战 ⚔"));
            m_startBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #F44336;"
                "  color: #FFFFFF;"
                "  font-size: 18px;"
                "  font-weight: bold;"
                "  border-radius: 8px;"
                "  border: none;"
                "}"
                "QPushButton:hover {"
                "  background-color: #FF5346;"
                "}"
                "QPushButton:pressed {"
                "  background-color: #D43326;"
                "}"
            );

            if (!m_player.isAlive()) {
                m_startBtn->setEnabled(false);
                m_startBtn->setText(QStringLiteral("💀 游戏结束 💀"));
                qDebug() << "[MainWindow] 玩家死亡，游戏结束！";
            }
        }

        // 装备掉落判定：每击杀一个敌人必定掉落一件基础装备
        {
            int killCount = m_gameManager->getBattleSystem()->getEnemiesKilledThisRound();
            if (killCount <= 0) {
                killCount = 1;  // 保底：至少掉落一件
            }
            for (int i = 0; i < killCount; ++i) {
                EquipmentType dropped = Equipment::rollDrop();
                if (dropped != EquipmentType::None && m_equipmentBar != nullptr) {
                    EquipmentInfo info = Equipment::getInfo(dropped);
                    m_equipmentBar->addEquipment(dropped);
                    qDebug().noquote() << "[MainWindow] 掉落装备:" << info.name;
                }
            }
            qDebug().noquote() << "[MainWindow] 击杀数：" << killCount;
        }

        qDebug().noquote() << QString("[MainWindow] 结算：%1，金币%2%3，经验%4%5，血量%6%7")
                                  .arg(playerWon ? "胜利" : "失败")
                                  .arg(goldChange >= 0 ? "+" : "")
                                  .arg(goldChange)
                                  .arg(expChange >= 0 ? "+" : "")
                                  .arg(expChange)
                                  .arg(hpChange >= 0 ? "+" : "")
                                  .arg(hpChange);

    } catch (const std::exception &e) {
        qDebug() << "[MainWindow] onSettlementResult 异常：" << e.what();
    } catch (...) {
        qDebug() << "[MainWindow] onSettlementResult 未知异常";
    }
}



void MainWindow::onGameOver()
{
    try {
        m_startBtn->setEnabled(false);
        m_startBtn->setText(QStringLiteral("💀 游戏结束 💀"));
        m_startBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: #333333;"
            "  color: #FF4444;"
            "  font-size: 20px;"
            "  font-weight: bold;"
            "  border-radius: 8px;"
            "  border: 2px solid #FF4444;"
            "}"
        );

        m_popupTimer->stop();

        m_settlementPopup->setText(QStringLiteral("💀 游戏结束 💀\n你的征程到此为止"));
        m_settlementPopup->setStyleSheet(
            "background-color: rgba(0, 0, 0, 220);"
            "color: #FF4444;"
            "font-size: 28px;"
            "font-weight: bold;"
            "border-radius: 15px;"
            "border: 3px solid #FF4444;"
        );
        m_settlementPopup->show();
        m_settlementPopup->raise();

        qDebug() << "[MainWindow] 游戏结束！玩家死亡";

    } catch (const std::exception &e) {
        qDebug() << "[MainWindow] onGameOver 异常：" << e.what();
    } catch (...) {
        qDebug() << "[MainWindow] onGameOver 未知异常";
    }
}

// 通关胜利处理

void MainWindow::onVictory()
{
    try {
        m_startBtn->setEnabled(false);
        m_startBtn->setText(QStringLiteral("🏆 通关！"));
        m_startBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: #FFD700;"
            "  color: #8B4513;"
            "  font-size: 20px;"
            "  font-weight: bold;"
            "  border-radius: 8px;"
            "  border: 2px solid #FFD700;"
            "}"
        );

        m_settlementPopup->setText(QStringLiteral("🏆 通关胜利！🏆\n恭喜你通过了所有考验！"));
        m_settlementPopup->setStyleSheet(
            "background-color: rgba(255, 215, 0, 220);"
            "color: #8B4513;"
            "font-size: 28px;"
            "font-weight: bold;"
            "border-radius: 15px;"
            "border: 3px solid #FFD700;"
        );
        m_settlementPopup->show();
        m_settlementPopup->raise();

        qDebug() << "[MainWindow] 通关！全部 10 轮已通过";

    } catch (const std::exception &e) {
        qDebug() << "[MainWindow] onVictory 异常：" << e.what();
    } catch (...) {
        qDebug() << "[MainWindow] onVictory 未知异常";
    }
}

// 保存游戏

void MainWindow::onSaveGame()
{
    try {
        if (m_currentPhase == GamePhase::Battle || m_currentPhase == GamePhase::Settlement) {
            QMessageBox::information(this, QStringLiteral("提示"),
                                     QStringLiteral("战斗阶段不能存档！"));
            return;
        }

        qDebug() << "\n========== [MainWindow] 存档前状态 ==========";
        qDebug() << "  玩家: 金币=" << m_player.getGold()
                 << " 血量=" << m_player.getHp()
                 << " 等级=" << m_player.getLevel()
                 << " 轮次=" << m_player.getCurrentStage()
                 << " 人口=" << getPlayerUnitCount() << "/" << m_player.getMaxPopulation();
        int benchCount = 0;
        for (int i = 0; i < BenchWidget::BENCH_SIZE; ++i)
            if (m_benchWidget->getUnitAt(i) != nullptr) benchCount++;
        qDebug() << "  备战区: " << benchCount << "个单位";
        int boardPlayerCount = 0;
        for (int r = 0; r < BoardWidget::GRID_ROWS; ++r)
            for (int c = 0; c < BoardWidget::GRID_COLS; ++c)
                if (m_boardWidget->getUnitAt(r, c) != nullptr &&
                    m_boardWidget->getUnitAt(r, c)->getOwner() == Owner::PlayerCtrl)
                    boardPlayerCount++;
        qDebug() << "  棋盘玩家单位: " << boardPlayerCount;

        bool success = SaveLoadManager::saveGame(0, &m_player,
                                                   m_benchWidget, m_boardWidget,
                                                   m_shop, m_equipmentBar);
        if (success) {
            QMessageBox::information(this, QStringLiteral("存档"),
                                     QStringLiteral("存档成功！"));
        } else {
            QMessageBox::warning(this, QStringLiteral("存档"),
                                 QStringLiteral("存档失败，请重试！"));
        }
    } catch (const std::exception &e) {
        qDebug() << "[MainWindow] onSaveGame 异常：" << e.what();
    } catch (...) {
        qDebug() << "[MainWindow] onSaveGame 未知异常";
    }
}

void MainWindow::onLoadGame()
{
    try {
        if (m_currentPhase == GamePhase::Battle || m_currentPhase == GamePhase::Settlement) {
            QMessageBox::information(this, QStringLiteral("提示"),
                                     QStringLiteral("战斗阶段不能读档！"));
            return;
        }

        if (!SaveLoadManager::isValidSave(0)) {
            QMessageBox::information(this, QStringLiteral("读档"),
                                     QStringLiteral("没有找到存档！"));
            return;
        }

        qDebug() << "\n========== [MainWindow] 读档前状态 ==========";
        qDebug() << "  玩家: 金币=" << m_player.getGold()
                 << " 血量=" << m_player.getHp()
                 << " 等级=" << m_player.getLevel()
                 << " 轮次=" << m_player.getCurrentStage()
                 << " 人口=" << getPlayerUnitCount() << "/" << m_player.getMaxPopulation();
        int benchCount = 0;
        for (int i = 0; i < BenchWidget::BENCH_SIZE; ++i) {
            if (m_benchWidget->getUnitAt(i) != nullptr) benchCount++;
        }
        qDebug() << "  备战区: " << benchCount << "个单位";
        int boardPlayerCount = 0;
        for (int r = 4; r < BoardWidget::GRID_ROWS; ++r)
            for (int c = 0; c < BoardWidget::GRID_COLS; ++c)
                if (m_boardWidget->getUnitAt(r, c) != nullptr &&
                    m_boardWidget->getUnitAt(r, c)->getOwner() == Owner::PlayerCtrl)
                    boardPlayerCount++;
        qDebug() << "  棋盘玩家单位: " << boardPlayerCount;

        bool success = SaveLoadManager::loadGame(0, &m_player,
                                                   m_benchWidget, m_boardWidget,
                                                   m_shop, m_equipmentBar);
        if (success) {
            qDebug() << "\n========== [MainWindow] 读档后状态 ==========";
            qDebug() << "  玩家: 金币=" << m_player.getGold()
                     << " 血量=" << m_player.getHp()
                     << " 等级=" << m_player.getLevel()
                     << " 轮次=" << m_player.getCurrentStage()
                     << " 人口=" << getPlayerUnitCount() << "/" << m_player.getMaxPopulation();
            benchCount = 0;
            for (int i = 0; i < BenchWidget::BENCH_SIZE; ++i) {
                Unit *u = m_benchWidget->getUnitAt(i);
                if (u != nullptr) {
                    benchCount++;
                    qDebug() << "    备战区[" << i << "]:" << u->getName() << "星级=" << u->getStarLevel();
                }
            }
            qDebug() << "  备战区: " << benchCount << "个单位";
            boardPlayerCount = 0;
            for (int r = 4; r < BoardWidget::GRID_ROWS; ++r) {
                for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
                    Unit *u = m_boardWidget->getUnitAt(r, c);
                    if (u != nullptr && u->getOwner() == Owner::PlayerCtrl) {
                        boardPlayerCount++;
                        qDebug() << "    棋盘[" << r << "," << c << "]:" << u->getName() << "星级=" << u->getStarLevel();
                    }
                }
            }
            qDebug() << "  棋盘玩家单位: " << boardPlayerCount;
            for (int i = 0; i < 5; ++i) {
                auto slot = m_shop->getSlot(i);
                if (slot.has_value())
                    qDebug() << "  商店[" << i << "]:" << slot->heroName << "价格=" << slot->price;
                else
                    qDebug() << "  商店[" << i << "]:空";
            }

            EnemyWaveGenerator::clearWave(m_enemyUnits);

            // 设置 m_isLoading = true 跳过商店刷新，保留存档中的商店数据
            m_isLoading = true;
            m_gameManager->resetToPreparation();
            m_isLoading = false;

            if (m_shopWidget) {
                m_shopWidget->updateDisplay();
            }
            m_boardWidget->update();
            m_benchWidget->update();
            updateInfoLabel();
            recalcSynergy();
            QMessageBox::information(this, QStringLiteral("读档"),
                                     QStringLiteral("读档成功！"));
        } else {
            QMessageBox::warning(this, QStringLiteral("读档"),
                                 QStringLiteral("读档失败，存档可能已损坏！"));
        }
    } catch (const std::exception &e) {
        qDebug() << "[MainWindow] onLoadGame 异常：" << e.what();
    } catch (...) {
        qDebug() << "[MainWindow] onLoadGame 未知异常";
    }
}



void MainWindow::onBuyPopulation()
{
    try {
        if (m_currentPhase != GamePhase::Preparation) {
            QMessageBox::information(this, QStringLiteral("提示"),
                                     QStringLiteral("只能在准备阶段购买人口！"));
            return;
        }

        if (m_player.buyBonusPopulation()) {
            updateInfoLabel();
            qDebug() << "[MainWindow] 购买额外人口成功，当前人口上限="
                     << m_player.getMaxPopulation();
        } else {
            QMessageBox::information(this, QStringLiteral("提示"),
                                     QStringLiteral("金币不足！需要 1 金币。"));
        }
    } catch (const std::exception &e) {
        qDebug() << "[MainWindow] onBuyPopulation 异常：" << e.what();
    } catch (...) {
        qDebug() << "[MainWindow] onBuyPopulation 未知异常";
    }
}

// 花金币升级等级

void MainWindow::onBuyLevelUp()
{
    try {
        if (m_currentPhase != GamePhase::Preparation) {
            QMessageBox::information(this, QStringLiteral("提示"),
                                     QStringLiteral("只能在准备阶段升级等级！"));
            return;
        }

        int cost = m_player.getLevelUpCost();
        if (cost < 0) {
            QMessageBox::information(this, QStringLiteral("提示"),
                                     QStringLiteral("已达最高等级！"));
            return;
        }

        if (m_player.buyLevelUp()) {
            updateInfoLabel();
            qDebug() << "[MainWindow] 升级成功，当前等级="
                     << m_player.getLevel() << "，人口上限="
                     << m_player.getMaxPopulation();
        } else {
            QMessageBox::information(this, QStringLiteral("提示"),
                                     QStringLiteral("金币不足！升级需要 %1 金币。").arg(cost));
        }
    } catch (const std::exception &e) {
        qDebug() << "[MainWindow] onBuyLevelUp 异常：" << e.what();
    } catch (...) {
        qDebug() << "[MainWindow] onBuyLevelUp 未知异常";
    }
}

void MainWindow::showStarUpPopup(const QString &heroName)
{
    try {
        QString msg = QStringLiteral("⭐ 升星成功！%1 → 2星 ⭐").arg(heroName);
        m_settlementPopup->setText(msg);
        m_settlementPopup->setStyleSheet(
            "background-color: rgba(0, 0, 0, 220);"
            "color: #FFD700;"
            "font-size: 22px;"
            "font-weight: bold;"
            "border-radius: 15px;"
            "border: 2px solid #FFD700;"
        );
        int popupX = (480 - 400) / 2;
        int popupY = (480 - 120) / 2 + 40;
        m_settlementPopup->move(popupX, popupY);
        m_settlementPopup->show();
        m_popupTimer->start();
    } catch (const std::exception &e) {
        qDebug() << "[MainWindow] showStarUpPopup 异常：" << e.what();
    } catch (...) {
        qDebug() << "[MainWindow] showStarUpPopup 未知异常";
    }
}



void MainWindow::onEquipmentCrafted(const QString &heroName, const QString &equipName)
{
    try {
        QString msg = QStringLiteral("⚡ 装备合成！%1 → %2 ⚡").arg(heroName, equipName);
        m_settlementPopup->setText(msg);
        m_settlementPopup->setStyleSheet(
            "background-color: rgba(255, 215, 0, 220);"
            "color: #8B4513;"
            "font-size: 24px;"
            "font-weight: bold;"
            "border-radius: 15px;"
            "border: 2px solid #FFD700;"
        );
        int popupX = (480 - 400) / 2;
        int popupY = (480 - 120) / 2 + 40;
        m_settlementPopup->move(popupX, popupY);
        m_settlementPopup->show();
        m_popupTimer->start();

        updateInfoLabel();
        recalcSynergy();

        qDebug().noquote() << "[装备合成]" << heroName << "获得" << equipName;
    } catch (const std::exception &e) {
        qDebug() << "[MainWindow] onEquipmentCrafted 异常：" << e.what();
    } catch (...) {
        qDebug() << "[MainWindow] onEquipmentCrafted 未知异常";
    }
}
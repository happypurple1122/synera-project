#include "gui/ShopWidget.h"
#include "core/Shop.h"
#include "core/Player.h"
#include "gui/BenchWidget.h"
#include "core/Unit.h"
#include "core/Equipment.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QRandomGenerator>

ShopWidget::ShopWidget(QWidget *parent)
    : QWidget(parent)
    , m_shop(nullptr)
    , m_player(nullptr)
    , m_bench(nullptr)
{
    setupUI();
}

void ShopWidget::setShop(Shop *shop)
{
    m_shop = shop;
}

void ShopWidget::setPlayer(Player *player)
{
    m_player = player;
}

void ShopWidget::setBench(BenchWidget *bench)
{
    m_bench = bench;
}

void ShopWidget::setupUI()
{
    try {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(4, 4, 4, 4);
        mainLayout->setSpacing(5);

        // 英雄槽位 + 刷新按钮
        QHBoxLayout *heroRow = new QHBoxLayout();
        heroRow->setSpacing(4);

        for (int i = 0; i < 5; ++i) {
            QVBoxLayout *slotLayout = new QVBoxLayout();
            slotLayout->setContentsMargins(0, 0, 0, 0);
            slotLayout->setSpacing(2);

            QLabel *label = new QLabel(QStringLiteral("空"), this);
            label->setAlignment(Qt::AlignCenter);
            label->setFixedSize(60, 26);
            label->setStyleSheet(
                "QLabel {"
                "  background-color: #3A3A3A;"
                "  color: #CCCCCC;"
                "  font-size: 10px;"
                "  font-weight: bold;"
                "  border-radius: 4px;"
                "  border: 1px solid #555555;"
                "}"
            );
            m_slotLabels[i] = label;
            slotLayout->addWidget(label);

            QPushButton *btn = new QPushButton(QStringLiteral("买"), this);
            btn->setFixedSize(60, 24);
            btn->setCursor(Qt::PointingHandCursor);
            btn->setEnabled(false);
            btn->setStyleSheet(
                "QPushButton {"
                "  background-color: #4A7CF7;"
                "  color: #FFFFFF;"
                "  font-size: 10px;"
                "  font-weight: bold;"
                "  border-radius: 4px;"
                "  border: none;"
                "}"
                "QPushButton:hover {"
                "  background-color: #5A8CF7;"
                "}"
                "QPushButton:pressed {"
                "  background-color: #3A6CE7;"
                "}"
                "QPushButton:disabled {"
                "  background-color: #555555;"
                "  color: #999999;"
                "}"
            );
            m_slotButtons[i] = btn;
            slotLayout->addWidget(btn);

            heroRow->addLayout(slotLayout);

            connect(btn, &QPushButton::clicked, this, [this, i]() {
                onSlotClicked(i);
            });
        }

        // 刷新按钮
        m_refreshBtn = new QPushButton(QStringLiteral("刷新\n2金"), this);
        m_refreshBtn->setFixedSize(48, 56);
        m_refreshBtn->setCursor(Qt::PointingHandCursor);
        m_refreshBtn->setEnabled(false);
        m_refreshBtn->setStyleSheet(
            "QPushButton {"
            "  background-color: #FF9800;"
            "  color: #FFFFFF;"
            "  font-size: 9px;"
            "  font-weight: bold;"
            "  border-radius: 4px;"
            "  border: none;"
            "}"
            "QPushButton:hover {"
            "  background-color: #FFB74D;"
            "}"
            "QPushButton:pressed {"
            "  background-color: #F57C00;"
            "}"
            "QPushButton:disabled {"
            "  background-color: #555555;"
            "  color: #999999;"
            "}"
        );
        heroRow->addWidget(m_refreshBtn);
        heroRow->addStretch();

        mainLayout->addLayout(heroRow);

        // 装备商店
        QHBoxLayout *equipRow = new QHBoxLayout();
        equipRow->setSpacing(4);

        QLabel *sep = new QLabel(QStringLiteral("装备"), this);
        sep->setAlignment(Qt::AlignCenter);
        sep->setFixedSize(40, 52);
        sep->setStyleSheet(
            "QLabel {"
            "  color: #FFD700;"
            "  font-size: 10px;"
            "  font-weight: bold;"
            "}"
        );
        equipRow->addWidget(sep);

        for (int i = 0; i < 2; ++i) {
            QVBoxLayout *eqLayout = new QVBoxLayout();
            eqLayout->setContentsMargins(0, 0, 0, 0);
            eqLayout->setSpacing(2);

            QLabel *eqLabel = new QLabel(QStringLiteral("空"), this);
            eqLabel->setAlignment(Qt::AlignCenter);
            eqLabel->setFixedSize(60, 26);
            eqLabel->setStyleSheet(
                "QLabel {"
                "  background-color: #3A3A3A;"
                "  color: #FFD700;"
                "  font-size: 10px;"
                "  font-weight: bold;"
                "  border-radius: 4px;"
                "  border: 1px solid #FFD700;"
                "}"
            );
            m_equipLabels[i] = eqLabel;
            eqLayout->addWidget(eqLabel);

            QPushButton *eqBtn = new QPushButton(QStringLiteral("买"), this);
            eqBtn->setFixedSize(60, 24);
            eqBtn->setCursor(Qt::PointingHandCursor);
            eqBtn->setEnabled(false);
            eqBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #FF9800;"
                "  color: #FFFFFF;"
                "  font-size: 10px;"
                "  font-weight: bold;"
                "  border-radius: 4px;"
                "  border: none;"
                "}"
                "QPushButton:hover {"
                "  background-color: #FFB74D;"
                "}"
                "QPushButton:pressed {"
                "  background-color: #F57C00;"
                "}"
                "QPushButton:disabled {"
                "  background-color: #555555;"
                "  color: #999999;"
                "}"
            );
            m_equipBtns[i] = eqBtn;
            eqLayout->addWidget(eqBtn);

            equipRow->addLayout(eqLayout);

            connect(eqBtn, &QPushButton::clicked, this, [this, i]() {
                onEquipSlotClicked(i);
            });
        }

        equipRow->addStretch();
        mainLayout->addLayout(equipRow);

        setFixedHeight(130);

        connect(m_refreshBtn, &QPushButton::clicked, this, [this]() {
            onRefreshClicked();
        });

    } catch (const std::exception &e) {
        qDebug() << "[ShopWidget] setupUI 异常：" << e.what();
    }
}

void ShopWidget::updateDisplay()
{
    try {
        if (m_shop == nullptr || m_player == nullptr || m_bench == nullptr) {
            for (int i = 0; i < 5; ++i)
                m_slotLabels[i]->setText(QStringLiteral("-"));
            m_refreshBtn->setText(QStringLiteral("刷新\n--"));
            m_refreshBtn->setEnabled(false);
            for (int i = 0; i < 2; ++i)
                m_equipLabels[i]->setText(QStringLiteral("-"));
            return;
        }

        int playerGold = m_player->getGold();
        bool benchFull = m_bench->isFull();

        for (int i = 0; i < 5; ++i) {
            auto slotOpt = m_shop->getSlot(i);
            if (slotOpt.has_value()) {
                const ShopSlot &slot = slotOpt.value();
                m_slotLabels[i]->setText(
                    QStringLiteral("%1\n%2G").arg(slot.heroName).arg(slot.price));
                m_slotButtons[i]->setEnabled((playerGold >= slot.price) && !benchFull);
            } else {
                m_slotLabels[i]->setText(QStringLiteral("售罄"));
                m_slotButtons[i]->setEnabled(false);
            }
        }

        for (int i = 0; i < 2; ++i) {
            auto eqOpt = m_shop->getEquipSlot(i);
            if (eqOpt.has_value()) {
                const EquipmentShopSlot &eqSlot = eqOpt.value();
                QString eqName = Equipment::getInfo(eqSlot.type).name;
                m_equipLabels[i]->setText(
                    QStringLiteral("%1\n%2G").arg(eqName).arg(eqSlot.price));
                m_equipBtns[i]->setEnabled(playerGold >= eqSlot.price);
            } else {
                m_equipLabels[i]->setText(QStringLiteral("空"));
                m_equipBtns[i]->setEnabled(false);
            }
        }

        bool canRefresh = (playerGold >= Shop::REFRESH_COST);
        m_refreshBtn->setEnabled(canRefresh);
        m_refreshBtn->setText(
            QStringLiteral("刷新\n%1G").arg(Shop::REFRESH_COST));

    } catch (const std::exception &e) {
        qDebug() << "[ShopWidget] updateDisplay 异常：" << e.what();
    } catch (...) {
        qDebug() << "[ShopWidget] updateDisplay 未知异常";
    }
}

void ShopWidget::onSlotClicked(int index)
{
    try {
        if (m_shop == nullptr || m_player == nullptr || m_bench == nullptr) {
            qDebug() << "[ShopWidget] 数据源未设置";
            return;
        }

        Unit *boughtUnit = m_shop->buy(index, *m_player, *m_bench);
        if (boughtUnit != nullptr) {
            qDebug() << "[ShopWidget] 购买英雄成功：" << boughtUnit->getName();
            emit unitPurchased(boughtUnit->getName());
            updateDisplay();
        } else {
            qDebug() << "[ShopWidget] 购买英雄失败";
            updateDisplay();
        }
    } catch (const std::exception &e) {
        qDebug() << "[ShopWidget] onSlotClicked 异常：" << e.what();
    } catch (...) {
        qDebug() << "[ShopWidget] onSlotClicked 未知异常";
    }
}

void ShopWidget::onEquipSlotClicked(int index)
{
    try {
        if (m_shop == nullptr || m_player == nullptr) {
            qDebug() << "[ShopWidget] 装备数据源未设置";
            return;
        }

        EquipmentType boughtType = m_shop->buyEquipment(index, *m_player);
        if (boughtType != EquipmentType::None) {
            QString eqName = Equipment::getInfo(boughtType).name;
            qDebug() << "[ShopWidget] 装备购买成功：" << eqName;
            emit equipmentPurchased(boughtType);
            updateDisplay();
        } else {
            qDebug() << "[ShopWidget] 装备购买失败";
            updateDisplay();
        }
    } catch (const std::exception &e) {
        qDebug() << "[ShopWidget] onEquipSlotClicked 异常：" << e.what();
    } catch (...) {
        qDebug() << "[ShopWidget] onEquipSlotClicked 未知异常";
    }
}

void ShopWidget::onRefreshClicked()
{
    try {
        if (m_shop == nullptr || m_player == nullptr) return;

        if (m_player->getGold() < Shop::REFRESH_COST) {
            qDebug() << "[ShopWidget] 金币不足，无法刷新";
            return;
        }

        // 扣金币并刷新
        m_player->spendGold(Shop::REFRESH_COST);
        int currentStage = m_player->getCurrentStage();
        if (currentStage <= 0) currentStage = 1;
        m_shop->refresh(currentStage);
        m_shop->refreshEquipment(currentStage);
        updateDisplay();
        qDebug() << "[ShopWidget] 商店已刷新（阶段" << currentStage << "）";

    } catch (const std::exception &e) {
        qDebug() << "[ShopWidget] onRefreshClicked 异常：" << e.what();
    } catch (...) {
        qDebug() << "[ShopWidget] onRefreshClicked 未知异常";
    }
}

void ShopWidget::setWidgetEnabled(bool enabled)
{
    for (int i = 0; i < 5; ++i)
        m_slotButtons[i]->setEnabled(enabled);
    for (int i = 0; i < 2; ++i)
        m_equipBtns[i]->setEnabled(enabled);
    m_refreshBtn->setEnabled(enabled);
}
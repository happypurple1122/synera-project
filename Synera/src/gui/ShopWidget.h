#ifndef SHOPWIDGET_H
#define SHOPWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>

#include "core/Equipment.h"

class Shop;
class Player;
class BenchWidget;

// 商店 UI 组件，显示 5 个英雄槽位 + 2 个装备槽位 + 刷新按钮
class ShopWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ShopWidget(QWidget *parent = nullptr);
    ~ShopWidget() override = default;

    void setShop(Shop *shop);
    void setPlayer(Player *player);
    void setBench(BenchWidget *bench);

    // 根据 Shop 当前槽位数据刷新按钮和标签
    void updateDisplay();

    // 战斗阶段禁用，准备阶段启用
    void setWidgetEnabled(bool enabled);

signals:
    // 购买英雄成功后发射
    void unitPurchased(const QString &heroName);

    // 购买装备成功后发射
    void equipmentPurchased(EquipmentType type);

private:
    Shop *m_shop = nullptr;
    Player *m_player = nullptr;
    BenchWidget *m_bench = nullptr;

    QPushButton *m_slotButtons[5];   // 5 个英雄购买按钮
    QLabel *m_slotLabels[5];         // 5 个英雄信息标签
    QPushButton *m_equipBtns[2];     // 2 个装备购买按钮
    QLabel *m_equipLabels[2];        // 2 个装备信息标签
    QLabel *m_equipTitleLabel;       // 装备商店标题
    QPushButton *m_refreshBtn;       // 刷新按钮

    // 初始化 UI 布局
    void setupUI();

    // 点击英雄购买按钮
    void onSlotClicked(int index);

    // 点击装备购买按钮
    void onEquipSlotClicked(int index);

    // 点击刷新按钮（花费 2 金币）
    void onRefreshClicked();
};

#endif // SHOPWIDGET_H
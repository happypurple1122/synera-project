#ifndef SYNERGYPANELWIDGET_H
#define SYNERGYPANELWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMap>

#include "core/SynergySystem.h"

// 羁绊面板 UI 组件，使用 QPainter 绘制羁绊激活状态
class SynergyPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SynergyPanelWidget(QWidget *parent = nullptr);
    ~SynergyPanelWidget() override = default;

    // 传入标签统计、激活阈值和羁绊定义，触发重绘
    void updateDisplay(const QMap<QString, int> &traitCounts,
                       const QMap<QString, int> &activeThresholds,
                       const QList<SynergyTrait> &allTraits);

protected:
    // 逐行绘制羁绊：名称+计数+进度条+增益描述
    void paintEvent(QPaintEvent *event) override;

private:
    // 根据羁绊标签获取职业专属颜色（已激活/未激活）
    QColor getColorForTag(const QString &tag, bool isActive) const;

    QMap<QString, int> m_traitCounts;       // 标签 -> 单位数量
    QMap<QString, int> m_activeThresholds;  // 标签 -> 激活的最高阈值
    QList<SynergyTrait> m_allTraits;        // 所有羁绊定义
};

#endif // SYNERGYPANELWIDGET_H
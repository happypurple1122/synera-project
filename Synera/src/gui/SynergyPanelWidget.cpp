#include "gui/SynergyPanelWidget.h"
#include <QDebug>

SynergyPanelWidget::SynergyPanelWidget(QWidget *parent)
    : QWidget(parent)
{
    try {
        setMinimumSize(200, 220);
        setStyleSheet(
            "SynergyPanelWidget {"
            "  border: 2px solid #FFD700;"   // 金色边框
            "  border-radius: 8px;"
            "  background-color: #1a1a2e;"    // 深紫黑底色
            "}"
        );

        qDebug() << "[SynergyPanelWidget] 羁绊面板初始化完成，size=" << size();
    } catch (const std::exception &e) {
        qDebug() << "[SynergyPanelWidget] 构造函数异常：" << e.what();
    }
}

// 职业颜色映射
QColor SynergyPanelWidget::getColorForTag(const QString &tag, bool isActive) const
{
    struct TagColor { QString tag; QColor active; QColor inactive; };
    static const TagColor colors[] = {
        { QStringLiteral("战士"),   QColor(239, 83, 80),   QColor(160, 80, 70) },   // 红色
        { QStringLiteral("法师"),   QColor(126, 87, 194),  QColor(90, 60, 130) },   // 紫色
        { QStringLiteral("游侠"),   QColor(76, 175, 80),   QColor(60, 120, 65) },   // 绿色
        { QStringLiteral("刺客"),   QColor(80, 80, 90),    QColor(50, 50, 55) },    // 黑色
        { QStringLiteral("守护者"), QColor(141, 110, 99),  QColor(100, 80, 70) },   // 棕色
        { QStringLiteral("圣职者"), QColor(255, 248, 225), QColor(180, 170, 150) } // 米金色
    };

    for (const auto &c : colors) {
        if (tag == c.tag) {
            return isActive ? c.active : c.inactive;
        }
    }
    // 未知标签：默认绿色/灰色
    return isActive ? QColor(76, 255, 80) : QColor(200, 200, 200);
}

void SynergyPanelWidget::updateDisplay(
    const QMap<QString, int> &traitCounts,
    const QMap<QString, int> &activeThresholds,
    const QList<SynergyTrait> &allTraits)
{
    try {
        m_traitCounts = traitCounts;
        m_activeThresholds = activeThresholds;
        m_allTraits = allTraits;
        update();  // 触发重绘
        qDebug() << "[SynergyPanelWidget] updateDisplay 调用，羁绊数=" << allTraits.size()
                 << "size=" << size();
    } catch (const std::exception &e) {
        qDebug() << "[SynergyPanelWidget] updateDisplay 异常：" << e.what();
    }
}

void SynergyPanelWidget::paintEvent(QPaintEvent *event)
{
    try {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        painter.fillRect(rect(), QColor(26, 26, 46));

        painter.fillRect(0, 0, width(), 24, QColor(255, 215, 0, 40));
        QFont titleFont = painter.font();
        titleFont.setPixelSize(13);
        titleFont.setBold(true);
        painter.setFont(titleFont);
        painter.setPen(QColor(255, 215, 0));
        painter.drawText(QRect(8, 2, width() - 16, 20),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("▌羁绊系统"));
        painter.setPen(QPen(QColor(255, 215, 0, 120), 1));
        painter.drawLine(8, 22, width() - 8, 22);

        int lineHeight = 22;
        int startY = 28;
        int startX = 8;

        // 设置默认字体（比标题小一点）
        QFont font = painter.font();
        font.setPixelSize(11);
        font.setBold(false);
        painter.setFont(font);

        int barMaxWidth = width() - startX * 2 - 8;

        for (int i = 0; i < m_allTraits.size(); ++i) {
            const SynergyTrait &trait = m_allTraits[i];
            int y = startY + i * (lineHeight + 2);

            int count = m_traitCounts.value(trait.tag, 0);
            int activeThreshold = m_activeThresholds.value(trait.tag, 0);
            bool isActive = (count >= activeThreshold && activeThreshold > 0);

            int currentX = startX;

            QFont nameFont = painter.font();
            nameFont.setPixelSize(11);
            nameFont.setBold(true);
            painter.setFont(nameFont);

            QColor tagColor = getColorForTag(trait.tag, isActive);
            painter.setPen(tagColor);
            QString thresholdStr = isActive
                ? QString::number(activeThreshold)
                : (trait.buffs.isEmpty() ? QStringLiteral("2") : QString::number(trait.buffs.first().threshold));
            QString nameText = QStringLiteral("%1 %2 %3/%4")
                                   .arg(isActive ? QStringLiteral("●") : QStringLiteral("○"))
                                   .arg(trait.name).arg(count).arg(thresholdStr);
            painter.drawText(currentX, y, 150, lineHeight,
                             Qt::AlignLeft | Qt::AlignVCenter, nameText);
            currentX += 155;

            int totalThreshold = trait.buffs.isEmpty() ? 2 : trait.buffs.last().threshold;
            int barY = y + lineHeight / 2 - 3;
            int barH = 6;
            int barW = qMin(barMaxWidth, width() - currentX - 10);
            if (barW > 20) {
                // 背景
                painter.fillRect(currentX, barY, barW, barH, QColor(60, 60, 80));
                // 前景
                int fillW = qMin(count, totalThreshold) * barW / totalThreshold;
                if (fillW > 0) {
                    QColor barColor = tagColor;
                    barColor.setAlpha(180);
                    painter.fillRect(currentX, barY, fillW, barH, barColor);
                }
                // 边框
                painter.setPen(QPen(QColor(140, 140, 160), 1));
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(currentX, barY, barW, barH);
            }

            if (isActive) {
                QFont descFont = painter.font();
                descFont.setPixelSize(10);
                descFont.setBold(false);
                painter.setFont(descFont);
                painter.setPen(QColor(180, 220, 255));  // 浅蓝色描述
                for (const SynergyBuff &buff : trait.buffs) {
                    if (buff.threshold == activeThreshold) {
                        QString desc = buff.description;
                        if (buff.bonusHp > 0) desc += QStringLiteral(" +%1HP").arg(buff.bonusHp);
                        if (buff.bonusAtk > 0) desc += QStringLiteral(" +%1ATK").arg(buff.bonusAtk);
                        painter.drawText(currentX, y + 8, width() - currentX - 10, lineHeight - 8,
                                         Qt::AlignLeft | Qt::AlignVCenter, desc);
                        break;
                    }
                }
            }
        }

        // 调用父类 paintEvent
        QWidget::paintEvent(event);

    } catch (const std::exception &e) {
        qDebug() << "[SynergyPanelWidget] paintEvent 异常：" << e.what();
    } catch (...) {
        qDebug() << "[SynergyPanelWidget] paintEvent 未知异常";
    }
}
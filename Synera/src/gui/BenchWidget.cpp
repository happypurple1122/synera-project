#include "gui/BenchWidget.h"
#include "core/Unit.h"

#include <QDebug>
#include <QPainterPath>
#include <QPen>
#include <QtMath>

// 局部辅助函数

// 绘制五角星辅助函数（备战区用�?static void drawBenchStar(QPainter &painter, int centerX, int centerY,
                          int outerRadius, const QColor &color)
{
    const int innerRadius = outerRadius * 38 / 100;
    const int numPoints = 5;
    const double step = M_PI * 2.0 / numPoints;
    const double startAngle = -M_PI / 2.0;

    QPainterPath path;
    for (int i = 0; i < numPoints * 2; ++i) {
        double radius = (i % 2 == 0) ? outerRadius : innerRadius;
        double angle = startAngle + i * step / 2.0;
        int x = centerX + static_cast<int>(radius * qCos(angle));
        int y = centerY + static_cast<int>(radius * qSin(angle));
        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }
    path.closeSubpath();

    painter.setBrush(color);
    painter.setPen(QPen(QColor(0, 0, 0), 2));  // 黑色边框�?px�?    painter.drawPath(path);
}

// 构造函数

BenchWidget::BenchWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(BENCH_SIZE * SLOT_SIZE, SLOT_SIZE);
}


bool BenchWidget::addUnit(Unit *unit)
{
    if (unit == nullptr) {
        qDebug() << "[BenchWidget] addUnit 失败：unit 为空指针";
        return false;
    }
    int emptyIndex = findEmptySlot();
    if (emptyIndex == -1) {
        qDebug() << "[BenchWidget] addUnit 失败：备战区已满";
        return false;
    }
    m_slots[emptyIndex] = unit;
    update();
    qDebug() << "[BenchWidget] addUnit 成功：放入槽�? << emptyIndex;
    return true;
}


Unit *BenchWidget::removeUnit(int index)
{
    if (index < 0 || index >= BENCH_SIZE) {
        qDebug() << "[BenchWidget] removeUnit 失败：索引越�? << index;
        return nullptr;
    }
    Unit *unit = m_slots[index];
    if (unit != nullptr) {
        m_slots[index] = nullptr;
        update();
        qDebug() << "[BenchWidget] removeUnit 成功：移除了槽位" << index;
    } else {
        qDebug() << "[BenchWidget] removeUnit：槽�? << index << "原本就为�?;
    }
    return unit;
}


bool BenchWidget::swapSlot(int i, int j)
{
    if (i < 0 || i >= BENCH_SIZE || j < 0 || j >= BENCH_SIZE) {
        qDebug() << "[BenchWidget] swapSlot 失败：索引越�?(" << i << "," << j << ")";
        return false;
    }
    Unit *temp = m_slots[i];
    m_slots[i] = m_slots[j];
    m_slots[j] = temp;
    update();
    qDebug() << "[BenchWidget] swapSlot 成功：交换了槽位" << i << "�? << j;
    return true;
}


void BenchWidget::setUnitAt(int index, Unit *unit)
{
    if (index < 0 || index >= BENCH_SIZE) {
        qDebug() << "[BenchWidget] setUnitAt 失败：索引越�? << index;
        return;
    }
    m_slots[index] = unit;
    update();
}


Unit *BenchWidget::getUnitAt(int index) const
{
    if (index < 0 || index >= BENCH_SIZE) {
        return nullptr;
    }
    return m_slots[index];
}

// 查找空槽位

int BenchWidget::findEmptySlot() const
{
    for (int i = 0; i < BENCH_SIZE; ++i) {
        if (m_slots[i] == nullptr) {
            return i;
        }
    }
    return -1;
}


bool BenchWidget::isFull() const
{
    for (int i = 0; i < BENCH_SIZE; ++i) {
        if (m_slots[i] == nullptr) {
            return false;
        }
    }
    return true;
}

// QPainter 绘制备战区

void BenchWidget::paintEvent(QPaintEvent *event)
{
    try {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        painter.fillRect(rect(), QColor(245, 240, 200));  // 浅黄背景

        for (int i = 0; i < BENCH_SIZE; ++i) {
            int x = i * SLOT_SIZE;
            QRect slotRect(x, 0, SLOT_SIZE, SLOT_SIZE);

            Unit *unit = m_slots[i];

            if (unit == nullptr) {
                // 空槽位：虚线矩形 + 编号
                QPen dashPen(QColor(180, 180, 180), 1, Qt::DashLine);
                painter.setPen(dashPen);
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(slotRect.adjusted(2, 2, -2, -2));

                painter.setPen(QColor(160, 160, 160));
                QFont smallFont = painter.font();
                smallFont.setPixelSize(10);
                painter.setFont(smallFont);
                painter.drawText(slotRect.adjusted(0, 0, -4, -4),
                                 Qt::AlignBottom | Qt::AlignRight,
                                 QString::number(i));
            } else {
                // 有单位：根据 Owner 染色 + 血条蓝�?+ 名称
                int centerX = x + SLOT_SIZE / 2;
                int centerY = SLOT_SIZE / 2;
                int radius = SLOT_SIZE / 3;

                QColor unitColor;
                if (unit->getOwner() == Owner::PlayerCtrl) {
                    unitColor = QColor(120, 180, 255);  // 玩家：蓝�?                } else {
                    unitColor = QColor(255, 80, 80);    // 敌方：红�?                }

                painter.setBrush(unitColor);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPoint(centerX, centerY), radius, radius);

                // 血条（红色�?                int barWidth  = radius * 2;
                int barHeight = 4;
                int barX = centerX - radius;
                int hpBarY = centerY - radius - 6;
                painter.fillRect(barX, hpBarY, barWidth, barHeight, QColor(80, 0, 0));
                int hpWidth = static_cast<int>(barWidth * unit->getHpRatio());
                if (hpWidth > 0) {
                    painter.fillRect(barX, hpBarY, hpWidth, barHeight, QColor(255, 50, 50));
                }

                // 蓝条（蓝色）
                int manaBarY = centerY + radius + 2;
                painter.fillRect(barX, manaBarY, barWidth, barHeight, QColor(0, 0, 80));
                int manaWidth = static_cast<int>(barWidth * unit->getManaRatio());
                if (manaWidth > 0) {
                    painter.fillRect(barX, manaBarY, manaWidth, barHeight, QColor(50, 130, 255));
                }

                // 单位名称
                painter.setPen(QColor(255, 255, 255));
                QFont font = painter.font();
                font.setPixelSize(11);
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(QRect(centerX - radius, centerY - radius,
                                       radius * 2, radius * 2),
                                 Qt::AlignCenter, unit->getName());

                // 星级显示：金色五角星
                int starLevel = unit->getStarLevel();
                if (starLevel > 0) {
                    const QColor starColor(255, 215, 0);
                    int starRadius = 6;
                    int starY = centerY - radius - starRadius - 14;
                    int totalWidth = starLevel * (starRadius * 2 + 1) - 1;
                    int startX = centerX - totalWidth / 2 + starRadius;
                    for (int s = 0; s < starLevel; ++s) {
                        int starX = startX + s * (starRadius * 2 + 1);
                        drawBenchStar(painter, starX, starY, starRadius, starColor);
                    }
                }

                // 装备图标：金色发光方�?                int eqCount = unit->getEquipmentCount();
                int eqSize = 10;
                int eqGap = 3;
                for (int e = 0; e < eqCount; ++e) {
                    int eqX = centerX - radius + 3 + e * (eqSize + eqGap);
                    int eqY = centerY - radius + 3;
                    painter.setBrush(QColor(255, 200, 50, 60));
                    painter.setPen(Qt::NoPen);
                    painter.drawRoundedRect(eqX - 2, eqY - 2, eqSize + 4, eqSize + 4, 3, 3);
                    painter.fillRect(eqX, eqY, eqSize, eqSize, QColor(255, 160, 40));
                    painter.setPen(QPen(QColor(255, 220, 100), 2));
                    painter.setBrush(Qt::NoBrush);
                    painter.drawRoundedRect(eqX - 1, eqY - 1, eqSize + 2, eqSize + 2, 2, 2);
                    painter.setPen(QPen(QColor(255, 255, 200, 120), 1));
                    painter.drawLine(eqX + 2, eqY + 2, eqX + eqSize - 2, eqY + 2);
                    painter.drawLine(eqX + 2, eqY + 2, eqX + 2, eqY + eqSize - 2);
                }
            }
        }

        // 槽位之间的竖直分隔线
        {
            QPen linePen(QColor(200, 200, 200), 1, Qt::SolidLine);
            painter.setPen(linePen);
            for (int i = 1; i < BENCH_SIZE; ++i) {
                int x = i * SLOT_SIZE;
                painter.drawLine(x, 0, x, SLOT_SIZE);
            }
        }

        QWidget::paintEvent(event);

    } catch (const std::exception &e) {
        qDebug() << "[BenchWidget] paintEvent 异常�? << e.what();
    } catch (...) {
        qDebug() << "[BenchWidget] paintEvent 未知异常";
    }
}


void BenchWidget::mousePressEvent(QMouseEvent *event)
{
    try {
        if (event->button() != Qt::LeftButton) {
            QWidget::mousePressEvent(event);
            return;
        }

        int mouseX = event->pos().x();
        int index = mouseX / SLOT_SIZE;

        if (index >= 0 && index < BENCH_SIZE) {
            qDebug() << "[BenchWidget] 鼠标点击备战�?-> 槽位:" << index;
            if (m_slots[index] != nullptr) {
                qDebug() << "[BenchWidget] 该槽有单�?;
            } else {
                qDebug() << "[BenchWidget] 该槽为空";
            }
        } else {
            qDebug() << "[BenchWidget] 点击位置超出备战区范�?;
        }

        QWidget::mousePressEvent(event);

    } catch (const std::exception &e) {
        qDebug() << "[BenchWidget] mousePressEvent 异常�? << e.what();
    } catch (...) {
        qDebug() << "[BenchWidget] mousePressEvent 未知异常";
    }
}
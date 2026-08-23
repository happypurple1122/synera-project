#include "gui/BoardWidget.h"
#include "core/Unit.h"

#include <QDebug>
#include <QPainterPath>
#include <QPen>
#include <QtMath>

// 局部辅助函数

// 绘制五角星辅助函数
static void drawStar(QPainter &painter, int centerX, int centerY,
                     int outerRadius, const QColor &color)
{
    const int innerRadius = outerRadius * 38 / 100;  // 内径约为外径�?38%
    const int numPoints = 5;
    const double step = M_PI * 2.0 / numPoints;
    const double startAngle = -M_PI / 2.0;  // 从正上方开�?
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

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(GRID_COLS * CELL_SIZE, GRID_ROWS * CELL_SIZE);
}


bool BoardWidget::isPlayerHalf(int row) const
{
    return row >= 4;
}

// 格子空判断

bool BoardWidget::isCellEmpty(int row, int col) const
{
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) {
        return false;
    }
    return m_grid[row][col] == nullptr;
}


bool BoardWidget::placeUnit(Unit *unit, int row, int col)
{
    if (unit == nullptr) {
        qDebug() << "[BoardWidget] placeUnit 失败：unit 为空指针";
        return false;
    }
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) {
        qDebug() << "[BoardWidget] placeUnit 失败：行列越�?(" << row << "," << col << ")";
        return false;
    }
    if (m_grid[row][col] != nullptr) {
        qDebug() << "[BoardWidget] placeUnit 失败：格子已被占 (" << row << "," << col << ")";
        return false;
    }
    // PlayerCtrl �?只能放玩家半�?(row>=4)，EnemyCtrl �?只能放敌方半�?(row<4)
    if (unit->getOwner() == Owner::PlayerCtrl && !isPlayerHalf(row)) {
        qDebug() << "[BoardWidget] placeUnit 失败：玩家单位不能放到敌方半�?(" << row << "," << col << ")";
        return false;
    }
    if (unit->getOwner() == Owner::EnemyCtrl && isPlayerHalf(row)) {
        qDebug() << "[BoardWidget] placeUnit 失败：敌方单位不能放到玩家半�?(" << row << "," << col << ")";
        return false;
    }
    m_grid[row][col] = unit;
    unit->setPosX(col);
    unit->setPosY(row);
    update();
    return true;
}


Unit *BoardWidget::removeUnit(int row, int col)
{
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) {
        qDebug() << "[BoardWidget] removeUnit 失败：行列越�?(" << row << "," << col << ")";
        return nullptr;
    }
    Unit *unit = m_grid[row][col];
    if (unit != nullptr) {
        m_grid[row][col] = nullptr;
        update();
        qDebug() << "[BoardWidget] removeUnit 成功：移除了 (" << row << "," << col << ") 的单�?;
    } else {
        qDebug() << "[BoardWidget] removeUnit：该格子原本就为�?(" << row << "," << col << ")";
    }
    return unit;
}


bool BoardWidget::forcePlaceUnit(Unit *unit, int row, int col)
{
    if (unit == nullptr) return false;
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) return false;
    if (m_grid[row][col] != nullptr) return false;  // 格子有单位不覆盖

    m_grid[row][col] = unit;
    unit->setPosX(col);
    unit->setPosY(row);
    update();
    return true;
}


bool BoardWidget::swapUnit(int r1, int c1, int r2, int c2)
{
    if (r1 < 0 || r1 >= GRID_ROWS || c1 < 0 || c1 >= GRID_COLS ||
        r2 < 0 || r2 >= GRID_ROWS || c2 < 0 || c2 >= GRID_COLS) {
        qDebug() << "[BoardWidget] swapUnit 失败：坐标越�?;
        return false;
    }
    Unit *temp = m_grid[r1][c1];
    m_grid[r1][c1] = m_grid[r2][c2];
    m_grid[r2][c2] = temp;
    update();
    qDebug() << "[BoardWidget] swapUnit 成功：交换了 (" << r1 << "," << c1
             << ") �?(" << r2 << "," << c2 << ") 的单�?;
    return true;
}


void BoardWidget::clearBoard()
{
    for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
            m_grid[row][col] = nullptr;
        }
    }
    m_attackEffects.clear();
    update();
    qDebug() << "[BoardWidget] clearBoard 已清空整个棋盘和攻击特效";
}

void BoardWidget::clearAttackEffects()
{
    m_attackEffects.clear();
    update();
    qDebug() << "[BoardWidget] clearAttackEffects 已清空攻击特�?;
}


Unit *BoardWidget::getUnitAt(int row, int col) const
{
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) {
        return nullptr;
    }
    return m_grid[row][col];
}


void BoardWidget::addAttackEffect(int fromRow, int fromCol,
                                  int toRow, int toCol,
                                  AttackEffectType type)
{
    AttackEffect effect;
    effect.fromRow = fromRow;
    effect.fromCol = fromCol;
    effect.toRow   = toRow;
    effect.toCol   = toCol;
    effect.type    = type;
    effect.timer   = 10;  // 10 �?�?500ms
    m_attackEffects.append(effect);
}

void BoardWidget::tickEffects()
{
    for (int i = m_attackEffects.size() - 1; i >= 0; --i) {
        m_attackEffects[i].timer--;
        if (m_attackEffects[i].timer <= 0) {
            m_attackEffects.removeAt(i);
        }
    }
}


void BoardWidget::paintEvent(QPaintEvent *event)
{
    try {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        // 整体背景色（深灰色）
        painter.fillRect(rect(), QColor(50, 50, 50));

        // 逐格子绘制敌我半场底�?        for (int row = 0; row < GRID_ROWS; ++row) {
            for (int col = 0; col < GRID_COLS; ++col) {
                int x = col * CELL_SIZE;
                int y = row * CELL_SIZE;
                QRect cellRect(x, y, CELL_SIZE, CELL_SIZE);

                if (isPlayerHalf(row)) {
                    painter.fillRect(cellRect, QColor(200, 220, 255));  // 玩家半场：浅�?                } else {
                    painter.fillRect(cellRect, QColor(255, 210, 210));  // 敌方半场：浅�?                }
            }
        }

        // 半场分隔线（3px 粗虚线）
        {
            QPen dashPen(QColor(100, 100, 100), 3, Qt::DashLine);
            painter.setPen(dashPen);
            int dividerY = 4 * CELL_SIZE;
            painter.drawLine(0, dividerY, GRID_COLS * CELL_SIZE, dividerY);
        }

        // 普通网格线�?px 灰色实线�?        {
            QPen gridPen(QColor(180, 180, 180), 1, Qt::SolidLine);
            painter.setPen(gridPen);

            for (int row = 0; row <= GRID_ROWS; ++row) {
                int y = row * CELL_SIZE;
                painter.drawLine(0, y, GRID_COLS * CELL_SIZE, y);
            }
            for (int col = 0; col <= GRID_COLS; ++col) {
                int x = col * CELL_SIZE;
                painter.drawLine(x, 0, x, GRID_ROWS * CELL_SIZE);
            }
        }

        // 绘制有单位的格子（含血条、蓝条、星级、装备图标、装备视觉特效）
        for (int row = 0; row < GRID_ROWS; ++row) {
            for (int col = 0; col < GRID_COLS; ++col) {
                Unit *unit = m_grid[row][col];
                if (unit == nullptr) {
                    continue;
                }

                int centerX = col * CELL_SIZE + CELL_SIZE / 2;
                int centerY = row * CELL_SIZE + CELL_SIZE / 2;
                int radius = CELL_SIZE / 3;

                QColor unitColor;
                if (unit->getOwner() == Owner::PlayerCtrl) {
                    unitColor = QColor(120, 180, 255);  // 玩家：蓝�?                } else {
                    unitColor = QColor(255, 80, 80);    // 敌方：红�?                }

                bool isDead = !unit->isAlive();
                if (isDead) {
                    unitColor = QColor(120, 120, 120);  // 死亡：灰�?                }

                painter.setBrush(unitColor);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPoint(centerX, centerY), radius, radius);

                if (isDead) {
                    QPen xPen(QColor(80, 80, 80), 4, Qt::SolidLine);
                    painter.setPen(xPen);
                    int offset = radius / 3;
                    painter.drawLine(centerX - offset, centerY - offset,
                                     centerX + offset, centerY + offset);
                    painter.drawLine(centerX + offset, centerY - offset,
                                     centerX - offset, centerY + offset);
                    continue;
                }

                // 血条（红色�?                int barWidth  = radius * 2;
                int barHeight = 5;
                int barX = centerX - radius;
                int hpBarY = centerY - radius - 7;

                painter.fillRect(barX, hpBarY, barWidth, barHeight, QColor(80, 0, 0));
                int hpWidth = static_cast<int>(barWidth * unit->getHpRatio());
                if (hpWidth > 0) {
                    painter.fillRect(barX, hpBarY, hpWidth, barHeight, QColor(255, 50, 50));
                }

                painter.setPen(QColor(255, 255, 255));
                QFont hpFont = painter.font();
                hpFont.setPixelSize(8);
                hpFont.setBold(true);
                painter.setFont(hpFont);
                QString hpText = QStringLiteral("%1/%2").arg(unit->getHp()).arg(unit->getMaxHp());
                painter.drawText(QRect(barX, hpBarY - 1, barWidth, barHeight + 2),
                                 Qt::AlignCenter, hpText);

                // 蓝条（蓝色）
                int manaBarY = centerY + radius + 2;
                painter.fillRect(barX, manaBarY, barWidth, barHeight, QColor(0, 0, 80));
                int manaWidth = static_cast<int>(barWidth * unit->getManaRatio());
                if (manaWidth > 0) {
                    painter.fillRect(barX, manaBarY, manaWidth, barHeight, QColor(50, 130, 255));
                }

                painter.setPen(QColor(200, 200, 255));
                painter.setFont(hpFont);
                QString manaText = QStringLiteral("%1/%2").arg(unit->getMana()).arg(unit->getMaxMana());
                painter.drawText(QRect(barX, manaBarY - 1, barWidth, barHeight + 2),
                                 Qt::AlignCenter, manaText);

                QFont normalFont = painter.font();
                normalFont.setPixelSize(11);
                painter.setFont(normalFont);

                // 单位名称（白色粗体）
                painter.setPen(QColor(255, 255, 255));
                QFont font = painter.font();
                font.setPixelSize(11);
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(QRect(centerX - radius, centerY - radius,
                                       radius * 2, radius * 2),
                                 Qt::AlignCenter, unit->getName());

                // 星级显示：金色五角星（单位头顶上方）
                int starLevel = unit->getStarLevel();
                if (starLevel > 0) {
                    const QColor starColor(255, 215, 0);
                    int starRadius = 6;
                    int starY = centerY - radius - starRadius - 16;
                    int totalWidth = starLevel * (starRadius * 2 + 1) - 1;
                    int startX = centerX - totalWidth / 2 + starRadius;
                    for (int s = 0; s < starLevel; ++s) {
                        int starX = startX + s * (starRadius * 2 + 1);
                        drawStar(painter, starX, starY, starRadius, starColor);
                    }
                }

                // 装备图标：金色发光方块（左上角）
                int eqCount = unit->getEquipmentCount();
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

                // 英雄装备视觉特效（单位圆形之外）
                {
                    EquipmentType eqType = unit->getEquippedType();
                    if (eqType != EquipmentType::None) {
                        const int offset = 5;
                        QPen savedPen = painter.pen();
                        QBrush savedBrush = painter.brush();

                        switch (eqType) {
                        case EquipmentType::IronSword: {
                            int sx = centerX + radius + offset;
                            int sy = centerY;
                            QPolygon sword;
                            sword << QPoint(sx, sy - 5)
                                  << QPoint(sx + 8, sy)
                                  << QPoint(sx, sy + 5);
                            painter.setBrush(QColor(255, 140, 0));
                            painter.setPen(QPen(QColor(200, 100, 0), 1));
                            painter.drawPolygon(sword);
                            break;
                        }
                        case EquipmentType::ChainMail: {
                            QPen bluePen(QColor(50, 130, 255), 2, Qt::SolidLine);
                            painter.setPen(bluePen);
                            painter.setBrush(Qt::NoBrush);
                            painter.drawEllipse(QPoint(centerX, centerY), radius + 2, radius + 2);
                            break;
                        }
                        case EquipmentType::SwiftGlove: {
                            painter.setBrush(QColor(80, 200, 80));
                            painter.setPen(Qt::NoPen);
                            painter.drawEllipse(QPoint(centerX - radius - offset, centerY), 4, 4);
                            break;
                        }
                        case EquipmentType::BlueCrystal: {
                            int dx = centerX;
                            int dy = centerY - radius - offset - 5;
                            QPolygon diamond;
                            diamond << QPoint(dx, dy - 5)
                                    << QPoint(dx + 5, dy)
                                    << QPoint(dx, dy + 5)
                                    << QPoint(dx - 5, dy);
                            painter.setBrush(QColor(170, 70, 190));
                            painter.setPen(QPen(QColor(130, 40, 150), 1));
                            painter.drawPolygon(diamond);
                            break;
                        }
                        case EquipmentType::RevivalArmor: {
                            painter.setBrush(QColor(255, 215, 0, 60));
                            painter.setPen(QPen(QColor(255, 215, 0, 120), 2));
                            painter.drawEllipse(QPoint(centerX, centerY), radius + 6, radius + 6);
                            break;
                        }
                        case EquipmentType::VampireBlade: {
                            const int mx = centerX;
                            const int my = centerY + radius + offset + 2;
                            painter.setBrush(QColor(200, 0, 0));
                            painter.setPen(Qt::NoPen);
                            painter.drawChord(mx - 6, my - 4, 12, 8, 0, 180 * 16);
                            break;
                        }
                        case EquipmentType::TimeStaff: {
                            const int tx = centerX;
                            const int ty = centerY - radius - offset - 3;
                            QPen greenPen(QColor(50, 200, 50), 3, Qt::SolidLine, Qt::RoundCap);
                            painter.setPen(greenPen);
                            painter.drawLine(tx - 5, ty, tx + 5, ty);
                            painter.drawLine(tx, ty - 5, tx, ty + 5);
                            break;
                        }
                        case EquipmentType::FireCannon: {
                            const int fx = centerX + radius + offset;
                            const int fy = centerY;
                            QPolygon flame;
                            flame << QPoint(fx, fy - 6)
                                  << QPoint(fx + 10, fy)
                                  << QPoint(fx, fy + 6);
                            painter.setBrush(QColor(255, 100, 0));
                            painter.setPen(QPen(QColor(200, 60, 0), 1));
                            painter.drawPolygon(flame);
                            painter.setBrush(QColor(255, 200, 50, 150));
                            painter.setPen(Qt::NoPen);
                            painter.drawEllipse(QPoint(fx + 4, fy), 3, 3);
                            break;
                        }
                        default:
                            break;
                        }

                        painter.setPen(savedPen);
                        painter.setBrush(savedBrush);
                    }
                }
            }
        }

        // 绘制攻击特效
        for (const AttackEffect &effect : m_attackEffects) {
            if (effect.timer <= 0) continue;

            int x1 = effect.fromCol * CELL_SIZE + CELL_SIZE / 2;
            int y1 = effect.fromRow * CELL_SIZE + CELL_SIZE / 2;
            int x2 = effect.toCol   * CELL_SIZE + CELL_SIZE / 2;
            int y2 = effect.toRow   * CELL_SIZE + CELL_SIZE / 2;

            int alpha = effect.timer * 25;
            if (alpha > 255) alpha = 255;

            if (effect.type == AttackEffectType::Arrow) {
                QPen arrowPen(QColor(255, 220, 50, alpha), 2, Qt::SolidLine);
                painter.setPen(arrowPen);
                painter.drawLine(x1, y1, x2, y2);
                QPointF arrowTip(x2, y2);
                double angle = std::atan2(y2 - y1, x2 - x1);
                int arrowSize = 6;
                QPointF p1(x2 - arrowSize * std::cos(angle - 0.5),
                           y2 - arrowSize * std::sin(angle - 0.5));
                QPointF p2(x2 - arrowSize * std::cos(angle + 0.5),
                           y2 - arrowSize * std::sin(angle + 0.5));
                QPolygonF arrowHead;
                arrowHead << arrowTip << p1 << p2;
                painter.setBrush(QColor(255, 220, 50, alpha));
                painter.drawPolygon(arrowHead);

            } else if (effect.type == AttackEffectType::Fireball) {
                QPen firePen(QColor(255, 120, 20, alpha), 4, Qt::SolidLine);
                painter.setPen(firePen);
                painter.drawLine(x1, y1, x2, y2);
                painter.setBrush(QColor(255, 80, 0, alpha));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPointF(x2, y2), 8, 8);
                painter.setBrush(QColor(255, 255, 100, alpha));
                painter.drawEllipse(QPointF(x2, y2), 4, 4);

            } else if (effect.type == AttackEffectType::Slash) {
                QPen glowPen(QColor(255, 200, 50, alpha / 2), 8, Qt::SolidLine, Qt::RoundCap);
                painter.setPen(glowPen);
                painter.drawLine(x1, y1, x2, y2);

                QPen corePen(QColor(255, 240, 150, alpha), 3, Qt::SolidLine, Qt::RoundCap);
                painter.setPen(corePen);
                painter.drawLine(x1, y1, x2, y2);

                int s = 12;
                QPen slashPen(QColor(255, 255, 200, alpha), 4, Qt::SolidLine, Qt::RoundCap);
                painter.setPen(slashPen);
                painter.drawLine(x2 - s, y2 - s, x2 + s, y2 + s);
                painter.drawLine(x2 + s, y2 - s, x2 - s, y2 + s);

            } else if (effect.type == AttackEffectType::Backstab) {
                QPen shadowPen(QColor(80, 0, 120, alpha / 2), 6, Qt::SolidLine, Qt::RoundCap);
                painter.setPen(shadowPen);
                painter.drawLine(x1, y1, x2, y2);

                QPen bladePen(QColor(180, 50, 255, alpha), 2, Qt::SolidLine, Qt::RoundCap);
                painter.setPen(bladePen);
                painter.drawLine(x1, y1, x2, y2);

                int s = 10;
                QPen jagPen(QColor(200, 100, 255, alpha), 3, Qt::SolidLine, Qt::RoundCap);
                painter.setPen(jagPen);
                painter.drawLine(x2 - s, y2 - s, x2, y2);
                painter.drawLine(x2, y2, x2 + s, y2 - s);
                painter.drawLine(x2 + s, y2 - s, x2, y2 + s / 2);
                painter.setBrush(QColor(200, 50, 255, alpha));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPointF(x2, y2), 5, 5);

                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(255, 255, 255, alpha));
                painter.drawEllipse(QPointF(x2, y2), 3, 3);
            }
        }

        QWidget::paintEvent(event);

    } catch (const std::exception &e) {
        qDebug() << "[BoardWidget] paintEvent 异常捕获�? << e.what();
    } catch (...) {
        qDebug() << "[BoardWidget] paintEvent 未知异常捕获";
    }
}


void BoardWidget::mousePressEvent(QMouseEvent *event)
{
    try {
        if (event->button() != Qt::LeftButton) {
            QWidget::mousePressEvent(event);
            return;
        }

        int mouseX = event->pos().x();
        int mouseY = event->pos().y();

        int col = mouseX / CELL_SIZE;
        int row = mouseY / CELL_SIZE;

        if (row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS) {
            qDebug() << "[BoardWidget] 鼠标点击棋盘 -> �?" << row << " �?" << col;
            if (m_grid[row][col] != nullptr) {
                qDebug() << "[BoardWidget] 该格有单�?;
            } else {
                qDebug() << "[BoardWidget] 该格为空";
            }
        } else {
            qDebug() << "[BoardWidget] 点击位置超出棋盘范围";
        }

        QWidget::mousePressEvent(event);

    } catch (const std::exception &e) {
        qDebug() << "[BoardWidget] mousePressEvent 异常捕获�? << e.what();
    } catch (...) {
        qDebug() << "[BoardWidget] mousePressEvent 未知异常捕获";
    }
}
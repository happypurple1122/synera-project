#include "gui/DragDropMgr.h"
#include "gui/BoardWidget.h"
#include "gui/BenchWidget.h"
#include "gui/EquipmentBarWidget.h"
#include "core/Unit.h"
#include "core/Player.h"
#include "core/Equipment.h"

DragDropMgr::DragDropMgr(BoardWidget *boardWidget,
                         BenchWidget *benchWidget,
                         Player *player,
                         EquipmentBarWidget *equipmentBar,
                         QObject *parent)
    : QObject(parent)
    , m_boardWidget(boardWidget)
    , m_benchWidget(benchWidget)
    , m_player(player)
    , m_equipmentBar(equipmentBar)
    , m_dragSource(DragSource::None)  // 初始无拖拽
    , m_draggedUnit(nullptr)
    , m_srcBenchIndex(-1)
    , m_srcBoardRow(-1)
    , m_srcBoardCol(-1)
{
    if (m_boardWidget) {
        m_boardWidget->installEventFilter(this);
    }
    if (m_benchWidget) {
        m_benchWidget->installEventFilter(this);
    }
}

bool DragDropMgr::eventFilter(QObject *obj, QEvent *event)
{
    try {
        // 如果拖拽被禁用（战斗中），不处理任何拖拽事件
        if (!m_enabled) {
            return false;
        }

        if (event->type() != QEvent::MouseButtonPress &&
            event->type() != QEvent::MouseButtonRelease) {
            return false;
        }

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (mouseEvent->button() == Qt::RightButton && event->type() == QEvent::MouseButtonPress) {
            if (obj == m_boardWidget) {
                int col = mouseEvent->pos().x() / BoardWidget::CELL_SIZE;
                int row = mouseEvent->pos().y() / BoardWidget::CELL_SIZE;
                if (row >= 0 && row < BoardWidget::GRID_ROWS &&
                    col >= 0 && col < BoardWidget::GRID_COLS) {
                    Unit *unit = m_boardWidget->getUnitAt(row, col);
                    if (unit != nullptr && unit->getOwner() == Owner::PlayerCtrl
                        && unit->getEquipmentCount() > 0) {
                        EquipmentType eqType = unit->getEquippedType();
                        if (eqType != EquipmentType::None) {
                            Equipment::removeEffect(unit, eqType);
                            if (m_equipmentBar) {
                                m_equipmentBar->addEquipment(eqType);
                            }
                            qDebug().noquote() << "[DragDropMgr] 右键取消装备："
                                               << unit->getName() << "移除"
                                               << Equipment::getInfo(eqType).name;
                            emit dragCompleted();
                            return true;
                        }
                    }
                }
            }
            if (obj == m_benchWidget) {
                int index = mouseEvent->pos().x() / BenchWidget::SLOT_SIZE;
                if (index >= 0 && index < BenchWidget::BENCH_SIZE) {
                    Unit *unit = m_benchWidget->getUnitAt(index);
                    if (unit != nullptr && unit->getEquipmentCount() > 0) {
                        EquipmentType eqType = unit->getEquippedType();
                        if (eqType != EquipmentType::None) {
                            Equipment::removeEffect(unit, eqType);
                            if (m_equipmentBar) {
                                m_equipmentBar->addEquipment(eqType);
                            }
                            qDebug().noquote() << "[DragDropMgr] 右键取消装备："
                                               << unit->getName() << "移除"
                                               << Equipment::getInfo(eqType).name;
                            emit dragCompleted();
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        if (mouseEvent->button() != Qt::LeftButton) {
            return false;
        }

        if (event->type() == QEvent::MouseButtonPress) {
            if (obj == m_boardWidget) {
                int col = mouseEvent->pos().x() / BoardWidget::CELL_SIZE;
                int row = mouseEvent->pos().y() / BoardWidget::CELL_SIZE;
                if (row >= 0 && row < BoardWidget::GRID_ROWS &&
                    col >= 0 && col < BoardWidget::GRID_COLS) {
                    handleBoardPress(row, col);
                    return true;
                }
            }
            if (obj == m_benchWidget) {
                int index = mouseEvent->pos().x() / BenchWidget::SLOT_SIZE;
                if (index >= 0 && index < BenchWidget::BENCH_SIZE) {
                    handleBenchPress(index);
                    return true;
                }
            }
            return false;
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            QPoint globalPos = mouseEvent->globalPosition().toPoint();

            QPoint boardLocal = m_boardWidget->mapFromGlobal(globalPos);
            if (m_boardWidget->rect().contains(boardLocal)) {
                int col = boardLocal.x() / BoardWidget::CELL_SIZE;
                int row = boardLocal.y() / BoardWidget::CELL_SIZE;
                if (row >= 0 && row < BoardWidget::GRID_ROWS &&
                    col >= 0 && col < BoardWidget::GRID_COLS) {
                    handleBoardRelease(row, col);
                    return true;
                }
            }

            QPoint benchLocal = m_benchWidget->mapFromGlobal(globalPos);
            if (m_benchWidget->rect().contains(benchLocal)) {
                int index = benchLocal.x() / BenchWidget::SLOT_SIZE;
                if (index >= 0 && index < BenchWidget::BENCH_SIZE) {
                    handleBenchRelease(index);
                    return true;
                }
            }

            // 释放到空白区域 → 回弹
            if (m_draggedUnit != nullptr) {
                cancelDrag();
                return true;
            }
            return false;
        }

        return false;

    } catch (const std::exception &e) {
        qDebug() << "[DragDropMgr] eventFilter 异常：" << e.what();
        cancelDrag();
        return true;
    } catch (...) {
        qDebug() << "[DragDropMgr] eventFilter 未知异常";
        cancelDrag();
        return true;
    }
}

void DragDropMgr::handleBenchPress(int index)
{
    try {
        Unit *unit = m_benchWidget->getUnitAt(index);
        if (unit == nullptr) return;

        if (m_equipmentBar != nullptr && m_equipmentBar->getSelectedSlot() >= 0) {
            int eqSlot = m_equipmentBar->getSelectedSlot();
            EquipmentType eqType = m_equipmentBar->getEquipmentAt(eqSlot);
            if (eqType != EquipmentType::None && unit->canEquip()) {
                if (unit->getEquipmentCount() == 1) {
                    EquipmentType existingType = unit->getEquippedType();
                    if (existingType != EquipmentType::None) {
                        EquipmentType craftedType = Equipment::checkRecipe(existingType, eqType);
                        if (craftedType != EquipmentType::None) {
                            Equipment::removeEffect(unit, existingType);
                            m_equipmentBar->removeEquipment(eqSlot);
                            m_equipmentBar->clearSelection();
                            Equipment::applyEffect(unit, craftedType);
                            QString equipName = Equipment::getInfo(craftedType).name;
                            qDebug().noquote() << "[合成]" << unit->getName() << "合成" << equipName;
                            emit equipmentCrafted(unit->getName(), equipName);
                            emit dragCompleted();
                            return;
                        }
                    }
                }
                Equipment::applyEffect(unit, eqType);
                m_equipmentBar->removeEquipment(eqSlot);
                m_equipmentBar->clearSelection();
                qDebug().noquote() << "[DragDropMgr] 装备" << Equipment::getInfo(eqType).name
                                   << "→ 备战区" << unit->getName();
                emit dragCompleted();
                return;
            } else if (eqType != EquipmentType::None && !unit->canEquip()) {
                qDebug() << "[DragDropMgr] 单位装备已满，无法装备";
                return;
            }
        }

        m_dragSource = DragSource::FromBench;
        m_srcBenchIndex = index;
        m_draggedUnit = unit;
        qDebug() << "[DragDropMgr] 从备战区槽位" << index << "开始拖拽";
    } catch (const std::exception &e) {
        qDebug() << "[DragDropMgr] handleBenchPress 异常：" << e.what();
        cancelDrag();
    }
}

void DragDropMgr::handleBoardPress(int row, int col)
{
    try {
        Unit *unit = m_boardWidget->getUnitAt(row, col);
        if (unit == nullptr) return;

        if (unit->getOwner() == Owner::EnemyCtrl) return;

        if (m_equipmentBar != nullptr && m_equipmentBar->getSelectedSlot() >= 0) {
            int eqSlot = m_equipmentBar->getSelectedSlot();
            EquipmentType eqType = m_equipmentBar->getEquipmentAt(eqSlot);
            if (eqType != EquipmentType::None && unit->canEquip()) {
                if (unit->getEquipmentCount() == 1) {
                    EquipmentType existingType = unit->getEquippedType();
                    if (existingType != EquipmentType::None) {
                        EquipmentType craftedType = Equipment::checkRecipe(existingType, eqType);
                        if (craftedType != EquipmentType::None) {
                            Equipment::removeEffect(unit, existingType);
                            m_equipmentBar->removeEquipment(eqSlot);
                            m_equipmentBar->clearSelection();
                            Equipment::applyEffect(unit, craftedType);
                            QString equipName = Equipment::getInfo(craftedType).name;
                            qDebug().noquote() << "[合成]" << unit->getName() << "合成" << equipName;
                            emit equipmentCrafted(unit->getName(), equipName);
                            emit dragCompleted();
                            return;
                        }
                    }
                }
                Equipment::applyEffect(unit, eqType);
                m_equipmentBar->removeEquipment(eqSlot);
                m_equipmentBar->clearSelection();
                qDebug().noquote() << "[DragDropMgr] 装备" << Equipment::getInfo(eqType).name
                                   << "→ 棋盘" << unit->getName();
                emit dragCompleted();
                return;
            } else if (eqType != EquipmentType::None && !unit->canEquip()) {
                qDebug() << "[DragDropMgr] 单位装备已满，无法装备";
                return;
            }
        }

        m_dragSource = DragSource::FromBoard;
        m_srcBoardRow = row;
        m_srcBoardCol = col;
        m_draggedUnit = unit;
        qDebug() << "[DragDropMgr] 从棋盘(" << row << "," << col << ")开始拖拽";
    } catch (const std::exception &e) {
        qDebug() << "[DragDropMgr] handleBoardPress 异常：" << e.what();
        cancelDrag();
    }
}

void DragDropMgr::handleBoardRelease(int row, int col)
{
    try {
        if (m_draggedUnit == nullptr) return;

        if (!m_boardWidget->isPlayerHalf(row)) {
            qDebug() << "[DragDropMgr] 敌方半场，取消拖拽";
            cancelDrag();
            return;
        }

        if (!m_boardWidget->isCellEmpty(row, col)) {
            qDebug() << "[DragDropMgr] 目标格子已被占用，弹回原位";
            cancelDrag();
            return;
        }

        if (m_dragSource == DragSource::FromBench && m_player != nullptr) {
            int playerUnitCount = 0;
            for (int r = 4; r < BoardWidget::GRID_ROWS; ++r) {
                for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
                    Unit *u = m_boardWidget->getUnitAt(r, c);
                    if (u != nullptr && u->getOwner() == Owner::PlayerCtrl) {
                        playerUnitCount++;
                    }
                }
            }

            int maxPop = m_player->getMaxPopulation();
            if (playerUnitCount >= maxPop) {
                qDebug().noquote() << QString("[DragDropMgr] 人口已达上限 %1，无法上阵")
                                          .arg(maxPop);
                cancelDrag();
                return;
            }
        }

        if (m_dragSource == DragSource::FromBench) {
            m_benchWidget->removeUnit(m_srcBenchIndex);
        } else {
            m_boardWidget->removeUnit(m_srcBoardRow, m_srcBoardCol);
        }

        m_boardWidget->placeUnit(m_draggedUnit, row, col);
        qDebug() << "[DragDropMgr] 放置单位到棋盘(" << row << "," << col << ")";

        m_draggedUnit = nullptr;
        m_dragSource = DragSource::None;
        m_srcBenchIndex = -1;
        m_srcBoardRow = -1;
        m_srcBoardCol = -1;

        emit dragCompleted();

    } catch (const std::exception &e) {
        qDebug() << "[DragDropMgr] handleBoardRelease 异常：" << e.what();
        cancelDrag();
    }
}

void DragDropMgr::handleBenchRelease(int index)
{
    try {
        if (m_draggedUnit == nullptr) return;

        if (m_benchWidget->getUnitAt(index) != nullptr) {
            qDebug() << "[DragDropMgr] 备战区目标槽位已被占用，弹回原位";
            cancelDrag();
            return;
        }

        if (m_dragSource == DragSource::FromBoard) {
            m_boardWidget->removeUnit(m_srcBoardRow, m_srcBoardCol);
        }
        if (m_dragSource == DragSource::FromBench) {
            m_benchWidget->removeUnit(m_srcBenchIndex);
        }

        m_benchWidget->setUnitAt(index, m_draggedUnit);
        qDebug() << "[DragDropMgr] 放置单位到备战区槽位" << index;

        m_draggedUnit = nullptr;
        m_dragSource = DragSource::None;
        m_srcBenchIndex = -1;
        m_srcBoardRow = -1;
        m_srcBoardCol = -1;

        emit dragCompleted();

    } catch (const std::exception &e) {
        qDebug() << "[DragDropMgr] handleBenchRelease 异常：" << e.what();
        cancelDrag();
    }
}

void DragDropMgr::cancelDrag()
{
    m_draggedUnit = nullptr;
    m_dragSource = DragSource::None;
    m_srcBenchIndex = -1;
    m_srcBoardRow = -1;
    m_srcBoardCol = -1;
}
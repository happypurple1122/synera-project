#include "gui/EquipmentBarWidget.h"
#include <QDebug>

// 构造函数：计算固定尺寸，初始化所有槽位为 None
EquipmentBarWidget::EquipmentBarWidget(QWidget *parent)
    : QWidget(parent)
    , m_selectedSlot(-1)
{
    int totalWidth = BAR_SIZE * (SLOT_WIDTH + SPACING) - SPACING + 4;
    setFixedSize(totalWidth, SLOT_HEIGHT + 4);

    // 所有 m_slots 初始化为 EquipmentType::None（默认初始化已做）
}

// 将装备放入第一个空槽位
bool EquipmentBarWidget::addEquipment(EquipmentType type)
{
    if (type == EquipmentType::None) return false;

    for (int i = 0; i < BAR_SIZE; ++i) {
        if (m_slots[i] == EquipmentType::None) {
            m_slots[i] = type;
            update();  // 触发重绘
            qDebug().noquote() << "[EquipmentBar] 添加装备" << getShortName(type)
                               << "到槽位" << i;
            return true;
        }
    }

    qDebug() << "[EquipmentBar] 装备栏已满，无法添加" << getShortName(type);
    return false;
}

// 移除指定槽位的装备
EquipmentType EquipmentBarWidget::removeEquipment(int index)
{
    if (index < 0 || index >= BAR_SIZE) return EquipmentType::None;
    if (m_slots[index] == EquipmentType::None) return EquipmentType::None;

    EquipmentType removed = m_slots[index];
    m_slots[index] = EquipmentType::None;

    // 如果移除的是当前选中的槽，清除选中
    if (m_selectedSlot == index) {
        m_selectedSlot = -1;
    }

    update();  // 触发重绘
    qDebug().noquote() << "[EquipmentBar] 从槽位" << index
                       << "移除装备" << getShortName(removed);
    return removed;
}

// 选中状态

int EquipmentBarWidget::getSelectedSlot() const
{
    return m_selectedSlot;
}

EquipmentType EquipmentBarWidget::getEquipmentAt(int index) const
{
    if (index < 0 || index >= BAR_SIZE) return EquipmentType::None;
    return m_slots[index];
}

void EquipmentBarWidget::clearSelection()
{
    m_selectedSlot = -1;
    update();
}

// 绘制 8 个水平排列的装备槽
void EquipmentBarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 背景
    painter.fillRect(rect(), QColor(0x1E, 0x1E, 0x1E));

    for (int i = 0; i < BAR_SIZE; ++i) {
        int x = 2 + i * (SLOT_WIDTH + SPACING);
        int y = 2;
        QRect slotRect(x, y, SLOT_WIDTH, SLOT_HEIGHT);

        if (m_slots[i] == EquipmentType::None) {
            // 空槽位：灰色虚线边框
            painter.setPen(QPen(QColor(0x66, 0x66, 0x66), 1, Qt::DashLine));
            painter.setBrush(Qt::NoBrush);
            painter.drawRoundedRect(slotRect, 4, 4);
        } else {
            // 有装备
            QColor fillColor = getColorForType(m_slots[i]);
            bool isAdv = Equipment::isAdvanced(m_slots[i]);

            if (isAdv) {
                // 高级装备：金色边框 + 深色填充
                painter.setPen(QPen(QColor(0xFF, 0xD7, 0x00), 2));
                painter.setBrush(QColor(fillColor.red(), fillColor.green(),
                                        fillColor.blue(), 180));
            } else if (m_selectedSlot == i) {
                // 选中状态：浅黄背景 + 金色边框
                painter.setPen(QPen(QColor(0xFF, 0xD7, 0x00), 2));
                painter.setBrush(QColor(fillColor.red(), fillColor.green(),
                                        fillColor.blue(), 200));
            } else {
                // 正常状态
                painter.setPen(Qt::NoPen);
                painter.setBrush(fillColor);
            }
            painter.drawRoundedRect(slotRect, 4, 4);

            // 绘制短名称
            painter.setPen(isAdv ? QColor(0xFF, 0xD7, 0x00) : Qt::white);
            QFont font = painter.font();
            font.setPixelSize(10);
            font.setBold(true);
            painter.setFont(font);
            painter.drawText(slotRect, Qt::AlignCenter, getShortName(m_slots[i]));

            // 高级装备：右下角绘制星星标记
            if (isAdv) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(0xFF, 0xD7, 0x00));
                painter.drawEllipse(slotRect.right() - 10, slotRect.bottom() - 10, 6, 6);
                QFont starFont = painter.font();
                starFont.setPixelSize(7);
                starFont.setBold(true);
                painter.setFont(starFont);
                painter.setPen(QColor(0x8B, 0x45, 0x13));
                painter.drawText(QRect(slotRect.right() - 10, slotRect.bottom() - 10, 6, 6),
                                 Qt::AlignCenter, QStringLiteral("★"));
            }
        }
    }

    // 拖拽模式下：高亮显示可合成的目标槽位（金色虚线边框）
    if (m_isDragging && m_dragIndex >= 0) {
        EquipmentType dragType = m_slots[m_dragIndex];
        for (int i = 0; i < BAR_SIZE; ++i) {
            if (i == m_dragIndex) continue;
            if (m_slots[i] == EquipmentType::None) continue;

            EquipmentType result = Equipment::checkRecipe(dragType, m_slots[i]);
            if (result != EquipmentType::None) {
                int x = 2 + i * (SLOT_WIDTH + SPACING);
                int y = 2;
                QRect slotRect(x, y, SLOT_WIDTH, SLOT_HEIGHT);

                painter.setPen(QPen(QColor(0xFF, 0xD7, 0x00), 2, Qt::DashLine));
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundedRect(slotRect.adjusted(-1, -1, 1, 1), 5, 5);
            }
        }
    }
}

// 鼠标按下：切换选中 / 记录拖拽起始
void EquipmentBarWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    int x = event->pos().x();
    int index = (x - 2) / (SLOT_WIDTH + SPACING);

    if (index < 0 || index >= BAR_SIZE) return;

    // 记录拖拽起始信息
    m_dragStartPos = event->pos();
    m_isDragging = false;
    m_dragIndex = (m_slots[index] != EquipmentType::None) ? index : -1;

    if (m_slots[index] == EquipmentType::None) return;

    // 原有的选中切换逻辑
    if (m_selectedSlot == index) {
        m_selectedSlot = -1;
    } else {
        m_selectedSlot = index;
    }

    update();
}

// 鼠标移动：检测拖拽启动阈值
void EquipmentBarWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragIndex < 0) return;
    if (!(event->buttons() & Qt::LeftButton)) return;

    // 检测是否移动了足够距离开始拖拽
    QPoint delta = event->pos() - m_dragStartPos;
    if (delta.manhattanLength() >= QApplication::startDragDistance()) {
        if (!m_isDragging) {
            m_isDragging = true;
            qDebug().noquote() << "[EquipmentBar] 开始拖拽槽位" << m_dragIndex
                               << getShortName(m_slots[m_dragIndex]);
            update();  // 刷新绘制，显示可合成目标高亮
        }
    }
}

// 鼠标释放：拖拽合成检测 / 选中逻辑
void EquipmentBarWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;

    if (m_isDragging && m_dragIndex >= 0) {
        // 拖拽模式：尝试合成
        int x = event->pos().x();
        int dropIndex = (x - 2) / (SLOT_WIDTH + SPACING);

        if (dropIndex >= 0 && dropIndex < BAR_SIZE && dropIndex != m_dragIndex) {
            EquipmentType srcType = m_slots[m_dragIndex];
            EquipmentType dstType = m_slots[dropIndex];

            if (srcType != EquipmentType::None && dstType != EquipmentType::None) {
                EquipmentType result = Equipment::checkRecipe(srcType, dstType);
                if (result != EquipmentType::None) {
                    // 合成成功
                    qDebug().noquote() << "[EquipmentBar] 合成成功:"
                                       << getShortName(srcType) << "+"
                                       << getShortName(dstType) << "→"
                                       << getShortName(result);

                    // 移除两件基础装备
                    removeEquipment(m_dragIndex);
                    removeEquipment(dropIndex);

                    // 放入第一个空槽位
                    for (int i = 0; i < BAR_SIZE; ++i) {
                        if (m_slots[i] == EquipmentType::None) {
                            m_slots[i] = result;
                            qDebug().noquote() << "[EquipmentBar] 合成装备放入槽位" << i;
                            break;
                        }
                    }

                    update();
                    emit synthesized(result);
                } else {
                    qDebug().noquote() << "[EquipmentBar] 合成失败，不匹配:"
                                       << getShortName(srcType) << "+"
                                       << getShortName(dstType);
                    // 回弹原位 — 无需额外操作
                }
            }
        }

        // 重置拖拽状态
        m_dragIndex = -1;
        m_isDragging = false;
        update();
        return;
    }

    // 非拖拽模式：选中逻辑已在 mousePressEvent 中完成，无需额外处理
    m_dragIndex = -1;
    m_isDragging = false;
}

// 获取装备短名称（用于槽位显示）
QString EquipmentBarWidget::getShortName(EquipmentType type) const
{
    switch (type) {
    case EquipmentType::IronSword:    return QStringLiteral("剑");
    case EquipmentType::ChainMail:    return QStringLiteral("甲");
    case EquipmentType::SwiftGlove:   return QStringLiteral("手");
    case EquipmentType::BlueCrystal:  return QStringLiteral("晶");
    case EquipmentType::RevivalArmor: return QStringLiteral("复活");
    case EquipmentType::VampireBlade: return QStringLiteral("吸血");
    case EquipmentType::TimeStaff:    return QStringLiteral("时光");
    case EquipmentType::FireCannon:   return QStringLiteral("火炮");
    default:                          return QStringLiteral("");
    }
}

// 获取装备类型对应的填充颜色
QColor EquipmentBarWidget::getColorForType(EquipmentType type) const
{
    switch (type) {
    case EquipmentType::IronSword:    return QColor(0xFF, 0x70, 0x43);  // 橙色
    case EquipmentType::ChainMail:    return QColor(0x42, 0xA5, 0xF5);  // 蓝色
    case EquipmentType::SwiftGlove:   return QColor(0x66, 0xBB, 0x6A);  // 绿色
    case EquipmentType::BlueCrystal:  return QColor(0xAB, 0x47, 0xBC);  // 紫色
    case EquipmentType::RevivalArmor: return QColor(0xB8, 0x86, 0x0B);  // 暗金
    case EquipmentType::VampireBlade: return QColor(0xFF, 0x45, 0x00);  // 橙红
    case EquipmentType::TimeStaff:    return QColor(0x00, 0xCE, 0xD1);  // 深青
    case EquipmentType::FireCannon:   return QColor(0xFF, 0x63, 0x47);  // 番茄红
    default:                          return QColor(0x88, 0x88, 0x88);  // 灰色
    }
}
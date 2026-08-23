#ifndef EQUIPMENTBARWIDGET_H
#define EQUIPMENTBARWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include "core/Equipment.h"

// 装备栏 UI 组件，管理 8 个装备槽位，支持单击选中和拖拽合成
class EquipmentBarWidget : public QWidget
{
    Q_OBJECT

public:
    static constexpr int BAR_SIZE   = 8;   // 装备槽总数
    static constexpr int SLOT_WIDTH = 44;  // 每个槽位宽
    static constexpr int SLOT_HEIGHT= 44;  // 每个槽位高
    static constexpr int SPACING    = 4;   // 槽位间距

    explicit EquipmentBarWidget(QWidget *parent = nullptr);
    ~EquipmentBarWidget() override = default;

    // 将装备放入第一个空槽位
    bool addEquipment(EquipmentType type);

    // 移除指定槽位的装备，返回类型
    EquipmentType removeEquipment(int index);

    // 获取当前选中的槽位索引（-1 = 无选中）
    int getSelectedSlot() const;

    // 获取指定槽位的装备类型
    EquipmentType getEquipmentAt(int index) const;

    // 清除选中状态
    void clearSelection();

signals:
    // 装备穿戴成功信号
    void equipmentUsed(int slotIndex, EquipmentType type);

    // 装备合成成功信号
    void synthesized(EquipmentType type);

protected:
    // 绘制 8 个水平排列的装备槽
    void paintEvent(QPaintEvent *event) override;

    // 鼠标按下：切换选中 / 记录拖拽起始
    void mousePressEvent(QMouseEvent *event) override;

    // 鼠标移动：检测拖拽启动阈值
    void mouseMoveEvent(QMouseEvent *event) override;

    // 鼠标释放：拖拽合成检测 / 选中逻辑
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // 获取装备短名称（用于槽位显示）
    QString getShortName(EquipmentType type) const;

    // 获取装备类型对应的填充颜色
    QColor getColorForType(EquipmentType type) const;

    EquipmentType m_slots[BAR_SIZE] = {};  // 8 个装备槽
    int m_selectedSlot = -1;               // 当前选中槽位

    int m_dragIndex = -1;                  // 拖拽来源槽位
    bool m_isDragging = false;             // 是否正在拖拽
    QPoint m_dragStartPos;                 // 拖拽起始位置
};

#endif // EQUIPMENTBARWIDGET_H
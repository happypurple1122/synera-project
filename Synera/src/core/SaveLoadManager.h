#ifndef SAVELOADMANAGER_H
#define SAVELOADMANAGER_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "core/Equipment.h"

class Player;
class BenchWidget;
class BoardWidget;
class Shop;
class EquipmentBarWidget;

// 存档/读档管理器，使用 QJsonDocument 进行 JSON 序列化/反序列化
// 存档文件：saves/save_XX.json（3 个槽位）
class SaveLoadManager
{
public:
    // 保存游戏到指定槽位（0~2），使用临时文件 + rename 策略防止写入中断损坏
    static bool saveGame(int slotIndex,
                         Player *player,
                         BenchWidget *bench,
                         BoardWidget *board,
                         Shop *shop,
                         EquipmentBarWidget *equipmentBar);

    static bool loadGame(int slotIndex,
                         Player *player,
                         BenchWidget *bench,
                         BoardWidget *board,
                         Shop *shop,
                         EquipmentBarWidget *equipmentBar);

    static bool isValidSave(int slotIndex);

    static QString getSavesDirectory();

private:
    static constexpr int SAVE_VERSION = 1;
    static constexpr int MAX_SLOTS = 3;

    static QJsonObject serializePlayer(const Player *player);
    static QJsonArray serializeBench(const BenchWidget *bench);
    static QJsonArray serializeBoard(const BoardWidget *board);
    static QJsonObject serializeShop(const Shop *shop);
    static QJsonArray serializeEquipmentBar(const EquipmentBarWidget *equipmentBar);
    static QJsonObject serializeUnit(const class Unit *unit);

    static void deserializePlayer(const QJsonObject &json, Player *player);
    static void deserializeBench(const QJsonArray &json, BenchWidget *bench, Shop *shop);
    static void deserializeBoard(const QJsonArray &json, BoardWidget *board, Shop *shop);
    static void deserializeShop(const QJsonObject &json, Shop *shop);
    static void deserializeEquipmentBar(const QJsonArray &json, EquipmentBarWidget *equipmentBar);
    static Unit *deserializeUnit(const QJsonObject &json, Shop *shop);
    static EquipmentType equipmentTypeFromString(const QString &str);
};

#endif // SAVELOADMANAGER_H
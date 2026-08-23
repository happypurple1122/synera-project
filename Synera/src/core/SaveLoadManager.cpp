#include "core/SaveLoadManager.h"
#include "core/Player.h"
#include "core/Shop.h"
#include "core/Equipment.h"
#include "core/Unit.h"
#include "gui/BenchWidget.h"
#include "gui/BoardWidget.h"
#include "gui/EquipmentBarWidget.h"

#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <optional>

// 保存游戏：序列化各模块 → 写入临时文件 → rename 为正式文件
bool SaveLoadManager::saveGame(int slotIndex,
                                Player *player,
                                BenchWidget *bench,
                                BoardWidget *board,
                                Shop *shop,
                                EquipmentBarWidget *equipmentBar)
{
    try {
        if (slotIndex < 0 || slotIndex >= MAX_SLOTS) {
            qDebug() << "[SaveLoad] saveGame 失败：槽位索引无效" << slotIndex;
            return false;
        }

        // 构建 JSON 文档
        QJsonObject root;
        root["version"]   = SAVE_VERSION;
        root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

        // 序列化各模块数据
        root["player"]       = serializePlayer(player);
        root["bench"]        = serializeBench(bench);
        root["board"]        = serializeBoard(board);
        root["shop"]         = serializeShop(shop);
        root["equipmentBar"] = serializeEquipmentBar(equipmentBar);

        // 写入 JSON 到文件
        QJsonDocument doc(root);
        QString dirPath = getSavesDirectory();
        QString tempPath = dirPath + QStringLiteral("save_%1.tmp").arg(slotIndex);
        QString finalPath = dirPath + QStringLiteral("save_%1.json").arg(slotIndex);

        // 先写临时文件
        QFile tempFile(tempPath);
        if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qDebug() << "[SaveLoad] saveGame 无法创建临时文件：" << tempPath;
            return false;
        }
        tempFile.write(doc.toJson(QJsonDocument::Indented));
        tempFile.close();

        // 删除旧存档
        if (QFile::exists(finalPath)) {
            QFile::remove(finalPath);
        }

        // rename 临时文件为正式文件
        if (!QFile::rename(tempPath, finalPath)) {
            qDebug() << "[SaveLoad] saveGame rename 失败";
            QFile::remove(tempPath);
            return false;
        }

        qDebug().noquote() << QString("[SaveLoad] 存档成功：槽位%1").arg(slotIndex);
        return true;

    } catch (const std::exception &e) {
        qDebug() << "[SaveLoad] saveGame 异常：" << e.what();
        return false;
    } catch (...) {
        qDebug() << "[SaveLoad] saveGame 未知异常";
        return false;
    }
}

// 读取存档：读取 JSON → 验证版本 → 反序列化各模块 → 恢复状态
bool SaveLoadManager::loadGame(int slotIndex,
                                Player *player,
                                BenchWidget *bench,
                                BoardWidget *board,
                                Shop *shop,
                                EquipmentBarWidget *equipmentBar)
{
    try {
        if (slotIndex < 0 || slotIndex >= MAX_SLOTS) {
            qDebug() << "[SaveLoad] loadGame 失败：槽位索引无效" << slotIndex;
            return false;
        }

        QString filePath = getSavesDirectory() + QStringLiteral("save_%1.json").arg(slotIndex);

        if (!QFile::exists(filePath)) {
            qDebug() << "[SaveLoad] loadGame 失败：存档文件不存在" << filePath;
            return false;
        }

        // 读取文件
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "[SaveLoad] loadGame 无法打开文件：" << filePath;
            return false;
        }

        QByteArray data = file.readAll();
        file.close();

        // 解析 JSON
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "[SaveLoad] loadGame JSON 解析错误：" << parseError.errorString();
            return false;
        }

        QJsonObject root = doc.object();

        // 验证版本号
        int version = root["version"].toInt(0);
        if (version != SAVE_VERSION) {
            qDebug() << "[SaveLoad] loadGame 版本不匹配：期望" << SAVE_VERSION << "，实际" << version;
            return false;
        }

        // 反序列化各模块
        deserializePlayer(root["player"].toObject(), player);
        deserializeBench(root["bench"].toArray(), bench, shop);
        deserializeBoard(root["board"].toArray(), board, shop);
        deserializeShop(root["shop"].toObject(), shop);
        deserializeEquipmentBar(root["equipmentBar"].toArray(), equipmentBar);

        qDebug().noquote() << QString("[SaveLoad] 读档成功：槽位%1").arg(slotIndex);
        return true;

    } catch (const std::exception &e) {
        qDebug() << "[SaveLoad] loadGame 异常：" << e.what();
        return false;
    } catch (...) {
        qDebug() << "[SaveLoad] loadGame 未知异常";
        return false;
    }
}

bool SaveLoadManager::isValidSave(int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= MAX_SLOTS) return false;
    QString filePath = getSavesDirectory() + QStringLiteral("save_%1.json").arg(slotIndex);
    return QFile::exists(filePath);
}

QString SaveLoadManager::getSavesDirectory()
{
    // 使用可执行文件所在目录
    QString appDir = QCoreApplication::applicationDirPath();
    QString savesDir = appDir + QStringLiteral("/saves/");

    QDir dir(savesDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    return savesDir;
}

QJsonObject SaveLoadManager::serializePlayer(const Player *player)
{
    QJsonObject json;
    json["gold"]            = player->getGold();
    json["hp"]              = player->getHp();
    json["level"]           = player->getLevel();
    json["currentStage"]    = player->getCurrentStage();
    json["exp"]             = player->getCurrentExp();
    json["expToLevel"]      = player->getExpToLevel();
    json["bonusPopulation"] = player->getBonusPopulation();
    return json;
}

QJsonArray SaveLoadManager::serializeBench(const BenchWidget *bench)
{
    QJsonArray arr;
    for (int i = 0; i < BenchWidget::BENCH_SIZE; ++i) {
        Unit *unit = bench->getUnitAt(i);
        if (unit != nullptr) {
            arr.append(serializeUnit(unit));
        } else {
            arr.append(QJsonValue::Null);
        }
    }
    return arr;
}

QJsonArray SaveLoadManager::serializeBoard(const BoardWidget *board)
{
    QJsonArray arr;
    for (int r = 0; r < BoardWidget::GRID_ROWS; ++r) {
        QJsonArray rowArr;
        for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
            Unit *unit = board->getUnitAt(r, c);
            if (unit != nullptr && unit->getOwner() == Owner::PlayerCtrl) {
                rowArr.append(serializeUnit(unit));
            } else {
                rowArr.append(QJsonValue::Null);
            }
        }
        arr.append(rowArr);
    }
    return arr;
}

QJsonObject SaveLoadManager::serializeShop(const Shop *shop)
{
    QJsonObject json;
    QJsonArray slotsArr;

    for (int i = 0; i < Shop::SLOT_COUNT; ++i) {
        auto slotOpt = shop->getSlot(i);
        if (slotOpt.has_value()) {
            QJsonObject slotJson;
            slotJson["name"]  = slotOpt->heroName;
            slotJson["price"] = slotOpt->price;
            slotsArr.append(slotJson);
        } else {
            slotsArr.append(QJsonValue::Null);
        }
    }

    json["slots"] = slotsArr;
    return json;
}

QJsonArray SaveLoadManager::serializeEquipmentBar(const EquipmentBarWidget *equipmentBar)
{
    QJsonArray arr;
    for (int i = 0; i < EquipmentBarWidget::BAR_SIZE; ++i) {
        EquipmentType type = equipmentBar->getEquipmentAt(i);
        if (type == EquipmentType::None) {
            arr.append(QJsonValue::Null);
        } else {
            QString typeStr;
            switch (type) {
            case EquipmentType::IronSword:   typeStr = "IronSword"; break;
            case EquipmentType::ChainMail:   typeStr = "ChainMail"; break;
            case EquipmentType::SwiftGlove:  typeStr = "SwiftGlove"; break;
            case EquipmentType::BlueCrystal: typeStr = "BlueCrystal"; break;
            default:                         typeStr = "None"; break;
            }
            arr.append(typeStr);
        }
    }
    return arr;
}

QJsonObject SaveLoadManager::serializeUnit(const Unit *unit)
{
    QJsonObject json;
    json["id"]          = unit->getId();
    json["name"]        = unit->getName();
    json["starLevel"]   = unit->getStarLevel();
    json["hp"]          = unit->getHp();
    json["maxHp"]       = unit->getMaxHp();
    json["atk"]         = unit->getAtk();
    json["range"]       = unit->getRange();
    json["attackSpeed"] = unit->getAttackSpeed();
    json["mana"]        = unit->getMana();
    json["maxMana"]     = unit->getMaxMana();
    json["posX"]        = unit->getPosX();
    json["posY"]        = unit->getPosY();
    json["equipmentCount"] = unit->getEquipmentCount();
    QJsonArray tagArr;
    for (const QString &tag : unit->getTraitTags()) {
        tagArr.append(tag);
    }
    json["traitTags"] = tagArr;
    return json;
}

void SaveLoadManager::deserializePlayer(const QJsonObject &json, Player *player)
{
    if (player == nullptr) return;

    player->setGold(json["gold"].toInt(10));
    player->setHp(json["hp"].toInt(100));
    player->setLevel(json["level"].toInt(1));
    player->setCurrentStage(json["currentStage"].toInt(1));
    player->setCurrentExp(json["exp"].toInt(0));
    player->setExpToLevel(json["expToLevel"].toInt(2));
    player->setBonusPopulation(json["bonusPopulation"].toInt(0));
}

void SaveLoadManager::deserializeBench(const QJsonArray &json, BenchWidget *bench, Shop *shop)
{
    if (bench == nullptr || shop == nullptr) return;

    // 清空备战区
    for (int i = BenchWidget::BENCH_SIZE - 1; i >= 0; --i) {
        Unit *oldUnit = bench->removeUnit(i);
        delete oldUnit;
    }

    // 恢复单位
    for (int i = 0; i < json.size() && i < BenchWidget::BENCH_SIZE; ++i) {
        if (!json[i].isNull()) {
            QJsonObject unitJson = json[i].toObject();
            Unit *unit = deserializeUnit(unitJson, shop);
            if (unit != nullptr) {
                bench->addUnit(unit);
            }
        }
    }
}

void SaveLoadManager::deserializeBoard(const QJsonArray &json, BoardWidget *board, Shop *shop)
{
    if (board == nullptr || shop == nullptr) return;

    // 清空棋盘上的玩家单位
    for (int r = 0; r < BoardWidget::GRID_ROWS; ++r) {
        for (int c = 0; c < BoardWidget::GRID_COLS; ++c) {
            Unit *u = board->getUnitAt(r, c);
            if (u != nullptr && u->getOwner() == Owner::PlayerCtrl) {
                board->removeUnit(r, c);
                delete u;
            }
        }
    }

    // 恢复单位
    for (int r = 0; r < json.size() && r < BoardWidget::GRID_ROWS; ++r) {
        QJsonArray rowArr = json[r].toArray();
        for (int c = 0; c < rowArr.size() && c < BoardWidget::GRID_COLS; ++c) {
            if (!rowArr[c].isNull()) {
                QJsonObject unitJson = rowArr[c].toObject();
                Unit *unit = deserializeUnit(unitJson, shop);
                if (unit != nullptr) {
                    board->placeUnit(unit, r, c);
                }
            }
        }
    }
}

void SaveLoadManager::deserializeShop(const QJsonObject &json, Shop *shop)
{
    if (shop == nullptr) return;

    // 清空商店所有槽位
    for (int i = 0; i < Shop::SLOT_COUNT; ++i) {
        shop->restoreSlot(i, QString(), 0);
    }

    // 恢复商店槽位
    QJsonArray slotsArr = json["slots"].toArray();
    for (int i = 0; i < slotsArr.size() && i < Shop::SLOT_COUNT; ++i) {
        if (!slotsArr[i].isNull()) {
            QJsonObject slotJson = slotsArr[i].toObject();
            QString name = slotJson["name"].toString();
            int price = slotJson["price"].toInt(0);
            if (!name.isEmpty()) {
                shop->restoreSlot(i, name, price);
            }
        }
    }
}

void SaveLoadManager::deserializeEquipmentBar(const QJsonArray &json, EquipmentBarWidget *equipmentBar)
{
    if (equipmentBar == nullptr) return;

    // 清空装备栏
    for (int i = EquipmentBarWidget::BAR_SIZE - 1; i >= 0; --i) {
        equipmentBar->removeEquipment(i);
    }

    // 恢复装备
    for (int i = 0; i < json.size() && i < EquipmentBarWidget::BAR_SIZE; ++i) {
        if (!json[i].isNull()) {
            EquipmentType type = equipmentTypeFromString(json[i].toString());
            if (type != EquipmentType::None) {
                equipmentBar->addEquipment(type);
            }
        }
    }
}

Unit *SaveLoadManager::deserializeUnit(const QJsonObject &json, Shop *shop)
{
    if (shop == nullptr) return nullptr;

    QString name = json["name"].toString();
    if (name.isEmpty()) return nullptr;

    int id = json["id"].toInt(0);
    Unit *unit = Shop::createUnit(id, name, Owner::PlayerCtrl);
    if (unit == nullptr) return nullptr;

    // 恢复属性
    unit->setStarLevel(json["starLevel"].toInt(1));
    unit->setMaxHp(json["maxHp"].toInt(unit->getMaxHp()));
    unit->setHp(json["hp"].toInt(unit->getMaxHp()));
    unit->setAtk(json["atk"].toInt(unit->getAtk()));
    unit->setRange(json["range"].toInt(unit->getRange()));
    unit->setAttackSpeed(json["attackSpeed"].toInt(unit->getAttackSpeed()));
    unit->setMana(json["mana"].toInt(0));
    unit->setMaxMana(json["maxMana"].toInt(unit->getMaxMana()));
    unit->setPosX(json["posX"].toInt(-1));
    unit->setPosY(json["posY"].toInt(-1));

    unit->setEquipmentCount(json["equipmentCount"].toInt(0));
    if (json.contains("traitTags")) {
        QJsonArray tagArr = json["traitTags"].toArray();
        for (const QJsonValue &v : tagArr) {
            unit->addTraitTag(v.toString());
        }
    }

    return unit;
}

EquipmentType SaveLoadManager::equipmentTypeFromString(const QString &str)
{
    if (str == "IronSword")    return EquipmentType::IronSword;
    if (str == "ChainMail")    return EquipmentType::ChainMail;
    if (str == "SwiftGlove")   return EquipmentType::SwiftGlove;
    if (str == "BlueCrystal")  return EquipmentType::BlueCrystal;
    return EquipmentType::None;
}
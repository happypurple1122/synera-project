#include "core/EnemyWaveGenerator.h"
#include "core/Unit.h"
#include "core/Hero_Warrior.h"
#include "core/Hero_Mage.h"
#include "core/Hero_Archer.h"
#include "core/Hero_Assassin.h"
#include "gui/BoardWidget.h"

QList<Unit *> EnemyWaveGenerator::generateWave(int round)
{
    QList<Unit *> wave;

    try {
        // 参数保护：轮次至少为 1
        if (round < 1) {
            round = 1;
        }

        // 计算本轮敌方单位数量
        int count = calculateEnemyCount(round);
        qDebug() << "[EnemyWaveGenerator] 生成第" << round << "轮敌方单位，数量:" << count;

        // 逐个生成敌方单位
        for (int i = 0; i < count; ++i) {
            // 随机选择英雄类型
            int heroType = pickHeroType(round);

            // 生成唯一 ID：轮次*100 + 序号（如 301 表示第3轮第1个）
            int id = round * 100 + i;

            Unit *enemy = nullptr;
            switch (heroType) {
                case 1: // 战士
                    enemy = new Hero_Warrior(id, Owner::EnemyCtrl);
                    break;
                case 2: // 弓箭手
                    enemy = new Hero_Archer(id, Owner::EnemyCtrl);
                    break;
                case 3: // 法师
                    enemy = new Hero_Mage(id, Owner::EnemyCtrl);
                    break;
                case 4: // 刺客
                    enemy = new Hero_Assassin(id, Owner::EnemyCtrl);
                    break;
                default: // 默认生成战士
                    enemy = new Hero_Warrior(id, Owner::EnemyCtrl);
                    break;
            }

            if (enemy) {
                // 判断是否为精英/Boss
                // 第5轮起：第一个单位是精英（1.5x 属性，命名"精英 XXX"）
                // 第8轮起：第一个单位是 Boss（2x 属性，覆盖精英判断）
                bool isBoss = (round >= 8 && i == 0);
                bool isElite = (round >= 5 && i == 0 && !isBoss);

                // 应用轮次难度倍率
                applyStatScaling(enemy, round, isElite, isBoss);

                // 精英/Boss 特殊命名
                if (isBoss) {
                    enemy->setName(QStringLiteral("Boss ") + enemy->getName());
                } else if (isElite) {
                    enemy->setName(QStringLiteral("精英 ") + enemy->getName());
                }

                wave.append(enemy);
                qDebug() << "[EnemyWaveGenerator] 生成:" << enemy->getName()
                         << "(HP=" << enemy->getMaxHp()
                         << ", ATK=" << enemy->getAtk()
                         << ", Range=" << enemy->getRange()
                         << ", Mana=" << enemy->getMaxMana()
                         << ")";
                if (isBoss) {
                    qDebug() << "[EnemyWaveGenerator] Boss 单位，属性 x2";
                } else if (isElite) {
                    qDebug() << "[EnemyWaveGenerator] 精英单位，属性 x1.5";
                }
            }
        }

        qDebug() << "[EnemyWaveGenerator] 第" << round << "轮敌方单位生成完毕，共" << wave.size() << "个单位";

    } catch (const std::exception &e) {
        qDebug() << "[EnemyWaveGenerator] generateWave 异常：" << e.what();
    } catch (...) {
        qDebug() << "[EnemyWaveGenerator] generateWave 未知异常";
    }

    return wave;
}

// 部署敌方单位到敌方半场（row 0~3），从 row=3 开始从左到右填充
void EnemyWaveGenerator::deployWave(BoardWidget *board, const QList<Unit *> &units)
{
    try {
        if (board == nullptr) {
            qDebug() << "[EnemyWaveGenerator] deployWave 失败：board 为空指针";
            return;
        }

        if (units.isEmpty()) {
            qDebug() << "[EnemyWaveGenerator] deployWave：无单位需部署";
            return;
        }

        qDebug() << "[EnemyWaveGenerator] 开始部署" << units.size() << "个敌方单位到敌方半场";

        // 部署策略：从 row=3（靠近中线）开始，从右到左填充
        // 这样坦克型单位在前排，输出型在后排
        int unitIndex = 0;
        for (int row = 3; row >= 0 && unitIndex < units.size(); --row) {
            for (int col = 0; col < BoardWidget::GRID_COLS && unitIndex < units.size(); ++col) {
                if (board->isCellEmpty(row, col)) {
                    Unit *enemy = units[unitIndex];
                    // 设置单位棋盘坐标
                    enemy->setPosX(col);
                    enemy->setPosY(row);
                    // 部署到棋盘
                    board->placeUnit(enemy, row, col);
                    qDebug() << "[EnemyWaveGenerator] 部署" << enemy->getName()
                             << "到敌方半场(" << row << "," << col << ")";
                    unitIndex++;
                }
            }
        }

        // 更新棋盘显示
        board->update();
        qDebug() << "[EnemyWaveGenerator] 部署完成，实际部署" << unitIndex << "个单位";

    } catch (const std::exception &e) {
        qDebug() << "[EnemyWaveGenerator] deployWave 异常：" << e.what();
    } catch (...) {
        qDebug() << "[EnemyWaveGenerator] deployWave 未知异常";
    }
}

void EnemyWaveGenerator::clearWave(QList<Unit *> &units)
{
    try {
        qDebug() << "[EnemyWaveGenerator] 清空" << units.size() << "个敌方单位";

        for (Unit *unit : units) {
            delete unit;  // 销毁 Unit 对象
        }
        units.clear();  // 清空指针列表

        qDebug() << "[EnemyWaveGenerator] 敌方单位已全部销毁";

    } catch (const std::exception &e) {
        qDebug() << "[EnemyWaveGenerator] clearWave 异常：" << e.what();
    } catch (...) {
        qDebug() << "[EnemyWaveGenerator] clearWave 未知异常";
    }
}

// 轮次难度倍率：roundMult = 1.0 + ((round-1)/3)*0.2，精英×1.5，Boss×2.0
void EnemyWaveGenerator::applyStatScaling(Unit *unit, int round, bool isElite, bool isBoss)
{
    try {
        if (unit == nullptr) return;

        // 轮次倍率：每 3 轮 +20%
        double roundMult = 1.0 + ((round - 1) / 3) * 0.2;

        // 类型倍率
        double typeMult = 1.0;
        if (isBoss) {
            typeMult = 2.0;      // Boss：2倍
        } else if (isElite) {
            typeMult = 1.5;      // 精英：1.5倍
        }

        // 最终倍率 = 轮次倍率 × 类型倍率
        double finalMult = roundMult * typeMult;

        // 小于 1.0 不应用（不削弱）
        if (finalMult <= 1.0) return;

        // 应用到最大生命值和当前生命值
        int newMaxHp = static_cast<int>(unit->getMaxHp() * finalMult);
        unit->setMaxHp(newMaxHp);
        unit->setHp(newMaxHp);  // 满血

        // 应用到攻击力
        int newAtk = static_cast<int>(unit->getAtk() * finalMult);
        unit->setAtk(newAtk);

        qDebug() << "[EnemyWaveGenerator] 难度倍率: 轮次" << round << "→ roundMult=" << roundMult
                 << ", typeMult=" << typeMult << ", finalMult=" << finalMult;

    } catch (const std::exception &e) {
        qDebug() << "[EnemyWaveGenerator] applyStatScaling 异常：" << e.what();
    } catch (...) {
        qDebug() << "[EnemyWaveGenerator] applyStatScaling 未知异常";
    }
}

// 根据轮次计算敌人数量：round 1→2, 2→3, 3→4, 4~5→5, 6+→6
int EnemyWaveGenerator::calculateEnemyCount(int round)
{
    // Round 1 = 2, Round 2 = 3, Round 3 = 4, Round 4-5 = 5, Round 6+ = 6
    int count = 2 + (round / 2);  // round 1→2, 2→3, 3→3, 4→4, 5→4, 6→5
    if (round >= 6) {
        count = 6;  // 上限 6 个（适应敌方半场 32 格）
    } else if (round >= 4) {
        count = 5;
    } else if (round >= 3) {
        count = 4;
    }
    qDebug() << "[EnemyWaveGenerator] 轮次" << round << "→ 敌人数量:" << count;
    return count;
}

// 根据轮次按概率随机选择英雄类型：1=战士, 2=弓箭手, 3=法师, 4=刺客
int EnemyWaveGenerator::pickHeroType(int round)
{
    int roll = QRandomGenerator::global()->bounded(100);  // 0~99

    if (round == 1) {
        // 第1轮：全是战士（教学关）
        return 1;
    } else if (round == 2) {
        // 第2轮：60% 战士, 25% 弓箭手, 15% 刺客
        if (roll < 60) return 1;  // 战士
        else if (roll < 85) return 2;  // 弓箭手
        else return 4;            // 刺客
    } else if (round == 3) {
        // 第3轮：40% 战士, 25% 弓箭手, 20% 法师, 15% 刺客
        if (roll < 40) return 1;       // 战士
        else if (roll < 65) return 2;  // 弓箭手
        else if (roll < 85) return 3;  // 法师
        else return 4;                 // 刺客
    } else {
        // 第4轮+：35% 战士, 25% 弓箭手, 20% 法师, 20% 刺客
        if (roll < 35) return 1;       // 战士
        else if (roll < 60) return 2;  // 弓箭手
        else if (roll < 80) return 3;  // 法师
        else return 4;                 // 刺客
    }
}
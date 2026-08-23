#include "core/StarUpSystem.h"
#include "core/Unit.h"
#include "gui/BenchWidget.h"
#include "gui/BoardWidget.h"

#include <QtMath>

// 持续扫描备战区和棋盘，3 合 1 升星，循环直到无更多可合并组
bool StarUpSystem::checkAndMerge(BenchWidget *bench, BoardWidget *board)
{
    try {
        if (bench == nullptr || board == nullptr) {
            qDebug() << "[StarUpSystem] checkAndMerge 失败：参数为空";
            return false;
        }

        bool anyMerge = false;

        // 循环直到没有可合并的组
        bool mergedThisPass = true;
        while (mergedThisPass) {
            mergedThisPass = false;

            // 收集所有玩家单位
            QList<Unit*> allPlayerUnits;

            // 从备战区收集
            for (int i = 0; i < BenchWidget::BENCH_SIZE; ++i) {
                Unit *u = bench->getUnitAt(i);
                if (u != nullptr && u->getOwner() == Owner::PlayerCtrl) {
                    allPlayerUnits.append(u);
                }
            }

            // 从棋盘玩家半场收集（row 4~7）
            for (int row = 4; row < BoardWidget::GRID_ROWS; ++row) {
                for (int col = 0; col < BoardWidget::GRID_COLS; ++col) {
                    Unit *u = board->getUnitAt(row, col);
                    if (u != nullptr && u->getOwner() == Owner::PlayerCtrl) {
                        allPlayerUnits.append(u);
                    }
                }
            }

            if (allPlayerUnits.isEmpty()) break;

            // 按 (name, starLevel) 分组
            QMap<QPair<QString, int>, QList<Unit*>> groups;
            for (Unit *u : allPlayerUnits) {
                QPair<QString, int> key(u->getName(), u->getStarLevel());
                groups[key].append(u);
            }

            // 遍历每组
            for (auto it = groups.begin(); it != groups.end(); ++it) {
                const QString &name = it.key().first;
                int starLevel = it.key().second;

                // 跳过 >= 2 星（目前仅支持 1星→2星）
                if (starLevel >= 2) continue;

                QList<Unit*> &sameUnits = it.value();

                // 当有 3 个及以上同单位时，取前 3 个合并
                while (sameUnits.size() >= 3) {
                    QList<Unit*> toMerge;
                    toMerge.append(sameUnits.takeFirst());
                    toMerge.append(sameUnits.takeFirst());
                    toMerge.append(sameUnits.takeFirst());

                    mergeUnits(toMerge, bench, board);
                    mergedThisPass = true;
                    anyMerge = true;

                    qDebug().noquote() << QString("[升星] %1 合并成功 → 2星").arg(name);
                }
            }
        }

        return anyMerge;

    } catch (const std::exception &e) {
        qDebug() << "[StarUpSystem] checkAndMerge 异常：" << e.what();
        return false;
    } catch (...) {
        qDebug() << "[StarUpSystem] checkAndMerge 未知异常";
        return false;
    }
}

// 合并 3 个同单位：保留第 1 个、升星、移除并销毁其余 2 个
void StarUpSystem::mergeUnits(QList<Unit*> &sameUnits,
                              BenchWidget *bench, BoardWidget *board)
{
    try {
        if (sameUnits.size() < 3) {
            qDebug() << "[StarUpSystem] mergeUnits 失败：少于 3 个单位";
            return;
        }

        // 保留第一个单位
        Unit *kept = sameUnits[0];
        if (kept == nullptr) {
            qDebug() << "[StarUpSystem] mergeUnits 失败：保留单位为 nullptr";
            return;
        }

        // 移除并销毁第 2 和第 3 个单位
        for (int i = 1; i <= 2; ++i) {
            Unit *toRemove = sameUnits[i];
            if (toRemove == nullptr) continue;

            bool removed = false;

            // 优先从备战区查找移除
            for (int j = 0; j < BenchWidget::BENCH_SIZE; ++j) {
                if (bench->getUnitAt(j) == toRemove) {
                    bench->removeUnit(j);
                    removed = true;
                    break;
                }
            }

            // 备战区没找到，去棋盘找
            if (!removed) {
                for (int row = 4; row < BoardWidget::GRID_ROWS; ++row) {
                    for (int col = 0; col < BoardWidget::GRID_COLS; ++col) {
                        if (board->getUnitAt(row, col) == toRemove) {
                            board->removeUnit(row, col);
                            removed = true;
                            break;
                        }
                    }
                    if (removed) break;
                }
            }

            if (!removed) {
                qWarning() << "[StarUpSystem] mergeUnits 警告：单位"
                           << toRemove->getName() << "既不在备战区也不在棋盘";
            }

            // 销毁被合并的单位
            delete toRemove;
        }

        // 提升保留单位的星级
        kept->setStarLevel(kept->getStarLevel() + 1);

        // 应用升星属性加成
        applyStarUpBonus(kept);

        qDebug().noquote() << QString("[升星] %1 升星成功 → %2星 (HP:%3 ATK:%4 攻速:%5)")
                                  .arg(kept->getName())
                                  .arg(kept->getStarLevel())
                                  .arg(kept->getMaxHp())
                                  .arg(kept->getAtk())
                                  .arg(kept->getAttackSpeed());

    } catch (const std::exception &e) {
        qDebug() << "[StarUpSystem] mergeUnits 异常：" << e.what();
    } catch (...) {
        qDebug() << "[StarUpSystem] mergeUnits 未知异常";
    }
}

// 升星属性加成：HP/ATK ×1.6, 攻速×0.85, 重置装备计数
void StarUpSystem::applyStarUpBonus(Unit *unit)
{
    try {
        if (unit == nullptr) return;

        int newStarLevel = unit->getStarLevel();

        if (newStarLevel == 2) {
            // 1星 → 2星 公式
            int newMaxHp = qFloor(unit->getMaxHp() * 1.6);
            int newAtk = qFloor(unit->getAtk() * 1.6);
            int newAttackSpeed = qMax(qFloor(unit->getAttackSpeed() * 0.85), 1);

            unit->setMaxHp(newMaxHp);
            unit->setHp(newMaxHp);     // 满血恢复
            unit->setAtk(newAtk);
            unit->setAttackSpeed(newAttackSpeed);
            unit->setEquipmentCount(0); // 重置装备数（2 星可装备 2 件）

            qDebug().noquote() << QString("  [升星属性] maxHp=%1 atk=%2 attackSpeed=%3")
                                      .arg(newMaxHp).arg(newAtk).arg(newAttackSpeed);
        }
        // 未来可添加 2星→3星 分支

    } catch (const std::exception &e) {
        qDebug() << "[StarUpSystem] applyStarUpBonus 异常：" << e.what();
    } catch (...) {
        qDebug() << "[StarUpSystem] applyStarUpBonus 未知异常";
    }
}
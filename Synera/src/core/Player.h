#ifndef PLAYER_H
#define PLAYER_H

#include <QString>
#include <QDebug>

// 玩家全局控制实体，管理资源与进度状态（金币、血量、等级、人口上限、当前关卡）
class Player
{
public:
    // 构造与析构

    // 初始：金币=10, 血量=100, 等级=1, 关卡=0, 经验=0, 升级所需经验=2
    explicit Player();

    ~Player() = default;

    

    void addGold(int amount);

    // 花费金币，余额不足返回 false
    bool spendGold(int amount);

    // 经验与升级

    // 经验累计到阈值自动升级，多余经验保留
    void addExp(int amount);

    // 等级+1，升级所需经验+2
    void levelUp();

    

    // 玩家血量归零则游戏失败
    bool isAlive() const;

    // Getter / Setter

    int getGold() const;
    void setGold(int gold);

    int getHp() const;
    void setHp(int hp);

    int getLevel() const;
    void setLevel(int level);

    // 人口上限 = level + 2 + bonusPopulation
    int getMaxPopulation() const;

    int getCurrentStage() const;
    void setCurrentStage(int stage);

    int getCurrentExp() const;
    int getExpToLevel() const;

    // 人口升级

    // 花金币升等级，费用 = level × 4
    bool buyLevelUp();

    int getLevelUpCost() const;

    static constexpr int MAX_LEVEL = 10;

    

    // 花 1 金币永久提高 1 人口上限
    bool buyBonusPopulation();

    int getBonusPopulation() const { return m_bonusPopulation; }

    void setBonusPopulation(int bonus) { m_bonusPopulation = bonus; }

    // 存档/读档支持

    void setCurrentExp(int exp);
    void setExpToLevel(int exp);

    // 辅助方法

    void debugPrint() const;

private:
    // 私有属性

    int m_gold;
    int m_hp;
    int m_level;
    int m_currentStage;
    int m_currentExp;
    int m_expToLevel;
    int m_bonusPopulation = 0;  // 额外人口加成
};

#endif // PLAYER_H
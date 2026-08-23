#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <QList>
#include <QPair>

class BoardWidget;

// A* 寻路工具类，在 8×8 棋盘上计算避开障碍的最短路径
class PathFinder {
public:
    // acceptAdjacent=true 时，终点被占则接受终点相邻的任意空格为终点
    // 返回路径坐标列表 (row, col)，不含起点，含终点；无路返回空列表
    static QList<QPair<int,int>> findPath(
        int startRow, int startCol,
        int endRow, int endCol,
        const BoardWidget *board,
        bool acceptAdjacent = true);
};

#endif // PATHFINDER_H
#include "core/PathFinder.h"
#include "gui/BoardWidget.h"

#include <cstdlib>
#include <QDebug>

// 曼哈顿距离
static int manhattan(int r1, int c1, int r2, int c2)
{
    return std::abs(r1 - r2) + std::abs(c1 - c2);
}

// A* 寻路，终点被占时 acceptAdjacent 接受邻接格
QList<QPair<int,int>> PathFinder::findPath(
    int startRow, int startCol,
    int endRow, int endCol,
    const BoardWidget *board,
    bool acceptAdjacent)
{
    try {
        const int ROWS = 8, COLS = 8;

        if (startRow < 0 || startRow >= ROWS ||
            startCol < 0 || startCol >= COLS ||
            endRow   < 0 || endRow   >= ROWS ||
            endCol   < 0 || endCol   >= COLS) {
            return {};
        }

        // 终点就是起点 → 不用走
        if (startRow == endRow && startCol == endCol) {
            return {};
        }

        int  gScore[ROWS][COLS];
        int  fScore[ROWS][COLS];
        bool closed[ROWS][COLS];
        bool open[ROWS][COLS];
        int  cameFromR[ROWS][COLS];
        int  cameFromC[ROWS][COLS];

        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                gScore[r][c]   = 999999;
                fScore[r][c]   = 999999;
                closed[r][c]   = false;
                open[r][c]     = false;
                cameFromR[r][c] = -1;
                cameFromC[r][c] = -1;
            }
        }

        gScore[startRow][startCol] = 0;
        fScore[startRow][startCol] = manhattan(startRow, startCol, endRow, endCol);
        open[startRow][startCol] = true;

        bool endOccupied = !board->isCellEmpty(endRow, endCol);

        const int dr[4] = {-1, 1, 0, 0};
        const int dc[4] = { 0, 0,-1, 1};

        int curR = -1, curC = -1;
        bool found = false;

        while (true) {
            int minF = 999999;
            curR = -1; curC = -1;
            for (int r = 0; r < ROWS; ++r)
                for (int c = 0; c < COLS; ++c)
                    if (open[r][c] && fScore[r][c] < minF) {
                        minF = fScore[r][c];
                        curR = r; curC = c;
                    }
            if (curR == -1) break;

            bool reached = (curR == endRow && curC == endCol);
            if (!reached && acceptAdjacent && endOccupied
                && manhattan(curR, curC, endRow, endCol) == 1) {
                reached = true;  // 终点被占，到邻接格即可
            }
            if (reached) { found = true; break; }

            open[curR][curC] = false;
            closed[curR][curC] = true;

            for (int i = 0; i < 4; ++i) {
                int nr = curR + dr[i];
                int nc = curC + dc[i];
                if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
                if (closed[nr][nc]) continue;

                bool isEnd = (nr == endRow && nc == endCol);
                if (!board->isCellEmpty(nr, nc) && !isEnd) continue;
                if (isEnd && endOccupied) continue;  // 终点被占不能踩上去

                int tg = gScore[curR][curC] + 1;
                if (!open[nr][nc] || tg < gScore[nr][nc]) {
                    open[nr][nc] = true;
                    gScore[nr][nc] = tg;
                    fScore[nr][nc] = tg + manhattan(nr, nc, endRow, endCol);
                    cameFromR[nr][nc] = curR;
                    cameFromC[nr][nc] = curC;
                }
            }
        }

        if (!found) return {};

        QList<QPair<int,int>> path;
        int r = curR, c = curC;
        while (r != -1 && c != -1 && !(r == startRow && c == startCol)) {
            path.prepend(qMakePair(r, c));
            int pr = cameFromR[r][c];
            int pc = cameFromC[r][c];
            r = pr; c = pc;
        }
        return path;

    } catch (const std::exception &e) {
        qDebug() << "[PathFinder] 异常：" << e.what();
        return {};
    } catch (...) {
        qDebug() << "[PathFinder] 未知异常";
        return {};
    }
}
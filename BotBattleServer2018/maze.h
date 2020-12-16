#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include "vec2d.h"
#include "random.h"

namespace mssm {
class Graphics;
}

class Maze {
public:
    class Pos {
    public:
        int row;
        int col;
    };
    enum class Direction {
        North,
        South,
        East,
        West
    };


    class WallPos {
    public:
        int row;
        int col;
        Direction dir;
    };
private:
    int numRows;
    int numCols;
    std::vector<std::vector<bool>> verticalWalls;
    std::vector<std::vector<bool>> horizontalWalls;
    std::vector<std::vector<bool>> markers;
    std::vector<std::vector<int>>  value;
    Random rnd;
public:
    Maze(int rows, int cols);
    void init(int rows, int cols);
#ifndef NOGRAPHICS
    void draw(mssm::Graphics& g, bool altStyle);
#endif
    void generate();
    void solve();
    std::vector<std::pair<Vec2d,Vec2d>> walls(bool includeOuter, Vec2d offset = Vec2d{0.0,0.0}, Vec2d scale = Vec2d{1.0,1.0});
    std::vector<Vec2d> centers(Vec2d offset = Vec2d{0.0,0.0}, Vec2d scale = Vec2d{1.0,1.0});
    int width()  { return numCols; }
    int height() { return numRows; }
private:
    bool isMarked(int row, int col);
    bool isMarked(int row, int col, Direction dir);
    void setMarked(int row, int col, bool marked);

    int  getValue(int row, int col);
    int  getValue(int row, int col, Direction dir);
    void setValue(int row, int col, int value);

    bool isOuterWall(int row, int col, Direction dir);
    bool hasWall(int row, int col, Direction dir);
    void setWall(int row, int col, Direction dir, bool hasWall);
    void buildMaze(int row, int col);
    void clearMarked();
    void buildMaze2();

    void propogateValue(int row, int col, int oldValue, int newValue);
    void markByValue(int row, int col, int v);
    bool solve(int row, int col, int destRow, int destCol);
};


#endif // MAZE_H

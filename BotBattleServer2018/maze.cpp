#include "maze.h"


#ifndef NOGRAPHICS
#include "graphics.h"
#endif

using namespace std;
using namespace mssm;

const std::vector<Maze::Direction> compassRose = { Maze::Direction::North, Maze::Direction::South, Maze::Direction::East, Maze::Direction::West };


int rowInDir(int row, Maze::Direction dir)
{
    switch (dir) {
    case Maze::Direction::North: return row-1;
    case Maze::Direction::South: return row+1;
    default: return row;
    }
}

int colInDir(int col, Maze::Direction dir)
{
    switch (dir) {
    case Maze::Direction::West: return col-1;
    case Maze::Direction::East: return col+1;
    default: return col;
    }
}

Maze::Maze(int rows, int cols)
{
    init(rows, cols);
}

void Maze::init(int rows, int cols)
{
    numRows = rows;
    numCols = cols;
    verticalWalls = vector<vector<bool>>(rows, vector<bool>(cols+1, true));
    horizontalWalls = vector<vector<bool>>(rows+1, vector<bool>(cols, true));
    markers = vector<vector<bool>>(rows, vector<bool>(cols+1, false));

    value = vector<vector<int>>(rows, vector<int>(cols, 0));
}

#ifndef NOGRAPHICS
void drawGrid(Graphics& g, int numRows, int numCols, std::function<mssm::Color(int row, int col)> cellColor)
{
    constexpr int margin = 10;
    double sizeX = (g.width()-2*margin)/numCols;
    double sizeY = (g.height()-2*margin)/numRows;

    double blockSize = min(sizeX, sizeY);

    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            double y = i * blockSize + margin;
            double x = j * blockSize + margin;
            Color color = cellColor(i,j);
            g.rect(x+1,y+1,blockSize-2,blockSize-2,color,color);
        }
    }
}
#endif // NOGRAPHICS

std::vector<std::pair<Vec2d,Vec2d>> Maze::walls(bool includeOuter, Vec2d offset, Vec2d scale)
{
    // start and end wall indices
    int sh = includeOuter ? 0 : 1;
    int eh = includeOuter ? numRows : numRows-1;
    int sv = sh;
    int ev = includeOuter ? numCols : numCols-1;

    std::vector<std::pair<Vec2d,Vec2d>> mergedWalls;

    // generate merged horizontal walls
    for (int r = sh; r <= eh; r++) {
        double startx = 0;
        bool wasWall = false;
        for (int c = 0; c < numCols; c++) {
            if (horizontalWalls[r][c]) {
                if (wasWall) {
                    // continue wall
                }
                else {
                    // start wall
                    startx = c;
                    wasWall = true;
                }
            }
            else {
                if (wasWall) {
                    // finish wall
                    Vec2d p1(startx, r);
                    Vec2d p2(c, r);
                    mergedWalls.push_back(make_pair(p1,p2));
                    wasWall = false;
                }
                else {
                    // no wall
                }
            }
        }
        if (wasWall) {
            // finish wall
            Vec2d p1(startx, r);
            Vec2d p2(numCols, r);
            mergedWalls.push_back(make_pair(p1,p2));
        }
    }

    // generate merged vertical walls
    for (int c = sv; c <= ev; c++) {
        double starty = 0;
        bool wasWall = false;
        for (int r = 0; r < numRows; r++) {
            if (verticalWalls[r][c]) {
                if (wasWall) {
                    // continue wall
                }
                else {
                    // start wall
                    starty = r;
                    wasWall = true;
                }
            }
            else {
                if (wasWall) {
                    // finish wall
                    Vec2d p1(c, starty);
                    Vec2d p2(c, r);
                    mergedWalls.push_back(make_pair(p1,p2));
                    wasWall = false;
                }
                else {
                    // no wall
                }
            }
        }
        if (wasWall) {
            // finish wall
            Vec2d p1(c, starty);
            Vec2d p2(c, numRows);
            mergedWalls.push_back(make_pair(p1,p2));
        }
    }

    // apply scale and offset
    for (pair<Vec2d,Vec2d>& wall : mergedWalls) {
        wall.first.x *= scale.x;
        wall.first.y *= scale.y;
        wall.second.x *= scale.x;
        wall.second.y *= scale.y;
        wall.first = wall.first + offset;
        wall.second = wall.second + offset;
    }

    return mergedWalls;
}

// returns list of center points (middle of each cell) within the maze
std::vector<Vec2d> Maze::centers(Vec2d offset, Vec2d scale)
{
    std::vector<Vec2d> retval;

    offset = offset + Vec2d{scale.x/2.0, scale.y/2.0};

    for (int x = 0; x < this->numCols; x++) {
        for (int y = 0; y < this->numRows; y++) {
            retval.push_back(Vec2d(x*scale.x,y*scale.y)+offset);
        }
    }

    return retval;
}


#ifndef NOGRAPHICS
void Maze::draw(Graphics& g, bool altStyle)
{
    if (altStyle) {
        drawGrid(g, numRows*2+1, numCols*2+1, [this](int r, int c) {
           constexpr Color wallColor = BLACK;
           constexpr Color floorColor = WHITE;

           bool rowOdd = r % 2 == 1;
           bool colOdd = c % 2 == 1;

           if (r == numRows*2) {
               // last row, just look at horizontal wall below
               if (colOdd) {
                   return horizontalWalls[r/2][c/2] ? wallColor : floorColor;
               }
               return wallColor;
           }
           if (c == numCols*2) {
               if (rowOdd) {
                   return verticalWalls[r/2][c/2] ? wallColor : floorColor;;
               }
               return wallColor;
           }

           if (rowOdd && colOdd) {
               if (markers[r/2][c/2]) {
                   return GREEN;
               }
               else {
                   return floorColor;
               }
           }
           else if (!rowOdd && !colOdd){
               return wallColor;
           }
           else if (rowOdd && !colOdd){
               if ((c/2) < numCols && verticalWalls[r/2][c/2]) {
                   return wallColor;
               }
               return floorColor;
           }
           else {
               if ((r/2) < numRows && horizontalWalls[r/2][c/2]) {
                   return wallColor;
               }
               return floorColor;
           }
        });
    }
    else {
        constexpr int margin = 10;
        double sizeX = (g.width()-2*margin)/numCols;
        double sizeY = (g.height()-2*margin)/numRows;
        double blockSize = min(sizeX, sizeY);

        // draw vertical walls
        for (int i = 0; i < numRows; i++) {
            for (int j = 0; j <= numCols; j++) {
                if (verticalWalls[i][j]) {
                    double x = j * blockSize + margin;
                    double y1 = i * blockSize + margin;
                    double y2 = (i+1) * blockSize + margin;
                    g.line(x,y1,x,y2,WHITE);
                }
            }
        }

        // draw horizontal walls
        for (int i = 0; i <= numRows; i++) {
            for (int j = 0; j < numCols; j++) {
                if (horizontalWalls[i][j]) {
                    double y = i * blockSize + margin;
                    double x1 = j * blockSize + margin;
                    double x2 = (j+1) * blockSize + margin;
                    g.line(x1,y,x2,y,WHITE);
                }
            }
        }

        for (int i = 0; i < numRows; i++) {
            for (int j = 0; j < numCols; j++) {
                if (markers[i][j]) {
                    double y = i * blockSize + margin;
                    double x = j * blockSize + margin;
                //    g.rect(x+2,y+2,blockSize-4,blockSize-4, GREEN, GREEN);
                    g.ellipse(x+blockSize/2-2, y+blockSize/2-2, 4, 4, GREEN, GREEN);
                }
            }
        }
    }
}
#endif // NOGRAPHICS

void Maze::generate()
{
    init(numRows, numCols);
    buildMaze(0, 0);
    clearMarked();
}

void Maze::solve()
{
    solve(0,0,numRows-1, numCols-1);
}

////////// hamlin methods below



void Maze::markByValue(int row, int col, int v)
{
    if (value[row][col] == v) {
        setMarked(row, col, true);
        for (Direction dir : compassRose) {
            if (!isOuterWall(row, col, dir) &&
                    !isMarked(row, col, dir) &&
                    getValue(row, col, dir) == v) {
                markByValue(rowInDir(row, dir), colInDir(col, dir), v);
            }
        }
    }
}


bool Maze::solve(int row, int col, int destRow, int destCol)
{
    setMarked(row, col, true);

    if (row == destRow && col == destCol) {
        clearMarked();
        setMarked(row, col, true);
        return true;
    }

    for (Direction dir : compassRose) {
        if (!isOuterWall(row, col, dir) && !hasWall(row, col, dir) && !isMarked(row, col, dir)) {
            if (solve(rowInDir(row, dir), colInDir(col, dir), destRow, destCol)) {
                setMarked(row, col, true);
                return true;
            }
        }
    }

    return false;
}

void Maze::buildMaze2()
{
    init(numRows, numCols);

    vector<WallPos> allWalls;

    // vertical walls
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols-1; j++) {
            allWalls.push_back(WallPos{i,j,Direction::East});
        }
    }

    // horizontal walls
    for (int i = 0; i < numRows-1; i++) {
        for (int j = 0; j < numCols; j++) {
            allWalls.push_back(WallPos{i,j,Direction::South});
        }
    }

    allWalls = rnd.shuffled(allWalls);

    int count = 1;
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            setValue(i,j,count++);
        }
    }

    for (WallPos wall : allWalls) {
        if (getValue(wall.row, wall.col) != getValue(wall.row, wall.col, wall.dir)) {
            // merge across this wall
            setWall(wall.row, wall.col, wall.dir, false);
            propogateValue(wall.row, wall.col, getValue(wall.row, wall.col), getValue(wall.row, wall.col, wall.dir));
            //g.clear();
            //draw(g, 30, 20, 20);
            //g.draw();
        }
    }
}


void Maze::propogateValue(int row, int col, int oldValue, int newValue) {
    setValue(row, col, newValue);
    for (Direction dir : compassRose) {
        if (!isOuterWall(row, col, dir) && getValue(row, col, dir) == oldValue) {
            propogateValue(rowInDir(row, dir), colInDir(col, dir), oldValue, newValue);
        }
    }
}

void Maze::clearMarked()
{
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            markers[i][j] = false;
        }
    }
}

bool Maze::isMarked(int row, int col)
{
    return markers[row][col];
}

void Maze::setMarked(int row, int col, bool v)
{
    markers[row][col] = v;
}

bool Maze::isMarked(int row, int col, Direction dir)
{
    return isMarked(rowInDir(row, dir), colInDir(col, dir));
}

int Maze::getValue(int row, int col)
{
    return value[row][col];
}

int Maze::getValue(int row, int col, Direction dir)
{
    return getValue(rowInDir(row, dir), colInDir(col, dir));
}

void Maze::setValue(int row, int col, int v)
{
    value[row][col] = v;
}

bool Maze::isOuterWall(int row, int col, Direction dir)
{
    switch (dir) {
    case Direction::North: return row == 0;
    case Direction::South: return row == numRows-1;
    case Direction::East: return col == numCols-1;
    case Direction::West: return col == 0;
    }
    return false;
}

bool Maze::hasWall(int row, int col, Direction dir)
{
    switch (dir) {
    case Direction::North:
        return horizontalWalls[row][col];
    case Direction::South:
        return horizontalWalls[row+1][col];
    case Direction::East:
        return verticalWalls[row][col+1];
    case Direction::West:
    default:
        return verticalWalls[row][col];
    }
}

void Maze::setWall(int row, int col, Direction dir, bool hasWall)
{
    switch (dir) {
    case Direction::North:
        horizontalWalls[row][col] = hasWall;
        return;
    case Direction::South:
        horizontalWalls[row+1][col] = hasWall;
        return;
    case Direction::East:
        verticalWalls[row][col+1] = hasWall;
        return;
    case Direction::West:
        verticalWalls[row][col] = hasWall;
        return;
    }
}

void Maze::buildMaze(int row, int col)
{
    setMarked(row, col, true);
    for (Direction dir : rnd.shuffled(compassRose)) {
        if (!isOuterWall(row, col, dir) && !isMarked(row, col, dir)) {
            setWall(row, col, dir, false);
            buildMaze(rowInDir(row, dir), colInDir(col, dir));
        }
    }
}

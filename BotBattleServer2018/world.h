#ifndef WORLD_H
#define WORLD_H

#define _USE_MATH_DEFINES

#include <vector>
#include <set>
#include <queue>
#include <memory>

#include "bot.h"
#include "vec2d.h"
#include "range2d.h"
#include "uniqueid.h"

//#include <QMutex>
//#include <QMutexLocker>
#include "worldeventqueue.h"

class BulletHitWallEvt;
class BotHitWallEvt;
class BotCmdFinishedEvt;
class BulletHitBotEvt;
class BotHitBotEvt;

class Wall
{
public:
    bool   vertical;
    double location; // if vertical, this is the x position
    double minBound; // if vertical, this is the min y position
    double maxBound; // if vertical, this is the max y position
public:
#ifndef NOGRAPHICS
    void draw(mssm::Graphics& g) const;
#endif
    Vec2d p1() const { return vertical ? Vec2d{location, minBound} : Vec2d{ minBound, location }; }
    Vec2d p2() const { return vertical ? Vec2d{location, maxBound} : Vec2d{ maxBound, location }; }
    void save(std::ostream& strm) const;
};

class BotHistory {
public:
    int id;
    std::string name;
    ActorState initialState;
    std::vector<Motion> motionHistory;
    int alreadyGotten{-1}; // -1 = nothing 0 means initial state has been sent, 1 means first motionHistory has been sent, etc.

    void sort();

    bool getJson(std::ostream& strm, bool &firstRecord);

    void save(std::ostream& strm) const;
    void load(std::istream& strm);
};

class FoodHistory {
public:
    Vec2d position;
    double startTime;
    double endTime;
    bool sentStart{false};
    bool sentEnd{false};
public:
    bool getJson(std::ostream& strm, bool &firstRecord);
};

class BotHistorian
{
private:
    std::vector<FoodHistory> activeFood;
    std::vector<FoodHistory> consumedFood;
    std::vector<BotHistory>  allBotHistory;
public:
    void save(std::ostream& strm) const;
    void clear();

    void beginBotHistory(size_t id, const std::string& name, const ActorState& initialState);
    void appendBotHistory(size_t id, const std::vector<Motion>& motionHist, bool isDead);

    void beginFood(Vec2d position, double time);
    void endFood(Vec2d position, double time);

    bool getJson(std::ostream& strm);
};

enum class LastUpdateState {
    Waiting,
    Completed,
    SingleStep,
    MaxCompLimit,
    NoEvents
};

#undef assert

class World
{
private:
    static constexpr double moveStunThreshold = 0.001;

    std::vector<std::shared_ptr<Food>> food;
    std::vector<std::shared_ptr<Bot>> newBots;
    std::vector<std::shared_ptr<Bot>> bots;
    std::vector<std::shared_ptr<Bullet>> bullets;
    std::vector<int>     lastView;
    std::vector<std::shared_ptr<Bot>> deadBots;
    std::vector<std::pair<std::string,int>> messages;

    bool messagesSuppressed{false};

    double foodRadius{5};
    std::vector<Vec2d> foodSpawnPoints;
    std::vector<Vec2d> usedSpawnPoints;

    std::vector<Wall> innerWalls;
    std::vector<Wall> allWalls;

    BotHistorian historian;

    UniqueId ids;

    Range2d field;

    int nextBotColor;

    double gameTime;

    bool stalePositions; // true when gameTime has advanced, but advanceGameTime has not yet been called to update the positions of bots/bullets/etc

    int clockMult;

    WorldEventQueue eventQueue;

public:
    static constexpr int defaultDesiredFoodCount{10};

    int minBotCount;
    int desiredFoodCount;

    bool paused;

    LastUpdateState lastUpdateState{LastUpdateState::Completed};

    int remoteBotBullets{5};
    int remoteBotHealth{5};

    int localBotBullets{5};
    int localBotHealth{5};

public:
    World(Range2d field, int mazeCols, int mazeRows);
   ~World();

#ifndef NOGRAPHICS
    void draw(mssm::Graphics& g);
#endif

    void faster();
    void slower();
    int  update(double realElapsedTime, double maxCalculationTime, bool singleStep, bool alwaysAdvance);

public:

    int  getNextBotId();
    void releaseBotId(int id);

    void advanceGameTime();
    void killDisconnectedBots(int maxWaitTimeMs);

    void endGame();
    void clearHistory();
    std::string streamHistory();

    void saveHistory(std::ostream& strm) const;
    void loadHistory(std::istream& strm);

    bool assert(bool mustBeTrue, const std::string& msg);
public:

    // these should only be called by the corresponding event objects
    void bulletHitWallEvt(Bullet* bullet);
    void botHitWallEvt( std::shared_ptr<Bot> bot, Vec2d pos, double collisionAngle);
    void botCmdFinishedEvt(std::shared_ptr<Bot> bot);
    void bulletHitBotEvt(Bullet* bullet, std::shared_ptr<Bot> bot);
    void botHitBotEvt(std::shared_ptr<Bot> bot1, std::shared_ptr<Bot> bot2, Vec2d pos1, Vec2d pos2, double angle1, double angle2);
    void botHitFoodEvt(std::shared_ptr<Bot> bot, std::shared_ptr<Food>, Vec2d pos, double angle);

public:

    std::vector<int> look(/*mssm::Graphics* g, */double fieldOfView, Vec2d cameraPos, double cameraDirection, int resolution = 500, bool distance = false);
    double currentTime() { return gameTime; }
    void fire(std::shared_ptr<Bot> shooter, Vec2d bulletPos, double bulletDirection, double bulletOffset);
    void fakeFire(Vec2d bulletPos, double bulletDirection);
    void placeFood(Vec2d foodPos);
    void updateBotEvents(std::shared_ptr<Bot> bot);
    void message(const std::string& msg, int color) { messages.push_back(std::make_pair(msg, color)); }
    void addBot(std::shared_ptr<Bot> bot, int maxHealth, int initBullets);
    void addBot(std::shared_ptr<Bot> bot, Vec2d pos, double dir, int maxHealth, int initBullets);

    void spawnFood(int minFood);
    Vec2d nextSpawnPoint();

    int updateWaitingBots();

private:
    void updateHistory();

    bool introduceNewBots(bool addOneAtMost);
    void killBot(std::shared_ptr<Bot> bot, std::string reason);
    void killFood(std::shared_ptr<Food> food);

    void flushDeadBots();

    double fixedWallCollisionTime(Vec2d pos, Vec2d vel, double radius, Vec2d& collidePos, double& wallNormal);

    void updateBot(std::shared_ptr<Bot> bot, bool transmitEvents = true);

    bool   isFreeSpace(Vec2d pos, double radius);

    void   deleteBullet(Bullet* b);

    void   updateBulletEvents(std::shared_ptr<Bullet> bullet);
    void   updateFoodEvents(std::shared_ptr<Food> food);

    double calculateSubTime(double elapsedTime);

    int hitWall(Vec2d& position, double radius, double& wallNormalAngle);  // 0 = no collision bits 1-4 = top,right,bottom,left
};

#endif // WORLD_H

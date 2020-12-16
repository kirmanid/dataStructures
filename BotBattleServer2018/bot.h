#ifndef BOT_H
#define BOT_H

#define _USE_MATH_DEFINES

#include <string>
#include <vector>
#include "vec2d.h"
#include "serverbotcmd.h"
#include "botai.h"
#include "range.h"
#include "uniqueid.h"

double subtractAngles(double a1, double a2);
double negateAngle(double angle);

class World;
class Bot;

namespace mssm {
class Graphics;
}

#if defined __GNUC__
# define LIKELY(EXPR)  __builtin_expect(!!(EXPR), 1)
#else
# define LIKELY(EXPR)  (!!(EXPR))
#endif

#if defined NDEBUG
# define X_ASSERT(CHECK) void(0)
#else
# define X_ASSERT(CHECK) \
    ( LIKELY(CHECK) ?  void(0) : []{assert(!#CHECK);}() )
#endif


enum class MotionType
{
    Unknown,
    Translate,
    Rotate,  // x component of start/end/velocity is angle
    Health,  // x component of start/end/velocity is health.  Timespan may be zero
    Scan,    // x component of start/end/velocity is scan position wrt bot heading
    Fire,    // x component of start/end/velocity is 0-1 charge level. Bullet launch occurs at end of timespan
    Bullet,  // same as translate  (is this necessary to distinguish?)
    PowerUp, // x component of start/end/velocity is bullets.  Timespan may be zero
    Sleep,   // timespan starts out "infinite"
    Resign,  // x component of start/end/velocity is 0-1.  Damage applied at end of timespan?
};

class ActorState {
private:
    Vec2d        position;
    Vec2d        velocity;
    double       direction;
    double       health;
    int          bulletCount;
public:
    ActorState();
    void init(Vec2d position, Vec2d velocity, double direction, double health, int bullets);
    double getHealth() const { return health; }
    Vec2d  getVelocity() const { return velocity; }
    Vec2d  getPosition() const { return position; }
    double getDirection() const { return direction; }
    int    getBullets() const { return bulletCount; }
    friend class Motion;

    void save(std::ostream& strm) const;
    void load(std::istream& strm);

    double speed() const;
};

class Motion {
private:
    MotionType type{MotionType::Unknown};
    Vec2d start;
    Vec2d end;
    Vec2d rate;
    Range timeSpan;
public:
    Motion(MotionType type, double startTime);
    //void changeType(MotionType newType);
    void setEndTime(double endTime);
    Vec2d curr(double time) const { return start + rate*(time - timeSpan.minVal); }
    double progress(double time) const { return parameterizeConstrained(time, timeSpan.minVal, timeSpan.maxVal); }
private:
    void fixRate();
    void set(MotionType type, double startTime, double endTime, Vec2d startPos, Vec2d endPos);
    void set(MotionType type, double startTime, double endTime, double startVal, double endVal);
public:
    void initSleep(double time);
    void initBullet(double time, double duration, Vec2d startPos, Vec2d endPos);
    void initMove(double time, double duration, Vec2d startPos, Vec2d endPos);
    void initTurn(double time, double duration, double startHeading, double endHeading);
    void initScan(double time, double duration, double startScan, double endScan);
    void initFire(double time, double duration, int startBullets);
    void initHealth(double time, double duration, double startHealth, double endHealth);
    void initPowerUp(double time, double duration, double startBullets, double endBullets);
    void initResign(double time, double duration);

    void complete(double time);
    void interrupt(double time, Vec2d pos); // note: Pos only used by Bullet/Move motions



    void invalidate();

    void apply(ActorState& actor, double time, bool forward) const;

    void save(std::ostream& strm) const;
    void load(std::istream& strm);

    MotionType motionType() const { return type; }
    double startTime() const { return timeSpan.minVal; }
    double endTime() const { return timeSpan.maxVal; }
    Vec2d  getRate() const { return rate; }
    double startScalar() const { return start.x; }
    double endScalar() const { return end.x; }
    double rateScalar() const { return rate.x; }
};

/*
What is truly complete?   Every (completed) event whose end point occurs before (or at the same time as?) the beginning
                          of any of the active events
                          completed implies that the end is before or coincident with the current time

GotShot
GotHit
HitWall

MoveComplete
MoveBlockedByWall
MoveBlockedByBot
TurnComplete
ScanComplete
FireComplete
NoBullets

GotShot
GotHit

Move
Scan
Fire
Turn
Ignore (GotShot or GotHit)
*/

class MotionTracker {
private:
    std::vector<std::unique_ptr<Motion>> active;
    std::vector<std::unique_ptr<Motion>> completed;
public:
    Motion* create(MotionType type, double startTime);
    void complete(Motion* motion);
    std::vector<Motion> removeCompleted();
    double earliestActiveTime();
    std::vector<Motion> removeCompletedBefore(double time);
};

class Food
{
private:
    Vec2d  location;
    double radius;
    double createTime;
    double deleteTime;
public:
    Food(Vec2d pos, double radius, double createTime);
    Vec2d getPos() const { return location; }
    double getRadius() const { return radius; }
    void finish(double time);
#ifndef NOGRAPHICS
    void draw(mssm::Graphics& g) const;
#endif
};


class Shooter {
public:
    MotionTracker motionTracker;
protected:
    int        strikes{0};
    int        kills{0};
public:
    virtual ~Shooter();
    void recordBullet(Motion* motion);
    virtual std::string name() const { return "DummyShooter"; }

    void recordStrike() { strikes++; }
    void recordKill()   { kills++; }
};

class Bullet
{
public:
    static int   bulletCount;

    Motion*      motion; // owned by motionTracker in Bot
    ActorState   state;

private:
    std::shared_ptr<Shooter> shooter;
    std::vector<Vec2d> pts;
public:
    Bullet(std::shared_ptr<Shooter> shooter, double startTime, Vec2d pos, double angle, Vec2d velocity);
    virtual ~Bullet();
#ifndef NOGRAPHICS
    void draw(mssm::Graphics& g) const;
#endif
    void updatePosition(double currentTime);
    void finish(Vec2d pos, double time);
    static constexpr double radius() { return 2.5; }
    std::string shooterName() const;
    std::shared_ptr<Shooter> getShooter() const { return shooter; }
    bool isShooter(Shooter* bot) const { return shooter.get() == bot; }
    double wallTime() const { return motion->endTime(); }
};

class Bot : public Shooter
{
private:
    static int botCount;

public:
    unsigned int color;
private:
    double       diameter;
    size_t       botId;
    bool         inCommWait;
    std::chrono::time_point<std::chrono::system_clock> waitStart;  // in real wall clock time, not game time
public:

    ActorState   state;
    Motion*      currentMotion;

    ActorState   initialState;

private:
    std::unique_ptr<ServerBotCmd> currentCommand;

    World *world;

    std::unique_ptr<BotEvent> eventToSend;

    bool lastEventWasIgnorable{false};

    ServerBotAI*       brain;
    Range        shotEffect;
    Range        collideEffect;

public:
    Bot(World *world, ServerBotAI* brain, size_t id, bool dummy);
    virtual ~Bot();
    unsigned int getColor()  const { return color; }

#ifndef NOGRAPHICS
    void draw(mssm::Graphics& g, double currentTime) const;
#endif

    bool hasOngoingVisual(double currentTime) const;

    int getId() const { return botId; }

    Vec2d getPos() const { return state.getPosition(); }     // note: only valid if advanceGameTime has been called
    Vec2d getVel() const { return state.getVelocity(); }
    double getDir() const { return state.getDirection(); }   // note: only valid if advanceGameTime has been called


    std::string name()  const override { return brain->getName(); }
    bool willNotFinishBefore(double time) const { return !currentCommand || time < currentCommand->finishTime();  }
    double commandFinishTime() const { return currentCommand->finishTime(); }
    double commandStartTime() const { return currentCommand->startTime(); }
    std::string commandName() const { return currentCommand->name(); }
    bool   isMoving() const  { return currentCommand && currentCommand->isMoving(); }
    bool isBlockingCollision(double relativeCollisionDir);
    double radius() const { return diameter / 2; }
    bool isDead()  const { return state.getHealth() <= 0;  }
    bool isDisconnected() const { return brain->isDisconnected(); }
    int commWaitTimeMs(std::chrono::time_point<std::chrono::system_clock> nowTime) const;
    bool isWaiting() const { return inCommWait; }
    bool hasCommand() const { return currentCommand.operator bool(); }
    bool notAsleep() const { return hasCommand() && currentCommand->type() != BotCmdType::Asleep; }
    BotCmdType cmdType() const { return currentCommand->type(); }

    ServerBotAI* getAI() { return brain; }

    void initializePosition(Vec2d pos, double direction, int maxHealth, int initBullets);

    void setDisconnected();

    void finishCommand(std::shared_ptr<Bot> thisBot, double currentTime, bool interrupted, Vec2d interruptPos = Vec2d{0,0});
    void updateCommand(double currentTime);
    //void saveMotion();


    void newCurrentMotion(double time);
    void completeCurrentMotion(double currentTime, bool interrupted, Vec2d interruptPos);

    double earliestActiveTime() { return motionTracker.earliestActiveTime(); }
    std::vector<Motion> removeCompletedBefore(double time) { return motionTracker.removeCompletedBefore(time); }

    void transmitEvents(double currentTime);
    bool receiveNewCommand(std::shared_ptr<Bot> thisBot, double currentTime);  // if it returns true, the world must call updateBotEvents!

    void applyDamage(double eventTime, bool instaKill);  // doesn't handle death...
    void applyPowerUp(double eventTime); // doesn't handle death...

    // record events to be sent to the AI brain.  Some of these have side effects (gotShotEvt, shotFireEvt, and doScanEvt)
    void gotFoodEvt(double eventTime, double foodAngle);
    void gotShotEvt(double eventTime, std::shared_ptr<Shooter> shooter, double bulletAngle);
    void hitWallEvt(double eventTime, double wallNormalAngle);
    void hitBotEvt(double eventTime, double otherBotDirection);
    void hitByBotEvt(double eventTime, double otherBotDirection);
    void moveCompleteEvt(double eventTime);
    void turnCompleteEvt(double eventTime);
    void shotFireEvt(std::shared_ptr<Bot> thisBot, double eventTime);
    void noBulletEvt(double eventTime);
    void doScanEvt(double eventTime, double fov);
    void resignEvt(double eventTime);

private:

    BotEvent& addEvent(BotEventType et, double eventTime);

    //int  turnDir() const  { return currentCommand->turnDir(); }

    // called by receiveNewCommand
    bool beginCommand(std::shared_ptr<Bot> thisBot, double currentTime, BotCmd cmd);
    void beginCommandSub(std::shared_ptr<Bot> thisBot, std::unique_ptr<ServerBotCmd> cmd);
};

#endif // BOT_H

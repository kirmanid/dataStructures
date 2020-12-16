#ifndef SERVERBOTCMD_H
#define SERVERBOTCMD_H

#include <memory>
#include "botcmd.h"
#include "vec2d.h"

class Bot;

class ServerBotCmd
{
protected:
    static constexpr double minCmdTime = 0.1;
public:
    static int cmdCount;
protected:
    double sTime;
public:
    ServerBotCmd(double sTime);
    virtual ~ServerBotCmd();

    virtual BotCmdType type() const = 0;

    virtual void   beginCmd(Bot* bot, double currentTime) = 0;
    virtual void   updateCmd(Bot* bot, double currentTime) = 0;
    virtual void   finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos) = 0;

    virtual double finishTime() const = 0;
    double startTime() const { return sTime; }

    virtual bool   isMoving() const { return false; }
    virtual int    turnDir() const { return 0; }

    virtual double distanceMoved(double currentTime) const ;
    virtual double angleTurned(double currentTime) const ;

    double elapsedTime(double currentTime) const ;
    double progress(double currentTime) const ; // percent complete based on time

    virtual std::string name() const  = 0;
};

class ServerCmdMove : public ServerBotCmd
{
private:
    double speed;
    Vec2d  startPos;
    Vec2d  velocity;
    double moveTime;
public:
    ServerCmdMove(double sTime, Vec2d startPos, double angle, double speed, double time);

    virtual BotCmdType type() const  { return BotCmdType::Move; }

    virtual void   beginCmd(Bot* bot, double currentTime);
    virtual void   updateCmd(Bot* bot, double currentTime);
    virtual void   finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos);

    virtual double finishTime() const;

    Vec2d positionAtTime(double currentTime) const ;

    bool isMoving()  const { return true; }

    double distanceMoved(double currentTime) const ;

    std::string name() const  { return "Move"; }
};

class ServerCmdTurn : public ServerBotCmd
{
private:
    double startAngle;
    double turnAngle;
    double turnTime;
    double turnRate;
public:
    ServerCmdTurn(double sTime, double startAngle, double turnAngle);

    virtual BotCmdType type() const  { return BotCmdType::Turn; }

    virtual void   beginCmd(Bot* bot, double currentTime);
    virtual void   updateCmd(Bot* bot, double currentTime);
    virtual void   finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos);
    virtual double finishTime() const;

    double angleAtTime(double currentTime) const ;

    int    turnDir()  const { return turnRate < 0 ? -1 : 1; }

    double angleTurned(double currentTime) const ;

    std::string name()  const { return "Turn"; }
};

class ServerCmdFire : public ServerBotCmd
{
private:
    double timeToFire;
    int    startBullets;
public:
    ServerCmdFire(double sTime, int startBullets);

    virtual BotCmdType type()  const { return BotCmdType::Fire; }

    virtual void   beginCmd(Bot* bot, double currentTime);
    virtual void   updateCmd(Bot* bot, double currentTime);
    virtual void   finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos);
    virtual double finishTime() const;

    std::string name()  const { return "Fire"; }
};

class ServerCmdScan : public ServerBotCmd
{
private:
    double scanAngle;
    double scanRate;
    double scanTime;
public:
    ServerCmdScan(double sTime, double angle);

    virtual BotCmdType type()  const { return BotCmdType::Scan; }

    virtual void   beginCmd(Bot* bot, double currentTime);
    virtual void   updateCmd(Bot* bot, double currentTime);
    virtual void   finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos);

    virtual double finishTime() const;

    std::string name() const  { return "Scan"; }
};

class ServerCmdAsleep : public ServerBotCmd
{
public:
    ServerCmdAsleep(double sTime);

    virtual BotCmdType type()  const { return BotCmdType::Asleep; }

    virtual void   beginCmd(Bot* bot, double currentTime);
    virtual void   updateCmd(Bot* bot, double currentTime);
    virtual void   finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos);

    virtual double finishTime() const;

    std::string name() const  { return "Asleep"; }
};

class ServerCmdResign : public ServerBotCmd
{
    double timeToResign;
public:
    ServerCmdResign(double sTime);

    virtual BotCmdType type()  const { return BotCmdType::Resign; }

    virtual void   beginCmd(Bot* bot, double currentTime);
    virtual void   updateCmd(Bot* bot, double currentTime);
    virtual void   finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos);

    virtual double finishTime() const;

    std::string name() const  { return "Resign"; }
};

#endif // SERVERBOTCMD_H

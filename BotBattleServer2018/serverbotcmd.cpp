#include "serverbotcmd.h"
#include "bot.h"
//#include "graphics.h"
#include "range.h"
#include <exception>

int ServerBotCmd::cmdCount = 0;

int colorD(double r, double g, double b)
{
    int rcomp = constrain((int)(0xFF * constrain(r, 0.0, 1.0)), 0, 255);
    int gcomp = constrain((int)(0xFF * constrain(g, 0.0, 1.0)), 0, 255);
    int bcomp = constrain((int)(0xFF * constrain(b, 0.0, 1.0)), 0, 255);

    return rcomp << 16 | gcomp << 8 | bcomp;
}

ServerBotCmd::ServerBotCmd(double startTime) : sTime(startTime)
{
    cmdCount++;
}

ServerBotCmd::~ServerBotCmd()
{
    cmdCount--;
}

double ServerBotCmd::progress(double currentTime)  const // percent complete based on time
{
    return parameterizeConstrained(currentTime, startTime(), finishTime());
}

double ServerBotCmd::elapsedTime(double currentTime) const
{
    return constrain(currentTime, startTime(), finishTime()) - startTime();
}

double ServerBotCmd::distanceMoved(double /*currentTime*/) const
{
    return 0;
}

double ServerBotCmd::angleTurned(double /*currentTime*/) const
{
    return 0;
}

ServerCmdMove::ServerCmdMove(double startTime, Vec2d startPos, double direction, double rawSpeed, double time): ServerBotCmd(startTime), speed{rawSpeed}, startPos(startPos)
{
    speed = constrain(speed, -50.0, 50.0);

    if (time < minCmdTime) {
        double distance = speed*time;
        speed = distance / minCmdTime;
        time = minCmdTime;
    }
    moveTime  = time;
    velocity  = Vec2d(speed, 0).rotated(direction);
}

double ServerCmdMove::finishTime() const
{
    return startTime() + moveTime;
}

Vec2d ServerCmdMove::positionAtTime(double currentTime) const
{
    return startPos + velocity * elapsedTime(currentTime);
}

double ServerCmdMove::distanceMoved(double currentTime) const
{
    return speed * elapsedTime(currentTime);
}

void ServerCmdMove::beginCmd(Bot* bot, double currentTime)
{
    bot->newCurrentMotion(currentTime);
    bot->currentMotion->initMove(currentTime, moveTime, startPos, startPos + moveTime*velocity);
    bot->currentMotion->apply(bot->state, currentTime, true);
}

void ServerCmdMove::updateCmd(Bot* bot, double currentTime)
{
    bot->currentMotion->apply(bot->state, currentTime, true);
}

void ServerCmdMove::finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos)
{
    bot->completeCurrentMotion(currentTime, interrupted, interruptPos);

    if (!interrupted) {
        bot->moveCompleteEvt(currentTime);
    }
}

ServerCmdTurn::ServerCmdTurn(double startTime, double startAngle, double turnAngle) : ServerBotCmd(startTime), startAngle(startAngle), turnAngle(turnAngle)
{
    if (turnAngle < 0)
    {
        turnRate = -1.0;
    }
    else
    {
        turnRate = 1.0;
    }

    turnTime = turnAngle / turnRate;

    if (turnTime < minCmdTime) {
        turnRate = turnAngle / minCmdTime;
        turnTime = minCmdTime;
    }
}

double ServerCmdTurn::finishTime() const
{
    return startTime() + turnTime;
}

double ServerCmdTurn::angleAtTime(double currentTime) const
{
    if (currentTime >= finishTime())
    {
        return startAngle + turnAngle;
    }
    return startAngle + turnRate * elapsedTime(currentTime);
}

double ServerCmdTurn::angleTurned(double currentTime) const
{
    return turnRate * elapsedTime(currentTime);
}

void ServerCmdTurn::beginCmd(Bot* bot, double currentTime)
{
    bot->newCurrentMotion(currentTime);
    bot->currentMotion->initTurn(currentTime, turnTime, bot->getDir(), bot->getDir() + turnAngle);
    bot->currentMotion->apply(bot->state, currentTime, true);
}

void ServerCmdTurn::updateCmd(Bot* bot, double currentTime)
{
    bot->currentMotion->apply(bot->state, currentTime, true);
}

void ServerCmdTurn::finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos)
{
    bot->completeCurrentMotion(currentTime, interrupted, interruptPos);

    if (!interrupted) {
        bot->turnCompleteEvt(currentTime);
    }
}

ServerCmdFire::ServerCmdFire(double startTime, int startBullets) : ServerBotCmd(startTime), startBullets{startBullets}
{
    timeToFire = startBullets > 0 ? 1.0 : 1.0; //  minCmdTime;
}

double ServerCmdFire::finishTime() const
{
    return startTime() + timeToFire;
}

void ServerCmdFire::beginCmd(Bot* bot, double currentTime)
{
    bot->newCurrentMotion(currentTime);
    bot->currentMotion->initFire(currentTime, timeToFire, startBullets);
    bot->currentMotion->apply(bot->state, currentTime, true);
}

void ServerCmdFire::updateCmd(Bot* bot, double currentTime)
{
    bot->currentMotion->apply(bot->state, currentTime, true);
}

void ServerCmdFire::finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos)
{
    bot->completeCurrentMotion(currentTime, interrupted, interruptPos);

    if (!interrupted) {
        if (startBullets > 0) {
            bot->shotFireEvt(bot, currentTime);
        }
        else {
            bot->noBulletEvt(currentTime);
        }
    }
}

ServerCmdScan::ServerCmdScan(double startTime, double angle) : ServerBotCmd(startTime)
{
    angle = constrain(angle, M_PI/180, M_PI);

    scanAngle = angle;
    scanRate  = 1.0;
    scanTime  = angle / scanRate;

    if (scanTime < minCmdTime) {
        scanRate = scanAngle / minCmdTime;
        scanTime = minCmdTime;
    }
}

double ServerCmdScan::finishTime() const
{
    return startTime() + scanTime;
}


void ServerCmdScan::beginCmd(Bot* bot, double currentTime)
{
    bot->newCurrentMotion(currentTime);
    bot->currentMotion->initScan(currentTime, scanTime, bot->getDir()-scanAngle/2, bot->getDir()+scanAngle/2);
    bot->currentMotion->apply(bot->state, currentTime, true);
}

void ServerCmdScan::updateCmd(Bot* bot, double currentTime)
{
    bot->currentMotion->apply(bot->state, currentTime, true);
}

void ServerCmdScan::finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos)
{
    bot->completeCurrentMotion(currentTime, interrupted, interruptPos);

    if (!interrupted) {
        bot->doScanEvt(currentTime, scanAngle);
    }
}

ServerCmdAsleep::ServerCmdAsleep(double sTime) : ServerBotCmd(sTime)
{
}

void ServerCmdAsleep::beginCmd(Bot* bot, double currentTime)
{
    bot->newCurrentMotion(currentTime);
    bot->currentMotion->initSleep(currentTime);
    bot->currentMotion->apply(bot->state, currentTime, true);
}

void ServerCmdAsleep::updateCmd(Bot* bot, double currentTime)
{
    bot->currentMotion->apply(bot->state, currentTime, true);
}

void ServerCmdAsleep::finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos)
{
    bot->completeCurrentMotion(currentTime, interrupted, interruptPos);

    if (!interrupted) {
        std::cout << "What to do with Asleep here?" << std::endl;
        // bot->doScanEvt(currentTime, scanAngle);
    }
}

double ServerCmdAsleep::finishTime() const
{
    return std::numeric_limits<double>::max();  // never really finishes
}

ServerCmdResign::ServerCmdResign(double sTime) : ServerBotCmd(sTime)
{
    timeToResign = minCmdTime;
}

void ServerCmdResign::beginCmd(Bot *bot, double currentTime)
{
    bot->newCurrentMotion(currentTime);
    bot->currentMotion->initResign(currentTime, timeToResign);
    bot->currentMotion->apply(bot->state, currentTime, true);
}

void ServerCmdResign::updateCmd(Bot *bot, double currentTime)
{
    bot->currentMotion->apply(bot->state, currentTime, true);
}

void ServerCmdResign::finishCmd(std::shared_ptr<Bot> bot, double currentTime, bool interrupted, Vec2d interruptPos)
{
    bot->completeCurrentMotion(currentTime, interrupted, interruptPos);

    bot->resignEvt(currentTime);
}

double ServerCmdResign::finishTime() const
{
    return startTime() + timeToResign;
}

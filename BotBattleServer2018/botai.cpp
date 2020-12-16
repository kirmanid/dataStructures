#define _USE_MATH_DEFINES
#include "botai.h"
#include <iostream>

using namespace std;

ServerBotAI::ServerBotAI()
{
    wasDisconnected = false;
    hasNextCommand = false;
}

ServerBotAI::~ServerBotAI()
{

}

void ServerBotAI::setDisconnected()
{
    //std::cout << "BotAI Set to disconnected" << std::endl;
    wasDisconnected = true;
}

bool ServerBotAI::isDisconnected()
{
    return wasDisconnected;
}

void ServerBotAI::setCmd(BotCmd cmd)
{
    //cout << "Command set: " << (int)cmd << endl;
    hasNextCommand  = true;
    nextCmd = cmd;
}

bool ServerBotAI::getCmd(BotCmd& cmd)
{
    if (!hasNextCommand) {
        return false;
    }

    if (wasDisconnected) {
        cout << "Asked for command when disconnected" << endl;
    }

    cmd = nextCmd;
    hasNextCommand = false;

    return true;
}

void ServerBotAI::handleEvents(BotEvent&  /*event*/, double /*currentTime*/)
{

}

void ServerBotAI::Turn(double angle)
{
    setCmd(BotCmd{BotCmdType::Turn, angle, 0});
}

void ServerBotAI::Move(double speed, double time)
{
    if (time < 0) {
        time = -time;
        speed = -speed;
    }
    setCmd(BotCmd{BotCmdType::Move, speed, time});
}

void ServerBotAI::Fire()
{
    setCmd(BotCmd{BotCmdType::Fire, 0, 0});
}

void ServerBotAI::Ignore()
{
    setCmd(BotCmd{BotCmdType::Ignore, 0, 0});
}

void ServerBotAI::Resign()
{
    setCmd(BotCmd{BotCmdType::Resign, 0, 0});
}

void ServerBotAI::Scan(double fieldOfView)
{
    fieldOfView = abs(fieldOfView);
    setCmd(BotCmd{BotCmdType::Scan, fieldOfView, 0});
}

//void BotAI::ResetCmd()
//{
// setCmd(BotCmdType::NoCommand, 0, 0);
//}

LocalBotAI::LocalBotAI(std::unique_ptr<BotAI> ai) : ai{std::move(ai)}
{
}

void LocalBotAI::handleEvents(BotEvent&  event, double /*currentTime*/)
{
    setCmd(ai->handleEvents(event));
}

void LocalBotAI::setKilled()
{
    ai->setKilled();
}

std::string LocalBotAI::getName()
{
    return ai->getName();
}

int BotAI::botCount = 0;

BotAI::BotAI()
{
    botNum = ++botCount;
    setName("Unnamed");
}

BotAI::~BotAI()
{
    //cout << "BotAI Deleted" << endl;
}

void BotAI::setName(const std::string& name)
{
    this->name = name + " " + to_string(botNum);
}

std::string BotAI::getName()
{
    return name;
}


BotCmd BotAI::Turn(double angle)
{
    return BotCmd{BotCmdType::Turn, angle, 0};
}

BotCmd BotAI::Move(double speed, double time)
{
    return BotCmd{BotCmdType::Move, speed, time};
}

BotCmd BotAI::Fire()
{
    return BotCmd{BotCmdType::Fire, 0, 0};
}

BotCmd BotAI::Scan(double fieldOfView)
{
    return BotCmd{BotCmdType::Scan, fieldOfView, 0};
}

BotCmd BotAI::Ignore()
{
    return BotCmd{BotCmdType::Ignore, 0, 0};
}


#ifndef BOTAI_H
#define BOTAI_H

#define _USE_MATH_DEFINES

#include <vector>

#include <cmath>

#include <mutex>
#include <condition_variable>

#include "botcmd.h"

class ServerBotAI
{
private:
    bool         wasDisconnected;
    bool         hasNextCommand;
    BotCmd       nextCmd;

public:

    ServerBotAI();

    virtual ~ServerBotAI();

    void setCmd(BotCmd cmd);
    bool getCmd(BotCmd& cmd);

    virtual void handleEvents(BotEvent&  event, double currentTime) = 0;
    virtual void setKilled() = 0;
protected:

    void Turn(double angle);
    void Move(double speed, double time = 1000);
    void Fire();
    void Scan(double fieldOfView);
    void Ignore();
    void Resign();

public:
    void setDisconnected();
    bool isDisconnected();

    virtual std::string getName() = 0;
};

class BotAI
{
public:
    std::string name;
    static int botCount;
    int botNum;
public:
    BotAI();
    virtual ~BotAI();
    virtual BotCmd handleEvents(BotEvent& event) = 0;
    virtual void setKilled() = 0;
    virtual std::string getName();
protected:
    void setName(const std::string& name);
    BotCmd Turn(double angle);
    BotCmd Move(double speed, double time = 1000);
    BotCmd Fire();
    BotCmd Scan(double fieldOfView);
    BotCmd Ignore();
};

class LocalBotAI : public ServerBotAI
{
private:
    std::unique_ptr<BotAI> ai;
public:
    LocalBotAI(std::unique_ptr<BotAI> ai);
    virtual void handleEvents(BotEvent&  event, double currentTime) override;
    virtual std::string getName() override;
    virtual void setKilled() override;
};

#endif // BOTAI_H

#ifndef REMOTEAI_H
#define REMOTEAI_H

#define _USE_MATH_DEFINES

#include <sstream>
#include <vector>
#include <functional>

#include "botai.h"
#include "world.h"

class RemoteAI;

class RemoteBotManager {
private:
    std::vector<std::shared_ptr<Bot>> remoteBots;
    std::function<void(int, const std::string&)> sender;
    std::function<void(int)> closer;
public:
    RemoteBotManager(std::function<void(int, const std::string&)> sender, std::function<void(int)> closer);
    void createBot(World& world, int id);
    void removeBot(int id);
    void disconnect(int id);
    void error(int id, const std::string& info);
    void receiveData(int id, const std::string& data);  // handle data received from remote client
    void sendData(int id, const std::string& data);     // send data to remote client ai
};

class RemoteAI: public ServerBotAI
{
private:
    int remoteId; // index within remoteBotManager
    RemoteBotManager& manager;
    std::string botName;
public:

    RemoteAI(int id, RemoteBotManager& botManager);
   ~RemoteAI();

    void handleEvents(BotEvent& event, double currentTime) override;
    void setKilled() override;
    bool handleCommand(std::string cmd);
    std::string getName() override;
};

#endif // REMOTEAI_H

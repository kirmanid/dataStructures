#ifndef BOTMANAGER_H
#define BOTMANAGER_H

#include "connectedbot.h"

class BotManager {
private:
    mssm::Graphics& g;
    int port;
    std::string host;
    std::vector<std::unique_ptr<ConnectedBot>> bots;
public:
    BotManager(mssm::Graphics& g, int port, const std::string& host);
   ~BotManager();
    void addBot(std::unique_ptr<BotAI> brain);
    bool processEvent(const mssm::Event& event);
    int numBots() { return bots.size(); }
};

#endif // BOTMANAGER_H

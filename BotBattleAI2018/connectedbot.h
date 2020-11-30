#ifndef CONNECTEDBOT_H
#define CONNECTEDBOT_H

#include "graphics.h"
//#include "vec2d.h"
#include "bot.h"
#include "networkplugin.h"
#include <memory>

class ConnectedBot {
private:
    mssm::Graphics& g;
    Bot bot;
    NetworkClientPlugin connection;
public:
    ConnectedBot(mssm::Graphics& g, std::unique_ptr<BotAI> brain, int port, const std::string& host);
    ConnectedBot(ConnectedBot&& other);
    bool processEvent(const mssm::Event& event);
    bool isDead() { return connection.wasDisconnected(); }
};


#endif // CONNECTEDBOT_H

#include "connectedbot.h"

using namespace std;
using namespace mssm;

ConnectedBot::ConnectedBot(ConnectedBot&& other)
  : g{other.g}, bot{std::move(other.bot.brain)}, connection{other.connection}
{
}


ConnectedBot::ConnectedBot(Graphics& g, std::unique_ptr<BotAI> brain, int port, const std::string& host)
    : g{g}, bot{std::move(brain)}, connection{g, port, host}
{
}

bool ConnectedBot::processEvent(const Event &event)
{
    NetworkSocketEvent socketEvent;
    string socketData;

    if (connection.handleEvent(event, socketEvent, socketData)) {
        // got a network event of some sort
        switch (socketEvent)
        {
        case NetworkSocketEvent::connected:
            //cout  << "Connected to: " << socketData.c_str() << endl;
            connection.send("Name " + bot.name() + "\n");
            break;
        case NetworkSocketEvent::disconnected:
            //cout  << "Disconnected " << socketData.c_str() << endl;
            //connection.reConnect(1234, "localhost");
            //gameServer.closePlugin();
            break;
        case NetworkSocketEvent::error:
            //cout  << "Network Error " << socketData.c_str() << endl;
            break;
        case NetworkSocketEvent::other:
            //cout  << "Network Other " << socketData.c_str() << endl;
            break;
        case NetworkSocketEvent::data:
            //cout  << "Data Packet1: " << socketData.c_str() << endl;
            // bot has gotten some data from the server... handle it
            bot.handleReceivedData(g, socketData);
            break;
        }

        if (bot.hasOutgoingData()) {
            // cout  << "Bot1 has outgoing data to send" << endl;
            connection.send(bot.getOutgoingData());
        }

        return true;
    }

    return false;
}




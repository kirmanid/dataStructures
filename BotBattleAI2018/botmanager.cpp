#include "botmanager.h"

using namespace std;
using namespace mssm;

BotManager::BotManager(Graphics& g, int port, const std::string& host)
    : g{g}, port{port}, host{host}
{
}

BotManager::~BotManager()
{
}

void BotManager::addBot(std::unique_ptr<BotAI> brain)
{
    bots.push_back(std::make_unique<ConnectedBot>(g, std::move(brain), port, host));
}

bool BotManager::processEvent(const Event& event)
{
    bots.erase(std::remove_if(bots.begin(), bots.end(), [](std::unique_ptr<ConnectedBot>& bot) { return bot->isDead(); }), bots.end());

    switch (event.evtType) {
    case EvtType::PluginClosed:
        //cout  << "BotManager::processEvent: PluginClosed " << event.data.substr(0, 100).c_str() << endl;
        break;
    case EvtType::PluginCreated:
        //cout  << "BotManager::processEvent: PluginCreated " << event.data.substr(0, 100).c_str() << endl;
        break;
    case EvtType::PluginMessage:
        //cout  << "BotManager::processEvent: PluginMessage " << event.data.substr(0, 100).c_str() << endl;
        break;
    default:
        // not a plugin message.. probably a keyboard or mouse, etc
        return false;
    }

    if (bots.empty()) {
        //cout  << "No Bots!!" << endl;
    }

    for (std::unique_ptr<ConnectedBot>& b : bots) {
        if (b->processEvent(event)) {
            return true;
        }
    }

    if (bots.empty()) {
        //cout  << "No Bots processed that message" << endl;
    }

    return false;
}

#include "vec2d.h"
#include "remoteai.h"
#include <sstream>

using namespace std;

string clean(string botName)
{
    for (int i = 0; i < (int)botName.length(); i++) {
        if ((botName[i] >= '0' && botName[i] <= '9') ||
                (botName[i] >= 'a' && botName[i] <= 'z') ||
                (botName[i] >= 'A' && botName[i] <= 'Z') ||
                (botName[i] == ' ' && botName[i] <= '/')) {
            // ok
        }
        else {
            botName[i] = '_';
        }
    }

    if (botName.size() > 20) {
        return botName.substr(0,20);
    }

    return botName;
}

RemoteBotManager::RemoteBotManager(std::function<void(int, const std::string&)> sender,
                                   std::function<void(int)> closer)
    : sender{sender}, closer{closer}
{

}


void RemoteBotManager::createBot(World& world, int remoteId)
{
    if ((int)remoteBots.size() <= remoteId) {
        remoteBots.resize(remoteId+1, nullptr);
    }
    remoteBots[remoteId] = make_shared<Bot>(&world, new RemoteAI(remoteId, *this), world.getNextBotId(), false);
    world.addBot(remoteBots[remoteId], world.remoteBotHealth, world.remoteBotBullets);
}

void RemoteBotManager::removeBot(int remoteId)  // called by world upon death of bot
{
    cout << "RemoteBotManager: remoteBot being removed from manager: " << remoteId << endl;
    closer(remoteId);
    disconnect(remoteId);
}

void RemoteBotManager::disconnect(int remoteId) // called by communications service if remote ai disconnected
{
    // cout << "RemoteBotManager: Client disconnected: " << remoteId << endl;
    if (remoteId < (int)remoteBots.size() && remoteBots[remoteId] != nullptr) {
        remoteBots[remoteId]->setDisconnected();
        remoteBots[remoteId].reset();
    }
}

void RemoteBotManager::error(int /*id*/, const std::string& /*info*/) // called by communications service
{
    //cout << "RemoteBotManager: Comm error for client: " << id << " " << info << endl;
}

void RemoteBotManager::receiveData(int remoteId, const std::string& data)  // handle data received by communications service from remote client
{
    //cout << "RemoteBotManager: Got some data from client: " << remoteId << " " << data << endl;
    if (remoteId < (int)remoteBots.size() && remoteBots[remoteId] != nullptr) {
        static_cast<RemoteAI*>(remoteBots[remoteId]->getAI())->handleCommand(data);
    }
    else {
        cout << "NOBODY TO RECEIVE IT!!" << endl;
    }
}

void RemoteBotManager::sendData(int remoteId, const std::string& data)     // send data to remote client ai
{
    //cout << "RemoteBotManager: Sending data to client " << remoteId << ": '" << data << "'" << endl;
    sender(remoteId, data);
}

RemoteAI::RemoteAI(int remoteId, RemoteBotManager& botManager)
    : remoteId{remoteId}, manager{botManager}
{
}

RemoteAI::~RemoteAI()
{
    cout << "REMOTE AI DESTRUCTOR" << endl;
    //manager.removeBot(remoteId);
}

void RemoteAI::handleEvents(BotEvent& event, double /*currentTime*/)
{
    std::stringstream ss;
    event.write(ss);
    manager.sendData(remoteId, ss.str());
}

void RemoteAI::setKilled()
{
    manager.removeBot(remoteId);
}

std::string RemoteAI::getName()
{
    return botName;
}

bool RemoteAI::handleCommand(std::string cmd)
{
    //std::cout << "Remote command received for bot: " << botName << " Command is: " << cmd << std::endl;

    if (cmd.size() < 1)
    {
        return false;
    }

    std::stringstream ss(cmd);

    std::string cmdName;

    double arg1 = 0;
    double arg2 = 0;

    ss >> cmdName;

    switch (cmd[0])
    {
    case 'F': // fire
        Fire();
        return true;
    case 'M': // move
        ss >> arg1;
        ss >> arg2;
        Move(arg1, arg2);
        return true;
    case 'T': // turn
        ss >> arg1;
        Turn(arg1);
        return true;
    case 'S': // scan
        ss >> arg1;
        Scan(arg1);
        return true;
    case 'I': // ignore
        Ignore();
        return true;
    case 'R': // resign
        // kill this bot
        Resign();
        return false;
    case 'N':
       // cout << "Just setting the bot name: " << endl;
        ss >> botName;
        botName = clean(botName);
        ss.ignore(10000,'\n');
        cmd = std::string(std::istreambuf_iterator<char>(ss), {});
        if (cmd.size() > 0) {
            //cout << "Got another command?: " << cmd << endl;
            return handleCommand(cmd);
        }
        return false;
    default:
        return false;
    }
}

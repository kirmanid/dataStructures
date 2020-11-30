#include <iostream>
#include <sstream>
#include "bot.h"

using namespace std;

Bot::Bot(std::unique_ptr<BotAI> brain)
    : brain{std::move(brain)}
{
}

Bot::~Bot()
{
}

string formatEvent(std::string name, const BotEvent& event)
{
    stringstream ss;
    ss << name << ": ";
    event.write(ss, false);
    return ss.str();
}

string formatCmd(std::string name, BotCmd cmd)
{
    std::stringstream ss;
    ss << name << ": ";
    cmd.write(ss, false);
    return ss.str();
}

BotCmd Bot::handleEvents(mssm::Graphics& g, BotEvent& evt)
{
    brain->logEvent(g, formatEvent(brain->getName(), evt));
    brain->resetCommandIssued();
    BotCmd cmd = brain->handleEvents(g, evt);
    brain->logCommand(g, formatCmd(brain->getName(), cmd));
    return cmd;
}


void Bot::handleReceivedData(mssm::Graphics& g, const std::string& incoming)
{
    receivedData.append(incoming);

    auto idx = receivedData.find_first_of('\n');

    if (idx != string::npos) {
        string commandString{receivedData.substr(0, idx)};
        receivedData = receivedData.substr(idx+1);
        //cout << "Received: '" << commandString << endl;

        std::stringstream evtStream(commandString);
        event = BotEvent::read(evtStream);

        BotCmd nextCommand = handleEvents(g, event);

        std::stringstream cmdStream;
        nextCommand.write(cmdStream, true);
        outgoingCommand = cmdStream.str();
        //cout << "Outgoing: ";
        //cout << outgoingCommand << endl;
    }
}

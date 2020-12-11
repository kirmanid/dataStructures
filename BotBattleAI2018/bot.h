#ifndef BOT_H
#define BOT_H

#include <vector>
#include <memory>
#include "botcmd.h"
#include "botai.h"

namespace mssm {
class Graphics;
}

class Bot
{
private:

    BotEvent     event{BotEventType::PowerUp, 0, 5, 5};
    std::string  receivedData;
    std::string  outgoingCommand;
public:
    std::unique_ptr<BotAI> brain;

public:

    Bot(std::unique_ptr<BotAI> brain);

    virtual ~Bot();

    void handleReceivedData(mssm::Graphics& g, const std::string& str);
    std::string getOutgoingData() { std::string tmp = std::move(outgoingCommand); outgoingCommand.clear(); return tmp; }
    bool hasOutgoingData() { return outgoingCommand.size(); }

    std::string name() { return brain->getName(); }

    bool isDead() { return brain->isDead(); }

protected:

    BotCmd handleEvents(mssm::Graphics& g, BotEvent& event);

//    BotCmd Turn(double angle);
//    BotCmd Move(double speed, double time = 1000);
//    BotCmd Fire();
//    BotCmd Scan(double fieldOfView);
};

#endif // BOT_H

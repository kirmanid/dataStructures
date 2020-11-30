#ifndef BOTAI_H
#define BOTAI_H

#include "botcmd.h"
#include <vector>

namespace mssm {
class Graphics;
}

class BotAI
{
private:
    std::string name;
    static int  botCount;
    int         botNum;
    bool        commandIssued{false};
public:
    BotAI();
    virtual ~BotAI();
    virtual BotCmd handleEvents(mssm::Graphics& g, BotEvent& event) = 0;
    virtual std::string getName();
    void resetCommandIssued() { commandIssued = false; }
    virtual void logEvent(mssm::Graphics& g, std::string event) = 0;
    virtual void logCommand(mssm::Graphics& g, std::string command) = 0;
protected:
    void checkCommandIssued();
    void setName(const std::string& name);
    BotCmd Turn(double angle);
    BotCmd MoveForward(double timeInSeconds);
    BotCmd MoveBackward(double timeInSeconds);
    BotCmd Fire();
    BotCmd Scan(double fieldOfView);
    BotCmd Ignore();
};

#endif // BOTAI_H

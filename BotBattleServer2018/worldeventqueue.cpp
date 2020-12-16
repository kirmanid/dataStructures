#include "worldeventqueue.h"
#include "world.h"

#include <algorithm>
#include <iostream>
#include <memory>

#ifndef NOGRAPHICS
#include "graphics.h"
#endif

#undef assert


using namespace std;

template <class T, class Comp, class Alloc, typename Predicate>
void discard_if(std::set<T, Comp, Alloc>& c, Predicate pred) {
    for (auto it{c.begin()}, end{c.end()}; it != end; ) {
        if (pred(*it)) {
            it = c.erase(it);
        }
        else {
            ++it;
        }
    }
}

WorldEvent::WorldEvent(double time) : time(time)
{
    if (time < 0)
    {
        cout << "Negative time?!" << endl;
    }
}

WorldEvent::~WorldEvent()
{

}

#ifndef NOGRAPHICS
void WorldEvent::draw(mssm::Graphics& /*g*/, bool /*isFirstEvent*/) const
{
}

void BotHitFoodEvt::draw(mssm::Graphics& /*g*/, bool /*isFirstEvent*/) const
{
//    g.line(bot->getPos(), botPos, mssm::Color(255,255,0,20));

//    if (isFirstEvent || (bot->hasCommand() && (eventTime() < bot->commandFinishTime())))
//    {
//        g.ellipseC(food->getPos(), 3, 3, mssm::YELLOW);
//    }
//    else
//    {
//        g.ellipseC(food->getPos(), 3, 3, mssm::RED);
//    }
}

void BulletHitWallEvt::draw(mssm::Graphics& /*g*/, bool /*isFirstEvent*/) const
{
    //g.line(bullet->state.getPosition(), pos, mssm::Color(0,255,255,20));
}


void BulletHitBotEvt::draw(mssm::Graphics& g, bool isFirstEvent) const
{
    g.line(bot->getPos(), botPos, mssm::Color(255,255,0,20));
    g.line(bullet->state.getPosition(), bulletPos, mssm::Color(255,255,0,20));

    if (isFirstEvent || (bot->hasCommand() && (eventTime() < bot->commandFinishTime())))
    {
        g.ellipseC(bot->getPos(), bot->radius()*2.2, bot->radius()*2.2, mssm::YELLOW);
        g.ellipseC(bullet->state.getPosition(), 4, 4, mssm::YELLOW, mssm::YELLOW);
    }
    else
    {
        g.ellipseC(bot->getPos(), bot->radius()*2.2, bot->radius()*2.2, mssm::RED);
        g.ellipseC(bullet->state.getPosition(), 3, 3, mssm::RED);
    }
}

void WorldEventQueue::draw(mssm::Graphics& g) const
{
    bool isFirstEvent = true;
    for (auto const& e : pqueue) {
        e->draw(g, isFirstEvent);
        isFirstEvent = false;
    }
}

#endif

bool WorldEvent::involvesFood(Food* /*food*/) const
{
    return false;
}

bool WorldEvent::involvesBot(Bot* /*bot*/) const
{
    return false;
}

bool WorldEvent::involvesBullet(Bullet* /*bullet*/) const
{
    return false;
}

BulletHitWallEvt::BulletHitWallEvt(double time, std::shared_ptr<Bullet> bullet, Vec2d pos)
    : WorldEvent(time), bullet{std::move(bullet)}, pos{pos}
{
}

std::string BulletHitWallEvt::desc() const
{
    return "A bullet hit a wall at " + to_string(time);
}

bool BulletHitWallEvt::involvesBullet(Bullet* b) const
{
    return b == bullet.get();
}

void BulletHitWallEvt::execute(World& world)
{
    world.assert(time == world.currentTime(), "Time Out Of Sync");
    bullet->finish(pos, time);
    world.bulletHitWallEvt(bullet.get());
}



BotHitWallEvt::BotHitWallEvt(double time, std::shared_ptr<Bot>  bot, Vec2d pos, double collisionAngle)
    : WorldEvent(time), bot(bot), pos(pos), collisionAngle(collisionAngle)
{

}

std::string BotHitWallEvt::desc() const
{
    return bot->name() + " hit a wall at " + to_string(time);
}

bool BotHitWallEvt::involvesBot(Bot* bot) const
{
    bool involved = bot == this->bot.get();
    if (involved) {
        //cout << "Wall Hit Event removed for bot: " << bot->name() << endl;
    }
    return involved;
}

void BotHitWallEvt::execute(World& world)
{
    world.assert(time == world.currentTime(), "Time Out Of Sync");
    world.botHitWallEvt(bot, pos, collisionAngle);
}

BotCmdFinishedEvt::BotCmdFinishedEvt(double time, const std::string& cmdName, std::shared_ptr<Bot>  bot)
    : WorldEvent(time), bot(bot), cmdName(cmdName)
{

}

std::string BotCmdFinishedEvt::desc() const
{
    if (cmdName == "CommWait") {
        return "";
    }
    return bot->name() + " finished its last command (" + cmdName + ") at " + to_string(time);
}


bool BotCmdFinishedEvt::involvesBot(Bot* bot) const
{
    bool involved = bot == this->bot.get();
    if (involved) {
        //cout << "Command finished event removed for bot: " << bot->name() << endl;
    }
    return involved;

}

void BotCmdFinishedEvt::execute(World& world)
{
    world.botCmdFinishedEvt(bot);
}

BulletHitBotEvt::BulletHitBotEvt(double time, std::shared_ptr<Bullet> bullet, std::shared_ptr<Bot>  bot, Vec2d p1, Vec2d p2)
    : WorldEvent(time), bullet{std::move(bullet)}, bot{std::move(bot)}, botPos{p1}, bulletPos{p2}
{

}


std::string BulletHitBotEvt::desc() const
{
    return bot->name() + " was hit by a bullet at " + to_string(time);
}


bool BulletHitBotEvt::involvesBot(Bot* bot) const
{
    bool involved = bot == this->bot.get();
    if (involved) {
        //cout << "Bullet Hit Event removed for bot: " << bot->name() << endl;
    }
    return involved;
}

bool BulletHitBotEvt::involvesBullet(Bullet* bullet) const
{
    return bullet == this->bullet.get();
}

void BulletHitBotEvt::execute(World& world)
{
    world.assert(time == world.currentTime(), "Time Out Of Sync");
    bullet->finish(bulletPos, time);
    world.bulletHitBotEvt(bullet.get(), bot);
}

BotHitFoodEvt::BotHitFoodEvt(double time, std::shared_ptr<Bot> bot, std::shared_ptr<Food> food, Vec2d p1, double angle)
    : WorldEvent(time), food{std::move(food)}, bot{std::move(bot)}, botPos{p1}, angle{angle}
{

}


std::string BotHitFoodEvt::desc() const
{
    return bot->name() + " hit food at " + to_string(time);
}

bool BotHitFoodEvt::involvesFood(Food* food) const
{
    bool involved = food == this->food.get();
    if (involved) {
        //cout << "Bullet Hit Event removed for bot: " << bot->name() << endl;
    }
    return involved;
}

bool BotHitFoodEvt::involvesBot(Bot* bot) const
{
    bool involved = bot == this->bot.get();
    if (involved) {
        //cout << "Bullet Hit Event removed for bot: " << bot->name() << endl;
    }
    return involved;
}

void BotHitFoodEvt::execute(World& world)
{
    world.assert(time == world.currentTime(), "Time Out Of Sync");
    food->finish(time);
    world.botHitFoodEvt(bot, food, botPos, angle);
}


BotHitBotEvt::BotHitBotEvt(double time, std::shared_ptr<Bot>  bot1, std::shared_ptr<Bot>  bot2, Vec2d pos1, Vec2d pos2, double angle1, double angle2)
    : WorldEvent(time), bot1{std::move(bot1)}, bot2{std::move(bot2)}, pos1(pos1), pos2(pos2), angle1(angle1), angle2(angle2)
{

}

std::string BotHitBotEvt::desc() const
{
    return bot1->name() + " and " + bot2->name() + " collided at " + to_string(time);
}


bool BotHitBotEvt::involvesBot(Bot* bot) const
{
    bool involved = bot == this->bot1.get() || bot == this->bot2.get();
    if (involved) {
        //cout << "Bot Hit Bot Event removed for bot: " << bot->name() << endl;
    }
    return involved;

}

void BotHitBotEvt::execute(World& world)
{
    world.assert(time == world.currentTime(), "Time Out Of Sync");
    world.botHitBotEvt(bot1, bot2, pos1, pos2, angle1, angle2);
}



WorldEventQueue::WorldEventQueue(World* world)
    : world(world)
{

}



void WorldEventQueue::dump()
{
    cout << "{\n";
    for (auto& e : pqueue)
    {
        cout << "  " << e->desc() << endl;
    }
    cout << "}" << endl;
}

bool evtComp(const std::unique_ptr<WorldEvent>& e1, const std::unique_ptr<WorldEvent>& e2)
{
    return e1->eventTime() > e2->eventTime();
}

void WorldEventQueue::add(std::unique_ptr<WorldEvent> evt)
{
    pqueue.push_back(std::move(evt));
    std::push_heap(pqueue.begin(), pqueue.end(), evtComp);
}


void WorldEventQueue::addBulletHitWallEvt(double time, std::shared_ptr<Bullet> bullet, Vec2d pos)
{
    add(make_unique<BulletHitWallEvt>(time, bullet, pos));
}

void WorldEventQueue::addBotHitWallEvt(double time, shared_ptr<Bot> bot, Vec2d pos, double collisionAngle)
{
    add(make_unique<BotHitWallEvt>(time, std::move(bot), pos, collisionAngle));
}

void WorldEventQueue::addBotCmdFinishedEvt(double time, const std::string& cmdName, shared_ptr<Bot> bot)
{
    add(make_unique<BotCmdFinishedEvt>(time, cmdName, std::move(bot)));
}

void WorldEventQueue::addBulletHitBotEvt(double time, std::shared_ptr<Bullet> bullet, shared_ptr<Bot> bot, Vec2d botPos, Vec2d bulletPos)
{
    add(make_unique<BulletHitBotEvt>(time, std::move(bullet), std::move(bot), botPos, bulletPos));
}

void WorldEventQueue::addBotHitBotEvt(double time, shared_ptr<Bot> bot1, shared_ptr<Bot> bot2, Vec2d pos1, Vec2d pos2, double absoluteAngle1, double absoluteAngle2)
{
    add(make_unique<BotHitBotEvt>(time, std::move(bot1), std::move(bot2), pos1, pos2, absoluteAngle1, absoluteAngle2));
}

void WorldEventQueue::addBotHitFoodEvt(double time, std::shared_ptr<Bot> bot, std::shared_ptr<Food> food, Vec2d botPos, double angle)
{
    add(make_unique<BotHitFoodEvt>(time, std::move(bot), std::move(food), botPos, angle));
}

void WorldEventQueue::clearAllInvolved(Food* food)
{
    pqueue.erase(std::remove_if(pqueue.begin(), pqueue.end(), [food](const std::unique_ptr<WorldEvent>& evt) { return evt->involvesFood(food); }), pqueue.end());
    std::make_heap(pqueue.begin(), pqueue.end(), evtComp);
}

void WorldEventQueue::clearAllInvolved(Bot* bot)
{
    pqueue.erase(std::remove_if(pqueue.begin(), pqueue.end(), [bot](const std::unique_ptr<WorldEvent>& evt) { return evt->involvesBot(bot); }), pqueue.end());
    std::make_heap(pqueue.begin(), pqueue.end(), evtComp);
}

void WorldEventQueue::clearAllInvolved(Bullet* bullet)
{
    pqueue.erase(std::remove_if(pqueue.begin(), pqueue.end(), [bullet](const std::unique_ptr<WorldEvent>& evt) { return evt->involvesBullet(bullet); }), pqueue.end());
    std::make_heap(pqueue.begin(), pqueue.end(), evtComp);
}

std::unique_ptr<WorldEvent> WorldEventQueue::pop()
{
    std::unique_ptr<WorldEvent> evt(std::move(pqueue[0]));
    std::pop_heap(pqueue.begin(), pqueue.end(), evtComp);
    pqueue.pop_back();
    return evt;
}

// will return null if
//     1. event queue is empty
//     2. there are no events within the "timeToUse"
//
// TODO in single step mode, if timeToUse < timeUntilEvent should we step? (currently we do)

unique_ptr<WorldEvent> WorldEventQueue::getNextEventToExecute(double& gameTime, double& timeToUse, double& elapsedTime, bool singleStepMode)
{  
    if (pqueue.empty()) {
        cout << "No events in queue at time: " << gameTime <<  endl;
        elapsedTime = 0;
        return unique_ptr<WorldEvent>{};
    }

    double nextEventTime = topEvent()->eventTime();

    double timeUntilEvent = nextEventTime - gameTime;

    if (timeUntilEvent < 0)
    {
        cout <<"game clock is ahead of event time: Really bad!!!\n"<<endl;
    }

    if (singleStepMode) {
        timeToUse = timeUntilEvent;
    }

    if (timeToUse >= timeUntilEvent)
    {
        elapsedTime = timeUntilEvent;
        gameTime = nextEventTime;  // advance the clock to this event's time
        timeToUse -= timeUntilEvent;

        return pop();
    }

    // cout << "Nothing ready to execute: " << gameTime << endl;

    elapsedTime = timeToUse;
    gameTime += elapsedTime; // advance the clock
    timeToUse = 0;

    return unique_ptr<WorldEvent>{};
}


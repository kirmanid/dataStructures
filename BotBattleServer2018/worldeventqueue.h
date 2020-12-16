#ifndef WORLDEVENTQUEUE_H
#define WORLDEVENTQUEUE_H

#include <memory>
#include <vector>
#include <queue>
#include <set>
#include "vec2d.h"

class Bot;
class World;
class Bullet;
class Food;

namespace mssm {
    class Graphics;
}

class WorldEvent
{
protected:
    double  time;
public:
    WorldEvent(double time);
    virtual ~WorldEvent();
    double eventTime() const { return time; }
    virtual bool involvesFood(Food* food) const;
    virtual bool involvesBot(Bot* bot) const;
    virtual bool involvesBullet(Bullet* bullet) const;
    virtual void execute(World& world) = 0;
    virtual std::string desc() const = 0;
    bool operator<(const WorldEvent& other) const { return time < other.time; }
#ifndef NOGRAPHICS
    virtual void draw(mssm::Graphics& g, bool isNextEvent) const;
#endif
};

class BulletHitWallEvt : public WorldEvent
{
private:
    std::shared_ptr<Bullet> bullet;
    Vec2d pos;
public:
    BulletHitWallEvt(double time, std::shared_ptr<Bullet> bullet, Vec2d pos);
    virtual bool involvesBullet(Bullet* bullet) const override;
    virtual void execute(World& world) override;
    std::string desc() const override;
#ifndef NOGRAPHICS
    virtual void draw(mssm::Graphics& g, bool isNextEvent) const override;
#endif
};

class BotHitWallEvt : public WorldEvent
{
private:
    std::shared_ptr<Bot> bot;
    Vec2d  pos;
    double collisionAngle;
public:
    BotHitWallEvt(double time, std::shared_ptr<Bot> bot, Vec2d pos, double collisionAngle);
    virtual bool involvesBot(Bot* bot) const override;
    virtual void execute(World& world) override;
    std::string desc() const override;
};

class BotCmdFinishedEvt : public WorldEvent
{
private:
    std::shared_ptr<Bot> bot;
    std::string cmdName;
public:
    BotCmdFinishedEvt(double time, const std::string& cmdName, std::shared_ptr<Bot> bot);
    virtual bool involvesBot(Bot* bot) const override;
    virtual void execute(World& world) override;
    std::string desc() const override;
};

class BulletHitBotEvt : public WorldEvent
{
private:
    std::shared_ptr<Bullet> bullet;
    std::shared_ptr<Bot>    bot;
    Vec2d botPos;
    Vec2d bulletPos;
public:
    BulletHitBotEvt(double time, std::shared_ptr<Bullet> bullet, std::shared_ptr<Bot> bot, Vec2d botPos, Vec2d bulletPos);
    virtual bool involvesBot(Bot* bot) const override;
    virtual bool involvesBullet(Bullet* bullet) const override;
    virtual void execute(World& world) override;
    std::string desc() const override;
#ifndef NOGRAPHICS
    void draw(mssm::Graphics& g, bool isNextEvent) const override ;
#endif
};

class BotHitFoodEvt : public WorldEvent
{
private:
    std::shared_ptr<Food> food;
    std::shared_ptr<Bot>  bot;
    Vec2d botPos;
    double angle;
public:
    BotHitFoodEvt(double time, std::shared_ptr<Bot> bot, std::shared_ptr<Food> food, Vec2d botPos, double angle);
    virtual bool involvesFood(Food* food) const override;
    virtual bool involvesBot(Bot* bot) const override;
    virtual void execute(World& world) override;
    std::string desc() const override;
#ifndef NOGRAPHICS
    void draw(mssm::Graphics& g, bool isNextEvent) const override ;
#endif
};



class BotHitBotEvt : public WorldEvent
{
private:
    std::shared_ptr<Bot>   bot1;
    std::shared_ptr<Bot>   bot2;
    Vec2d  pos1;
    Vec2d  pos2;
    double angle1;
    double angle2;
public:
    BotHitBotEvt(double time, std::shared_ptr<Bot> bot1, std::shared_ptr<Bot> bot2, Vec2d pos1, Vec2d pos2, double angle1, double angle2);
    virtual bool involvesBot(Bot* bot) const override;
    virtual void execute(World& world) override;
    std::string desc() const override;
};

class WorldEventQueue
{
private:
    World* world;
    std::vector<std::unique_ptr<WorldEvent>> pqueue;  // priority queue (uses  make_heap, push_heap, pop_heap
public:
    WorldEventQueue(World* world);

    void addBulletHitWallEvt(double time, std::shared_ptr<Bullet> bullet, Vec2d pos);
    void addBotHitWallEvt(double time, std::shared_ptr<Bot> bot, Vec2d pos, double collisionAngle);
    void addBotCmdFinishedEvt(double time, const std::string& cmdName, std::shared_ptr<Bot> bot);
    void addBulletHitBotEvt(double time, std::shared_ptr<Bullet> bullet, std::shared_ptr<Bot> bot, Vec2d botPos, Vec2d bulletPos);
    void addBotHitBotEvt(double time, std::shared_ptr<Bot> bot1, std::shared_ptr<Bot> bot2, Vec2d pos1, Vec2d pos2, double angle1, double angle2);
    void addBotHitFoodEvt(double time, std::shared_ptr<Bot> bot, std::shared_ptr<Food> food, Vec2d pos, double angle);

    void clearAllInvolved(Food* food);
    void clearAllInvolved(Bot* bot);
    void clearAllInvolved(Bullet* bullet);

    std::unique_ptr<WorldEvent> getNextEventToExecute(double& gameTime, double& timeToUse, double& elapsedTime, bool singleStepMode);

    int size() { return pqueue.size(); }

    void dump();

#ifndef NOGRAPHICS
    void draw(mssm::Graphics& g) const;
#endif

private:
    void add(std::unique_ptr<WorldEvent> evt);

    WorldEvent* topEvent() { return pqueue[0].get(); }

    std::unique_ptr<WorldEvent> pop();
};

#endif // WORLDEVENTQUEUE_H

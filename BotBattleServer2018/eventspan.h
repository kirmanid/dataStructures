#ifndef EVENTSPAN_H
#define EVENTSPAN_H

#include "vec2d.h"
#include <vector>

// trace/history of everything that happens in the world, kind of a recording
// that allows reconstruction of world at any point in time, and that supports
// playing forward or backwards. Contains all necessary information to display
// the world:  Bot birth, death, motion, executed commands.  Bullet birth, death, collisions
// etc
// some events are instantaneous (birth, death, collision)
// some take place over a time span, and may involve continous change/interpolation of some
// state (bot position, direction, bullet position, etc
//
// examples:
//  bot created:
//     time, name/id, position, direction
//  bot moving
//     start time, end time, end position
//  bot turning
//     start time, end time, end direction
//  bot scanning
//     start time, end time, scan angle
//  bot firing
//     start time, end time    (maybe break this up to gun warmup, gun fire, gun cooldown?
//  bot death
//     time
//  bullet created
//     id, time, position, direction, shooter
//  bullet bullet deleted
//     id, time
//  OR bullet moving, which could include create/delete
//     id, start time, end time, start position, end position

/*
enum class EventSpanType
{
    BulletPath,
    BotMove,
    BotTurn,
    BotScan,
    BotFire,
};

enum class ActorType
{
    Bot,
    Bullet,
    // food?
};

class Actor
{
public:
    ActorType type;
    int   id;
    int   color;
    Vec2d position;
    Vec2d direction;
};

class EventSpan
{
protected:
    EventSpanType type;
    int eventSpanId;
    int objectId;
    double startTime;
    double endTime;
    Vec2d startPos;
    Vec2d endPos;
    double startDir;
    double endDir;
public:
    EventSpan();
};

class Timeline
{
    std::vector<std::unique<Actor>> actors;
    std::vector<std::unique<EventSpan>> events;
};
*/

#endif // EVENTSPAN_H

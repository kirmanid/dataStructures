#ifndef BOTCMD
#define BOTCMD

#include <iostream>
#include <vector>

enum class BotCmdType
{
    Turn,
    Move,
    Fire,
    Scan,
    Ignore,
    Resign,
};

class BotCmd
{
public:
    BotCmdType cmd;
    double arg1;
    double arg2;
public:
    void write(std::ostream& strm, bool includeCR = true);
};

enum class BotEventType
{
    TurnComplete,
    MoveComplete,
    ScanComplete,
    FireComplete,
    NoBullets,
    MoveBlockedByWall,
    MoveBlockedByBot,
    HitByBot,           // ignorable
    HitByBullet,        // ignorable
    PowerUp,            // ignorable
};

class BotEvent
{
public:
    BotEventType eventType;
    double       eventTime;
    int          health;
    int          bulletCount;
    double       collisionAngle;  // angle with respect to the direction the bot is facing
    double       travelDistance;
    double       angleTurned;
    std::vector<int> scanData;

public:
    BotEvent(BotEventType eventType, double eventTime, int health, int bulletCount);
    void write(std::ostream& strm, bool includeCR = true) const;
    static BotEvent read(std::istream& strm);  // note!  negative event time indicates an error reading from the stream!!
private:
    void write(std::ostream& strm, const std::string& typeName, bool includeCollisionAngle, bool includeDistance, bool includeAngleTurned, bool includeScanData) const;
    void read(std::istream& strm, bool includeCollisionAngle, bool includeDistance, bool includeAngleTurned, bool includeScanData);
};

#endif // BOTCMD


#include "botcmd.h"

using namespace std;

void BotCmd::write(ostream &strm, bool includeCR)
{
    switch (cmd)
    {
    case BotCmdType::Fire:
        strm << "Fire";
        break;
    case BotCmdType::Scan:
        strm << "Scan " << arg1;
        break;
    case BotCmdType::Turn:
        strm << "Turn " << arg1;
        break;
    case BotCmdType::Move:
        strm << "Move " << arg1 << " " << arg2;
        break;
    case BotCmdType::Ignore:
        strm << "Ignore";
        break;
    case BotCmdType::Resign:
        strm << "Resign";
        break;
    }

    if (includeCR) {
        strm << "\n";
    }
}

int scanCharToBotId(char c) {
    switch (c) {
    case '_':
        return 0;
    case '-':
        return -1;
    case '+':
        return 99;
    }

    if (c > '0' && c <= '9') {
        return c - '0';   // 0-9 map to '0' to '9'
    }

    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10; // 10-35 map to 'a' to 'z'
    }

    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 36; // 36-61 map to 'A' to 'Z'
    }

    cout << "Unrecognized scan char!" << endl;

    return 100;
}

std::vector<int> readScanVector(std::istream& strm)
{
    std::vector<int> view;

    int sz;

    strm >> sz;

    if (!strm || sz < 0 || sz > 1000) {
        cout << "ERROR READING SCAN VECTOR" << endl;
        view.resize(500);
        return view;
    }

    view.resize(sz, 0);

    for (int i=0;i<sz;i++)
    {
        char c;
        strm >> c;
        view[i] = scanCharToBotId(c);
    }

    return view;
}

char botIdToScanChar(int id) {
    if (id < 0) {
        return '-';       // all negative numbers map to -
    }
    if (id >= 0 && id <= 9) {
        return '0' + id;  // 0-9 map to '0' to '9'
    }
    id -= 10;
    if (id < 26) {
        return 'a' + id;  // 10-35 map to 'a' to 'z'
    }
    id -= 26;
    if (id < 26) {
        return 'A' + id;  // 36-61 map to 'A' to 'Z'
    }
    return '+';           // > 61  map to +
}

void writeScanVector(const vector<int>& view, std::ostream& strm)
{
    strm << (int)view.size() << " ";
    for (size_t i=0;i<view.size();i++)
    {
        if (view[i] == 0)
        {
            strm << "_";
        }
        else
        {
            strm << botIdToScanChar(view[i]);
        }
    }
}

BotEvent::BotEvent(BotEventType eventType, double eventTime, int health, int bulletCount) :
    eventType{eventType},
    eventTime{eventTime},
    health{health},
    bulletCount{bulletCount},
    collisionAngle{0.0},
    travelDistance{0.0},
    angleTurned{0.0}
{
}

void BotEvent::write(std::ostream& strm, bool includeCR) const
{
    switch (eventType)
    {
    case BotEventType::TurnComplete:
        write(strm, "Turned", false, false, true, false);
        break;
    case BotEventType::MoveComplete:
        write(strm, "Moved", false, true, false, false);
        break;
    case BotEventType::FireComplete:
        write(strm, "Fired", false, false, false, false);
        break;
    case BotEventType::NoBullets:
        write(strm, "Click", false, false, false, false);
        break;
    case BotEventType::MoveBlockedByWall:
        write(strm, "HitWall", true, true, false, false);
        break;
    case BotEventType::MoveBlockedByBot:
        write(strm, "HitBot", true, true, false, false);
        break;
    case BotEventType::ScanComplete:
        write(strm, "Scanned", false, false, false, true);
        break;
    case BotEventType::HitByBot:
        write(strm, "HitByBot", true, true, true, false);
        break;
    case BotEventType::HitByBullet:
        write(strm, "Shot", true, true, true, false);
        break;
    case BotEventType::PowerUp:
        write(strm, "Food", false, true, true, false);
        break;
    }

    if (includeCR) {
        strm << "\r\n";
    }
}

void BotEvent::write(std::ostream& strm, const std::string& typeName, bool includeCollisionAngle, bool includeDistance, bool includeAngleTurned, bool includeScanData) const
{
    strm << typeName << " " << eventTime << " " << health << " " << bulletCount;
    if (includeCollisionAngle) {
        strm << " " << collisionAngle;
    }
    if (includeDistance) {
        strm << " " << travelDistance;
    }
    if (includeAngleTurned) {
        strm << " " << angleTurned;
    }
    if (includeScanData) {
        strm << " ";
        writeScanVector(scanData, strm);
    }
}

void BotEvent::read(std::istream& strm, bool includeCollisionAngle, bool includeDistance, bool includeAngleTurned, bool includeScanData)
{
    if (includeCollisionAngle) {
        strm >> collisionAngle;
    }
    if (includeDistance) {
        strm >> travelDistance;
    }
    if (includeAngleTurned) {
        strm >> angleTurned;
    }
    if (includeScanData) {
        scanData = readScanVector(strm);
    }
}


BotEvent BotEvent::read(std::istream& strm)
{
    std::string et;
    double eventTime;
    int health;
    int bulletCount;

    strm >> et;
    strm >> eventTime;
    strm >> health;
    strm >> bulletCount;

    BotEventType evtType;

    if (et == "Turned")       { evtType = BotEventType::TurnComplete; }
    else if (et == "Moved")   { evtType = BotEventType::MoveComplete; }
    else if (et == "Fired")   { evtType = BotEventType::FireComplete; }
    else if (et == "Click")   { evtType = BotEventType::NoBullets; }
    else if (et == "HitWall") { evtType = BotEventType::MoveBlockedByWall; }
    else if (et == "Scanned") { evtType = BotEventType::ScanComplete; }
    else if (et == "HitBot")  { evtType = BotEventType::MoveBlockedByBot; }
    else if (et == "HitByBot"){ evtType = BotEventType::HitByBot; }
    else if (et == "Shot")    { evtType = BotEventType::HitByBullet; }
    else if (et == "Food")    { evtType = BotEventType::PowerUp; }
    else {
        // invalid bot event received... what should we do about that?
        cout << "ERROR: Unable to parse event: " << et << endl;
        return BotEvent{BotEventType::PowerUp, -1, 0, 0};
    }

    BotEvent evt{evtType, eventTime, health, bulletCount};

    switch (evt.eventType)
    {
    case BotEventType::TurnComplete:
        evt.read(strm, false, false, true, false);
        break;
    case BotEventType::MoveComplete:
        evt.read(strm, false, true, false, false);
        break;
    case BotEventType::FireComplete:
    case BotEventType::NoBullets:
        evt.read(strm, false, false, false, false);
        break;
    case BotEventType::MoveBlockedByWall:
        evt.read(strm, true, true, false, false);
        break;
    case BotEventType::MoveBlockedByBot:
        evt.read(strm, true, true, false, false);
        break;
    case BotEventType::ScanComplete:
        evt.read(strm, false, false, false, true);
        break;
    case BotEventType::HitByBot:
        evt.read(strm, true, true, true, false);
        break;
    case BotEventType::HitByBullet:
        evt.read(strm, true, true, true, false);
        break;
    case BotEventType::PowerUp:
        evt.read(strm, false, true, true, false);
        break;
    }

    return evt;
}








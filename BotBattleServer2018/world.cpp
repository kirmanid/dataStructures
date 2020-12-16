#define _USE_MATH_DEFINES
#include <limits>
#include <algorithm>

#include "world.h"
#include "myai.h"
#include "random.h"
#include "maze.h"

//#include <QThread>
#ifndef NOGRAPHICS
#include "graphics.h"
#endif

#undef assert

extern Vec2d mousePos;  // set automatically to the current mouse location
extern double cameraAngle;

using namespace std;
using namespace mssm;

Random rnd;

Vec2d RayIntersect(const Vec2d& eye, double angle, const Vec2d& p1, const Vec2d& p2)
{
   Vec2d ray{1, 0};
   ray.rotate(angle);

   //TODO these can be pre-calculated?
   Vec2d p12 = p2-p1;
   Vec2d v1  = p1-eye;
   Vec2d v2  = p2-eye;

   double denom  = cross(ray, p12);
   double numerb = cross(v1, ray);

   double epsilon = 0.001;

   bool parallel = abs(denom) < epsilon;

   if (parallel) {
       if (dot(v1,ray) < dot(v2,ray)) {
           // p1 is closer
           return p1;
       }
       else {
           return p2;
       }
   }

   double mub = numerb / denom;

   return p1 + mub * p12;
}

double RayIntersectDist(const Vec2d& eye, double angle, const Vec2d& p1, const Vec2d& p2)
{
    return (eye - RayIntersect(eye, angle, p1, p2)).magnitude();
}

bool FovIntersect(Vec2d eye, double angle, Vec2d& p1, Vec2d& p2, bool leftEdge)
{
   double maxHorizon = 1e6;
   Vec2d ray{maxHorizon, 0};
   ray.rotate(angle);

   Vec2d p12 = p2-p1;

   double l1 = cross(ray, p1-eye);
   double l2 = cross(ray, p2-eye);

   bool p1good = leftEdge ? (l1 <= 0) : (l1 >= 0);
   bool p2good = leftEdge ? (l2 <= 0) : (l2 >= 0);

   if (!p1good && !p2good) {
       return false;
   }

   double denom  = cross(ray, p12);
   double numera = cross(p12, eye-p1);
   double numerb = -l1;

   //double pos = 50;
   //g.text(10,pos+=20,20,to_string(l1));
   //g.text(10,pos+=20,20,to_string(l2));
   //g.text(10,pos+=20,20,to_string(denom));
  // g.text(10,pos+=20,20,to_string(numera));
  // g.text(10,pos+=20,20,to_string(numerb));
   //g.text(10,70,20,to_string(mub));


   double epsilon = 0.001;

   bool parallel = abs(denom) < epsilon;

   if (parallel) {
       if (abs(numera) < epsilon || abs(cross(ray, eye-p2)) < epsilon) {
           // lines (not necessarily line segments) are coincident
           if (dot(p1-eye,ray) < 0) {
               if (dot(p2-eye,ray) < 0) {
                   return false;
               }
               else {
                   p1 = eye;
                   return true;
               }
           }
           else {
               if (dot(p2-eye,ray) < 0) {
                   p2 = eye;
                   return true;
               }
               else {
                   return true;
               }
           }
       }
       else {
           // parallel but not coincident
           return numera > 0;
       }
   }

   /* Is the intersection along the the segments */
   //double mua = numera / denom;
   double mub = numerb / denom;

   ///g.text(10,pos+=20,20,to_string(mua));
   ///g.text(10,pos+=20,20,to_string(mub));

   if (mub < 0 || mub > 1) {
       return true;
   }

   if (p1good) {
       p2 = p1 + mub * p12;
   }
   else {
       p1 = p1 + mub * p12;
   }

   return true;
}

Vec2d closestPointOnLine(Vec2d p, Vec2d p1, Vec2d p2, double& t)
{
    Vec2d v1 = p2 - p1;
    double segLen = v1.magSquared();
    t = dot(p - p1, v1) / segLen;
    return v1*t + p1;
}


bool seeSeg(/*mssm::Graphics* g, */double cameraAngle, double fieldOfView, Vec2d cameraPosition, Vec2d& p1, Vec2d& p2, double horizonDist)
{
    Vec2d leftRay{horizonDist, 0};
    Vec2d rightRay{horizonDist, 0};

    double leftEdgeAng = cameraAngle + fieldOfView/2;
    double rightEdgeAng = cameraAngle - fieldOfView/2;

    leftRay.rotate(leftEdgeAng);
    rightRay.rotate(rightEdgeAng);

    //g.line(cameraPosition, cameraPosition+leftRay, RED);
    //g.line(cameraPosition, cameraPosition+rightRay, GREEN);

    if (FovIntersect(cameraPosition, leftEdgeAng, p1, p2, true) &&
        FovIntersect(cameraPosition, rightEdgeAng, p1, p2, false)) {

//        if (g) {
//            g->line(cameraPosition, p1, RED);
//            g->line(cameraPosition, p2, GREEN);
//        }

        return true;
    }


    return false;
}



double wallCollisionTime(Vec2d pos, Vec2d vel, const Range2d& field, double radius, Vec2d& collidePos, double& wallNormal)
{
    return ::wallCollisionTime(pos, vel,
                               field.xRange.minVal, field.xRange.maxVal,
                               field.yRange.minVal, field.yRange.maxVal,
                               radius, collidePos, wallNormal);
}


std::vector<unsigned int> distinctColors =
{
    // https://eleanormaclure.files.wordpress.com/2011/03/colour-coding.pdf

        0xFFB300, // Vivid Yellow
        0x803E75, // Strong Purple
        0xFF6800, // Vivid Orange
        0xA6BDD7, // Very Light Blue
        //0xC10020, // Vivid Red
        0xCEA262, // Grayish Yellow
        0x817066, // Medium Gray

        // The following don't work well for people with defective color vision
        0x007D34, // Vivid Green
        0xF6768E, // Strong Purplish Pink
        0x00538A, // Strong Blue
        0xFF7A5C, // Strong Yellowish Pink
        0x53377A, // Strong Violet
        0xFF8E00, // Vivid Orange Yellow
        0xB32851, // Strong Purplish Red
        0xF4C800, // Vivid Greenish Yellow
        0x7F180D, // Strong Reddish Brown
        0x93AA00, // Vivid Yellowish Green
        0x593315, // Deep Yellowish Brown
        0xF13A13, // Vivid Reddish Orange
        0x232C16, // Dark Olive Green

};



Vec2d randomPosition(double minx, double maxx, double miny, double maxy)
{
    double x = rnd.randomDouble(minx, maxx);
    double y = rnd.randomDouble(miny, maxy);

    return Vec2d { x, y };
}

Vec2d randomPosition(double minx, double maxx, double miny, double maxy, double marginSize)
{
    return randomPosition(minx + marginSize, maxx - marginSize, miny + marginSize, maxy - marginSize);
}

Vec2d randomPosition(const Range2d& range, double marginSize)
{
    return randomPosition(range.xRange.minVal, range.xRange.maxVal, range.yRange.minVal, range.yRange.maxVal, marginSize);
}


double subtractAngles(double a1, double a2)
{
    double diff = a1 - a2;

    while (diff < -M_PI)
    {
        diff += 2*M_PI;
    }

    while (diff > M_PI)
    {
        diff -= 2*M_PI;
    }

    return diff;
}

double negateAngle(double angle)
{
    angle += M_PI;
    while (angle > 2*M_PI)
    {
        angle -= 2*M_PI;
    }
    return angle;
}


World::World(Range2d field, int mazeCols, int mazeRows)
  : field(field), eventQueue(this)
{
    paused = false;

    minBotCount = 0;
    desiredFoodCount = defaultDesiredFoodCount;

    nextBotColor = 0;

    gameTime = 0;
    stalePositions = false;

    clockMult = 0;

    Maze maze(mazeRows, mazeCols);

    maze.generate();

    for (pair<Vec2d,Vec2d> wall : maze.walls(false,
                                             field.corner(0),
    {field.xRange.size()/maze.width(),field.yRange.size()/maze.height()})) {
        if (wall.first.x == wall.second.x) {
            // vertical wall
            innerWalls.push_back(Wall{true, wall.first.x, wall.first.y, wall.second.y});
        }
        else {
            // horizontal wall
            innerWalls.push_back(Wall{false, wall.first.y, wall.first.x, wall.second.x});
        }
    }

    foodSpawnPoints = maze.centers(field.corner(0), {field.xRange.size()/maze.width(),field.yRange.size()/maze.height()});

    //innerWalls.push_back(Wall{false, field.yRange.interpolate(0.5), field.xRange.interpolate(0.4), field.xRange.interpolate(0.6)});

    allWalls = innerWalls;

    allWalls.push_back(Wall{true, field.xRange.minVal, field.yRange.minVal, field.yRange.maxVal});
    allWalls.push_back(Wall{true, field.xRange.maxVal, field.yRange.minVal, field.yRange.maxVal});
    allWalls.push_back(Wall{false, field.yRange.minVal, field.xRange.minVal, field.xRange.maxVal});
    allWalls.push_back(Wall{false, field.yRange.maxVal, field.xRange.minVal, field.xRange.maxVal});
}

World::~World()
{
}

void World::faster()
{
    clockMult++;
    clockMult = std::min(16, clockMult);
}

void World::slower()
{
    clockMult--;
    clockMult = std::max(-5, clockMult);
}

bool World::isFreeSpace(Vec2d pos, double radius)
{
    if (stalePositions) {
        cout << "Stale Positions Used in isFreeSpace!" << endl;
    }

    for (auto const& pu : food)
    {
        if ((pos - pu->getPos()).magnitude() < radius + pu->getRadius())
        {
            return false;
        }
    }

    for (auto const& b : bots)
    {
        if ((pos - b->getPos()).magnitude() < radius + b->radius())
        {
            return false;
        }
    }

    for (auto const& b : bullets)
    {
        if ((pos - b->state.getPosition()).magnitude() < radius + b->radius())
        {
            return false;
        }
    }

    for (Wall const& w : allWalls)
    {
        double t;
        double r2 = radius*radius;
        if (distToSegmentSquared(pos, w.p1(), w.p2(), t) < r2) {
            return false;
        }
    }

    return true;
}

//  0 = no collision bits 1-4 = top,right,bottom,left
int World::hitWall(Vec2d& position, double radius, double& wallNormalAngle)
{
    int result = 0;

    double bounce = field.xRange.size() / 10000.0;

    if (position.y + radius >= field.yRange.maxVal)
    {
        wallNormalAngle = 3.0*M_PI/2;
        result |= 0x01;
        position.y = field.yRange.maxVal - radius - bounce;
    }
    if (position.y - radius <= field.yRange.minVal)
    {
        wallNormalAngle = M_PI/2;
        result |= 0x04;
        position.y = field.yRange.minVal + radius + bounce;
    }

    if (position.x + radius >= field.xRange.maxVal)
    {
        wallNormalAngle = M_PI;
        result |= 0x02;
        position.x = field.xRange.maxVal - radius - bounce;
    }

    if (position.x - radius <= field.xRange.minVal)
    {
        wallNormalAngle = 0;
        result |= 0x08;
        position.x = field.xRange.minVal + radius + bounce;
    }

    return result;
}

double World::fixedWallCollisionTime(Vec2d pos, Vec2d vel, double radius, Vec2d& collidePos, double& collideAngle)
{
    double minWallTime = wallCollisionTime(pos, vel, field, radius, collidePos, collideAngle);

    for (auto const& w : innerWalls) {
        Vec2d collidePosTmp;
        double collideAngleTmp;
        double wallTime;
        if (w.vertical) {
            wallTime = verticalSegmentCollisionTime(pos, vel, w.location, w.minBound, w.maxBound, radius, collidePosTmp, collideAngleTmp);
        }
        else {
            wallTime = horizontalSegmentCollisionTime(pos, vel, w.location, w.minBound, w.maxBound, radius, collidePosTmp, collideAngleTmp);
        }
        if (wallTime >= 0 && wallTime < minWallTime) {
            minWallTime = wallTime;
            collidePos = collidePosTmp;
            collideAngle = collideAngleTmp;
        }
    }

    return gameTime + minWallTime;
}


// this bot was just added to world, or its command changed, so
// all events have been cleared and now must be re-generated for this bot
void World::updateBotEvents(std::shared_ptr<Bot> bot)
{
    if (stalePositions) {
        cout << "Stale Positions Used in updateBotEvents!" << endl;
    }


    eventQueue.clearAllInvolved(bot.get());

    //cout << bot->name() << ": updateBotEvents " << eventQueue.size() << " -> ";


    // first for the bot's cmd itself
    if (bot->notAsleep()) {

        // add event for when this new command finishes "naturally"
        eventQueue.addBotCmdFinishedEvt(bot->commandFinishTime(), bot->commandName(), bot);

        // if bot is moving, add wall collision event

        if (bot->isMoving())
        {
            Vec2d  collidePos;
            double collideAngle;

            double wallTime = fixedWallCollisionTime(bot->getPos(), bot->getVel(), bot->radius(), collidePos, collideAngle);

            if (bot->willNotFinishBefore(wallTime)) {
                eventQueue.addBotHitWallEvt(wallTime, bot, collidePos, collideAngle);
            }
        }
    }

    // check whether this bot collides with any other bot
    // this could cause a "stun" condition if it instantly hits another bot
    // and that collision is on the front of this bot
    for (auto const& b : bots)
    {
        if (b != bot && (b->isMoving() || bot->isMoving()))
        {
            // one or the other bot is moving, so they could collide

            // are they moving towards each other?

            Vec2d botRelV = bot->getVel() - b->getVel();
            Vec2d relPos = b->getPos() - bot->getPos();

            if (dot(botRelV, relPos) > 0) {
                // their relative velocity is indeed towards one another
                Vec2d pc;
                Vec2d qc;

                double t = collisionTime(bot->getPos(), bot->getVel(),
                                         b->getPos(), b->getVel(),
                                         pc, qc,
                                         bot->radius()+b->radius());

                if (t >= 0)
                {
                    double hittime = gameTime + t;

                    // only add the hit event if neither bots' commands will finish before the hit
                    if (bot->willNotFinishBefore(hittime) && b->willNotFinishBefore(hittime)) {
                        double absoluteAngle1 = atan2(qc.y - pc.y, qc.x - pc.x);
                        double absoluteAngle2 = atan2(pc.y - qc.y, pc.x - qc.x);
                        eventQueue.addBotHitBotEvt(gameTime + t, bot, b, pc, qc, absoluteAngle1, absoluteAngle2);
                    }
                }
            }
        }
    }

    for (auto const& b : bullets)
    {
        if (b->isShooter(bot.get())) {
            continue;  // can't shoot yourself
        }

        Vec2d pc;
        Vec2d qc;

        double t = gameTime + collisionTime(bot->getPos(), bot->getVel(),
                                 b->state.getPosition(), b->state.getVelocity(),
                                 pc, qc,
                                 bot->radius() + b->radius());

        if (t >= gameTime && t < b->wallTime())
        {
            eventQueue.addBulletHitBotEvt(t, b, bot, pc, qc);
        }
    }

    for (auto const& pu : food)
    {
        Vec2d pc;
        Vec2d qc;

        double t = gameTime + collisionTime(bot->getPos(), bot->getVel(),
                                 pu->getPos(), {0.0,0.0},
                                 pc, qc,
                                 bot->radius() + pu->getRadius());

        if (t >= gameTime)
        {
            double absoluteAngle = atan2(qc.y - pc.y, qc.x - pc.x);
            eventQueue.addBotHitFoodEvt(t, bot, pu, pc, absoluteAngle);
        }
    }

    //cout <<  eventQueue.size() << endl;
    //eventQueue.dump();
}

// this bullet was just added to the scene, so any bullet related events
// must now be generated
void World::updateBulletEvents(std::shared_ptr<Bullet> bullet)
{
    if (stalePositions) {
        cout << "Stale Positions Used in updateBulletEvents!" << endl;
    }


    Vec2d  collidePos;
    double collideAngle;

    // set "walltime"
    bullet->motion->setEndTime(fixedWallCollisionTime(bullet->state.getPosition(), bullet->state.getVelocity(), bullet->radius(), collidePos, collideAngle));

    eventQueue.addBulletHitWallEvt(bullet->wallTime(), bullet, collidePos);

    for (auto const& bot : bots)
    {
        if (bullet->isShooter(bot.get())) {
            continue;  // can't shoot yourself
        }

        Vec2d pc;
        Vec2d qc;

        double t = gameTime + collisionTime(bot->getPos(), bot->getVel(),
                                 bullet->state.getPosition(), bullet->state.getVelocity(),
                                 pc, qc,
                                 bot->radius() + bullet->radius());

        if (t >= gameTime && t < bullet->wallTime())
        {
            eventQueue.addBulletHitBotEvt(t, bullet, bot, pc, qc);
        }
    }
}

// this powerUp was just added to the scene, so any powerUp related events
// must now be generated
void World::updateFoodEvents(std::shared_ptr<Food> food)
{
    if (stalePositions) {
        cout << "Stale Positions Used in updateFoodEvents!" << endl;
    }

    for (auto const& bot : bots)
    {
        Vec2d pc;
        Vec2d qc;

        double t = gameTime + collisionTime(bot->getPos(), bot->getVel(),
                                 food->getPos(), {0.0, 0.0},
                                 pc, qc,
                                 bot->radius() + food->getRadius());

        if (t >= gameTime)
        {
            double angle = atan2(qc.y - pc.y, qc.x - pc.x);
            eventQueue.addBotHitFoodEvt(t, bot, food, pc, angle);
        }
    }

}



enum class EventClass
{
    None,
    BotHitsWall,
    BulletHitsWall,
    BotHitsBot,
    BulletHitsBot,
    BotCmdFinished
};

void World::deleteBullet(Bullet* bullet)
{
    eventQueue.clearAllInvolved(bullet);
    bullets.erase( remove_if(bullets.begin(), bullets.end(), [bullet](auto const& b) { return b.get() == bullet; }), bullets.end());
}

double angleWithCamera(const Vec2d& cameraPosition, double cameraAngle, const Vec2d& p)
{
    auto viewVector = p - cameraPosition;

    double botAngle = atan2(viewVector.y, viewVector.x);

    // atan2 returns +- M_PI

    if (botAngle < 0) {
        botAngle += 2 * M_PI;
    }

    // now botAngle is in range 0 - 2*M_PI, which is same as cameraAngle
    return subtractAngles(botAngle, cameraAngle);
}


std::vector<int> World::look(/*mssm::Graphics* g, */double fieldOfView, Vec2d cameraPosition, double cameraAngle, int resolution, bool useDistance)
{
    if (stalePositions) {
        cout << "Stale Positions Used in look!" << endl;
    }

    std::vector<int> botNums(resolution, 0);
    std::vector<int> distance(resolution, std::numeric_limits<int>::max());

    lastView.resize(resolution);

    for (int& c : lastView) {
        c = 0; // mssm::BLACK.toIntRGB();
    }

    double scale = (double)resolution / fieldOfView;

    for (auto b : bots)
    {
        auto viewVector = b->getPos() - cameraPosition;

        double botAngle = atan2(viewVector.y, viewVector.x);

        // atan2 returns +- M_PI

        if (botAngle < 0) botAngle += 2 * M_PI;

        // now botAngle is in range 0 - 2*M_PI, which is same as cameraAngle

        double botDist = viewVector.magnitude();
        int botDistI = botDist*1000;

        double relativeBotAngle = subtractAngles(botAngle, cameraAngle);


        //cout << "RelativeBotAngle: " << relativeBotAngle << endl;

        double angularRadius = asin(b->radius()/botDist);

        double botMaxAngle = relativeBotAngle + angularRadius;
        double botMinAngle = relativeBotAngle - angularRadius;

        if ((botMaxAngle > M_PI) || (botMinAngle < -M_PI))
        {
            // cout << "Behind!\n";
            continue;
        }

        int botMinPixel = resolution/2 + scale * botMinAngle;
        int botMaxPixel = resolution/2 + scale * botMaxAngle;

        if (botMinPixel < 0)
        {
            botMinPixel = 0;
        }

        if (botMaxPixel > resolution-1)
        {
            botMaxPixel = resolution-1;
        }

        if (botMaxPixel < 0 || botMinPixel > resolution-1)
        {
            continue;
        }

        int id = b->getId();

        for (int i = botMinPixel; i <= botMaxPixel; i++)
        {
            int idx = resolution-1-i;

            if (botDistI < distance[idx])
            {
                distance[idx] = botDistI;
                botNums[idx] = id;
                lastView[idx] = b->color;
            }
        }
    }

    Vec2d lookRay{1, 0};

    lookRay.rotate(cameraAngle);

    for (const Wall& w : allWalls) {
        Vec2d p1 = w.p1();
        Vec2d p2 = w.p2();

        if (seeSeg(/*g, */cameraAngle, fieldOfView, cameraPosition, p1, p2, 1000))
        {
            double angle1 = angleWithCamera(cameraPosition, cameraAngle, p1);
            double angle2 = angleWithCamera(cameraPosition, cameraAngle, p2);

            if (angle1 > angle2) {
                swap(angle1, angle2);
            }

            int minPixel = constrain((int)(resolution/2 + scale * angle1), 0, resolution-1);
            int maxPixel = constrain((int)(resolution/2 + scale * angle2), 0, resolution-1);

            double angle  = angle1;
            double dangle = minPixel == maxPixel ? 1 : ((angle2-angle1) / (maxPixel - minPixel));

            for (int i = minPixel; i <= maxPixel; i++)
            {
                int idx = resolution-1-i;

                //Vec2d ip = RayIntersect(cameraPosition, cameraAngle+angle, p1, p2);
                // if (g) {
                //     g->ellipseC(ip, 3,3,GREEN);
                // }

                double wallDist = RayIntersectDist(cameraPosition, cameraAngle+angle, p1, p2) * 1000;

                angle += dangle;

                if (wallDist < distance[idx])
                {
                    distance[idx] = wallDist;
                    botNums[idx] = 0; //  mssm::BLACK.toIntRGB();
                    lastView[idx] = 0; // mssm::BLACK.toIntRGB();
                }
            }

        }
    }
/*
    for (int i = 0; i < resolution; i++) {
        double ang = subtractAngles(cameraAngle, fieldOfView/2) + (resolution-1-i)/scale;
        Vec2d v{distance[i]/1000.0, 0.0};
        v.rotate(ang);
        g->line(cameraPosition, cameraPosition+v);
    }
*/
    return useDistance ? distance : botNums;
}

#ifndef NOGRAPHICS
void Wall::draw(Graphics &g) const
{
    g.line(p1(), p2());
}

std::string toTimeString(double seconds)
{
    constexpr double secondsPerHour = 3600;

    int hours   = seconds/secondsPerHour;
    int minutes = fmod(seconds,secondsPerHour)/60.0;
    int secs = fmod(seconds,60.0);
    double milliseconds = 1000.0*(fmod(seconds,60.0)-secs);

    stringstream ss;

    int days = hours/24;

    hours = hours%24;

    ss << days << "d " << hours << "h " << minutes << "m " << secs << "s " << milliseconds << "ms";

    return ss.str();
}

void World::draw(mssm::Graphics& g)
{
//    double fieldOfView = M_PI/2;
    g.rect(field.corner(0),
           field.xRange.size(),
           field.yRange.size(), WHITE);

    for (auto const& w : innerWalls) {
        w.draw(g);
    }

    for (auto const& b : bots)
    {
        b->draw(g, currentTime());
    }

    for (auto const& f : food)
    {
        f->draw(g);
    }

    for (auto const& b : deadBots)
    {
        b->draw(g, currentTime());
    }

    for (auto const& b : bullets)
    {
        b->draw(g);
    }

    if (lastView.size() > 0)
    {
        double PixelWidth = (field.xRange.size())/lastView.size();

        for (unsigned int i=0;i<lastView.size();i++)
        {
            g.rect(field.xRange.minVal + i*PixelWidth, field.yRange.maxVal + 2, PixelWidth, 20, lastView[i], lastView[i]);
        }
    }

    while (messages.size() > 39) {
        messages.erase(messages.begin());
    }

    for (unsigned int i=0;i<messages.size();i++)
    {
        g.text(field.xRange.maxVal+10, field.yRange.maxVal - 20*(i+2), 20, messages[i].first, messages[i].second);
    }

    std::string clock;

    if (clockMult >= 0)
    {
        clock = to_string(1 << clockMult);
    }
    else
    {
        clock = "1/" + to_string(1 << -clockMult);
    }

    Color c;

    switch (lastUpdateState) {
    case LastUpdateState::Completed:
        c = GREEN;
        break;
    case LastUpdateState::MaxCompLimit:
        c = YELLOW;
        break;
    case LastUpdateState::NoEvents:
        c = RED;
        break;
    case LastUpdateState::SingleStep:
        c = BLUE;
        break;
    case LastUpdateState::Waiting:
        c = WHITE;
        break;
    default:
        c = PURPLE;
        break;
    }

    g.text(g.width()-190, g.height()-20, 20, clock, c);

    g.text(g.width()-100, g.height()-20, 20, toTimeString(gameTime), c);


    eventQueue.draw(g);
}

#endif // NOGRAPHICS

std::ostream& prop(std::ostream& strm, const std::string& name, const std::string& value)
{
    strm << "\"" << name << "\":\"" << value << "\"";
    return strm;
}

std::ostream& prop(std::ostream& strm, const std::string& name, int value)
{
    strm << "\"" << name << "\":" << value;
    return strm;
}

std::ostream& prop(std::ostream& strm, const std::string& name, double value)
{
    strm << "\"" << name << "\":" << value;
    return strm;
}


std::ostream& prop(std::ostream& strm, const std::string& name, const Vec2d& value)
{
    strm << "\"" << name << "\":[" << value.x << "," << value.y << "]";
    return strm;
}

std::ostream& saveVec(const Vec2d& vec, std::ostream& strm)
{
    strm << vec.x << ",";
    strm << vec.y;
    return strm;
}

void Wall::save(std::ostream& strm) const
{
    strm << "{";
    prop(strm, "p1", p1()) << ",";
    prop(strm, "p2", p2()) << "}";
}

void ActorState::save(std::ostream& strm) const
{
    prop(strm, "pos", position) << ",";
    prop(strm, "vel", velocity) << ",";
    prop(strm, "dir", direction) << ",";
    prop(strm, "health", health);
}

void ActorState::load(std::istream& strm)
{
    strm >> position.x; strm.get();
    strm >> position.y; strm.get();
    strm >> velocity.x; strm.get();
    strm >> velocity.y; strm.get();
    strm >> direction; strm.get();
    strm >> health;
}

std::string motionTypeName(MotionType type)
{
    switch (type) {
    case MotionType::Bullet:   return "B";
    case MotionType::Fire:     return "F";
    case MotionType::Health:   return "H";
    case MotionType::Translate:return "T";
    case MotionType::Rotate:   return "R";
    case MotionType::Scan:     return "S";
    case MotionType::PowerUp:  return "P";
    case MotionType::Sleep:    return "Z";
    case MotionType::Resign:   return "Q";
    case MotionType::Unknown:  return "U";
    }

    return "U";
}

void Motion::save(std::ostream& strm) const
{
    strm << "{";
    prop(strm, "type", motionTypeName(type)) << ",";
    prop(strm, "start", start) << ",";
    prop(strm, "end", end) << ",";
    prop(strm, "rate", rate) << ",";
    prop(strm, "timeSpan", Vec2d{timeSpan.minVal, timeSpan.maxVal});
    strm << "}";
}

void Motion::load(std::istream& /*strm*/)
{
    throw new logic_error("Not Implemented");
}

bool CompMotionStartInc(const Motion& m1, const Motion& m2)
{
    return m1.startTime() < m2.startTime();
}

void BotHistory::sort()
{
    ::sort(motionHistory.begin(), motionHistory.end(), CompMotionStartInc);
}

bool BotHistory::getJson(std::ostream& strm, bool &firstRecord)
{
    if (alreadyGotten >= (int)motionHistory.size()) {
        return false;
    }
    if (!firstRecord) {
        strm << ", ";
    }
    firstRecord = false;
    strm << "{\"id\":" << id << ",\"rec\":[";
    if (alreadyGotten == -1) {
        // send initial state
        strm << "{\"type\":\"I\", ";
        initialState.save(strm);
        alreadyGotten = 0;
        strm << "}\n";
    }
    bool addComma = false;
    while (alreadyGotten < (int)motionHistory.size()) {
        if (addComma || alreadyGotten < 0) {
            strm << ",";
        }
        addComma = true;
        motionHistory[alreadyGotten].save(strm);
        alreadyGotten++;
    }
    strm << "]}\n";
    return true;
}

void BotHistory::save(std::ostream& strm) const
{

    strm << "{";
    prop(strm, "id", id) << ", ";
    prop(strm, "name", name) << ", ";
    strm << "init:";
    initialState.save(strm);
    strm << ", history: [\n";
    bool first = true;
    for (auto const& h : motionHistory) {
        if (!first) {
            strm << ", ";
        }
        first = false;
        h.save(strm);
        strm << "\n";
    }
    strm << "]}\n";
}

void BotHistory::load(std::istream& /*strm*/)
{

}

void BotHistorian::save(std::ostream& strm) const
{
    bool first = true;
    strm << "bots:[\n";
    for (auto const& h : allBotHistory) {
        if (!first) {
            strm << ",\n";
        }
        else {
            first = false;
        }
        h.save(strm);
    }
    strm << "\n]\n";
}

bool FoodHistory::getJson(std::ostream& strm, bool &firstRecord)
{
    if (sentStart && (endTime == std::numeric_limits<double>::max() || sentEnd)) {
        return false;
    }

    if (!firstRecord) {
        strm << ", ";
    }
    firstRecord = false;

    strm << "{";
    prop(strm, "position", position) << ",";
    prop(strm, "start", startTime);
    sentStart = true;
    if (endTime != std::numeric_limits<double>::max()) {
        strm << ",";
        prop(strm, "end", endTime);
        sentEnd = true;
    }
    strm << "}";

    return true;
}


bool BotHistorian::getJson(std::ostream& strm)
{
    bool hasData = false;
    bool first = true;
    strm << "[\n";

    // TODO output walls

    strm << "{\"id\":0,\"food\":[\n";

    for (FoodHistory& food : consumedFood) {
        if (food.getJson(strm, first)) {
            hasData = true;
        }
    }

    consumedFood.clear();

    for (FoodHistory& food : activeFood) {
        if (food.getJson(strm, first)) {
            hasData = true;
        }
    }

    strm << "]}\n";

    first = false;

    for (auto & h : allBotHistory) {
        if (h.getJson(strm, first)) {
            hasData = true;
        }
    }
    strm << "]\n";

    return hasData;
}

void BotHistorian::clear()
{
    allBotHistory.clear();
}

void BotHistorian::beginBotHistory(size_t id, const std::string& name, const ActorState& initialState)
{
    //cout << "Beginning history of bot: " << id << endl;
    if (id >= allBotHistory.size()) {
        allBotHistory.resize(id);
    }
    BotHistory& hist = allBotHistory[id-1];
    hist.id = id;
    hist.name = name;
    hist.initialState = initialState;
}

void BotHistorian::appendBotHistory(size_t id, const std::vector<Motion>& motionHist, bool isDead)
{
    if (motionHist.empty() && !isDead) {
        // nothing interesting happened
        return;
    }
/*
    cout << "More history for: " << id << endl;
    if (isDead) {
        cout << "He died: " << id << endl;
    }
*/
    if (id > allBotHistory.size()) {
        throw runtime_error("Invalid Bot History ID");
    }
    BotHistory& hist = allBotHistory[id-1];
    hist.motionHistory.insert(hist.motionHistory.end(), motionHist.begin(), motionHist.end());
}

void BotHistorian::beginFood(Vec2d position, double time)
{
    activeFood.push_back(FoodHistory{position, time, std::numeric_limits<double>::max()});
}

void BotHistorian::endFood(Vec2d position, double time)
{
    auto pos = find_if(activeFood.begin(), activeFood.end(), [&position](FoodHistory& fh) { return fh.position.equals(position, 0); });
    if (pos == activeFood.end()) {
        cout << "Didn't find food!" << endl;
        throw runtime_error("BotHistorian didn't find food");
    }
    consumedFood.push_back(*pos);
    consumedFood.back().endTime = time;
    activeFood.erase(pos);
}

void World::saveHistory(std::ostream& strm) const
{
    strm << "let world = {\n";

    strm << "walls:[\n";
    bool first = true;
    for (Wall const& w : allWalls) {
        if (!first) {
            strm << ",\n";
        }
        else {
            first = false;
        }
        w.save(strm);
    }

    strm << "],\n";

    historian.save(strm);

    strm << "}\n";

}

void World::loadHistory(std::istream& /*strm*/)
{

}


void World::fire(std::shared_ptr<Bot> shooter, Vec2d bulletPos, double bulletDirection, double startingOffset)
{
    Vec2d bulletVelocity { 100, 0 };
    bulletVelocity.rotate(bulletDirection);
    Vec2d bulletOffset { startingOffset, 0 };
    bulletOffset.rotate(bulletDirection);

    bullets.push_back(make_shared<Bullet>(shooter, currentTime(), bulletPos + bulletOffset, bulletDirection, bulletVelocity));

    updateBulletEvents(bullets.back());
}

void World::fakeFire(Vec2d bulletPos, double bulletDirection)
{
    Vec2d bulletVelocity { 100, 0 };
    bulletVelocity.rotate(bulletDirection);
    bullets.push_back(make_shared<Bullet>(make_shared<Shooter>(), currentTime(), bulletPos, bulletDirection, bulletVelocity));
    updateBulletEvents(bullets.back());
}

void World::placeFood(Vec2d foodPos)
{
    food.push_back(make_shared<Food>(foodPos, foodRadius, currentTime()));
    historian.beginFood(foodPos, currentTime());
    updateFoodEvents(food.back());
}

Vec2d World::nextSpawnPoint()
{
    if (foodSpawnPoints.size() == 0) {
        foodSpawnPoints = usedSpawnPoints;
        usedSpawnPoints.clear();
    }

    int idx = rnd.randomInt(0, foodSpawnPoints.size()-1);

    Vec2d pos = foodSpawnPoints[idx];

    usedSpawnPoints.push_back(pos);
    foodSpawnPoints.erase(foodSpawnPoints.begin() + idx);

    return pos;
}

void World::spawnFood(int minFood)
{
    int spaces  = foodSpawnPoints.size() + usedSpawnPoints.size();
    int current = food.size();

    minFood = std::min(spaces, minFood);

    while (current < minFood) {

        int maxTries = spaces;
        bool placedFood = false;

        while (maxTries > 0) {
            maxTries--;

            Vec2d pos = nextSpawnPoint();

            if (isFreeSpace(pos, foodRadius*2)) {
                placeFood(pos);
                placedFood = true;
                current++;
                spaces--;
                break;
            }
        }

        if (!placedFood) {
            cout << "Unable to place food (all spots occupied by bots)" << endl;
            break;
        }
    }
}


void World::addBot(std::shared_ptr<Bot> newBot, int maxHealth, int initBullets)
{
    auto pos   = randomPosition(field, newBot->radius()*1.1);
    double dir = rnd.randomDouble(-M_PI, M_PI);
    addBot(newBot, pos, dir, maxHealth, initBullets);
}

void World::addBot(std::shared_ptr<Bot> newBot, Vec2d pos, double dir, int maxHealth, int initBullets)
{
    unsigned int color = distinctColors[nextBotColor%distinctColors.size()];

    nextBotColor++;

    newBot->color = color;

    while (!isFreeSpace(pos, newBot->radius()*2))
    {
        pos = randomPosition(field, newBot->radius()*1.1);
    }

    newBot->initializePosition(pos, dir, maxHealth, initBullets);

    newBots.push_back(newBot);

    historian.beginBotHistory(newBot->getId(), newBot->name(), newBot->state);
}

bool World::assert(bool mustBeTrue, const string &msg)
{
    if (!mustBeTrue) {
        message("Assert Fail: " + msg, 0xFF0000);
        paused = true;
    }
    return mustBeTrue;
}

void World::endGame()
{
    while (!food.empty()) {
        killFood(food.back());
    }

    desiredFoodCount = 0;
    minBotCount = 0;

    while (!bots.empty()) {
        killBot(bots.back(), "Removed (End Game)");
    }

    do {
        update(1000, false, 1, false);
    }
    while (eventQueue.size() > 0);

    desiredFoodCount = defaultDesiredFoodCount;
}

void World::updateHistory()
{
    double earliest = currentTime();  // lets start by assuming that any point in the future is indeterminate

    for (auto const& bot : bots) {
        double ea = bot->motionTracker.earliestActiveTime();
        if (ea < earliest) {
            earliest = ea;
        }
    }

    // earliest is the beginning of the time when the state of the world is indeterminate
    // therefore everything before that time can be moved into the history

    for (auto const& bot : bots) {
        historian.appendBotHistory(bot->getId(), bot->motionTracker.removeCompletedBefore(earliest), false);
    }
}

std::string World::streamHistory()
{
    std::stringstream ss;

    if (historian.getJson(ss)) {
        return ss.str();
    }

    return "";
}

void World::clearHistory()
{
    historian.clear();
}

void World::killDisconnectedBots(int maxWaitTimeMs)
{
    vector<shared_ptr<Bot>> disconnectedBots;
    vector<shared_ptr<Bot>> notCommunicating;

    auto nowTime = std::chrono::system_clock::now();

    for (auto const& b : bots)
    {
        if (b->isDisconnected())
        {
            disconnectedBots.push_back(b);
        }
        else if (b->commWaitTimeMs(nowTime) > maxWaitTimeMs)
        {
            notCommunicating.push_back(b);
        }
    }

    for (auto const& b : disconnectedBots)
    {
        killBot(b, "Disconnected");
    }

    for (auto const& b : notCommunicating)
    {
        killBot(b, "Removed (Not Responding)");
    }
}

int World::getNextBotId()
{
    return ids.getId();
}

void World::releaseBotId(int id)
{
    ids.releaseId(id);
}

// this updates the position of bots, bullets, and the intermediate progress of
// things like scan and fire.  It is intended only to aid with animation and
// should not be needed when not animating
void World::advanceGameTime()
{
    // update stuff that didn't trigger an event
    // TODO make sure that _not_ calling advanceGameTime never affects the game outcome
    for (auto const& b : bullets)
    {
        b->updatePosition(currentTime());
    }

    for (auto const& b : bots)
    {
        b->updateCommand(currentTime());
    }

    stalePositions = false;
}


void World::killBot(std::shared_ptr<Bot> bot, string reason)
{
    message(bot->name() + " " + reason, 0xFF0000);
    eventQueue.clearAllInvolved(bot.get());
    historian.appendBotHistory(bot->getId(), bot->motionTracker.removeCompleted(), true);
    deadBots.push_back(bot);
    bot->getAI()->setKilled();
    bots.erase(remove(bots.begin(), bots.end(), bot), bots.end());
}

void World::killFood(std::shared_ptr<Food> oneFood)
{
    eventQueue.clearAllInvolved(oneFood.get());
    historian.endFood(oneFood->getPos(), currentTime());
    food.erase(remove(food.begin(), food.end(), oneFood), food.end());
}

// transmit events to the bot,  receive new command,  generate future events based on that command
void World::updateBot(std::shared_ptr<Bot> bot, bool transmitEvents)
{
    if (bot->isDead())
    {
        killBot(bot, "Was Destroyed");
        return;
    }

    if (transmitEvents) {
        //std::cout << " --- updateBegin ----- EVENT --------------------" << endl;
        bot->transmitEvents(currentTime());
    }
    else {
        //std::cout << " --- updateBegin ----- WAIT  --------------------" << endl;
    }
    if (bot->receiveNewCommand(bot, currentTime())) {
        updateBotEvents(bot);
    }
    //std::cout << " -- updateEnd -- "<< endl;
}

bool World::introduceNewBots(bool addOneAtMost)
{
    bool botWasAdded = false;

    while ((int)(bots.size() + newBots.size()) < minBotCount)
    {
        addBot(make_shared<Bot>(this, new LocalBotAI(std::make_unique<MyAI>(false)), getNextBotId(), false), localBotHealth, localBotBullets);
        botWasAdded = true;
        if (addOneAtMost) {
            break;
        }
    }

    // TODO is newbots needed?

    if (!newBots.empty())
    {
        for (auto const& bot : newBots)
        {
            bots.push_back(bot);
            updateBot(bot);
        }
        newBots.clear();
    }

    return botWasAdded;
}

void World::flushDeadBots()
{
    deadBots.erase(remove_if(deadBots.begin(), deadBots.end(), [this](auto&& b) {
        if (!b->hasOngoingVisual(this->currentTime())) {
            return true;
        }
        return false;
    }), deadBots.end());
}

// returns number of events executed
// alwaysAdvance updates bullet and bot positions when time passes
//
// realElapsedTime:    typically the real time between calls to update
// maxCalculationTime: when not in single step mode
int World::
update(double realElapsedTime, double maxCalculationTime, bool singleStepMode, bool alwaysAdvance)
{  
    auto beginTime   = std::chrono::steady_clock::now();

    double timeToUse = (clockMult >= 0) ? (realElapsedTime * (1 << clockMult)) : (realElapsedTime / (1 << -clockMult));

    int totalEventsExecuted = 0;

    lastUpdateState = LastUpdateState::Completed; // optimistic

    do
    {
        spawnFood(desiredFoodCount);

        if (timeToUse <= 0) {
            cout << "NO TIME TO USE" << endl;
        }

        killDisconnectedBots(30000);
        flushDeadBots();

        if (introduceNewBots(singleStepMode) && singleStepMode) {
            lastUpdateState = LastUpdateState::SingleStep;
            break;
        }

        if (updateWaitingBots() > 0) {
            // still waiting for some (remote) bots to receive their commands
            lastUpdateState = LastUpdateState::Waiting;
            break;
        }

        if (eventQueue.size() == 0) {

            if (bots.size() > 0 && !messagesSuppressed) {
                cout << "No events in queue but we still have bots!!!" << endl;
            }

            if (bullets.size() > 0 && !messagesSuppressed) {
                cout << "No events in queue but still have bullets!!!" << endl;
            }

            // TODO: consider "ongoing visuals" and whether game time needs to pass

            lastUpdateState = LastUpdateState::NoEvents;
            singleStepMode = true;
            messagesSuppressed = true;

            break;
        }
        else {
            messagesSuppressed = false;
        }

        double elapsedTime;

        std::unique_ptr<WorldEvent> evt = eventQueue.getNextEventToExecute(gameTime, timeToUse, elapsedTime, singleStepMode);

        if (elapsedTime > 5) {
            cout << "ElapsedTime >> 60 == " << elapsedTime << endl;
        }

        if (elapsedTime > 0) {
            stalePositions = true;
        }

        if (!evt) {

            if (elapsedTime == 0) {
                cout << "no event was ready, and no time has passed!!" << endl;
                lastUpdateState = LastUpdateState::NoEvents;
                singleStepMode = true;
            }
            else {
                lastUpdateState = LastUpdateState::Completed;
            }

            if (alwaysAdvance && elapsedTime > 0) {
                advanceGameTime();
            }


            break;
        }

        // TODO: is advanceGameTime only necessary if the new command will
        // necessitate recalculating collisions?  think more about stalePositions flag
        if (elapsedTime > 0) {
            advanceGameTime(); // update bullets and bots to the current game time (moves and turns)
        }

        //cout << "Executing Event: " << evt->desc() << endl;
        evt->execute(*this);
        totalEventsExecuted++;

        if (singleStepMode) {
            lastUpdateState = LastUpdateState::SingleStep;
            break;
        }

        auto currentTime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - beginTime).count()/1000.0 > maxCalculationTime) {
           // cout << "Max Calc Time exceeded" << endl;
            lastUpdateState = LastUpdateState::MaxCompLimit;
            break;
        }
    }
    while (timeToUse > 0);

    updateHistory();

    //cout << "Events: " << totalEventsExecuted << endl;

    return totalEventsExecuted;
}

int World::updateWaitingBots()
{
    int waitCount = 0;

    for (auto const& bot : bots)
    {
        if (bot->isWaiting()) {
            // cout << bot->name() << " was waiting... check for a new command" << endl;
            updateBot(bot, false);  // handle new recieved command, if any
            if (bot->isWaiting() && !bot->isDisconnected()) {
                waitCount++;
            }
        }
    }

    if (waitCount > 1) {
        //cout << waitCount << " Bots are waiting" << endl;
    }

    return waitCount;
}

void World::bulletHitWallEvt(Bullet* bullet)
{
    //cout << "Handling bulletHitWallEvt: " << endl;
    deleteBullet(bullet);
}

void World::botHitWallEvt(std::shared_ptr<Bot>  bot, Vec2d pos, double collisionAngle)
{
    //cout << "Handling botHitWallEvt: " << endl;

    if (fabs(bot->commandStartTime()-currentTime()) < moveStunThreshold) {
        message(bot->name() + " Drove into a wall", 0x00FFFF);
        bot->applyDamage(currentTime(), false);
    }

    bot->hitWallEvt(currentTime(), collisionAngle);
    bot->finishCommand(bot, currentTime(), true, pos);
    updateBot(bot);
}

void World::botCmdFinishedEvt(std::shared_ptr<Bot> bot)
{
    bot->finishCommand(bot, currentTime(), false);
    updateBot(bot);
}

void World::botHitFoodEvt(std::shared_ptr<Bot> bot, std::shared_ptr<Food> food, Vec2d /*pos*/, double angle)
{
    bot->gotFoodEvt(currentTime(), angle);
    killFood(food);
    updateBot(bot);
}

void World::bulletHitBotEvt(Bullet* bullet, std::shared_ptr<Bot>  bot)
{
    //cout << "Handling bulletHitBotEvt: " << endl;

    bot->gotShotEvt(currentTime(), bullet->getShooter(), bullet->state.getDirection());
    deleteBullet(bullet);
    updateBot(bot);
}

void World::botHitBotEvt(std::shared_ptr<Bot>  bot1, std::shared_ptr<Bot>  bot2, Vec2d pos1, Vec2d pos2, double absoluteAngle1, double absoluteAngle2)
{
    double relativeAngle1 = subtractAngles(absoluteAngle1, bot1->getDir());
    double relativeAngle2 = subtractAngles(absoluteAngle2, bot2->getDir());

    //cout << "Handling botHitBotEvt: " << endl;
    if (bot1->isBlockingCollision(relativeAngle1))
    {

        // blocked
        //cout << bot1->name() << " was blocked hitting bot " << relativeAngle1 << endl;

        if (fabs(bot1->commandStartTime()-currentTime()) < moveStunThreshold) {
            message(bot1->name() + " Quantum Feedback Damage!", 0x00FFFF);
            bot1->applyDamage(currentTime(), false);
        }

        bot1->hitBotEvt(currentTime(), absoluteAngle1);
        bot1->finishCommand(bot1, currentTime(), true, pos1);

    }
    else {
        // escapeable
        //cout << bot1->name() << " was hit by bot (but not blocked) " << relativeAngle1 << endl;
        bot1->hitByBotEvt(currentTime(), absoluteAngle1);
    }

    if (bot2->isBlockingCollision(relativeAngle2))
    {
        // blocked
        //cout << bot2->name() << " was blocked hitting bot " << relativeAngle2 << endl;

        if (fabs(bot2->commandStartTime()-currentTime()) < moveStunThreshold) {
            message(bot2->name() + " Quantum Feedback Damage!", 0x00FFFF);
            bot2->applyDamage(currentTime(), false);
        }

        bot2->hitBotEvt(currentTime(), absoluteAngle2);
        bot2->finishCommand(bot2, currentTime(), true, pos2);
    }
    else {
        // escapeable
        //cout << bot2->name() << " was hit by bot (but not blocked) " << relativeAngle2  << endl;
        bot2->hitByBotEvt(currentTime(), absoluteAngle2);
    }

    updateBot(bot1);
    updateBot(bot2);
}



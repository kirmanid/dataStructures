#define _USE_MATH_DEFINES
#include "bot.h"
#include "world.h"

#ifndef NOGRAPHICS
#include "graphics.h"
#endif

#include <iostream>
#include <algorithm>
#include <sstream>

using namespace std;
using namespace mssm;

constexpr int powerUpBulletInc = 5;

const double shotEffectTime = 1.0;

int Bot::botCount = 0;
int Bullet::bulletCount = 0;

ActorState::ActorState()
{
}

double ActorState::speed() const
{
    Vec2d forward{ 1.0, 0.0 };
    forward.rotate(direction);
    return dot(velocity, forward);
}

void ActorState::init(Vec2d pos, Vec2d vel, double dir, double hp, int numBullets)
{
    position  = pos;
    velocity  = vel;
    direction = dir;
    health    = hp;
    bulletCount = numBullets;
}


Motion* MotionTracker::create(MotionType type, double startTime)
{
    std::unique_ptr<Motion> mot(std::make_unique<Motion>(type, startTime));
//    mot->type = type;
//    mot->timeSpan.minVal = startTime;
   // Motion* ptr = mot.get();
    active.emplace_back(std::move(mot));
    return active.back().get();
}

void MotionTracker::complete(Motion* motion)
{
    auto pos = find_if(active.begin(), active.end(), [motion](std::unique_ptr<Motion>& mot) { return mot.get() == motion; });
    if (pos == active.end()) {
        throw runtime_error("Motion Tracker couldn't find motion to complete");
    }
    completed.emplace_back(std::move(*pos));
    active.erase(pos);
}

std::vector<Motion> MotionTracker::removeCompleted()
{
    // TODO make sure active is empty
   std::vector<Motion> comp;
   for (auto const& m : completed) {
       comp.push_back(*m);
   }
   completed.clear();
   return comp;
}

double MotionTracker::earliestActiveTime()
{
    double t = std::numeric_limits<double>::max();
    for (auto const& m : active) {
        if (m->startTime() < t) {
            t = m->startTime();
        }
    }
    return t;
}

std::vector<Motion> MotionTracker::removeCompletedBefore(double time)
{
    std::vector<Motion> comp;
    completed.erase(remove_if(completed.begin(), completed.end(), [time,&comp](std::unique_ptr<Motion>& mot) {
        if (mot->endTime() < time) {
            comp.push_back(*mot);
            return true;
        }
        return false;
    }), completed.end());
    return comp;
}


Motion::Motion(MotionType type, double startTime)
    : type{type}
{
    timeSpan.setMin(startTime);
}

//void Motion::changeType(MotionType newType)
//{
//    cout << "Why were we changing type???" << endl;
//    type = newType;
//}

void Motion::setEndTime(double endTime)
{
    timeSpan.setMax(endTime);
}

void Motion::fixRate()
{
    if (timeSpan.maxVal > timeSpan.minVal) {
        rate = (end-start)*(1.0/timeSpan.size());
    }
    else if (timeSpan.maxVal == timeSpan.minVal) {
        rate = end-start;
    }
    else {
        throw runtime_error("Motion::fixRate issue");
    }
}

void Motion::set(MotionType setType, double startTime, double endTime, Vec2d startPos, Vec2d endPos)
{
    type = setType;
    timeSpan.minVal = startTime;
    if (endTime) {
        timeSpan.maxVal = endTime;
    }
    start = startPos;
    end = endPos;
    fixRate();
}

void Motion::set(MotionType setType, double startTime, double endTime, double startVal, double endVal)
{
    set(setType, startTime, endTime, Vec2d{startVal, 0}, Vec2d{endVal, 0});
}

void Motion::initSleep(double time)
{
    set(MotionType::Sleep, time, std::numeric_limits<double>::max(), 0, 0);
}

void Motion::initBullet(double time, double duration, Vec2d startPos, Vec2d endPos)
{
    set(MotionType::Bullet, time, time+duration, startPos, endPos);
}

void Motion::initMove(double time, double duration, Vec2d startPos, Vec2d endPos)
{
    set(MotionType::Translate, time, time+duration, startPos, endPos);
}

void Motion::initTurn(double time, double duration, double startHeading, double endHeading)
{
    set(MotionType::Rotate, time, time+duration, startHeading, endHeading);
}

void Motion::initScan(double time, double duration, double startScan, double endScan)
{
    set(MotionType::Scan, time, time+duration, startScan, endScan);
}

void Motion::initFire(double time, double duration, int startBullets)
{
    set(MotionType::Fire, time, time+duration, startBullets, (startBullets > 0) ? startBullets-1 : 0);
}

void Motion::initHealth(double time, double duration, double startHealth, double endHealth)
{
    set(MotionType::Health, time, time+duration, startHealth, endHealth);
}

void Motion::initPowerUp(double time, double duration, double startBullets, double endBullets)
{
    set(MotionType::PowerUp, time, time+duration, startBullets, endBullets);
}

void Motion::initResign(double time, double duration)
{
    set(MotionType::Resign, time, time+duration, 0, 0);
}

void Motion::complete(double time)
{
    if (timeSpan.maxVal == numeric_limits<double>::max())
    {
        timeSpan.maxVal = time;
    }
    else if (time != timeSpan.maxVal) {
        throw runtime_error("Motion::complete issue");
    }
}

void Motion::interrupt(double time, Vec2d pos)
{
    switch (type) {
    case MotionType::Bullet:
    case MotionType::Translate:
        timeSpan.maxVal = time;
        end = pos;
        fixRate();
        break;
    default:
        end = curr(time);
        timeSpan.maxVal = time;
        fixRate();
        break;
    }
}

//void Motion::interruptMove(double time, Vec2d pos)
//{
//    switch (type) {
//    case MotionType::Bullet:
//    case MotionType::Translate:
//        timeSpan.maxVal = time;
//        end = pos;
//        fixRate();
//        break;
//    default:
//        throw runtime_error("Unhandled interruptMove issue");
//    }
//}

//void Motion::interruptNonMove(double time)
//{
//    switch (type) {
//    case MotionType::Bullet:
//    case MotionType::Translate:
//        throw runtime_error("interruptNonMove issue");
//    default:
//        end = curr(time);
//        timeSpan.maxVal = time;
//        fixRate();
//        break;
//    }
//}

void Motion::invalidate()
{
    type = MotionType::Unknown;
}

// limit range to from 0 to (almost) 2 PI
double normalizeAngle(double angle)
{
    while (angle < 0)
    {
        angle += 2*M_PI;
    }

    while (angle >= 2*M_PI)
    {
        angle -= 2*M_PI;
    }

    return angle;
}

void Motion::apply(ActorState& actor, double time, bool forward) const
{
    if (!timeSpan.contains(time)) {
        cout << "Time Span ApplyOutOfRange" << endl;
    }

    if (forward) {
        if (time == timeSpan.maxVal) {
            switch (type) {
            case MotionType::Sleep:
                cout << "Someone woke us up" << endl;
                break;
            case MotionType::Resign:
                cout << "Understand this resign state" << endl;
                break;
            case MotionType::Unknown:
                throw runtime_error("MotionType::Unknown in apply1");
            case MotionType::Translate:
            case MotionType::Bullet:
                actor.position = end;  // do this
                actor.velocity = Vec2d{0,0};  // do this
                break;
            case MotionType::Rotate:
                actor.direction = normalizeAngle(end.x); // do this
                break;
            case MotionType::Scan:
                break;
            case MotionType::Fire:
                actor.bulletCount = end.x;
                break;
            case MotionType::Health:
                actor.health = end.x;
                break;
            case MotionType::PowerUp:
                actor.bulletCount = end.x;
                break;
            }
        }
        else if (time == timeSpan.minVal) {  // will not hit this for zero duration events when going forward
            switch (type) {
            case MotionType::Unknown:
                throw runtime_error("MotionType::Unknown in apply2");
            case MotionType::Translate:
            case MotionType::Bullet:
                actor.position = start;  // just assert this ?
                actor.velocity = rate;  // do this
                break;
            case MotionType::Rotate:
                actor.direction = normalizeAngle(start.x);  // just assert this ?
                break;
            case MotionType::Scan:
                break;
            case MotionType::Fire:
                actor.bulletCount = start.x;
                break;
            case MotionType::Sleep:
                // goodnight.... going to sleep
                break;
            case MotionType::Resign:
                // resigning...
                break;
            case MotionType::Health:
                actor.health = start.x;  // just assert this ?
                break;
            case MotionType::PowerUp:
                actor.bulletCount = start.x;  // just assert this ?
                break;
            }
        }
        else {  // only for non-zero duration events
            switch (type) {
            case MotionType::Unknown:
                throw runtime_error("MotionType::Unknown in apply3");
            case MotionType::Sleep:
                //cout << "ZZZZzzzz...." << endl;
                break;
            case MotionType::Resign:
                //cout << "Resigning...." << endl;
                break;
            case MotionType::Translate:
            case MotionType::Bullet:
                actor.position = curr(time);  // do this
                actor.velocity = rate;  // assert this?
                break;
            case MotionType::Rotate:
                actor.direction = normalizeAngle(curr(time).x); // do this
                break;
            case MotionType::Scan:
                break;
            case MotionType::Fire:
                // actor.bulletCount = curr(time).x; // TODO make sure this is ok to not do...
                break;
            case MotionType::Health:
                actor.health = curr(time).x; // do this
                break;
            case MotionType::PowerUp:
                actor.bulletCount = curr(time).x; // ???
                cout << "This is probably a problem" << endl;
                break;
            }
        }
    }
    else {  // We are going backwards in time...
        if (time == timeSpan.minVal) {
            switch (type) {
            case MotionType::Unknown:
                throw runtime_error("MotionType::Unknown in apply4");
            case MotionType::Sleep:
                //cout << "Anything to do here for MotionType::Sleep4??" << endl;
                break;
            case MotionType::Resign:
                //cout << "Anything to do here for MotionType::Resign???" << endl;
                break;
            case MotionType::Translate:
            case MotionType::Bullet:
                actor.position = start;  // do this
                actor.velocity = Vec2d{0,0};  // do this
                break;
            case MotionType::Rotate:
                actor.direction = normalizeAngle(start.x);  // do this
                break;
            case MotionType::Scan:
                break;
            case MotionType::Fire:
                actor.bulletCount = start.x;
                break;
            case MotionType::Health:
                actor.health = start.x;
                break;
            case MotionType::PowerUp:
                actor.bulletCount = start.x;
                break;
            }
        }
        else if (time == timeSpan.maxVal) { // will not hit this for zero duration events when going backward
            switch (type) {
            case MotionType::Unknown:
                throw runtime_error("MotionType::Unknown in apply5");
            case MotionType::Sleep:
                //cout << "Anything to do here for MotionType::Sleep5??" << endl;
                break;
            case MotionType::Resign:
                //cout << "Anything to do here for MotionType::Resign5???" << endl;
                break;
            case MotionType::Translate:
            case MotionType::Bullet:
                actor.position = end;  // just assert this
                actor.velocity = rate;  // do this
                break;
            case MotionType::Rotate:
                actor.direction = normalizeAngle(end.x); // do this
                break;
            case MotionType::Scan:
                break;
            case MotionType::Fire:
                actor.bulletCount = end.x;
                break;
            case MotionType::Health:
                actor.health = end.x; // do this
                break;
            case MotionType::PowerUp:
                actor.bulletCount = end.x;  // do this
                break;
            }
        }
        else {  // only for non-zero duration events
            switch (type) {
            case MotionType::Unknown:
                throw runtime_error("MotionType::Unknown in apply6");
            case MotionType::Sleep:
                //cout << "....zzzzZZZZ" << endl;
                break;
            case MotionType::Resign:
                //cout << "....unresigning?" << endl;
                break;
            case MotionType::Translate:
            case MotionType::Bullet:
                actor.position = curr(time);  // do this
                actor.velocity = rate;  // assert this?
                break;
            case MotionType::Rotate:
                actor.direction = normalizeAngle(curr(time).x); // do this
                break;
            case MotionType::Scan:
                break;
            case MotionType::Fire:
                // actor.bulletCount = curr(time).x; // TODO: ok to leave blank?
                break;
            case MotionType::Health:
                actor.health = curr(time).x;
                break;
            case MotionType::PowerUp:
                actor.bulletCount = curr(time).x;
                break;
            }
        }
    }
}


/*
void Motion::beginTranslate(Vec2d pos, Vec2d vel, double time)
{
    type = MotionType::Translate;
    start = pos;
    end = Vec2d{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
    rate = vel;
    timeSpan.minVal = time;
    timeSpan.maxVal = std::numeric_limits<double>::infinity();

}

void Motion::endTranslate(Vec2d pos, double time)
{
    end = pos;
    timeSpan.maxVal = time;
}


void Motion::beginRotate(double startAngle, double turnAngle, double turnTime, double time)
{
    type = MotionType::Rotate;
    start = Vec2d{startAngle, 0};
    end = Vec2d{startAngle + turnAngle, 0};
    rate = Vec2d{turnAngle/turnTime, 0};
    timeSpan.minVal = time;
    timeSpan.maxVal = time + turnTime;
}

void Motion::endRotate(double endAngle, double time)
{
    end = Vec2d{endAngle, 0};
    timeSpan.maxVal = time;
}

void Motion::beginScan(double startAngle, double scanAngle, double srate, double time)
{
    type = MotionType::Scan;
    start = Vec2d{startAngle, 0};
    end = Vec2d{startAngle + scanAngle, 0};
    rate = Vec2d{srate, 0};
    timeSpan.minVal = time;
    timeSpan.maxVal = time + scanAngle/srate;
}

void Motion::endScan(double endAngle, double time)
{
    end = Vec2d{endAngle, 0};
    timeSpan.maxVal = time;
}

void Motion::changeHealth(double startHealth, double endHealth, double time)
{
    type = MotionType::Health;
    start = Vec2d{startHealth, 0};
    end = Vec2d{endHealth, 0};
    rate = Vec2d{0, 0};
    timeSpan.minVal = time;
    timeSpan.maxVal = time;
}

void Motion::beginFire(double timeToFire, double time)
{
    type = MotionType::Fire;
    start = Vec2d{0, 0};
    end = Vec2d{1, 0};
    rate = Vec2d{1.0/timeToFire, 0};
    timeSpan.minVal = time;
    timeSpan.maxVal = time + timeToFire;
}

void Motion::endFire(double time)
{
    timeSpan.maxVal = time;
}
*/

void transform(vector<Vec2d>& pts, double scale, double angle, Vec2d offset)
{
    for (Vec2d& p : pts)
    {
        p.scale(scale);
        p.rotate(angle);
        p.translate(offset);
    }
}

void transform(vector<Vec2d>& pts, Vec2d offset)
{
    for (Vec2d& p : pts)
    {
        p.translate(offset);
    }
}

Food::Food(Vec2d pos, double radius, double createTime)
    : location{pos}, radius{radius}, createTime{createTime}
{

}

void Food::finish(double time)
{
    deleteTime = time;
}


Bullet::Bullet(std::shared_ptr<Shooter> shooterArg, double startTime, Vec2d pos, double direction, Vec2d vel) :
    shooter{std::move(shooterArg)}
{
    state.init(pos, vel, direction, 0, 0);

    motion = shooter->motionTracker.create(MotionType::Bullet, startTime);

    // assume bullet won't travel for longer than 1 minute!
    motion->initBullet(startTime, 60, pos, pos + vel*60);
    motion->apply(state, startTime, true);

    bulletCount++;

    static vector<Vec2d> origPts = {{ 0, 1 }, { 0, -1 }, { -20, -1 }, { -20, 1 }};

    pts = origPts;

    transform(pts, 1, direction, {0, 0});
}

Bullet::~Bullet()
{
    bulletCount--;
}

void Bullet::finish(Vec2d pos, double time)
{
    motion->interrupt(time, pos);
    motion->apply(state, time, true);
    shooter->recordBullet(motion);
}

#ifndef NOGRAPHICS

int thrustColor(int i)
{
    return 0x8888FF - (0x111100 * i);
}


int colorD(double r, double g, double b);

void Food::draw(mssm::Graphics& g) const
{
    g.ellipseC(location, radius*2.0, radius*2.0, GREEN, GREEN);
}

void Bullet::draw(mssm::Graphics& g) const
{
    auto tmpPts = pts;
    transform(tmpPts, state.getPosition());
    g.polygon(tmpPts, 0x008888, 0x008888);
}

void drawBot(mssm::Graphics& g, double diameter, int primaryColor, double currentTime, ActorState const & state, Motion const* motion, const std::string& name, int kills)
{
    // this gives a figure with diameter 36, so it will need to be scaled by the
    // bot diameter
    static Vec2d AR {  1, 18 };
    static Vec2d BR {  3, 12 };
    //static Vec2d CR {  7,  8 };
    static Vec2d DR {  5,  0 };
    static Vec2d ER { 11,  2 };
    static Vec2d FR { 13,  2 };
    static Vec2d GR {  9, -2 };
    static Vec2d HR { 15, -2 };
    static Vec2d IR {  9, -8 };
    static Vec2d JR { 15, -8 };
    static Vec2d KR { 11,-12 };
    static Vec2d LR { 13,-12 };
    static Vec2d MR {  5, -6 };
    static Vec2d NR {  3, -8 };
    static Vec2d OR {  1, -4 };
    static Vec2d P  {  0,-10 };
    static Vec2d AL {- 1, 18 };
    static Vec2d BL {- 3, 12 };
    //static Vec2d CL {- 7,  8 };
    static Vec2d DL {- 5,  0 };
    static Vec2d EL {-11,  2 };
    static Vec2d FL {-13,  2 };
    static Vec2d GL {- 9, -2 };
    static Vec2d HL {-15, -2 };
    static Vec2d IL {- 9, -8 };
    static Vec2d JL {-15, -8 };
    static Vec2d KL {-11,-12 };
    static Vec2d LL {-13,-12 };
    static Vec2d ML {- 5, -6 };
    static Vec2d NL {- 3, -8 };
    static Vec2d OL {- 1, -4 };

    vector<Vec2d> body     { AL, AR, BR, DR, MR, NR, P, NL, ML, DL, BL };
    vector<Vec2d> rightPod { DR, GR, ER, FR, HR, JR, LR, KR, IR, MR };
    vector<Vec2d> leftPod  { DL, GL, EL, FL, HL, JL, LL, KL, IL, ML };
    vector<Vec2d> lines    { GR, IR, DR, MR, GL, IL, DL, ML };

    double scale = diameter / 36;
    double angle = state.getDirection() - M_PI/2;

    transform(body,     scale, angle, state.getPosition());
    transform(rightPod, scale, angle, state.getPosition());
    transform(leftPod,  scale, angle, state.getPosition());



    g.polygon( body,     primaryColor, primaryColor);
    g.polygon( leftPod,  primaryColor, primaryColor);
    g.polygon( rightPod, primaryColor, primaryColor);

    transform(lines,  scale, angle, state.getPosition());

    for (size_t i=0;i<lines.size();i+=2)
    {
        g.line(lines[i], lines[i+1], 0);
    }

    if (!motion) {
        //g.text(state.getPosition() + Vec2d { 15, 25}, 15, "NOMOTION", 0xFFFFFF);
    }
    else {
        switch (motion->motionType()) {
        case MotionType::Fire:
            {
                double cmdProgress = motion->progress(currentTime);
                int effectColor = (state.getBullets() == 0) ? colorD(1-cmdProgress, 1-cmdProgress, 1-cmdProgress): colorD(1, 1-cmdProgress, 1-cmdProgress);
                vector<Vec2d> gun { AL, AR, OR, OL };
                transform(gun, scale, angle, state.getPosition());
                g.polygon( gun, effectColor, effectColor);
            }
            break;
        case MotionType::Rotate:
            {
                if (motion->rateScalar() < 0)
                {
                    vector<Vec2d> pod1 { ER, FR, HR, GR };
                    vector<Vec2d> pod2 { JL, IL, KL, LL };
                    transform(pod1, scale, angle, state.getPosition());
                    transform(pod2, scale, angle, state.getPosition());
                    g.polygon( pod1, 0x00FFFF, 0x00FFFF);
                    g.polygon( pod2, 0x00FFFF, 0x00FFFF);
                }
                else
                {
                    vector<Vec2d> pod1 { EL, FL, HL, GL };
                    vector<Vec2d> pod2 { JR, IR, KR, LR };
                    transform(pod1, scale, angle, state.getPosition());
                    transform(pod2, scale, angle, state.getPosition());
                    g.polygon( pod1, 0x00FFFF, 0x00FFFF);
                    g.polygon( pod2, 0x00FFFF, 0x00FFFF);
                }

            }
            break;
        case MotionType::Translate:
            {
                vector<Vec2d> thrust   { ML, NL, P, NR, MR };

                Vec2d thrustVector { 2, 0 };

                thrustVector.rotate(state.getDirection());
                thrustVector.scale(-1);

                transform(thrust, scale, angle, state.getPosition());

                for (int i=0;i<8;i++)
                {
                    transform(thrust, thrustVector);
                    g.polyline(thrust, thrustColor(i));
                }
            }
            break;
        case MotionType::Scan:
            {
                double scanlen = motion->curr(currentTime).x - motion->startScalar();
                auto position = state.getPosition();
                double scanDiameter = diameter*5;
                double startAngle = motion->startScalar();
                g.pie(position - Vec2d{scanDiameter/2, scanDiameter/2}, scanDiameter, scanDiameter, startAngle, scanlen, mssm::BLUE);
            }
            break;
        case MotionType::Health:
        case MotionType::Bullet:
        case MotionType::PowerUp:
            break;
        case MotionType::Unknown:
            g.text(state.getPosition() + Vec2d { 15, 25}, 15, "UNKNOWNMOTIONTYPE", 0xFFFFFF);
            break;
        case MotionType::Sleep:
            g.text(state.getPosition() + Vec2d { 15, 25}, 15, "ZZZzzz...", 0xFFFFFF);
            break;
        case MotionType::Resign:
            g.text(state.getPosition() + Vec2d { 15, 25}, 15, "Resigning...", 0xFFFFFF);
            break;
        }
    }

    if (state.getHealth() < 5)
    {
        vector<Vec2d> damage { HL, GL, IL, JL };
        transform(damage, scale, angle, state.getPosition());
        g.polygon( damage, RED, RED);
    }

    if (state.getHealth() < 4)
    {
        vector<Vec2d> damage { HR, GR, IR, JR };
        transform(damage, scale, angle, state.getPosition());
        g.polygon( damage, RED, RED);
    }

    if (state.getHealth() < 3)
    {
        vector<Vec2d> damage { AL, AR, BR, BL };
        transform(damage, scale, angle, state.getPosition());
        g.polygon( damage, RED, RED);
    }

    if (state.getHealth() < 2)
    {
        vector<Vec2d> damage { ML, NL, P, NR, MR };
        transform(damage, scale, angle, state.getPosition());
        g.polygon( damage, RED, RED);
    }

    if ((!motion || motion->motionType() != MotionType::Fire) && state.getBullets() == 0) {
        vector<Vec2d> gun { AL, AR, OR, OL };
        transform(gun, scale, angle, state.getPosition());
        g.polygon( gun, BLACK, BLACK);
    }

    g.text(state.getPosition() + Vec2d { 15, 15}, 15, name, 0xFFFFFF);


//        if (showCollideDir)
//        {
//            // draw last collision angle
//            Vec2d collideVector{ diameter, 0 };
//            collideVector.rotate(getDirection());
//            collideVector.rotate(collideAngle);
//            g.line(position + collideVector*.4, position + collideVector*0.6, 0xFF8888);
//        }




    if (kills) {
        g.text(state.getPosition() + Vec2d { 15, -15}, 15, to_string(kills), 0xFFFFFF);
    }

    //g.text(state.getPosition() + Vec2d { 15, -25}, 15, to_string(state.getBullets()), 0xFFFFFF);
}

void Bot::draw(mssm::Graphics& g, double currentTime) const
{
    int primaryColor = color;

    if (!currentCommand) {
        primaryColor = 0x888888;
    }

    drawBot(g, diameter, primaryColor, currentTime, state, currentMotion, name(), kills);

    //bool showCollideDir = false;

    if (collideEffect.contains(currentTime))
    {
        //showCollideDir = true;
        g.ellipseC(state.getPosition(), diameter, diameter, 0x00FFFF);
    }

    if (shotEffect.contains(currentTime))
    {
        //showCollideDir = true;

        double explodeDiameter = interpolate(shotEffect.parameterize(currentTime), diameter, diameter*2);

        for (double d = 1.0; d <= explodeDiameter; d += 4.0)
        {
            g.ellipseC(state.getPosition(), d, d, state.getHealth() == 0 ? 0xFF0000 : 0xFFFF00);
        }
    }
}
#endif // NOGRAPHICS


void Bullet::updatePosition(double currentTime)
{
    motion->apply(state, currentTime, true);
}

std::string Bullet::shooterName() const
{
    return shooter->name();
}

Shooter::~Shooter()
{
}

Bot::Bot(World *world, ServerBotAI *brain, size_t id, bool dummy)
{
    currentMotion = nullptr;
   // bulletCount = initBullets;
    botCount++;

    inCommWait = false;

    if (id == 0) {
        // assertion
        // bot id must not be zero (one reason is that in bot history, 0 id is used to describe the world (food and walls, for example), not bots.
        throw runtime_error("bot id not zero error!");
    }

    botId = id;

    this->world = world;
    this->brain = brain;

    color = 0x000000;
    diameter = 40;

    collideEffect.nothing();
    shotEffect.nothing();

    addEvent(BotEventType::MoveComplete, world->currentTime());
}

Bot::~Bot()
{
    world->releaseBotId(botId);
    botCount--;
    delete brain;
}

// return true if:  bot is moving, and if the collisionDir is on the
// side towards which the bot is moving (takes negative speeds into account)
bool Bot::isBlockingCollision(double relativeCollisionDir)
{
    if (!isMoving()) {
        return false;
    }

    double speed = state.speed();

    //cout << "Bot Speed == " << speed << endl;

    if (speed > 0) {
        return fabs(relativeCollisionDir) < M_PI/2.0;
    }
    else {
        return fabs(relativeCollisionDir) > M_PI/2.0;
    }
}

BotEvent& Bot::addEvent(BotEventType eventType, double eventTime)
{
    switch (eventType) {
    case BotEventType::PowerUp:
    case BotEventType::HitByBot:
    case BotEventType::HitByBullet:
        lastEventWasIgnorable = true;
        break;
    default:
        lastEventWasIgnorable = false;
        break;
    }
    eventToSend = std::make_unique<BotEvent>(eventType, eventTime, state.getHealth(), state.getBullets());
    return *eventToSend;
}

void Bot::initializePosition(Vec2d pos, double dir, int maxHealth, int initBullets)
{
    state.init(pos, {0,0}, dir, maxHealth, initBullets);
    initialState = state;
}

void Bot::shotFireEvt(std::shared_ptr<Bot> thisBot, double eventTime)
{
    world->fire(thisBot, state.getPosition(), state.getDirection(), radius()*0.5);
    /*BotEvent& evt = */ addEvent(BotEventType::FireComplete, eventTime);
    //evt.collisionAngle = state.getDirection();
}

void Bot::noBulletEvt(double eventTime)
{
    addEvent(BotEventType::NoBullets, eventTime);
}

void Bot::moveCompleteEvt(double eventTime)
{
    BotEvent& evt = addEvent(BotEventType::MoveComplete, eventTime);
    evt.travelDistance = currentCommand->distanceMoved(eventTime);
}

void Bot::turnCompleteEvt(double eventTime)
{
    BotEvent& evt = addEvent(BotEventType::TurnComplete, eventTime);
    evt.angleTurned = currentCommand->angleTurned(eventTime);
}

void Bot::doScanEvt(double eventTime, double fov)
{
    BotEvent& evt = addEvent(BotEventType::ScanComplete, eventTime);
    evt.collisionAngle = state.getDirection();
    evt.angleTurned = fov;    
    evt.scanData = world->look(fov, state.getPosition(), state.getDirection());
}

void Bot::resignEvt(double eventTime)
{
    std::stringstream ss;
    ss << name() << " Resigned";
    world->message(ss.str(), 0xFFFF00);
    applyDamage(eventTime, true);
    cout << "Should we do more?" << endl;
//    BotEvent& evt = addEvent(BotEventType::HitByBullet, eventTime);
//    evt.collisionAngle = negateAngle(subtractAngles(bulletAngle, state.getDirection()));
//    evt.angleTurned = currentCommand->angleTurned(eventTime);
//    evt.travelDistance = currentCommand->distanceMoved(eventTime);
}

void Bot::gotFoodEvt(double eventTime, double foodAngle)
{
    applyPowerUp(eventTime);
    BotEvent& evt = addEvent(BotEventType::PowerUp, eventTime);
    evt.collisionAngle = foodAngle;
    evt.angleTurned = currentCommand->angleTurned(eventTime);
    evt.travelDistance = currentCommand->distanceMoved(eventTime);
}

void Bot::applyPowerUp(double eventTime)
{
    Motion* powerUp = motionTracker.create(MotionType::PowerUp, eventTime);
    int numBullets = state.getBullets();
    powerUp->initPowerUp(eventTime, 0, numBullets, numBullets+powerUpBulletInc);
    powerUp->apply(state, eventTime, true);
    motionTracker.complete(powerUp);
}


void Bot::applyDamage(double eventTime, bool instaKill)
{
    Motion* shot = motionTracker.create(MotionType::Health, eventTime);
    shot->initHealth(eventTime, 0, state.getHealth(), instaKill ? 0 : state.getHealth()-1);
    shot->apply(state, eventTime, true);
    motionTracker.complete(shot);

    shotEffect.minVal = eventTime;
    shotEffect.maxVal = eventTime + ((state.getHealth() == 0) ? shotEffectTime*2 : shotEffectTime);
}


void Bot::gotShotEvt(double eventTime, std::shared_ptr<Shooter> shooter, double bulletAngle)
{
    if (state.getHealth() == 0)
    {
        return;
    }

    std::stringstream ss;
    if (state.getHealth() == 1)
    {
        shooter->recordKill();
        ss << name() << " Killed by " << shooter->name();
    }
    else
    {
        shooter->recordStrike();
        ss << name() << " Shot by " << shooter->name();
    }
    world->message(ss.str(), 0xFFFF00);

    applyDamage(eventTime, false);

    BotEvent& evt = addEvent(BotEventType::HitByBullet, eventTime);
    evt.collisionAngle = negateAngle(subtractAngles(bulletAngle, state.getDirection()));
    evt.angleTurned = currentCommand->angleTurned(eventTime);
    evt.travelDistance = currentCommand->distanceMoved(eventTime);
}

void Bot::hitWallEvt(double eventTime, double wallNormalAngle)
{
    collideEffect.minVal = eventTime;
    collideEffect.maxVal = eventTime + 0.5;

    BotEvent& evt = addEvent(BotEventType::MoveBlockedByWall, eventTime);
    evt.collisionAngle = subtractAngles(negateAngle(wallNormalAngle), state.getDirection());
    evt.travelDistance = currentCommand->distanceMoved(eventTime);
}

void Bot::hitBotEvt(double eventTime, double otherBotDirection)
{
    collideEffect.minVal = eventTime;
    collideEffect.maxVal = eventTime + 0.5;

    BotEvent& evt = addEvent(BotEventType::MoveBlockedByBot, eventTime);
    evt.collisionAngle = subtractAngles(otherBotDirection, state.getDirection());
    evt.angleTurned = currentCommand->angleTurned(eventTime);
    evt.travelDistance = currentCommand->distanceMoved(eventTime);
}

void Bot::hitByBotEvt(double eventTime, double otherBotDirection)
{
    collideEffect.minVal = eventTime;
    collideEffect.maxVal = eventTime + 0.5;

    BotEvent& evt = addEvent(BotEventType::HitByBot, eventTime);
    evt.collisionAngle = subtractAngles(otherBotDirection, state.getDirection());
    evt.angleTurned = currentCommand->angleTurned(eventTime);
    evt.travelDistance = currentCommand->distanceMoved(eventTime);
}

void Bot::setDisconnected()
{
    brain->setDisconnected();
}

void Bot::beginCommandSub(std::shared_ptr<Bot> thisBot, std::unique_ptr<ServerBotCmd> cmd)
{
    if (currentCommand) {
        // TODO: think more
        // for prematurely terminated turn/fire/scan caused by getting hit????
        // make sure we don't hit this for move commands??
        //cout << "   Finishing (interrupt = true) current command: " << currentCommand->name() << endl;
        currentCommand->finishCmd(thisBot, cmd->startTime(), true, getPos());
    }

    currentCommand = std::move(cmd);

    //std::cout << "   Now currentCommand: " << currentCommand->name() << endl;
    currentCommand->beginCmd(this, currentCommand->startTime());

    if (currentMotion) {
        //std::cout << "   Now currentMotion: ";
        //currentMotion->save(cout);
        //cout << endl;
    }
    else {
        //std::cout << "   Now currentMotion = null" << endl;
    }
}

void Bot::updateCommand(double currentTime)
{
    if (currentCommand) {
        currentCommand->updateCmd(this, currentTime);
    }
    else {
        std::cout << "currentCommand is Null!" << endl;
    }
}

void Bot::finishCommand(std::shared_ptr<Bot> thisBot, double currentTime, bool interrupted, Vec2d interruptPos)
{
    currentCommand->finishCmd(thisBot, currentTime, interrupted, interruptPos);
    currentCommand.reset();
}

void Bot::newCurrentMotion(double time)
{
    currentMotion = motionTracker.create(MotionType::Unknown, time);
}

void Bot::completeCurrentMotion(double currentTime, bool interrupted, Vec2d interruptPos)
{
    if (interrupted) {
        currentMotion->interrupt(currentTime, interruptPos);
    }
    else {
        currentMotion->complete(currentTime);
    }
    currentMotion->apply(state, currentTime, true);

    motionTracker.complete(currentMotion);
    currentMotion = nullptr;
}

void Shooter::recordBullet(Motion* bulletMotion)
{
    //bulletMotion->changeType(MotionType::Bullet);
    motionTracker.complete(bulletMotion);
}

bool Bot::beginCommand(std::shared_ptr<Bot> thisBot, double currentTime, BotCmd cmd)
{
    constexpr bool debug = false;

    switch (cmd.cmd)
    {
    case BotCmdType::Resign:
        if (debug) { std::cout << "  Bot: " << botId << " Resigned" << endl; }
        beginCommandSub(thisBot, make_unique<ServerCmdResign>(currentTime));
        break;
    case BotCmdType::Fire:
        if (debug) { std::cout << "  Bot: " << botId << " gave command: Fire" << endl; }
        beginCommandSub(thisBot, make_unique<ServerCmdFire>(currentTime, state.getBullets()));
        //bulletCount = max(0, bulletCount-1);
        break;
    case BotCmdType::Scan:
        if (debug) { std::cout << "  Bot: " << botId << " gave command: Look " << cmd.arg1 << endl; }
        beginCommandSub(thisBot, make_unique<ServerCmdScan>(currentTime, cmd.arg1));
        break;
    case BotCmdType::Move:
        if (debug) { std::cout << "  Bot: " << botId << " gave command: Move " << cmd.arg1 << " " << cmd.arg2 << endl; }
        beginCommandSub(thisBot, make_unique<ServerCmdMove>(currentTime, state.getPosition(), state.getDirection(), cmd.arg1, cmd.arg2));
        break;
    case BotCmdType::Turn:
        if (debug) { std::cout << "  Bot: " << botId << " gave command: Turn " << cmd.arg1 << endl; }
        beginCommandSub(thisBot, make_unique<ServerCmdTurn>(currentTime, state.getDirection(), cmd.arg1));
        break;
    case BotCmdType::Ignore:
        if (debug) { std::cout << "  Bot: " << botId << " gave command: Ignore " << cmd.arg1 << endl; }
        if (lastEventWasIgnorable) {
            //cout << "They ignored the event... that is their right" << endl;
            return false;
        }
        else {
            //cout << "Someone chose to go to sleep by ignoring a non-ignorable event" << endl;
            beginCommandSub(thisBot, make_unique<ServerCmdAsleep>(currentTime));
            return true;
        }
    case BotCmdType::Asleep:
        if (debug) { std::cout << "  Bot: " << botId << " gave command: Asleep " << cmd.arg1 << endl; }
        throw runtime_error("Asleep is created by Ignore cmd...  How did we get here?");
    }

    return true;
}



void Bot::transmitEvents(double currentTime)
{
    bool debug = false;

    if (eventToSend)
    {
        if (debug)
        {

            if (currentMotion) {
                std::cout << " During currentMotion: ";
                currentMotion->save(cout);
                cout << endl;
            }
            if (currentCommand) {
                std::cout << " With currentCommand: " << currentCommand->name() << endl;
            }
            std::cout << " Bot " << botId << " handling event: ";
            eventToSend->write(std::cout);
        }

        brain->handleEvents(*eventToSend, currentTime);

        eventToSend.reset();
    }
}

int Bot::commWaitTimeMs(std::chrono::time_point<std::chrono::system_clock> nowTime) const
{
    if (!inCommWait) {
        return 0;
    }
    const auto waitTime = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - waitStart);
    //cout << waitTime.count() << " is the wait time" << endl;
    return static_cast<int>(waitTime.count());
}

// returns true if new commmand was received If it does, the world must call updateBotEvents!
bool Bot::receiveNewCommand(std::shared_ptr<Bot> thisBot, double currentTime)
{
    BotCmd cmd;

    if (!brain->getCmd(cmd)) {

        // brain doesn't have a command for us

        if (!inCommWait) {
            //cout << "Bot: " << name() << " Is now waiting..\n.";
            inCommWait = true;
            waitStart = std::chrono::system_clock::now();
        }
        else {
            //auto end = std::chrono::system_clock::now();
            //std::chrono::duration<double> diff = end-waitStart;
            //cout << "Bot: Wait time = " << diff.count() << endl;
        }
        return false;
    }

    if (inCommWait) {
        //cout << "Bot: " << name() << " Your wait is over!\n";
        inCommWait = false;
    }

    //cout << "receiveNewCommand: " << (int)cmd << endl;

    return beginCommand(thisBot, currentTime, cmd);
}


bool Bot::hasOngoingVisual(double currentTime) const
{
    return shotEffect.contains(currentTime);
}






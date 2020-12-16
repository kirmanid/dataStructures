#define _USE_MATH_DEFINES
#include "vec2d.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <iostream>

using namespace std;

double Vec2d::magnitude() const
{
    return sqrt(x*x + y*y);
}

void Vec2d::scale(double s)
{
    x *= s;
    y *= s;
}

void Vec2d::rotate(double radians)
{
    *this = { x * cos(radians) - y * sin(radians), x * sin(radians) + y * cos(radians) };
}

void Vec2d::translate(Vec2d offset)
{
    x += offset.x;
    y += offset.y;
}

bool Vec2d::equals(const Vec2d& other, double threshold) const
{
    return fabs(x - other.x) <= threshold && fabs(y - other.y) <= threshold;
}

Vec2d operator-(Vec2d p1, Vec2d p2)
{
    return Vec2d {p1.x - p2.x, p1.y - p2.y };
}

Vec2d operator+(Vec2d p1, Vec2d p2)
{
    return Vec2d {p1.x + p2.x, p1.y + p2.y };
}

Vec2d operator*(Vec2d v, double s)
{
    return Vec2d { v.x * s, v.y * s };
}

Vec2d operator*(double s, Vec2d v)
{
    return Vec2d { v.x * s, v.y * s };
}

double wallCollisionTime(Vec2d pos, Vec2d vel, double minx, double maxx, double miny, double maxy, double radius, Vec2d& collidePos, double& wallNormal)
{
    Vec2d wallNormalVec { 0, 0};

    if ((pos.x < minx + radius) ||
        (pos.x > maxx - radius) ||
        (pos.y < miny + radius) ||
        (pos.y > maxy - radius))
    {
        collidePos = pos;

        if (pos.x < minx + radius)
        {
            wallNormalVec.x = 1;
            collidePos.x = minx + radius;
        }

        if (pos.x > maxx - radius)
        {
            wallNormalVec.x = -1;
            collidePos.x = maxx - radius;
        }

        if (pos.y < miny + radius)
        {
            wallNormalVec.y = 1;
            collidePos.y = miny + radius;
        }

        if (pos.y > maxy - radius)
        {
            wallNormalVec.y = -1;
            collidePos.y = maxy - radius;
        }

        wallNormal = atan2(wallNormalVec.y, wallNormalVec.x);

        return 0;
    }

    double xtime = std::numeric_limits<double>::max();
    double ytime = std::numeric_limits<double>::max();

    if (vel.x < 0)
    {
        // check min x
        double wallDist = minx + radius - pos.x;  // it must be that wallDist <= 0
        xtime = wallDist / vel.x;                 // therefore xtime >= 0
        wallNormalVec.x = 1;
    }
    else if (vel.x > 0)
    {
        // check max x
        double wallDist = maxx - radius - pos.x;  // it must be that wallDist >= 0
        xtime = wallDist / vel.x;                 // therefore xtime >= 0
        wallNormalVec.x = -1;
    }
    // else xtime still is max double and wallNormalVec.x = 0

    if (vel.y < 0)
    {
        // check min y
        double wallDist = miny + radius - pos.y; // it must be that wallDist <= 0
        ytime = wallDist / vel.y;                // therefore xtime >= 0
        wallNormalVec.y = 1;
    }
    else if (vel.y > 0)
    {
        double wallDist = maxy - radius - pos.y; // it must be that wallDist >= 0
        ytime = wallDist / vel.y;                // therefore xtime >= 0
        wallNormalVec.y = -1;
    }
    // else ytime still is max double and wallNormalVec.y = 0

    double collideTime;

    if (xtime < ytime)
    {
        // x collision will be first, so kill the y normal
        wallNormalVec.y = 0;
        collideTime = xtime;
    }
    else if (ytime < xtime)
    {
        // y collision will be first, so kill the x normal
        wallNormalVec.x = 0;
        collideTime = ytime;
    }
    else
    {
        // simultaneous (corner) collision
        collideTime = ytime; // which also == xtime
    }

    collidePos = pos + collideTime * vel;

    if (collidePos.x < minx + radius)
    {
        collidePos.x = minx + radius;
    }

    if (collidePos.x > maxx - radius)
    {
        collidePos.x = maxx - radius;
    }

    if (collidePos.y < miny + radius)
    {
        collidePos.y = miny + radius;
    }

    if (collidePos.y > maxy - radius)
    {
        collidePos.y = maxy - radius;
    }

    wallNormal = atan2(wallNormalVec.y, wallNormalVec.x);


    return collideTime;
}

double verticalSegmentCollisionTime(Vec2d pos, Vec2d vel, double x, double miny, double maxy, double radius, Vec2d& collidePos, double& wallNormal)
{
    Vec2d wallNormalVec { 0, 0};
    constexpr double epsilon = 0.0001;
    constexpr double epsilon2 = epsilon*epsilon;
    constexpr bool debug = false;

    bool overlapsLine = false;

    // first check to see if we are already in contact with the segment
    if ((pos.x > x - radius) && (pos.x < x + radius)) {
        // we overlap the line (but not necessarily the segment)

        if (debug) cout << "A";

        overlapsLine = true;

        if ((pos.y >= miny) && (pos.y <= maxy)) {
            double retval;

            if (debug) cout << "B";

            // a solid hit (not just an end hit)
            collidePos = pos;
            if (pos.x < x) {
                collidePos.x = x - radius;
                wallNormalVec.x = -1;
                retval = vel.x < 0 ? -1 : 0; // check escapeable
                if (debug) cout << "C";
            }
            else {
                collidePos.x = x + radius;
                wallNormalVec.x = 1;
                retval = vel.x > 0 ? -1 : 0; // check escapeable
                if (debug) cout << "D";
            }
            wallNormal = atan2(wallNormalVec.y, wallNormalVec.x);  // TODO   No need for trig
            if (debug) cout << endl;
            return retval;
        }

        // are we hitting an end?

        if (debug) cout << "E";

        Vec2d endPoint;
        Vec2d offEnd;

        bool checkEnd = false;

        if ((pos.y < miny) && (pos.y > miny - radius)) {
            // maybe in contact with the min end?
            endPoint = {x, miny};
            offEnd = {x, miny-radius};
            checkEnd = true;
            if (debug) cout << "F";
        }
        else if ((pos.y > maxy) && (pos.y < maxy + radius)) {
            // maybe in contact with the max end?
            endPoint = {x, maxy};
            offEnd = {x, maxy+radius};
            checkEnd = true;
            if (debug) cout << "G";
        }

        if (checkEnd) {
            double dist2 = (pos-endPoint).magSquared();
            if (dist2 < (radius*radius)) {
                if (debug) cout << "H";
                if (dist2 < epsilon2) {
                    collidePos = offEnd;
                    wallNormalVec = collidePos - endPoint;
                    if (debug) cout << "I";
                }
                else {
                    wallNormalVec = pos - endPoint;
                    collidePos = endPoint + radius*wallNormalVec.unit();
                    if (debug) cout << "J";
                }
                wallNormal = atan2(wallNormalVec.y, wallNormalVec.x);
                if (debug) cout << endl;
                return dot(wallNormalVec, vel) > 0 ? -1 : 0;  // check escapeable
            }
        }
    }

    // didn't start out in contact, now see if we might run into the wall in the future

    if (overlapsLine) {
        // since it overlaps the line (not segement) it can only hit an endpoint
        if (debug) cout << "K";
    }
    else {
        double wallDist;
        double collideTime;

        if (pos.x < x && vel.x > 0) {
            if (debug) cout << "L";
            // headed towards the line from the less than x side
            wallDist = x - radius - pos.x;  // it must be that wallDist >= 0
            collideTime = wallDist / vel.x;                 // therefore xtime >= 0
            wallNormalVec.x = -1;
        }
        else if (pos.x > x && vel.x < 0) {
            if (debug) cout << "M";
            // headed towards the line from the greater than x side
            wallDist = x + radius - pos.x;  // it must be that wallDist <= 0
            collideTime = wallDist / vel.x;                 // therefore xtime >= 0
            wallNormalVec.x = 1;
        }
        else {
            if (debug) cout << "N" << endl;
            // headed away from the line... we aren't going to hit it
            return -1;
        }

        collidePos = pos + collideTime * vel;

        if (collidePos.y >= miny && collidePos.y <= maxy) {
            // collidePos is within y range.  A solid hit
            wallNormal = atan2(wallNormalVec.y, wallNormalVec.x);
            if (debug) cout << "O" << endl;
            return collideTime;
        }

        if (((collidePos.y <= miny - radius) && (vel.y <= 0)) ||
            ((collidePos.y >= maxy + radius) && (vel.y >= 0)))
        {
            // clear miss
            if (debug) cout << "P" << endl;
            return -1;
        }
    }

    // check endpoint collision
    Vec2d endPoint = (pos.y < miny) ? Vec2d{x, miny} : Vec2d{x, maxy};

    double collideTime = pointCollisionTime(pos, vel, endPoint, collidePos, radius);
    if (collideTime >= 0) {
        wallNormalVec = collidePos - endPoint;
        wallNormal = atan2(wallNormalVec.y, wallNormalVec.x);
        if (debug) cout << "Q" << endl;
        return collideTime;
    }

    if (debug) cout << "R" << endl;

    // near miss on one end
    return -1;
}

double horizontalSegmentCollisionTime(Vec2d pos, Vec2d vel, double y, double minx, double maxx, double radius, Vec2d& collidePos, double& wallNormal)
{
    Vec2d wallNormalVec { 0, 0};
    constexpr double epsilon = 0.0001;
    constexpr double epsilon2 = epsilon*epsilon;
    constexpr bool debug = false;

    bool overlapsLine = false;

    // first check to see if we are already in contact with the segment
    if ((pos.y > y - radius) && (pos.y < y + radius)) {
        // we overlap the line (but not necessarily the segment)

        if (debug) cout << "A";

        overlapsLine = true;

        if ((pos.x >= minx) && (pos.x <= maxx)) {
            double retval;

            if (debug) cout << "B";

            // a solid hit (not just an end hit)
            collidePos = pos;
            if (pos.y < y) {
                collidePos.y = y - radius;
                wallNormalVec.y = -1;
                retval = vel.y < 0 ? -1 : 0; // check escapeable
                if (debug) cout << "C";
            }
            else {
                collidePos.y = y + radius;
                wallNormalVec.y = 1;
                retval = vel.y > 0 ? -1 : 0; // check escapeable
                if (debug) cout << "D";
            }
            wallNormal = atan2(wallNormalVec.y, wallNormalVec.x);  // TODO   No need for trig
            if (debug) cout << endl;
            return retval;
        }

        // are we hitting an end?

        if (debug) cout << "E";

        Vec2d endPoint;
        Vec2d offEnd;

        bool checkEnd = false;

        if ((pos.x < minx) && (pos.x > minx - radius)) {
            // maybe in contact with the min end?
            endPoint = {minx, y};
            offEnd = {minx-radius, y};
            checkEnd = true;
            if (debug) cout << "F";
        }
        else if ((pos.x > maxx) && (pos.x < maxx + radius)) {
            // maybe in contact with the max end?
            endPoint = {maxx, y};
            offEnd = {maxx+radius, y};
            checkEnd = true;
            if (debug) cout << "G";
        }

        if (checkEnd) {
            double dist2 = (pos-endPoint).magSquared();
            if (dist2 < (radius*radius)) {
                if (debug) cout << "H";
                if (dist2 < epsilon2) {
                    collidePos = offEnd;
                    wallNormalVec = collidePos - endPoint;
                    if (debug) cout << "I";
                }
                else {
                    wallNormalVec = pos - endPoint;
                    collidePos = endPoint + radius*wallNormalVec.unit();
                    if (debug) cout << "J";
                }
                wallNormal = atan2(wallNormalVec.y, wallNormalVec.x);
                if (debug) cout << endl;
                return dot(wallNormalVec, vel) > 0 ? -1 : 0;  // check escapeable
            }
        }
    }

    // didn't start out in contact, now see if we might run into the wall in the future

    if (overlapsLine) {
        // since it overlaps the line (not segement) it can only hit an endpoint
        if (debug) cout << "K";
    }
    else {
        double wallDist;
        double collideTime;

        if (pos.y < y && vel.y > 0) {
            if (debug) cout << "L";
            // headed towards the line from the less than y side
            wallDist = y - radius - pos.y;  // it must be that wallDist >= 0
            collideTime = wallDist / vel.y;                 // therefore ytime >= 0
            wallNormalVec.y = -1;
        }
        else if (pos.y > y && vel.y < 0) {
            if (debug) cout << "M";
            // headed towards the line from the greater than y side
            wallDist = y + radius - pos.y;  // it must be that wallDist <= 0
            collideTime = wallDist / vel.y;                 // therefore ytime >= 0
            wallNormalVec.y = 1;
        }
        else {
            if (debug) cout << "N" << endl;
            // headed away from the line... we aren't going to hit it
            return -1;
        }

        collidePos = pos + collideTime * vel;

        if (collidePos.x >= minx && collidePos.x <= maxx) {
            // collidePos is within y range.  A solid hit
            wallNormal = atan2(wallNormalVec.y, wallNormalVec.x);
            if (debug) cout << "O" << endl;
            return collideTime;
        }

        if (((collidePos.x <= minx - radius) && (vel.x <= 0)) ||
            ((collidePos.x >= maxx + radius) && (vel.x >= 0)))
        {
            // clear miss
            if (debug) cout << "P" << endl;
            return -1;
        }
    }

    // check endpoint collision
    Vec2d endPoint = (pos.x < minx) ? Vec2d{minx, y} : Vec2d{maxx, y};

    double collideTime = pointCollisionTime(pos, vel, endPoint, collidePos, radius);
    if (collideTime >= 0) {
        wallNormalVec = collidePos - endPoint;
        wallNormal = atan2(wallNormalVec.y, wallNormalVec.x);
        if (debug) cout << "Q" << endl;
        return collideTime;
    }

    if (debug) cout << "R" << endl;

    // near miss on one end
    return -1;
}



double pointCollisionTime(Vec2d p0, Vec2d vp, Vec2d q0, Vec2d& p, double collisionDist)
{
    double parallelThreshold = 0.0001;

    double dpx = p0.x - q0.x;
    double dpy = p0.y - q0.y;

    double a = vp.x*vp.x + vp.y*vp.y;  // non-negative
    double b = 2 * dpx * vp.x + 2 * dpy * vp.y;
    double c = dpx * dpx + dpy * dpy - collisionDist*collisionDist;

    if (a < parallelThreshold)
    {
        p = p0;
        return -1;
    }

    double sq = b*b - 4*a*c;

    if (sq < 0)
    {
        // never collides
        p = p0;
        return -1;
    }

    sq = sqrt(sq);

    double t1 = (-b+sq)/(2*a);
    double t2 = (-b-sq)/(2*a);

    if (t1 < 0 && t2 < 0)
    {
        // collision happens in the past
        p = p0;
        return std::max(t1,t2);  // return the closest time to the present (will be negative)
    }

    if (t1 < 0 && t2 >= 0)
    {
        cout << "I don't think this can happen" << endl;
        p = p0;
        return 0;
    }

    if (t2 < 0 && t1 >= 0)
    {

        p = p0;

        if (-t2 > t1)
        {
            // escapeable collision (we are embedded, but pointed so as to decrease the
            // embedding
            //cout << "Escapeable " << -t2 << " " << t1 << endl;
            return -1;
        }

        // non escapeable
        //cout << "Non Escapeable " << -t2 << " " << t1 << endl;

        return 0;
    }

    double t = std::min(t1, t2);

    p = p0 + t * vp;

    return t;
}

double collisionTime(Vec2d p0, Vec2d vp, Vec2d q0, Vec2d vq, Vec2d& p, Vec2d& q, double collisionDist)
{
    double parallelThreshold = 0.0001;

    double dvx = vp.x - vq.x;
    double dvy = vp.y - vq.y;
    double dpx = p0.x - q0.x;
    double dpy = p0.y - q0.y;

    double a = dvx*dvx + dvy*dvy;  // non-negative
    double b = 2 * dpx * dvx + 2 * dpy * dvy;
    double c = dpx * dpx + dpy * dpy - collisionDist*collisionDist;

    if (a < parallelThreshold)
    {
        p = p0;
        q = q0;
        return -1;
    }

    double sq = b*b - 4*a*c;

    if (sq < 0)
    {
        // never collides
        p = p0;
        q = q0;
        return -1;
    }

    sq = sqrt(sq);

    double t1 = (-b+sq)/(2*a);
    double t2 = (-b-sq)/(2*a);

    if (t1 < 0 && t2 < 0)
    {
        // collision happens in the past
        p = p0;
        q = q0;
        return std::max(t1,t2);  // return the closest time to the present (will be negative)
    }

    if (t1 < 0 && t2 >= 0)
    {
        cout << "I don't think this can happen" << endl;
        p = p0;
        q = q0;
        return 0;
    }

    if (t2 < 0 && t1 >= 0)
    {

        p = p0;
        q = q0;

        if (-t2 > t1)
        {
            // escapeable collision (we are embedded, but pointed so as to decrease the
            // embedding
            //cout << "Escapeable " << -t2 << " " << t1 << endl;
            return -1;
        }

        // non escapeable
        //cout << "Non Escapeable " << -t2 << " " << t1 << endl;

        return 0;
    }

    double t = std::min(t1, t2);

    p = p0 + t * vp;
    q = q0 + t * vq;

    return t;
}

double dot(const Vec2d& v1, const Vec2d& v2)
{
    return v1.x*v2.x + v1.y*v2.y;
}

double cross(const Vec2d& v1, const Vec2d& v2)
{
    return v1.x * v2.y - v1.y * v2.x;
}

double distToSegmentSquared(Vec2d p, Vec2d p1, Vec2d p2, double& t)
{
    double epsilon = 1e-20; // FIX!!
    Vec2d v1 = p2 - p1;
    double segLen = v1.magSquared();
    if (segLen < epsilon) {
        t = 0;
        return (p1-p).magSquared();
    }
    t = dot(p - p1, v1) / segLen;
    if (t <= 0) {
        t = 0;
        return (p1-p).magSquared();
    }
    if (t >= 1) {
        t = 1;
        return (p2-p).magSquared();
    }
    return (p - (v1*t + p1)).magSquared();
}

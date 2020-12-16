#ifndef RANGE2d_H
#define RANGE2d_H

#include "vec2d.h"
#include "range.h"

class Range2d
{
public:
    Range xRange;
    Range yRange;
public:
    Range2d(); // defaults to everything
    Range2d(const Vec2d& p1, const Vec2d& p2); // make a range that contains both points
    bool contains(const Vec2d& point) const;
    bool doesNotContain(const Vec2d& point) const;
    void expandToFit(const Vec2d& point);
    void nothing();
    void everything();
    void shrink();
    Vec2d corner(int i) const; // treat slice like a box, and return one of the 8 corners: (-x,-y,-z), (-x,-y,+z), (-x,+y,-z), (-x,+y,+z), (+x,-y,-z), (+x,-y,+z), (+x,+y,-z), (+x,+y,+z)
    Vec2d center() const { return Vec2d{xRange.mid(), yRange.mid()}; }
    bool contains(const Range2d& inner) const;
    static Range2d make(const std::vector<Vec2d>& pts);
    double width() const { return xRange.size(); }
    double height() const { return yRange.size(); }
};


#endif // RANGE2d_H

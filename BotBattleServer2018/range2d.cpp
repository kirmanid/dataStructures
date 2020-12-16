#include "range2d.h"

Range2d::Range2d()
{

}

Range2d::Range2d(const Vec2d& p1, const Vec2d& p2) // make a range that contains both points
{
    nothing();
    expandToFit(p1);
    expandToFit(p2);
}

bool Range2d::contains(const Vec2d& point) const
{
    return xRange.contains(point.x) && yRange.contains(point.y);
}

bool Range2d::doesNotContain(const Vec2d& point) const
{
    return xRange.doesNotContain(point.x) || yRange.doesNotContain(point.y);
}


void Range2d::expandToFit(const Vec2d& point)
{
    xRange.expandToFit(point.x);
    yRange.expandToFit(point.y);
}

Range2d Range2d::make(const std::vector<Vec2d>& pts)
{
    Range2d range;
    range.nothing();
    for (auto& p : pts) {
        range.expandToFit(p);
    }
    return range;
}


void Range2d::nothing()
{
    xRange.nothing();
    yRange.nothing();
}

void Range2d::everything()
{
    xRange.everything();
    yRange.everything();
}

Vec2d Range2d::corner(int i) const
{
    switch (i) {
    case 0: return { xRange.minVal, yRange.minVal };
    case 1: return { xRange.maxVal, yRange.minVal };
    case 2: return { xRange.maxVal, yRange.maxVal };
    case 3: return { xRange.minVal, yRange.maxVal };
    }
    return { xRange.minVal, yRange.minVal };
}

bool Range2d::contains(const Range2d& inner) const
{
    return xRange.contains(inner.xRange) &&
           yRange.contains(inner.yRange);
}


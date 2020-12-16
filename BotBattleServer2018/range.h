#ifndef RANGE_H
#define RANGE_H

#include <cmath>

template<class T>
constexpr const T& constrain( const T& v, const T& lo, const T& hi)
{
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

template<class T>
constexpr const T interpolate( double t, const T& lo, const T& hi)
{
    return lo + (hi-lo)*t;
}

template<class T>
constexpr const T interpolateConstrained( double t, const T& lo, const T& hi)
{
    return constrain(lo + (hi-lo)*t, lo, hi);
}

template<class T>
constexpr double parameterize( const T& v, const T& lo, const T& hi)
{
    return (v-lo)/(hi-lo); ;
}

template<class T>
constexpr double parameterizeConstrained( const T& v, const T& lo, const T& hi)
{
    return constrain((v-lo)/(hi-lo), 0.0, 1.0);
}

class Range {
public:
    double minVal;
    double maxVal;
public:
    Range();
    void setMin(double minV) { minVal = minV; }
    void setMax(double maxV) { maxVal = maxV; }
    bool contains(double v)  const  { return v >= minVal && v <= maxVal; }
    bool doesNotContain(double v)  const  { return v < minVal || v > maxVal; }
    double mid()  const { return (minVal + maxVal)/2;     }
    double size()  const { return std::fabs(maxVal-minVal); }
    void expandToFit(double value);
    void nothing();
    void everything();
    bool contains(const Range& inner) const { return minVal <= inner.minVal && maxVal >= inner.maxVal; }
    double interpolate(double t) const { return ::interpolate(t, minVal, maxVal); }
    double parameterize(double value) const { return ::parameterize(value, minVal, maxVal); }
    double parameterizeConstrained(double value) const { return ::parameterizeConstrained(value, minVal, maxVal); }
    bool valueIsBelow(double v) const { return v < minVal; }
    bool valueIsAbove(double v) const { return v > maxVal; }
};

#endif // RANGE_H

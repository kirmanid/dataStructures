#include "range.h"
#include <limits>

using namespace std;

Range::Range()
{
    everything();
}

void Range::nothing()
{
    minVal = numeric_limits<double>::max();
    maxVal = numeric_limits<double>::lowest();
}

void Range::everything()
{
    minVal = numeric_limits<double>::lowest();
    maxVal = numeric_limits<double>::max();
}

void Range::expandToFit(double value)
{
    if (value < minVal) {
        minVal = value;
    }
    if (value > maxVal) {
        maxVal = value;
    }
}


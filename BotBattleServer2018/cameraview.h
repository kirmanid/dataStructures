#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#define _USE_MATH_DEFINES

#include <iostream>
#include <vector>

class CameraView
{
public:
    std::vector<double> distance;
    std::vector<int>    color;
public:
    CameraView() {}
};

#endif // CAMERAVIEW_H

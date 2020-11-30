#include <iostream>
#include <memory>
#include "graphics.h"
#include <cmath>

// For Windows using the gcc compiler (most of you)
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"

//For Mac, if the above doesn't work, you can try this:
//#pragma clang diagnostic ignored "-Wsign-conversion"
//#pragma clang diagnostic ignored "-Wsign-compare"

using namespace std;
using namespace mssm;   // <------ this is important

// Notes:
// 1. Your program will go in the function graphicsMain instead of main (but you
// must have a main function like the one at the bottom of this file)
//
// 2. All of the drawing functions (line, ellipse, text, point, etc) are
// methods on the Graphics object
//
// 3. There is a new type "Vec2d" that represents a pair of x,y coordinates.  Many
// drawing functions are overloaded to take either Vec2d's or individual x and y
// components.
//
// 4. The type "Graphics" must always be passed by reference...
// Pass in the graphics object g to any function that needs to do drawing
//
// 5. The type "Color" is used to represent the rgb components of a color.
//  Some colors are predefined:
//       WHITE,GREY,BLACK,RED,GREEN,BLUE,YELLOW,PURPLE and CYAN
//  You can also mix your own colors:
//       Color c{201,100,10}   (each rgb component goes from 0-255)

// Example 1:  Hello World
// no animation, no events... just display some stuff


//// Example 2:  Simple animation without events
//
//void graphicsMain(Graphics& g)
//{
//    double x = 100;
//    double y = 500;

//    while (g.draw()) {  // g.draw() causes the drawing to be sent to the screen

//        g.clear();  // get ready to draw (clear the screen)

//        g.line(0, 0, x, y, GREEN);

//        x = x + 1;

//        if (x > 700) {
//            x = 100;
//        }
//    }
//}

struct points{
    Vec2d point1;
    Vec2d point2;
    float distSq;
};

float distSq(Vec2d& p1, Vec2d& p2){
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

// assumes i0 is begin index, i1 is end index
points minSmallCloud(vector<Vec2d>& cloud, int i0, int i1){
    int size = i1 - i0 + 1;
    if (size != 2 || size != 3){
        /*throw logic_error, forgot syntax*/
    }
    points minPts;
    if (size == 2){
        minPts.point1 = cloud[i0];
        minPts.point2 = cloud[i1];
    } else { // size = 3
        float minSoFar = INFINITY;
        if (distSq(cloud[i0], cloud[i0+1]) < minSoFar){
            minPts.point1 = cloud[i0];
            minPts.point2 = cloud[i0+1];
        }
        if (distSq(cloud[i0+1], cloud[i0+2]) < minSoFar){
            minPts.point1 = cloud[i0+1];
            minPts.point2 = cloud[i0+2];
        }
        if (distSq(cloud[i0+2], cloud [i0]) < minSoFar){
            minPts.point1 = cloud[i0+2];
            minPts.point2 = cloud[i0];
        }
    }
    minPts.distSq = distSq(minPts.point1, minPts.point2);
    return minPts;
}

points dncCloud(vector<Vec2d>& cloud, int i0, int i1){
    if (i0 >= i1 || i0 < 0 || i1 >= cloud.size()){
        cout << "UH OH" << endl;
    }
    if (i1 - i0 < 3){ // if size is 3 or less
        return minSmallCloud(cloud, i0, i1);
    }
    int medianIndex = (i0 + i1) / 2;
    float medianX =  cloud[medianIndex].x;
    points minLeft = dncCloud(cloud, i0, medianIndex);
    points minRight = dncCloud(cloud, medianIndex + 1, i1);
    float minSoFar = (minRight.distSq > minLeft.distSq)? minLeft.distSq : minRight.distSq;
    vector<Vec2d> strip;
    for (int i = i0; i <= i1; i++){
        Vec2d point = cloud[i];
        if (point.x > medianX - minSoFar/2 || point.x < medianX + minSoFar/2){
            strip.push_back(point);
        }
    }
    points minPts;
    if (minRight.distSq < minLeft.distSq){
        minPts.point1 = minRight.point1;
        minPts.point2 = minRight.point2;
        minPts.distSq = minRight.distSq;
    } else {
        minPts.point1 = minLeft.point1;
        minPts.point2 = minLeft.point2;
        minPts.distSq = minLeft.distSq;
    }
    for (Vec2d pointOuter : strip){
        for (Vec2d pointInner : strip){
            if (abs(pointInner.y - pointOuter.y) > minSoFar || (pointInner.x == pointOuter.x && pointOuter.y == pointInner.y)){
                continue;
            }
            if (distSq(pointInner, pointOuter) < minPts.distSq){
                minPts.distSq = distSq(pointInner, pointOuter);
                minPts.point1 = pointOuter;
                minPts.point2 = pointInner;
            }
        }
    }
    if (cloud[i0].y > cloud[medianIndex + 1].y){ // merge subclouds by y-value
        vector<Vec2d> sorted;
        int i = 0;
        while (sorted.size() <= medianIndex - i0 + 1){
            if (cloud[i0+i].y < cloud[medianIndex + 1 + i].y){
                sorted.push_back(cloud[i0+i]);
            } else {
                sorted.push_back(cloud[medianIndex + 1 + i]);
            }
            i++;
        }
    }
    return minPts;
}

points minimumOfCloud(vector<Vec2d>& cloud){
    sort(cloud.begin(), cloud.end(), [](Vec2d a, Vec2d b){return a.x < b.x;});
    return dncCloud(cloud, 0, cloud.size() - 1);
}

points bruteForceMin(vector<Vec2d>& cloud){
    unsigned long searchSpace =  (cloud.size() - 1) * (cloud.size() - 1);
    points currentMin;
    currentMin.distSq = INFINITY;
    Vec2d candidate1;
    Vec2d candidate2;
    float candidateDistSq;
    while (searchSpace != 0){
        candidate1 = cloud[searchSpace % cloud.size()];
        candidate2 = cloud[searchSpace / cloud.size()];
        searchSpace--;
        if (candidate1.x == candidate2.x && candidate1.y == candidate2.y){
            continue;
        }
        candidateDistSq = distSq(candidate1, candidate2);
        if (candidateDistSq < currentMin.distSq){
            currentMin.point1 = candidate1;
            currentMin.point2 = candidate2;
            currentMin.distSq = candidateDistSq;
        }
    }
    return currentMin;
}

// Example 3:  Events
void graphicsMain(Graphics& g)
{
    srand(1321);
    vector<Vec2d> cloud;
    points minPts;
   while (g.draw()) {
         g.clear();
          g.points(cloud, GREEN);
         if (cloud.size() > 2){
             minPts = minimumOfCloud(cloud);
             g.ellipse(minPts.point1.x - 5, minPts.point1.y - 5, 10, 10, RED);
             g.ellipse(minPts.point2.x - 5, minPts.point2.y -5, 10, 10, RED);
             g.line(minPts.point1, minPts.point2, YELLOW);
         }
       for (const Event& e : g.events())
       {
           switch (e.evtType)
           {
           case EvtType::MousePress:
               break;
           case EvtType::MouseRelease:
               break;
           case EvtType::MouseWheel:
               break;
           case EvtType::MouseMove:
               break;
           case EvtType::KeyPress:
               cloud.clear();
               while (cloud.size() < 50 ){
                   cloud.push_back(Vec2d(g.width() * 1.0 * rand() / RAND_MAX, g.height() * 1.0 * rand() / RAND_MAX));        
               }
               break;
           case EvtType::KeyRelease:
               break;
           default:
               break;
           }
       }
   }
}

int main()
{
    // main should be empty except for the following line:
    Graphics g("Drawing", 800, 600, graphicsMain);
    return 0;
}

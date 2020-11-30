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

bool pointInPolygon(Graphics& g, vector<Vec2d> polygon, Vec2d point){
    vector<int> straddlers; // line segment denoted by index of starting point in polygon
    for (int i = 0; i < polygon.size(); i++){
        Vec2d beginPoint = polygon[i];
        Vec2d endPoint = polygon[(i+1) % polygon.size()];
        if (beginPoint.y >= point.y != endPoint.y >= point.y){
            straddlers.push_back(i);
        }
    }
    bool inside = false;
    for (int straddler : straddlers){
        Vec2d beginPoint = polygon[straddler];
        Vec2d endPoint = polygon[(straddler+1) % polygon.size()];
        float solutionX;
        if (beginPoint.x != endPoint.x){
            float slope = (beginPoint.y-endPoint.y) / (beginPoint.x-endPoint.x);
            solutionX = (point.y - (beginPoint.y - slope * beginPoint.x))/slope;
        } else {
            solutionX = beginPoint.x;
        }
        g.point(Vec2d(solutionX, point.y), PURPLE);
        if (solutionX > point.x){
            inside = !inside;
        }        
    }
    return inside;
}

float distToLine(Vec2d p, Vec2d b, Vec2d e ){ // short for point, begin, end
    float lineMagnitude = sqrt((e.x-b.x)*(e.x-b.x) + (e.y-b.y)*(e.y-b.y));
    float absDotProduct = abs((e.x - b.x)*(b.y - p.y) - (b.x - p.x)*(e.y - b.y));
    return absDotProduct/lineMagnitude;
}

float zCoordCrossProduct (Vec2d v1begin, Vec2d v1end, Vec2d v2begin, Vec2d v2end){
    return (v1end.x - v1begin.x) * (v2end.y - v2begin.y) - (v1end.y - v1begin.y) * (v2end.x - v2begin.x);
}

bool toRightOfLine(Vec2d p, Vec2d b, Vec2d e){
    return (0 < zCoordCrossProduct(b,p,b,e));
}

vector<Vec2d> quickHullRecursive(Vec2d begin, Vec2d end, vector<Vec2d>& cloud){
     vector<Vec2d> subcloud;
    for (Vec2d pt : cloud){
        if (toRightOfLine(pt, begin, end)){
            subcloud.push_back(pt);
        }
    }
    if (subcloud.size() == 0){
        vector<Vec2d> line;
        line.push_back(begin);
        line.push_back(end);
        return(line);
    }
    if (subcloud.size() == 1){
        vector<Vec2d> subhull;
        subhull.push_back(begin);
        subhull.push_back(subcloud[0]);
        subhull.push_back(end);
        return(subhull);
    }
    Vec2d farthest;
    float maxDistance = 0;
    for (Vec2d pt : subcloud){
        if (distToLine(pt, begin, end) > maxDistance){
            farthest = pt;
            maxDistance = distToLine(pt, begin, end);
        }
    }
    vector<Vec2d> segment1 = quickHullRecursive(begin, farthest, subcloud);
    vector<Vec2d> segment2 = quickHullRecursive(farthest, end, subcloud);
    segment1.pop_back();
    segment1.insert(segment1.end(), segment2.begin(), segment2.end());
    return segment1;


}

vector<Vec2d> quickHull(vector<Vec2d>& cloud){
    Vec2d leftmost, rightmost;
    leftmost.x = INFINITY;
    rightmost.x = -1 * INFINITY;
    for (Vec2d point : cloud){
        if (leftmost.x > point.x){
            leftmost = point;
        }
        if (rightmost.x < point.x){
            rightmost = point;
        }
    }
    vector<Vec2d> segment1 = quickHullRecursive(leftmost, rightmost, cloud);
    vector<Vec2d> segment2 = quickHullRecursive(rightmost, leftmost, cloud);
    segment1.pop_back();
    segment1.insert(segment1.end(), segment2.begin(), segment2.end());
    return segment1;
}


// Example 3:  Events
void graphicsMain(Graphics& g)
{
    vector<Vec2d> hull;
    vector<Vec2d> cloud;
    Vec2d mousePos;
   while (g.draw()) {
         g.clear();
         g.points(cloud, YELLOW);
         g.polygon(hull, GREEN);
       for (const Event& e : g.events())
       {
           switch (e.evtType)
           {
           case EvtType::MousePress:
               mousePos.x = e.x;
               mousePos.y = e.y;
               if (cloud.size() == 0){

               }
               hull = quickHull(cloud);
               break;
           case EvtType::MouseRelease:
               break;
           case EvtType::MouseWheel:
               break;
           case EvtType::MouseMove:
               break;
           case EvtType::KeyPress:
               cloud.clear();
               while (cloud.size() < 5e4 ){
                   cloud.push_back(Vec2d(g.width() * 1.0 * rand() / RAND_MAX, g.height() * 1.0 * rand() / RAND_MAX));
               }
//               cloud.push_back(Vec2d(100,100));
//               cloud.push_back(Vec2d(20,100));
//               cloud.push_back(Vec2d(100,20));
//               cloud.push_back(Vec2d(20,20));
               hull = quickHull(cloud);
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

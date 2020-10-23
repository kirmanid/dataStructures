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

// Example 3:  Events
void graphicsMain(Graphics& g)
{
    vector<Vec2d> polygon;
    Vec2d p = g.mousePos();
   while (g.draw()) {
       g.clear();
       Color c = pointInPolygon(g, polygon, p)? GREEN : RED;
       g.point(p,YELLOW);
       g.polygon(polygon, c);

       for (const Event& e : g.events())
       {
           switch (e.evtType)
           {
           case EvtType::MousePress:
               polygon.push_back(Vec2d(e.x, e.y));
               break;
           case EvtType::MouseRelease:
               break;
           case EvtType::MouseWheel:
               break;
           case EvtType::MouseMove:
               break;
           case EvtType::KeyPress:
                p = g.mousePos();

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

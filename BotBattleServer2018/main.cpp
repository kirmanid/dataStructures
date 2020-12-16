#include <iostream>
#include "graphics.h"
#include "vec3d.h"
#include <vector>
#include <fstream>
#include <iomanip>
#include <limits>
#include <functional>
#include <valarray>

#include "networkplugin.h"
#include "world.h"
#include "myai.h"
#include "remoteai.h"

#include "historyreader.h"

using namespace std;
using namespace mssm;

Vec2d perp(Vec2d v)  // perpendicular "to the left"
{
    return { -v.y, v.x };
}

// assumes ray already intersects the segment!

double distanceToLine(Vec2d p, Vec2d p1, Vec2d p2)
{
    return abs((p2.y-p1.y)*p.x - (p2.x-p1.x)*p.y + p2.x*p1.y - p2.y*p1.x) / (p2-p1).magnitude();
}

void drawScan(Graphics& g, Vec2d eye, double angle, double fov)
{
    Vec2d leftRay{1000, 0};
    Vec2d rightRay{1000, 0};

    double leftEdgeAng = angle + fov/2;
    double rightEdgeAng = angle - fov/2;

    leftRay.rotate(leftEdgeAng);
    rightRay.rotate(rightEdgeAng);

    g.line(eye, eye+leftRay, Color(255,255,255,100));
    g.line(eye, eye+rightRay, Color(255,255,255,100));
}

void outputPath(std::ostream& strm, const std::string& name, std::vector<Vec2d> points)
{
    strm << "let " << name << " = new Path2d('";
    bool first = true;
    for (Vec2d p : points) {
        if (first) {
            strm << "M ";
            first = false;
        }
        else {
            strm << "L ";
        }
        strm << p.x << " " << p.y << " ";
    }
    strm << "')\n";
}



void graphicsMain(Graphics& g)
{
    NetworkServerPlugin remoteClients(g, 1233);
    NetworkServerPlugin watchers(g, 3000);

    RemoteBotManager remoteBots(
                [&](int id, const std::string& data) { remoteClients.send(id, data); },
                [&](int id) { remoteClients.close(id); }
    );

    int numRows = 4;
    int numCols = 5;

    Range2d window{{10.,10.}, {1010, 810}};

    double w = window.width();
    double h = window.height();
    double gridSize = min(w/numCols, h/numRows);

    window.xRange.maxVal = window.xRange.minVal + gridSize*numCols;
    window.yRange.maxVal = window.yRange.minVal + gridSize*numRows;

    World world(window, numCols, numRows);

    auto lastTime = g.time();

    bool step = false;

    Vec2d eye{20,300};
    Vec2d p2{100,300};

    double currentTime;
    double challengerBirthday;
    rnn champ{24,15};
    champ.mutateParams(.001);
    rnn challenger{24,15};
    size_t hits = 0;
    size_t challengerFitness = 0;
    size_t champFitness = 0;
    double lifespan = 40;
    size_t generation = 0;
    std::ofstream log("log");

    bool look = false;

    try {
        while (g.draw())
        {
            auto newTime = g.time();
            auto elapsedMs = newTime - lastTime;

            if (elapsedMs > 50)
            {
                elapsedMs = 50; // no more than 50 ms per update
            }

            if (elapsedMs < 1) {
                elapsedMs = 1;
            }

            lastTime = newTime;

            g.clear();

            if (!world.paused || step) {

                int eventsExecuted = world.update(elapsedMs/1000.0, 0.2, step, true);

                std::string hist = world.streamHistory();
                if (!hist.empty()) {
                    HistoryReader reader;
                    reader.read(hist);
                }

                if (step) {
                    if (eventsExecuted > 0) {
                        step = false;
                    }
                }
            }

            if (look) {
                Vec2d viewVec = p2 - eye;
                double viewAngle = atan2(viewVec.y, viewVec.x);
                drawScan(g, eye, viewAngle, 1);
                world.look(/*&g, */1, eye, viewAngle);
            }

            world.draw(g);

            if (world.remoteBotBullets > 5) {
                g.text(g.width()-50,g.height()-50, 20, "MaxBullets");
            }

            if (BoiAI::numBots < 2){
                world.addBot(make_shared<Bot>(&world, new LocalBotAI(std::make_unique<BoiAI>(challenger, challengerBirthday, currentTime, challengerFitness, lifespan)), world.getNextBotId(), false), world.localBotHealth, world.localBotBullets);
            }

            for (const Event& e : g.events())
            {
                NetworkSocketEvent socketEvent;
                string socketData;
                int clientId;


                stringstream ss;
                ss << e;
                //cout  << ss.str().c_str() << endl;

                //cout << e << endl;


                if (watchers.handleEvent(e, socketEvent, clientId, socketData)) {
                    // got a network event of some sort
                    switch (socketEvent)
                    {
                    case NetworkSocketEvent::connected:
                        cout  << clientId << " Connected to: " << socketData.c_str() << endl;
                        break;
                    case NetworkSocketEvent::disconnected:
                        cout  << clientId << "Disconnected " << socketData.c_str() << endl;
                        break;
                    case NetworkSocketEvent::error:
                        cout  << clientId << "Network Error " << socketData.c_str() << endl;
                        break;
                    case NetworkSocketEvent::other:
                        cout  << "Network Other  " << socketData.c_str() << endl;
                        break;
                    case NetworkSocketEvent::data:
                        cout  << clientId << "Remote Bots got some data: " << socketData.c_str() << endl;
                        break;
                    }

                    continue;
                }

                if (remoteClients.handleEvent(e, socketEvent, clientId, socketData)) {

                    // got a network event of some sort
                    switch (socketEvent)
                    {
                    case NetworkSocketEvent::connected:
                        remoteBots.createBot(world, clientId);
                        //cout  << clientId << " Connected to: " << socketData.c_str() << endl;
                        break;
                    case NetworkSocketEvent::disconnected:
                        remoteBots.disconnect(clientId);
                        //cout  << clientId << "Disconnected " << socketData.c_str() << endl;
                        break;
                    case NetworkSocketEvent::error:
                        remoteBots.error(clientId, socketData);
                        //cout  << clientId << "Network Error " << socketData.c_str() << endl;
                        break;
                    case NetworkSocketEvent::other:
                        //world.remoteBotError(clientId, socketData);
                        //cout  << "Network Other  " << socketData.c_str() << endl;
                        break;
                    case NetworkSocketEvent::data:
                        cout  << clientId << "Remote Bots got some data: " << socketData.c_str() << endl;
                        remoteBots.receiveData(clientId, socketData);
                        break;
                    }

                    continue;
                }

                switch (e.evtType)
                {
                case EvtType::MousePress:
                    // cout << "MousePress: " << e.arg << endl;
                    if (e.arg == 4) {
                        world.placeFood(Vec2d(e.x, e.y));
                    }
                    else if (e.arg == 2) {
                        if (e.hasShift()) {
//                            world.addBot(make_shared<Bot>(&world, new LocalBotAI(std::make_unique<BoiAI>()), world.getNextBotId(), false), Vec2d(e.x, e.y), M_PI/2, world.localBotHealth, world.localBotBullets);
                        }
                        else {
                            world.addBot(make_shared<Bot>(&world, new LocalBotAI(std::make_unique<MyAI>(e.hasAlt())), world.getNextBotId(), false), Vec2d(e.x, e.y), M_PI/2, world.localBotHealth, world.localBotBullets);
                        }
                    }
                    else {
                        eye = e.mousePos();
                        p2 = eye;
                        look = false;
                    }
                    break;
                case EvtType::MouseMove:
                    if (e.arg == 1) {
                        p2 = e.mousePos();
                        if ((eye-p2).magnitude() > 10) {
                            look = true;
                        }
                        else {
                            look = false;
                        }
                    }
                    break;
                case EvtType::KeyPress:
                    switch (e.arg)
                    {
                    case 'G':
                        if (e.hasCtrl()) {
                            if (world.remoteBotBullets == 5) {
                                world.remoteBotBullets = 50000;
                            }
                            else {
                                world.remoteBotBullets = 5;
                            }
                        }
                        break;
                    case 'B':
                        if (look) {
                            Vec2d viewVec = p2 - eye;
                            double shootAngle = atan2(viewVec.y, viewVec.x);
                            world.fakeFire(eye, shootAngle);
                        }
                        break;
                    case 'E':
                    {
                        world.endGame();
                        std::string hist = world.streamHistory();
                        if (!hist.empty()) {
                            cout << "FinalUpdate: \n";
                            cout << hist << endl;
                        }
                        world.clearHistory();
                    }
                        break;
                    case 'M':
                        world.minBotCount = std::min(30, world.minBotCount+1);
                        break;
                    case 'L':
                        world.minBotCount = std::max(0,  world.minBotCount-1);
                        break;
                    case 'F':
                        world.faster();
                        break;
                    case 'S':
                        world.slower();
                        break;
                    case ' ':
                        world.paused = !world.paused;
                        break;
                    case 'P':
                        step = true;
                        break;
                    }
                    break;
                default:
                    break;
                }
            }
        }

    }
    catch (std::exception& ex) {
        cout << "Exception Thrown (and caught): " << ex.what() << endl;
        g.text(20,20,40,"Exception thrown!");
        g.text(20,120,40,ex.what());
        g.draw();
    }
    catch (...) {
        g.text(20,20,40,"Unknown exception thrown!");
        g.draw();
    }

}


int main()
{
    Graphics g("Bot Battle Server 2018", 1220, 820, graphicsMain);
}

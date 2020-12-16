#include "botmanager.h"
#include "networkplugin.h"
#include <cmath>

using namespace mssm;
using namespace std;

#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wnarrowing"


#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <./eigen-3.3.8/Eigen/Core>

using namespace Eigen;

class rnn{
public:
    rnn(size_t stateSize, size_t inputSize);
    void unroll(VectorXf& input);
    void mutateParams(float searchRadius);
    void adoptParams(rnn& other);
    void zeroParams();
    void zeroState();
    void writeParams(std::string filename);
    void readParams(std::string filename);
    VectorXf state;
    MatrixXf forgetWeights;
    VectorXf forgetBiases;
    MatrixXf weights;
    VectorXf biases;
    MatrixXf inputWeights;
private:
    template <typename M> void readMat (M& mat, std::istream& stream);
    template <typename M> void saveMat(M& mat, std::ostream& stream);
    void tweakMatrix(MatrixXf& matrix, float radius);
    void tweakVector(VectorXf& vector, float radius);
    float hardSigmoid(float x);
    float plu(float x);
    float hardTanh(float x);
    float reluSix(float x);
};

void rnn::readParams(std::string filename){
    std::ifstream file(filename);
    if (file.is_open()){
        readMat(forgetWeights, file);
        readMat(forgetBiases, file);
        readMat(weights, file);
        readMat(biases, file);
        readMat(inputWeights, file);
    }
}

template <typename M>
void rnn::readMat (M& mat, std::istream& stream){
    int numRows;
    int numCols;
    stream >> numRows >> numCols;
    mat.resize(numRows, numCols);
    for (int i = 0; i < numRows; i++){
        for(int j = 0; j < numCols; j++){
            stream >> mat(i,j);
        }
    }
}

void rnn::writeParams(std::string filename){
    std::ofstream file(filename);
    if (file.is_open()){
        saveMat(forgetWeights, file);
        saveMat(forgetBiases, file);
        saveMat(weights, file);
        saveMat(biases, file);
        saveMat(inputWeights, file);
    }
}

template<typename M>
void rnn::saveMat(M& mat, std::ostream& stream){
    stream << mat.rows() << "   " << mat.cols() << std::endl;
    stream << mat << std::endl;
}

void rnn::unroll(VectorXf& input){
    VectorXf forget = (forgetWeights * state + forgetBiases).unaryExpr([&](float x){return hardSigmoid(x);});
    VectorXf deltas = (weights * state + inputWeights * input + biases).unaryExpr([&](float x){return plu(x);});
    state = state.cwiseProduct(forget);
    deltas = deltas.cwiseProduct(VectorXf::Ones(deltas.size()) - forget);
    state = state + deltas;
}

rnn::rnn(size_t stateSize, size_t inputSize){
    forgetWeights = MatrixXf::Zero(stateSize, stateSize);
    weights = forgetWeights;
    forgetBiases = VectorXf::Zero(stateSize);
    biases = forgetBiases;
    inputWeights = MatrixXf::Zero(stateSize, inputSize);
    zeroState();
}

void rnn::tweakMatrix(MatrixXf& matrix, float radius){
    MatrixXf deltas(matrix);
    deltas.setRandom();
    deltas *= radius;
    matrix += deltas;
}

void rnn::tweakVector(VectorXf& vector, float radius){
    VectorXf deltas(vector);
    deltas.setRandom();
    deltas *= radius;
    vector += deltas;
}

void rnn::adoptParams(rnn& other){
    forgetBiases = other.forgetBiases;
    forgetWeights = other.forgetWeights;
    biases = other.biases;
    weights = other.weights;
    inputWeights = other.inputWeights;
}

void rnn::zeroParams(){
    forgetWeights = MatrixXf::Zero(forgetWeights.rows(), forgetWeights.cols());
    forgetBiases = VectorXf::Zero(forgetBiases.size());
    weights = MatrixXf::Zero(weights.rows(), weights.cols());
    biases = VectorXf::Zero(biases.size());
    inputWeights = MatrixXf::Zero(inputWeights.rows(), inputWeights.cols());
}

void rnn::zeroState(){
    state = VectorXf::Zero(biases.size());
}

float rnn::hardSigmoid (float x){
    if (x < -2.5){
        return 0;
    } else if (x < 2.5){
        return 0.2 * x + 0.5;
    } else {
        return 1;
    }
}

float rnn::plu(float x){
    if (x < -1){
        return 0.5 * x - 0.5;
    } else if (x < 1){
        return x;
    } else {
        return 0.5 * x + 0.5;
    }
}

float rnn::hardTanh(float x){
    if (x < -2){
        return -1;
    } else if (x < 2){
        return 0.5 * x;
    } else {
        return 1;
    }
}

float rnn::reluSix(float x){
    if (x < 0){
        return 0;
    } else if (x < 6){
        return x;
    } else {
        return 6;
    }
}

void rnn::mutateParams(float searchRadius){
    tweakMatrix(forgetWeights, searchRadius);
    tweakVector(forgetBiases, searchRadius);
    tweakMatrix(weights, searchRadius);
    tweakVector(biases, searchRadius);
    tweakMatrix(inputWeights, searchRadius);
}
/*
 *
 * Bot Event Types

 Not-Ignorable  (If you ignore these, you'll go to "sleep"
                 until someone hits you or shoots you... not good)

    BotEventType::TurnComplete:      // you just finished a turn
    BotEventType::MoveComplete:      // you just finished a move
    BotEventType::FireComplete:      // you just finished a fire
    BotEventType::NoBullets:         // you attempted to fire but had no bullets
    BotEventType::ScanComplete:      // you just finished a scan
    BotEventType::MoveBlockedByBot:  // you ran into a bot while moving
    BotEventType::MoveBlockedByWall: // you ran into a wall while moving

    For any of the Not-Ignorable events, you should return one of these commands

    Move
    Turn
    Scan
    Fire

  Ignorable events:
    These are "interrupt" points, where you can either issue a new command (canceling
    your last command, or you can Ignore to allow your previous command to finish

    BotEventType::HitByBot:           // a bot ran into you
    BotEventType::HitByBullet:        // you got hit by a bullet
    BotEventType::PowerUp:            // you got a power up (more bullets!)

    For any of the Ignorable events, you should return one of these commands

    Move
    Turn
    Scan
    Fire
    Ignore
*/

class MyBotAI : public BotAI
{
public:
    bool firstEvent;
    double lifespan;
    double& lastEventTime;
    double& birthdate;
    size_t& bulletsHit;
    rnn& brain;
    MyBotAI(rnn& brain_, double&, double&, size_t& bulletsHit, double lifespan);
    virtual BotCmd handleEvents(mssm::Graphics& g, BotEvent& event);
    virtual void logEvent(mssm::Graphics& g, std::string event);
    virtual void logCommand(mssm::Graphics& g, std::string command);
};

MyBotAI::MyBotAI(rnn& brain_, double& birthdate_, double& lastEventTime_, size_t& bulletsHit_, double lifespan_)
    : brain{brain_},
      lastEventTime{lastEventTime_},
      birthdate{birthdate_},
      bulletsHit{bulletsHit_},
      lifespan{lifespan_}
{
    firstEvent = true;
    setName("Hill Climber");
}

BotCmd MyBotAI::handleEvents(mssm::Graphics& g, BotEvent& event)
{
    if (firstEvent){
        firstEvent = false;
        birthdate = event.eventTime;
        lastEventTime = event.eventTime;
    }

    if (lastEventTime - birthdate > lifespan){
//        return Resign();
        return MoveForward(1);
    }

    cout<< event.eventTime << endl;
    float deltaTime;
    deltaTime = log10(0.1 + event.eventTime - lastEventTime);
    int eventType = static_cast<int>(event.eventType); // 0-9, inclusive
    lastEventTime = event.eventTime;
    float collisonAngle;
    if (event.collisionAngle > 3.2 || event.collisionAngle < -3.2){
        collisonAngle = 0;
    } else {
        collisonAngle = event.collisionAngle/3;
    }
    float mean = 0;
    float variance = 0;
    bool scanDataExists = event.scanData.size() != 0;
    if (scanDataExists){
        vector<int> locations;
       float sumLocations = 0;
        for (int i = 0; i < event.scanData.size(); i++){
            if (event.scanData[i] != 0){
                locations.push_back(i);
                sumLocations += i;
            }
        }
        if (!locations.empty()){
            mean = sumLocations/locations.size();
            for (int location : locations){
                variance += (location - mean) * (location - mean);
            }
            variance = log10(variance / locations.size());
            float center = event.scanData.size()/2;
            mean = (mean - center) / center;
        }
    }

    if (event.eventType == BotEventType::PowerUp){
        bulletsHit++;
    }
    cout << "Current Fitness: " << bulletsHit << endl;

    // build input vector, length 16
    VectorXf inputs = VectorXf::Zero(15);
    for (int i = 0; i < 9; i++){
        inputs[i] = (i == eventType? 1.0 : 0.0);
    }
    inputs[9] = deltaTime;
    inputs[10] = collisonAngle;
    inputs[11] = event.health / 5.0;
    inputs[12] = scanDataExists? 1.0 : 0.0;
    inputs[13] = mean;
    inputs[14] = variance;

    brain.unroll(inputs);
    VectorXf actions = brain.state.head(6);

    int action;
    float pAction = -INFINITY;
    bool ignorable = eventType >= 7 && eventType <= 9;
    for (int i = 0; i < (ignorable? 5 : 4); i++){
        if (actions[i] > pAction){
            action = i;
            pAction = actions[i];
        }
    }
    double time = pow(2, actions[5]  * 1e3 );

    switch(action){
        case 0:
            return MoveForward(time);
        case 1:
            return Turn(time);
        case 2:
            return Fire();
        case 3:
            return Scan(time);
        case 4:
            return Ignore();
        default:
            cout << "THIS SHOULD NEVER HAPPEN";
            return Turn(1);
    }
}

void MyBotAI::logEvent(mssm::Graphics& /*g*/, std::string event)
{
//    cout << "EVENT: " << event << endl;

}

void MyBotAI::logCommand(mssm::Graphics& /*g*/, std::string command)
{
//    cout << "CMD:   " << command << endl;

}

void botBrainLoop(Graphics& g)
{
    BotManager botManager(g, 1233, "localhost");

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

//    champ.readParams("params");
    botManager.addBot(std::make_unique<MyBotAI>(challenger, challengerBirthday, currentTime, hits, lifespan));


    while (g.draw())
    {
        g.clear();

        for (const Event& e : g.events())
        {
            if (botManager.processEvent(e)) {
                continue;
            }

            if (botManager.numBots() == 0) {

                challenger.zeroState();
                challengerFitness = hits;

                if (generation % 20 == 0){
                    log << generation << ", " << challengerFitness << ", " << champFitness << std::endl;
                    champ.writeParams("params");
                }

                cout << "\n\n\n\n Fitness: "<< challengerFitness << endl;
                cout << "Generation #: " << generation << endl;
                generation++;
//                challengerFitness = currentTime - challengerBirthday;
                if (challengerFitness >= champFitness){
                    champ.adoptParams(challenger);
                    champFitness = challengerFitness;
                } else {
                    challenger.adoptParams(champ);
                }
                challenger.mutateParams(0.01);

                hits = 0;
                botManager.addBot(std::make_unique<MyBotAI>(challenger, challengerBirthday, currentTime, hits, lifespan));
                continue;
            }

            switch (e.evtType)
            {
            case EvtType::KeyPress:
                break;
            default:
                break;
            }
        }

    }
}

void prompt(Graphics& g, string msg)
{
    while (g.draw())
    {
        g.clear();

        g.text(50,g.height()/2-10,20,msg);

        for (const Event& e : g.events())
        {
            switch (e.evtType)
            {
            case EvtType::KeyPress:
                if (e.arg == ' ') {
                    return;
                }
                break;
            default:
                break;
            }
        }
    }
}

void graphicsMain(Graphics& g)
{
    try {
        while (g.draw()) {
            botBrainLoop(g);
//            prompt(g, "Disconnected: Press space to reconnect to server");
        }
    }
    catch (const std::exception& ex) {
        prompt(g, string{"Exception: "} + ex.what());
    }
    catch (...) {
        prompt(g, "Unknown exception thrown!");
    }
}

int main()
{
    Graphics g("Bot Battle Client", 1000, 800, graphicsMain);
}

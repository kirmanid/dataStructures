#define _USE_MATH_DEFINES
#include "vec2d.h"
#include "myai.h"
#include "random.h"

Random aiRand;

using namespace std;

MyAI::MyAI(bool dumb)
    : dumb(dumb)
{
    setName("Boop");
    fireAfterMove = false;
}

MyAI::~MyAI()
{
}

void MyAI::setKilled()
{
}

BotCmd MyAI::handleEvents(BotEvent& event)
{

}

BoiAI::BoiAI(rnn& brain_, double& birthdate_, double& lastEventTime_, size_t& bulletsHit_, double lifespan_)
    : brain{brain_},
      lastEventTime{lastEventTime_},
      birthdate{birthdate_},
      bulletsHit{bulletsHit_},
      lifespan{lifespan_}
{
    firstEvent = true;
    setName("Hill Climber");
    numBots++;
}

BoiAI::~BoiAI()
{
}

int BoiAI::numBots{0};

void BoiAI::setKilled()
{
   // cout << "ThisGuyKilled" << endl;
}

BotCmd BoiAI::handleEvents(BotEvent& event)
{
    if (firstEvent){
        firstEvent = false;
        birthdate = event.eventTime;
        lastEventTime = event.eventTime;
    }

    if (event.health == 0 && !alreadyDecrementedNumBots && (event.eventType == BotEventType::HitByBot || event.eventType == BotEventType::HitByBullet || event.eventType == BotEventType::MoveBlockedByWall)){
        alreadyDecrementedNumBots = true;
        numBots--;
    }

    if (lastEventTime - birthdate > lifespan){
//        return Resign();
        return Move(1);
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
            return Move(time);
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

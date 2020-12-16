#ifndef MYAI_H
#define MYAI_H

#define _USE_MATH_DEFINES

#include "botai.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "./eigen-3.3.8/Eigen/Core"

using namespace Eigen;

class MyAI : public BotAI
{
    bool fireAfterMove;
    bool dumb;
public:
    MyAI(bool dumb);
    ~MyAI() override;
    virtual BotCmd handleEvents(BotEvent& event) override;
    void setKilled() override;
};

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


class BoiAI : public BotAI
{
public:
    bool targeted = false;
    static int numBots;
    BoiAI(rnn& brain_, double&, double&, size_t& bulletsHit, double lifespan);
    ~BoiAI() override;
    virtual BotCmd handleEvents(BotEvent& event) override;
    void setKilled() override;
    bool alreadyDecrementedNumBots{false};
    bool firstEvent;
    double lifespan;
    double& lastEventTime;
    double& birthdate;
    size_t& bulletsHit;
    rnn& brain;
};

#endif // MYAI_H

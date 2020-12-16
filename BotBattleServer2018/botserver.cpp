#define _USE_MATH_DEFINES
#include "botserver.h"
#include "remoteai.h"

#include <QMetaType>

BotServer::BotServer(World& w, QObject *parent) : QTcpServer(parent), world(w)
{
    qRegisterMetaType< QAbstractSocket::SocketState >();
    qRegisterMetaType< QAbstractSocket::SocketError >();
}

BotServer::~BotServer()
{

}

void BotServer::startServer()
{
    int port = 1234;
    if(!this->listen(QHostAddress::Any, port))
    {
        //cout  << "Could not start server" << endl;
    }
    else
    {
        //cout  << "Listening to port " << port << "..." << endl;
    }
}

// This function is called by QTcpServer when a new connection is available.

void BotServer::incomingConnection(qintptr socketDescriptor)
{
    // We have a new connection
    //cout  << "Bot Connecting" << endl;

    auto remoteAI = new RemoteAI(socketDescriptor);

    if (world.availableBotId() >= 50)
    {
        //cout  << "Bot Limit Reached... refusing" << endl;
        delete remoteAI;
    }
    else
    {
        world.addBot(new Bot(&world, remoteAI));
    }
}

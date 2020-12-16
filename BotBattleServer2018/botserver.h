#ifndef BOTSERVER_H
#define BOTSERVER_H

#define _USE_MATH_DEFINES

#include <QObject>
#include <QTcpServer>

#include "vec2d.h"

#include "world.h"

class BotServer : public QTcpServer
{
    Q_OBJECT

    World& world;

public:
    explicit BotServer(World& world, QObject *parent = 0);
    ~BotServer();

    void startServer();

signals:

public slots:

protected:

    void incomingConnection(qintptr socketDescriptor);

};

#endif // BOTSERVER_H

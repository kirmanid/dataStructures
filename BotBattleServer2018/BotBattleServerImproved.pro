QT += widgets
QT += multimedia

CONFIG += gui
CONFIG += c++14

HEADERS    = \
    window.h \
    graphics.h \
    vec2d.h \
    vec3d.h \
    networkclient.h \
    networkplugin.h \
    networkserver.h \
    plugin.h \
    world.h \
    worldeventqueue.h \
    random.h \
    bot.h \
    botcmd.h \
    botai.h \
    myai.h \
    remoteai.h \
    range.h \
    range2d.h \
    serverbotcmd.h \
    uniqueid.h \
    maze.h \
    eventspan.h \
    historyreader.h

SOURCES     = \
    main.cpp \
    graphics.cpp \
    vec2d.cpp \
    vec3d.cpp \
    networkclient.cpp \
    networkplugin.cpp \
    networkserver.cpp \
    plugin.cpp \
    world.cpp \
    worldeventqueue.cpp \
    random.cpp \
    bot.cpp \
    botcmd.cpp \
    botai.cpp \
    myai.cpp \
    remoteai.cpp \
    range.cpp \
    range2d.cpp \
    serverbotcmd.cpp \
    uniqueid.cpp \
    maze.cpp \
    eventspan.cpp \
    historyreader.cpp

DISTFILES += \
    notes.txt



TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lpthread
INCLUDEPATH += include

SOURCES += main.cpp \
    src/gtest-all.cc

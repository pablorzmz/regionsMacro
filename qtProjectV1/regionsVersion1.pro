TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

QMAKE_CXXFLAGS += -std=c++0x -lpthread
LIBS += -pthread

HEADERS += \
    region.h

QT       += core gui multimedia
QT       += core gui widgets
QT       += core gui widgets opengl
QT       += core gui multimedia
CONFIG += resources_big
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

SOURCES += main.cpp \
           gamewindow.cpp \
           player.cpp \
           tileitem.cpp \
           enemy.cpp \
           bumpingcoin.cpp \
           mushroom.cpp \
           staticitem.cpp

HEADERS += \
    bumpingcoin.h \
    enemy.h \
    gamewindow.h \
    constants.h \
    mushroom.h \
    player.h \
    staticitem.h \
    tileitem.h

RESOURCES += \
    res.qrc

TARGET = superMarion

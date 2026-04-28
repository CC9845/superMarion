QT       += core gui multimedia
QT       += core gui widgets
QT       += core gui widgets opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# 这里是关键：告诉 Qt 你的文件都在哪
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

# 指明输出文件的名字
TARGET = superMarion

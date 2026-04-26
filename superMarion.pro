QT       += core gui multimedia
QT       += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# 这里是关键：告诉 Qt 你的文件都在哪
SOURCES += \
    enemy.cpp \
    main.cpp \
    gamewindow.cpp \
    player.cpp \
    staticitem.cpp \
    tileitem.cpp

HEADERS += \
    enemy.h \
    gamewindow.h \
    constants.h \
    player.h \
    staticitem.h \
    tileitem.h

RESOURCES += \
    res.qrc

# 指明输出文件的名字
TARGET = superMarion

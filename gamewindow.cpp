#include "gamewindow.h"
#include "constants.h"
#include <QPixmap>

GameWindow::GameWindow(QWidget *parent) : QGraphicsView(parent) {
    // 1. 设置窗口外观
    this->setFixedSize(C::SCREEN_W, C::SCREEN_H);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setWindowTitle("Super Mario Qt v1.0");

    // 2. 初始化场景
    scene = new QGraphicsScene(0, 0, C::SCREEN_W, C::SCREEN_H, this);
    this->setScene(scene);

    initScene();
}

void GameWindow::initScene() {
    // --- 加载背景 ---
    // 路径适配你 qrc 中的 "/" 前缀和 "graphics" 文件夹
    QPixmap bgFull(":/graphics/level_1.png");

    // 按照 2.68 倍缩放，加 int() 强转消除警告
    QPixmap bgScaled = bgFull.scaled(int(bgFull.width() * C::BG_MULTI),
                                     int(bgFull.height() * C::BG_MULTI),
                                     Qt::KeepAspectRatio);
    background = new QGraphicsPixmapItem(bgScaled);
    scene->addItem(background);

    // --- 抠出马里奥 ---
    QPixmap marioSheet(":/graphics/mario_bros.png");

    // 按照坐标 (178, 32, 12, 16) 抠出站立动作
    QPixmap marioStand = marioSheet.copy(C::MARIO_STAND_X,
                                         C::MARIO_STAND_Y,
                                         C::MARIO_WIDTH,
                                         C::MARIO_HEIGHT);

    // 按照 2.9 倍缩放，加 int() 强转消除警告
    QPixmap marioScaled = marioStand.scaled(int(marioStand.width() * C::PLAYER_MULTI),
                                            int(marioStand.height() * C::PLAYER_MULTI),
                                            Qt::KeepAspectRatio);

    mario = new QGraphicsPixmapItem(marioScaled);

    // 设置初始位置 (x=110, y=480)
    mario->setPos(110, 480);
    scene->addItem(mario);
}

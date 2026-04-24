#include "gamewindow.h"
#include "constants.h"
#include <QPixmap>

GameWindow::GameWindow(QWidget *parent) : QGraphicsView(parent),
    x_vel(0), y_vel(0), isJumping(false), keyLeft(false), keyRight(false)
{
    this->setFixedSize(C::SCREEN_W, C::SCREEN_H);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setWindowTitle("Super Mario Qt v2.0");

    scene = new QGraphicsScene(0, 0, C::SCREEN_W, C::SCREEN_H, this);
    this->setScene(scene);

    initScene();

    // 启动 60 FPS 的定时器
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameWindow::gameLoop);
    timer->start(16);
}

void GameWindow::initScene() {
    QPixmap bgFull(":/graphics/level_1.png");
    QPixmap bgScaled = bgFull.scaled(int(bgFull.width() * C::BG_MULTI),
                                     int(bgFull.height() * C::BG_MULTI));
    background = new QGraphicsPixmapItem(bgScaled);
    scene->addItem(background);

    QPixmap marioSheet(":/graphics/mario_bros.png");
    QPixmap marioStand = marioSheet.copy(C::MARIO_STAND_X, C::MARIO_STAND_Y, C::MARIO_WIDTH, C::MARIO_HEIGHT);
    QPixmap marioScaled = marioStand.scaled(int(C::MARIO_WIDTH * C::PLAYER_MULTI),
                                            int(C::MARIO_HEIGHT * C::PLAYER_MULTI));
    mario = new QGraphicsPixmapItem(marioScaled);
    mario->setPos(110, C::GROUND_HEIGHT - (C::MARIO_HEIGHT * C::PLAYER_MULTI));
    scene->addItem(mario);
}

void GameWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Left) keyLeft = true;
    if (event->key() == Qt::Key_Right) keyRight = true;
    if (event->key() == Qt::Key_Up && !isJumping) {
        y_vel = C::JUMP_VELOCITY;
        isJumping = true;
    }
}

void GameWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Left) keyLeft = false;
    if (event->key() == Qt::Key_Right) keyRight = false;
}

void GameWindow::gameLoop() {
    // 水平速度计算
    if (keyLeft) x_vel = -C::MAX_WALK_SPEED;
    else if (keyRight) x_vel = C::MAX_WALK_SPEED;
    else x_vel = 0;

    // 垂直速度
    y_vel += C::GRAVITY;

    // 计算预想位置
    double nextX = mario->x() + x_vel;
    double nextY = mario->y() + y_vel;

    // 地面碰撞检测
    double marioBottom = nextY + (C::MARIO_HEIGHT * C::PLAYER_MULTI);
    if (marioBottom >= C::GROUND_HEIGHT) {
        nextY = C::GROUND_HEIGHT - (C::MARIO_HEIGHT * C::PLAYER_MULTI);
        y_vel = 0;
        isJumping = false;
    }

    // 更新位置
    mario->setPos(nextX, nextY);
}

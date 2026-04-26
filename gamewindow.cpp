#include "gamewindow.h"
#include "constants.h"
#include "staticitem.h"
#include "tileitem.h"
#include "enemy.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPixmap>
#include <QDebug>
#include <QGraphicsScene>
#include <QWidget>

GameWindow::GameWindow(QWidget *parent) : QGraphicsView(parent),
    x_vel(0), y_vel(0), isJumping(false), keyLeft(false), keyRight(false)
{
    this->setFixedSize(C::SCREEN_W, C::SCREEN_H);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setWindowTitle("Super Mario Qt v2.0");

    scene = new QGraphicsScene(this);
    this->setScene(scene);

    initScene();
    loadMapData();
    setupGroundItems();
    setupBricksBoxesAndCoins();
    setupEnemies();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameWindow::gameLoop);
    timer->start(16);
}

void GameWindow::initScene() {
    QPixmap bgFull(":/graphics/level_1.png");
    QPixmap bgScaled = bgFull.scaled(int(bgFull.width() * C::BG_MULTI), int(bgFull.height() * C::BG_MULTI));
    background = new QGraphicsPixmapItem(bgScaled);
    background->setZValue(-1);
    scene->addItem(background);
    scene->setSceneRect(0, 0, bgScaled.width(), bgScaled.height());

    QPixmap marioSheet(":/graphics/mario_bros.png");
    QPixmap marioStand = marioSheet.copy(C::MARIO_STAND_X, C::MARIO_STAND_Y, C::MARIO_WIDTH, C::MARIO_HEIGHT);
    mario = new QGraphicsPixmapItem(marioStand.scaled(int(C::MARIO_WIDTH * C::PLAYER_MULTI), int(C::MARIO_HEIGHT * C::PLAYER_MULTI)));

    mario->setPos(110, C::GROUND_HEIGHT - mario->pixmap().height());
    mario->setZValue(10);
    scene->addItem(mario);
}

void GameWindow::loadMapData() {
    QFile file(":/data/level_1.json");
    if (file.open(QIODevice::ReadOnly)) {
        mapData = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
    }
}

void GameWindow::setupGroundItems() {
    QStringList names = {"ground", "pipe", "step"};
    for (const QString &name : names) {
        QJsonArray items = mapData[name].toArray();
        for (auto v : items) {
            QJsonObject i = v.toObject();
            StaticItem* si = new StaticItem(i["x"].toInt(), i["y"].toInt(), i["width"].toInt(), i["height"].toInt(), name);
            scene->addItem(si);
            groundItems.append(si);
        }
    }
}

void GameWindow::setupBricksBoxesAndCoins() {
    QPixmap tileSheet(":/graphics/tile_set.png");
    QPixmap itemSheet(":/graphics/item_objects.png");

    QJsonArray coins = mapData["coin"].toArray();
    for (auto v : coins) {
        QJsonObject c = v.toObject();
        TileItem* coin = new TileItem(c["x"].toInt(), c["y"].toInt(), itemSheet, QRect(3, 98, 8, 14), C::BG_MULTI);
        scene->addItem(coin);
        interactiveItems.append(coin);
    }

    QJsonArray bricks = mapData["brick"].toArray();
    for (auto v : bricks) {
        QJsonObject b = v.toObject();
        TileItem* brick = new TileItem(b["x"].toInt(), b["y"].toInt(), tileSheet, QRect(16, 0, 16, 16), C::BG_MULTI);
        scene->addItem(brick);
        interactiveItems.append(brick);
    }

    QJsonArray boxes = mapData["box"].toArray();
    for (auto v : boxes) {
        QJsonObject b = v.toObject();
        TileItem* box = new TileItem(b["x"].toInt(), b["y"].toInt(), tileSheet, QRect(384, 0, 16, 16), C::BG_MULTI);
        scene->addItem(box);
        interactiveItems.append(box);
    }
}

void GameWindow::setupEnemies() {
    QPixmap enemySheet(":/graphics/enemies.png");
    // 如果加载失败，在控制台打印提示以便调试
    if (enemySheet.isNull()) qDebug() << "ERROR: enemies.png could not be loaded!";

    QJsonArray enemyGroups = mapData["enemy"].toArray();
    for (auto groupVal : enemyGroups) {
        QJsonObject groupObj = groupVal.toObject();
        for (const QString& key : groupObj.keys()) {
            QJsonArray enemyList = groupObj[key].toArray();
            for (auto eVal : enemyList) {
                QJsonObject e = eVal.toObject();
                Enemy* enemy = new Enemy(e["x"].toInt(), e["y"].toInt(), e["type"].toInt(), enemySheet);
                scene->addItem(enemy);
                enemies.append(enemy);
            }
        }
    }
}

void GameWindow::updateCamera() {
    double screenThird = this->width() / 3.0;
    if (mario->x() > screenThird) {
        this->centerOn(mario->x() + screenThird, C::SCREEN_H / 2);
    }
}

void GameWindow::gameLoop() {
    //水平移动及碰撞
    if (keyLeft) x_vel = -C::MAX_WALK_SPEED;
    else if (keyRight) x_vel = C::MAX_WALK_SPEED;
    else x_vel = 0;

    mario->setPos(mario->x() + x_vel, mario->y());

    for (StaticItem* item : groundItems) {
        if (mario->collidesWithItem(item)) {
            if (x_vel > 0) mario->setPos(item->x() - mario->pixmap().width(), mario->y());
            else if (x_vel < 0) mario->setPos(item->x() + item->rect().width(), mario->y());
            x_vel = 0;
        }
    }

    // 垂直移动：
    if (isJumping && y_vel < 0) {
        // 上升阶段使用较小的重力 (0.3)
        y_vel += C::ANTI_GRAVITY;
    } else {
        // 下落或平地阶段使用正常重力 (1.0)
        y_vel += C::GRAVITY;
    }

    mario->setPos(mario->x(), mario->y() + y_vel);

    bool onGround = false;
    for (StaticItem* item : groundItems) {
        if (mario->collidesWithItem(item)) {
            if (y_vel > 0) { // 落地检测
                mario->setPos(mario->x(), item->y() - mario->pixmap().height());
                y_vel = 0;
                isJumping = false;
                onGround = true;
            } else if (y_vel < 0) { // 顶头检测
                mario->setPos(mario->x(), item->y() + item->rect().height());
                y_vel = 2;
            }
        }
    }

    if (!onGround && !isJumping) isJumping = true;

    // 更新怪物和镜头
    for (Enemy* enemy : enemies) {
        enemy->updateLogic();
    }
    updateCamera();
}

void GameWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Left) keyLeft = true;
    if (event->key() == Qt::Key_Right) keyRight = true;
    if (event->key() == Qt::Key_Up && !isJumping) {
        y_vel = C::JUMP_VELOCITY; // 初始跳跃速度
        isJumping = true;
    }
}

void GameWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Left) keyLeft = false;
    if (event->key() == Qt::Key_Right) keyRight = false;
}

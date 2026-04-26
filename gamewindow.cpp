#include "gamewindow.h"
#include "constants.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

GameWindow::GameWindow(QWidget *parent) : QGraphicsView(parent),
    keyLeft(false), keyRight(false), keyUp(false)
{
    this->setFixedSize(C::SCREEN_W, C::SCREEN_H);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setWindowTitle("Super Mario Qt v2.0 - Integrated");

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
    player = new Player(marioSheet);
    player->setPos(110, C::GROUND_HEIGHT - player->pixmap().height());
    player->setZValue(10);
    scene->addItem(player);
}

void GameWindow::gameLoop() {
    if (player->isDead) {
        static int marioDeathTimer = 0;
        marioDeathTimer++;
        if (marioDeathTimer < 30) {
            player->y_vel += C::ANTI_GRAVITY;
        } else {
            player->y_vel += C::GRAVITY;
        }
        player->setPos(player->x(), player->y() + player->y_vel);

        if (marioDeathTimer > 180) {
            player->setPos(110, C::GROUND_HEIGHT - player->pixmap().height());
            player->isDead = false;
            player->state = Player::STAND;
            player->x_vel = 0;
            player->y_vel = 0;
            marioDeathTimer = 0;
        }
        return;
    }

    player->updateLogic(keyLeft, keyRight, keyUp);

    //  X 轴移动与碰撞
    player->setPos(player->x() + player->x_vel, player->y());
    auto handleXCollision = [&](QGraphicsItem* item) {
        if (player->collidesWithItem(item)) {
            QRectF itemRect = item->sceneBoundingRect();
            if (player->x_vel > 0) player->setPos(itemRect.left() - player->pixmap().width(), player->y());
            else if (player->x_vel < 0) player->setPos(itemRect.right(), player->y());
            player->x_vel = 0;
        }
    };
    for (StaticItem* item : groundItems) handleXCollision(item);
    for (TileItem* item : solidItems) handleXCollision(item);

    //Y 轴移动与碰撞
    player->setPos(player->x(), player->y() + player->y_vel);
    auto handleYCollision = [&](QGraphicsItem* item) {
        if (player->collidesWithItem(item)) {
            QRectF itemRect = item->sceneBoundingRect();
            if (player->y_vel > 0) {
                player->setPos(player->x(), itemRect.top() - player->pixmap().height());
                player->y_vel = 0;
                player->state = Player::WALK;
            } else if (player->y_vel < 0) {
                player->setPos(player->x(), itemRect.bottom());
                player->y_vel = 2;
                player->state = Player::FALL;
                TileItem* tile = dynamic_cast<TileItem*>(item);
                if (tile) tile->bump();
            }
        }
    };
    for (StaticItem* item : groundItems) handleYCollision(item);
    for (TileItem* item : solidItems) handleYCollision(item);

    // 1像素悬空探测
    player->setPos(player->x(), player->y() + 1);
    bool willFall = true;
    auto checkFall = [&](QGraphicsItem* item) {
        if (player->collidesWithItem(item)) willFall = false;
    };
    for (StaticItem* item : groundItems) checkFall(item);
    for (TileItem* item : solidItems) checkFall(item);
    player->setPos(player->x(), player->y() - 1); // 探测完立刻退回原位

    if (willFall && player->state != Player::JUMP) {
        player->state = Player::FALL;
    }

    // 物品与敌人更新
    for (TileItem* coin : coinsList) coin->updateLogic();
    for (TileItem* solid : solidItems) solid->updateLogic();

    for (int i = 0; i < coinsList.size(); ++i) {
        if (player->collidesWithItem(coinsList[i])) {
            scene->removeItem(coinsList[i]);
            delete coinsList[i];
            coinsList.removeAt(i);
            i--;
        }
    }

    for (int i = 0; i < enemies.size(); ++i) {
        Enemy* enemy = enemies[i];
        enemy->updateLogic();

        if (enemy->isRemovable) {
            scene->removeItem(enemy);
            delete enemy;
            enemies.removeAt(i);
            i--;
            continue;
        }

        if (!enemy->isDead && player->collidesWithItem(enemy)) {
            if (player->y_vel > 0 && player->y() + player->pixmap().height() < enemy->y() + enemy->pixmap().height() / 2 + 10) {
                enemy->goDie();
                player->y_vel = C::JUMP_VELOCITY * 0.5;
            } else {
                player->goDie();
            }
        }
    }

    updateCamera();
}

void GameWindow::updateCamera() {
    if (player->isDead) return;
    double screenThird = this->width() / 3.0;
    if (player->x() > screenThird) {
        this->centerOn(player->x() + screenThird, C::SCREEN_H / 2);
    }
}

// 防止长按时状态机疯狂复位
void GameWindow::keyPressEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) return;
    if (event->key() == Qt::Key_Left) keyLeft = true;
    if (event->key() == Qt::Key_Right) keyRight = true;
    if (event->key() == Qt::Key_Up) keyUp = true;
}

void GameWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->isAutoRepeat()) return;
    if (event->key() == Qt::Key_Left) keyLeft = false;
    if (event->key() == Qt::Key_Right) keyRight = false;
    if (event->key() == Qt::Key_Up) keyUp = false;
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
        TileItem* coin = new TileItem(c["x"].toInt(), c["y"].toInt(), TileItem::COIN, itemSheet, C::BG_MULTI);
        scene->addItem(coin);
        coinsList.append(coin);
    }
    QJsonArray bricks = mapData["brick"].toArray();
    for (auto v : bricks) {
        QJsonObject b = v.toObject();
        TileItem* brick = new TileItem(b["x"].toInt(), b["y"].toInt(), TileItem::BRICK, tileSheet, C::BG_MULTI);
        scene->addItem(brick);
        solidItems.append(brick);
    }
    QJsonArray boxes = mapData["box"].toArray();
    for (auto v : boxes) {
        QJsonObject b = v.toObject();
        TileItem* box = new TileItem(b["x"].toInt(), b["y"].toInt(), TileItem::BOX, tileSheet, C::BG_MULTI);
        scene->addItem(box);
        solidItems.append(box);
    }
}

void GameWindow::setupEnemies() {
    QPixmap enemySheet(":/graphics/enemies.png");
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

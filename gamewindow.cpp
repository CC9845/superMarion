#include "gamewindow.h"
#include "constants.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

GameWindow::GameWindow(QWidget *parent) : QGraphicsView(parent),
    keyLeft(false), keyRight(false), keyUp(false), isLevelFinished(false)
{
    this->setFixedSize(C::SCREEN_W, C::SCREEN_H);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setWindowTitle("Super Mario Qt v5.0 - Final Fix Verified");
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    score = 0; coins = 0; gameTime = 300; timerTickCount = 0;

    scene = new QGraphicsScene(this);
    this->setScene(scene);

    initScene();
    loadMapData();
    setupGroundItems();
    setupBricksBoxesAndCoins();
    setupEnemies();
    setupCheckpoints();
    setupFlagpole();

    timer = new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, &QTimer::timeout, this, &GameWindow::gameLoop);
    timer->start(16);
}

void GameWindow::initScene() {
    QPixmap bgFull(":/graphics/level_1.png");
    QPixmap bgScaled = bgFull.scaled(static_cast<int>(bgFull.width() * C::BG_MULTI),
                                     static_cast<int>(bgFull.height() * C::BG_MULTI));
    background = new QGraphicsPixmapItem(bgScaled);
    background->setZValue(-1);
    scene->addItem(background);
    scene->setSceneRect(0, 0, bgScaled.width(), bgScaled.height());

    // 直接根据JSON名字生成玩家
    player = new Player("mario");
    player->setPos(110, C::GROUND_HEIGHT - player->pixmap().height());
    player->setZValue(10);
    scene->addItem(player);
}

void GameWindow::spawnBumpingCoin(int x, int y) {
    QPixmap is(":/graphics/item_objects.png");
    BumpingCoin* bc = new BumpingCoin(x, y, is, C::BG_MULTI);
    scene->addItem(bc);
    bumpingCoins.append(bc);
}

void GameWindow::spawnMushroom(int x, int y) {
    QPixmap is(":/graphics/item_objects.png");
    Mushroom* m = new Mushroom(x, y, is, C::BG_MULTI);
    scene->addItem(m);
    mushrooms.append(m);
}
void GameWindow::gameLoop() {
    if (isLevelFinished) return;

    // 死亡复活逻辑
    if (player->isDead) {
        static int dTimer = 0; dTimer++;
        player->y_vel += (dTimer < 30) ? C::ANTI_GRAVITY : C::GRAVITY;
        player->setPos(player->x(), player->y() + player->y_vel);
        if (dTimer > 180) {
            player->setScale(1.0); // 复活时重置大小
            player->isBig = false;
            player->setPos(110, C::GROUND_HEIGHT - player->pixmap().height());
            player->isDead = false; player->state = Player::STAND;
            player->x_vel = 0; player->y_vel = 0; dTimer = 0; gameTime = 300;
        }
        return;
    }

    // 检查点生成敌人
    for (int i = 0; i < checkpoints.size(); ++i) {
        if (player->collidesWithItem(checkpoints[i])) {
            int gid = checkpoints[i]->enemyGroupId;
            if (inactiveEnemies.contains(gid)) {
                for (Enemy* e : inactiveEnemies[gid]) { scene->addItem(e); enemies.append(e); }
                inactiveEnemies.remove(gid);
            }
            scene->removeItem(checkpoints[i]); delete checkpoints[i]; checkpoints.removeAt(i--);
        }
    }

    timerTickCount++;
    if (timerTickCount >= 60) { gameTime--; timerTickCount = 0; if (gameTime <= 0) player->goDie(); }
    if (player->x() >= 8500) { isLevelFinished = true; score += gameTime * 50; }

    player->updateLogic(keyLeft, keyRight, keyUp);
    double pWidth = player->pixmap().width() * player->scale();
    double pHeight = player->pixmap().height() * player->scale();

    // ============= X 轴物理=============
    player->setPos(player->x() + player->x_vel, player->y());
    auto hX = [&](QGraphicsItem* it) {
        // X轴检测时，上下各收缩 2 像素，防止顶砖块时被误判为水平撞墙而发生瞬移穿透
        QRectF currentXRect = player->sceneBoundingRect().adjusted(0, 2, 0, -2);
        if (currentXRect.intersects(it->sceneBoundingRect())) {
            QRectF r = it->sceneBoundingRect();
            if (player->x_vel > 0) player->setPos(r.left() - pWidth, player->y());
            else if (player->x_vel < 0) player->setPos(r.right(), player->y());
            player->x_vel = 0;
        }
    };
    for (StaticItem* it : groundItems) hX(it);
    for (TileItem* it : solidItems) hX(it);


    // ============= Y 轴物理 =============
        // 记录这具身体在 Y 轴计算前的真实方向，防止判定中途速度突变引发连环 Bug
        double originalYVel = player->y_vel;
        player->setPos(player->x(), player->y() + player->y_vel);
        bool onGround = false;

        QRectF yCheckRect = player->sceneBoundingRect().adjusted(2, 0, -2, 0);

        // 地面、水管
        for (StaticItem* it : groundItems) {
            if (yCheckRect.intersects(it->sceneBoundingRect())) {
                QRectF r = it->sceneBoundingRect();
                if (originalYVel > 0) { // 使用缓存的原始速度来判断是掉落还是顶头
                    player->setPos(player->x(), r.top() - pHeight);
                    player->y_vel = 0; player->state = Player::WALK; onGround = true;
                } else if (originalYVel < 0) {
                    player->setPos(player->x(), r.bottom());
                    player->y_vel = 2; player->state = Player::FALL;
                }
            }
        }

        // 砖块、宝箱
        for (int i = 0; i < solidItems.size(); ++i) {
            TileItem* t = solidItems[i];
            if (yCheckRect.intersects(t->sceneBoundingRect())) {
                QRectF r = t->sceneBoundingRect();
                if (originalYVel > 0) {
                    player->setPos(player->x(), r.top() - pHeight);
                    player->y_vel = 0; player->state = Player::WALK; onGround = true;
                } else if (originalYVel < 0) {
                    // 顶碎砖块
                    if (t->type == TileItem::BRICK && player->isBig) {
                        scene->removeItem(t); delete t; solidItems.removeAt(i--);
                        player->y_vel = 1; score += 50;
                        continue;
                    } else {
                        player->setPos(player->x(), r.bottom());
                        player->y_vel = 2; player->state = Player::FALL;
                        if (!t->isUsed && t->type == TileItem::BOX) {
                            t->isUsed = true;
                            if (qrand() % 5 == 0 && !player->isBig) {
                                spawnMushroom(static_cast<int>(t->x()), static_cast<int>(t->y() - 16));
                            } else {
                                spawnBumpingCoin(static_cast<int>(t->x()), static_cast<int>(t->y() - 10));
                                coins++;
                            }
                            score += 100;
                        }
                        if (t->type == TileItem::BOX) t->bump();
                    }
                }
            }
        }

    // 防悬空探测
    if (!onGround && player->state != Player::JUMP) {
        player->setPos(player->x(), player->y() + 1);
        bool touchAnything = false;
        QRectF floatCheckRect = player->sceneBoundingRect().adjusted(2, 0, -2, 0);
        for (StaticItem* it : groundItems) if (floatCheckRect.intersects(it->sceneBoundingRect())) touchAnything = true;
        for (TileItem* it : solidItems) if (floatCheckRect.intersects(it->sceneBoundingRect())) touchAnything = true;
        player->setPos(player->x(), player->y() - 1);

        if (!touchAnything) player->state = Player::FALL;
        else onGround = true;
    }

    // ============= 物品与敌人 =============
    for (TileItem* c : coinsList) c->updateLogic();
    for (TileItem* s : solidItems) s->updateLogic();

    // 吃静态悬浮金币
    for (int i = 0; i < coinsList.size(); ++i) {
        if (player->collidesWithItem(coinsList[i])) {
            scene->removeItem(coinsList[i]); delete coinsList[i];
            coinsList.removeAt(i--); coins++; score += 100;
        }
    }

    // 敌人互动
    for (int i = 0; i < enemies.size(); ++i) {
            Enemy* e = enemies[i];

            e->updateLogic();
            if (e->isRemovable) { scene->removeItem(e); delete e; enemies.removeAt(i--); continue; }
            if (e->state == Enemy::SQUISHED) continue;

            // 1. 敌人地形物理 (落地与撞墙反弹)
            bool eOnGround = false;
            QRectF eCheckY = e->sceneBoundingRect().adjusted(4, 0, -4, 0);
            QRectF eCheckX = e->sceneBoundingRect().adjusted(0, 4, 0, -4);

            auto checkEnemyCollision = [&](QGraphicsItem* it) {
                QRectF r = it->sceneBoundingRect();

                // Y轴落地检测
                if (eCheckY.intersects(r)) {
                    if (e->y_vel > 0 && (e->y() + e->pixmap().height() - e->y_vel) <= r.top() + 8) {
                        e->setPos(e->x(), r.top() - e->pixmap().height());
                        e->y_vel = 0;
                        eOnGround = true;
                    }
                }

                // X轴撞墙反弹
                if (eCheckX.intersects(r)) {
                    if (e->x_vel > 0) {
                        e->setPos(r.left() - e->pixmap().width(), e->y());
                    } else if (e->x_vel < 0) {
                        e->setPos(r.right(), e->y());
                    }
                    e->x_vel *= -1;
                    e->facingRight = !e->facingRight;
                }
            };

            for (StaticItem* it : groundItems) checkEnemyCollision(it);
            for (TileItem* it : solidItems) checkEnemyCollision(it);

            if (!eOnGround && e->state != Enemy::SHELL_IDLE) {
                e->y_vel += C::GRAVITY * 0.5;
            }

            // 2. 龟壳击杀其他怪物
            if (e->state == Enemy::SHELL_SLIDING) {
                for (int j = 0; j < enemies.size(); ++j) {
                    if (i == j) continue;
                    Enemy* e2 = enemies[j];
                    if (e2->state != Enemy::SQUISHED && e->collidesWithItem(e2)) {
                        e2->dieToShell();
                        score += 500;
                    }
                }
            }

            // 3. 马里奥与怪物的互动
            if (player->collidesWithItem(e)) {
                // 判定踩怪的条件改为“中心点比较”
                if (player->y_vel > 0 && (player->y() + pHeight / 2) < (e->y() + e->pixmap().height() / 2)) {
                    e->stomped();
                    player->y_vel = player->jump_vel * 0.5;
                    score += 100;
                } else {
                    // 平行接触
                    if (e->state == Enemy::SHELL_IDLE) {
                        // 踢飞龟壳
                        bool fromLeft = player->x() < e->x();
                        e->kicked(fromLeft);
                        // 赋予龟壳一个初始位移脱离碰撞体积
                        e->setPos(e->x() + (fromLeft ? 5 : -5), e->y());
                    } else if (e->state == Enemy::SHELL_SLIDING || e->state == Enemy::WALK) {
                        // 受到伤害
                        if (player->isBig) {
                            player->isBig = false;
                            player->setScale(1.0);
                            player->setPos(player->x(), player->y() + player->pixmap().height() * 0.5);
                            e->x_vel *= -1;
                            e->facingRight = !e->facingRight;
                            e->setPos(e->x() + (e->x() > player->x() ? 20 : -20), e->y());
                        } else {
                            player->goDie();
                        }
                    }
                }
            }
        }
    // 弹出的金币动画
    for (int i = 0; i < bumpingCoins.size(); ++i) {
        BumpingCoin* bc = bumpingCoins[i];
        if (bc->updateLogic()) { scene->removeItem(bc); delete bc; bumpingCoins.removeAt(i--); }
    }

    // 蘑菇移动与拾取逻辑
    for (int i = 0; i < mushrooms.size(); ++i) {
        Mushroom* m = mushrooms[i];
        m->setPos(m->x() + m->x_vel, m->y() + m->y_vel);
        m->y_vel += C::GRAVITY * 0.5;
        for (StaticItem* g : groundItems) {
            if (m->collidesWithItem(g) && m->y_vel > 0) {
                m->setPos(m->x(), g->sceneBoundingRect().top() - m->pixmap().height());
                m->y_vel = 0;
            }
        }
        if (player->collidesWithItem(m)) {
            player->becomeBig(); score += 1000;
            scene->removeItem(m); delete m; mushrooms.removeAt(i--);
            continue;
        }
        if (m->y() > C::SCREEN_H) { scene->removeItem(m); delete m; mushrooms.removeAt(i--); }
    }

    updateCamera();
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
    QPixmap ts(":/graphics/tile_set.png"), is(":/graphics/item_objects.png");
    QJsonArray cs = mapData["coin"].toArray();
    for (auto v : cs) {
        QJsonObject c = v.toObject();
        TileItem* cn = new TileItem(c["x"].toInt(), c["y"].toInt(), TileItem::COIN, is, C::BG_MULTI);
        scene->addItem(cn); coinsList.append(cn);
    }
    QJsonArray bs = mapData["brick"].toArray();
    for (auto v : bs) {
        QJsonObject b = v.toObject();
        TileItem* br = new TileItem(b["x"].toInt(), b["y"].toInt(), TileItem::BRICK, ts, C::BG_MULTI);
        scene->addItem(br); solidItems.append(br);
    }
    QJsonArray bxs = mapData["box"].toArray();
    for (auto v : bxs) {
        QJsonObject bx = v.toObject();
        TileItem* box = new TileItem(bx["x"].toInt(), bx["y"].toInt(), TileItem::BOX, ts, C::BG_MULTI);
        scene->addItem(box); solidItems.append(box);
    }
}

void GameWindow::setupEnemies() {
    QPixmap sheet(":/graphics/enemies.png");
    QJsonArray groups = mapData["enemy"].toArray();
    for (auto gv : groups) {
        QJsonObject go = gv.toObject();
        for (const QString& k : go.keys()) {
            int gid = k.toInt();
            QJsonArray el = go[k].toArray();
            QList<Enemy*> ge;
            for (auto ev : el) {
                QJsonObject e = ev.toObject();
                Enemy* en = new Enemy(e["x"].toInt(), e["y"].toInt(), e["type"].toInt(), sheet);
                ge.append(en);
            }
            inactiveEnemies[gid] = ge;
        }
    }
}

void GameWindow::setupCheckpoints() {
    QJsonArray cps = mapData["checkpoint"].toArray();
    for (auto v : cps) {
        QJsonObject o = v.toObject();
        if (o["type"].toInt() == 0) {
            StaticItem* cp = new StaticItem(o["x"].toInt(), o["y"].toInt(), o["width"].toInt(), o["height"].toInt(), "checkpoint");
            cp->enemyGroupId = o["enemy_groupid"].toInt();
            scene->addItem(cp);
            checkpoints.append(cp);
        }
    }
}

void GameWindow::setupFlagpole() {
    QPixmap ts(":/graphics/tile_set.png");
    QJsonArray fs = mapData["flagpole"].toArray();
    for (auto v : fs) {
        QJsonObject f = v.toObject(); int t = f["type"].toInt(); QPixmap s;
        if (t == 0) s = ts.copy(256, 128, 16, 16);
        else if (t == 1) s = ts.copy(256, 144, 16, 16); else s = ts.copy(240, 128, 16, 16);
        QGraphicsPixmapItem* it = new QGraphicsPixmapItem(s.scaled(static_cast<int>(16 * C::BG_MULTI), static_cast<int>(16 * C::BG_MULTI)));
        it->setPos(f["x"].toInt(), f["y"].toInt()); it->setZValue(5);
        scene->addItem(it); flagpoleItems.append(it);
    }
}

void GameWindow::updateCamera() {
    if (player->isDead || isLevelFinished) return;
    double st = this->width() / 3.0;
    if (player->x() > st) {
        this->centerOn(static_cast<int>(player->x() + st), C::SCREEN_H / 2); // 防抖
    }
}

void GameWindow::drawForeground(QPainter *p, const QRectF &r) {
    Q_UNUSED(r); p->save(); p->resetTransform(); p->setPen(Qt::white);
    p->setFont(QFont("Courier", 16, QFont::Bold));
    p->drawText(50, 30, "MARIO"); p->drawText(50, 55, QString("%1").arg(score, 6, 10, QChar('0')));
    p->drawText(300, 30, "COINS"); p->drawText(300, 55, QString("x %1").arg(coins, 2, 10, QChar('0')));
    p->drawText(500, 30, "WORLD"); p->drawText(500, 55, "1-1");
    p->drawText(700, 30, "TIME"); p->drawText(700, 55, QString("%1").arg(gameTime, 3, 10, QChar('0')));
    if (isLevelFinished) { p->setPen(Qt::yellow); p->setFont(QFont("Courier", 36, QFont::Bold)); p->drawText(this->rect(), Qt::AlignCenter, "COURSE CLEAR!"); }
    p->restore();
}

void GameWindow::keyPressEvent(QKeyEvent *e) {
    if (!e->isAutoRepeat()) {
        if (e->key() == Qt::Key_Left) keyLeft = true;
        if (e->key() == Qt::Key_Right) keyRight = true;
        if (e->key() == Qt::Key_Up) keyUp = true;
    }
}

void GameWindow::keyReleaseEvent(QKeyEvent *e) {
    if (!e->isAutoRepeat()) {
        if (e->key() == Qt::Key_Left) keyLeft = false;
        if (e->key() == Qt::Key_Right) keyRight = false;
        if (e->key() == Qt::Key_Up) keyUp = false;
    }
}

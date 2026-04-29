#include "gamewindow.h"
#include "constants.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

// 【修复警告】：调整了初始化列表的顺序，使其与头文件声明顺序一致
GameWindow::GameWindow(QWidget *parent) : QGraphicsView(parent),
    keyEnter(false), keyLeft(false), keyRight(false), keyUp(false), isLevelFinished(false)
{
    this->setFixedSize(C::SCREEN_W, C::SCREEN_H);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setWindowTitle("Super Mario Qt v5.0 - Final Edition");
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    // 初始化状态机变量
    currentState = MENU;
    menuSelection = 0;
    lives = 3;
    stateTimer = 0;
    endPhase = 0;
    flagItem = nullptr;
    // 【修改】：初始化 topScore
    score = 0; coins = 0; gameTime = 300; timerTickCount = 0; topScore = 0;

    QPixmap titleSheet(":/graphics/title_screen.png");
    titleLogo = titleSheet.copy(1, 60, 176, 88).scaled(static_cast<int>(176 * C::BG_MULTI), static_cast<int>(88 * C::BG_MULTI));

    QPixmap marioSheet(":/graphics/mario_bros.png");
    loadingMario = marioSheet.copy(178, 32, 12, 16).scaled(static_cast<int>(12 * C::PLAYER_MULTI), static_cast<int>(16 * C::PLAYER_MULTI));

    QPixmap itemSheet(":/graphics/item_objects.png");
    menuCursor = itemSheet.copy(24, 160, 8, 8).scaled(static_cast<int>(8 * C::PLAYER_MULTI), static_cast<int>(8 * C::PLAYER_MULTI));

    scene = new QGraphicsScene(this);
    this->setScene(scene);

    initScene();
    loadMapData();

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

    player = new Player("mario");
    player->setPos(110, C::GROUND_HEIGHT - player->pixmap().height());
    player->setZValue(10);
    scene->addItem(player);
}

void GameWindow::resetLevel() {
    for (auto it : groundItems) { scene->removeItem(it); delete it; } groundItems.clear();
    for (auto it : solidItems) { scene->removeItem(it); delete it; } solidItems.clear();
    for (auto it : coinsList) { scene->removeItem(it); delete it; } coinsList.clear();
    for (auto it : enemies) { scene->removeItem(it); delete it; } enemies.clear();
    for (auto it : mushrooms) { scene->removeItem(it); delete it; } mushrooms.clear();
    for (auto it : bumpingCoins) { scene->removeItem(it); delete it; } bumpingCoins.clear();
    for (auto it : checkpoints) { scene->removeItem(it); delete it; } checkpoints.clear();
    for (auto it : flagpoleItems) { scene->removeItem(it); delete it; } flagpoleItems.clear();
    inactiveEnemies.clear();

    setupGroundItems();
    setupBricksBoxesAndCoins();
    setupEnemies();
    setupCheckpoints();
    setupFlagpole();

    player->show();
    player->setScale(1.0);
    player->isBig = false;
    player->isDead = false;
    player->state = Player::STAND;
    player->x_vel = 0;
    player->y_vel = 0;
    player->setPos(110, C::GROUND_HEIGHT - player->pixmap().height());

    gameTime = 300;
    timerTickCount = 0;
    endPhase = 0;
    this->centerOn(C::SCREEN_W / 2, C::SCREEN_H / 2);

    keyLeft = false; keyRight = false; keyUp = false;
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
    // 【修改】：实时更新最高分
    if (score > topScore) {
        topScore = score;
    }

    if (currentState == MENU) {
        stateTimer++;
        if (keyEnter) {
            currentState = LOADING;
            stateTimer = 0;
            lives = 3; score = 0; coins = 0;
        }
        scene->update();
        return;
    }

    if (currentState == LOADING) {
        stateTimer++;
        if (stateTimer > 120) {
            currentState = PLAYING;
            stateTimer = 0;
            resetLevel();
        }
        scene->update();
        return;
    }

    if (currentState == GAMEOVER) {
        stateTimer++;
        if (stateTimer > 180) {
            currentState = MENU;
            stateTimer = 0;
            // 【修改】：重置场景回到原点，修复穿帮
            resetLevel();
        }
        scene->update();
        return;
    }

    // 通关结算大动画状态机
    if (currentState == END_SEQUENCE) {
        if (endPhase == 0) {
            // 阶段 0：滑下旗杆
            if (player->y() < C::GROUND_HEIGHT - player->pixmap().height()) {
                player->setPos(player->x(), player->y() + 3);
                player->updateLogic(false, false, false);
            }
            if (flagItem && flagItem->y() < 420) {
                flagItem->setPos(flagItem->x(), flagItem->y() + 3);
            }
            if (player->y() >= C::GROUND_HEIGHT - player->pixmap().height()) {
                player->setPos(player->x(), C::GROUND_HEIGHT - player->pixmap().height());
                endPhase = 1;
            }
        }
        else if (endPhase == 1) {
            // 阶段 1：向右走向城堡
            player->updateLogic(false, true, false);
            player->setPos(player->x() + player->x_vel, player->y());

            if (player->x() >= 8760) {
                player->hide();
                endPhase = 2;
                stateTimer = 0;
            }
        }
        else if (endPhase == 2) {
            // 阶段 2：时间转换为分数
            if (gameTime > 0) {
                gameTime--;
                score += 50;
            } else {
                stateTimer++;
                if (stateTimer > 180) {
                    currentState = MENU;
                    // 【修改】：重置场景回到原点，修复穿帮
                    resetLevel();
                }
            }
        }
        updateCamera();
        scene->update();
        return;
    }

    // 死亡复活与扣命逻辑
    if (player->isDead) {
        stateTimer++;
        player->y_vel += (stateTimer < 30) ? C::ANTI_GRAVITY : C::GRAVITY;
        player->setPos(player->x(), player->y() + player->y_vel);

        if (stateTimer > 180) {
            lives--;
            if (lives > 0) currentState = LOADING;
            else currentState = GAMEOVER;
            stateTimer = 0;
        }
        return;
    }

    // 终点旗杆触发检测
    if (player->x() >= 8470 && player->x() < 8500) {
        currentState = END_SEQUENCE;
        endPhase = 0;
        player->x_vel = 0;
        player->y_vel = 0;
        player->setPos(8455, player->y());
        return;
    }

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
    if (timerTickCount >= 60) {
        gameTime--;
        timerTickCount = 0;
        if (gameTime <= 0) player->goDie();
    }

    // 跳崖判定
    if (player->y() > C::SCREEN_H) {
        player->goDie();
    }

    player->updateLogic(keyLeft, keyRight, keyUp);
    double pWidth = player->pixmap().width() * player->scale();
    double pHeight = player->pixmap().height() * player->scale();

    // ============= X 轴物理=============
    player->setPos(player->x() + player->x_vel, player->y());
    auto hX = [&](QGraphicsItem* it) {
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
    double originalYVel = player->y_vel;
    player->setPos(player->x(), player->y() + player->y_vel);
    bool onGround = false;
    QRectF yCheckRect = player->sceneBoundingRect().adjusted(2, 0, -2, 0);

    for (StaticItem* it : groundItems) {
        if (yCheckRect.intersects(it->sceneBoundingRect())) {
            QRectF r = it->sceneBoundingRect();
            if (originalYVel > 0) {
                player->setPos(player->x(), r.top() - pHeight);
                player->y_vel = 0; player->state = Player::WALK; onGround = true;
            } else if (originalYVel < 0) {
                player->setPos(player->x(), r.bottom());
                player->y_vel = 2; player->state = Player::FALL;
            }
        }
    }

    for (int i = 0; i < solidItems.size(); ++i) {
        TileItem* t = solidItems[i];
        if (yCheckRect.intersects(t->sceneBoundingRect())) {
            QRectF r = t->sceneBoundingRect();
            if (originalYVel > 0) {
                player->setPos(player->x(), r.top() - pHeight);
                player->y_vel = 0; player->state = Player::WALK; onGround = true;
            } else if (originalYVel < 0) {
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

    for (TileItem* c : coinsList) c->updateLogic();
    for (TileItem* s : solidItems) s->updateLogic();

    for (int i = 0; i < coinsList.size(); ++i) {
        if (player->collidesWithItem(coinsList[i])) {
            scene->removeItem(coinsList[i]); delete coinsList[i];
            coinsList.removeAt(i--); coins++; score += 100;
        }
    }

    for (int i = 0; i < enemies.size(); ++i) {
        Enemy* e = enemies[i];
        e->updateLogic();
        if (e->isRemovable) { scene->removeItem(e); delete e; enemies.removeAt(i--); continue; }
        if (e->state == Enemy::SQUISHED) continue;

        bool eOnGround = false;
        QRectF eCheckY = e->sceneBoundingRect().adjusted(4, 0, -4, 0);
        QRectF eCheckX = e->sceneBoundingRect().adjusted(0, 4, 0, -4);

        auto checkEnemyCollision = [&](QGraphicsItem* it) {
            QRectF r = it->sceneBoundingRect();
            if (eCheckY.intersects(r)) {
                if (e->y_vel > 0 && (e->y() + e->pixmap().height() - e->y_vel) <= r.top() + 8) {
                    e->setPos(e->x(), r.top() - e->pixmap().height());
                    e->y_vel = 0;
                    eOnGround = true;
                }
            }
            if (eCheckX.intersects(r)) {
                if (e->x_vel > 0) e->setPos(r.left() - e->pixmap().width(), e->y());
                else if (e->x_vel < 0) e->setPos(r.right(), e->y());
                e->x_vel *= -1;
                e->facingRight = !e->facingRight;
            }
        };

        for (StaticItem* it : groundItems) checkEnemyCollision(it);
        for (TileItem* it : solidItems) checkEnemyCollision(it);

        if (!eOnGround && e->state != Enemy::SHELL_IDLE) {
            e->y_vel += C::GRAVITY * 0.5;
        }

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

        if (player->collidesWithItem(e)) {
            if (player->y_vel > 0 && (player->y() + pHeight / 2) < (e->y() + e->pixmap().height() / 2)) {
                e->stomped();
                player->y_vel = player->jump_vel * 0.5;
                score += 100;
            } else {
                if (e->state == Enemy::SHELL_IDLE) {
                    bool fromLeft = player->x() < e->x();
                    e->kicked(fromLeft);
                    e->setPos(e->x() + (fromLeft ? 5 : -5), e->y());
                } else if (e->state == Enemy::SHELL_SLIDING || e->state == Enemy::WALK) {
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

    for (int i = 0; i < bumpingCoins.size(); ++i) {
        BumpingCoin* bc = bumpingCoins[i];
        if (bc->updateLogic()) { scene->removeItem(bc); delete bc; bumpingCoins.removeAt(i--); }
    }

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

        if (t == 2) {
            flagItem = it;
            flagItem->setZValue(4);
        }
    }
}

void GameWindow::updateCamera() {
    if (player->isDead) return;

    double st = C::SCREEN_W / 3.0;
    double targetX = C::SCREEN_W / 2.0;

    if (player->x() > st) {
        targetX = player->x() + st;
    }

    double maxTargetX = 9086 - (C::SCREEN_W / 2.0);

    if (targetX > maxTargetX) {
        targetX = maxTargetX;
    }

    this->centerOn(targetX, C::SCREEN_H / 2);
}

void GameWindow::drawForeground(QPainter *p, const QRectF &r) {
    Q_UNUSED(r); p->save(); p->resetTransform();

    // 顶部状态栏
    p->setPen(Qt::white);
    p->setFont(QFont("Courier", 16, QFont::Bold));
    p->drawText(50, 30, "MARIO"); p->drawText(50, 55, QString("%1").arg(score, 6, 10, QChar('0')));
    p->drawText(300, 30, "COINS"); p->drawText(300, 55, QString("x %1").arg(coins, 2, 10, QChar('0')));
    p->drawText(500, 30, "WORLD"); p->drawText(500, 55, "1-1");
    p->drawText(700, 30, "TIME");
    int displayTime = (currentState == PLAYING || currentState == END_SEQUENCE) ? gameTime : 0;
    p->drawText(700, 55, QString("%1").arg(displayTime, 3, 10, QChar('0')));

    // 状态机 UI 绘制
    if (currentState == MENU) {
        p->drawPixmap(170, 100, titleLogo);
        p->setPen(Qt::white);
        p->setFont(QFont("Courier", 18, QFont::Bold));
        p->drawText(QRect(272, 360, 300, 40), Qt::AlignLeft | Qt::AlignTop, "1  PLAYER  GAME");
        p->drawText(QRect(272, 405, 300, 40), Qt::AlignLeft | Qt::AlignTop, "2  PLAYER  GAME");

        // 【修改】：使用动态加载的 topScore 变量
        p->drawText(QRect(290, 465, 300, 40), Qt::AlignLeft | Qt::AlignTop, QString("TOP - %1").arg(topScore, 6, 10, QChar('0')));

        int cursorY = (menuSelection == 0) ? 362 : 407;
        p->drawPixmap(220, cursorY, menuCursor);
    }
    else if (currentState == LOADING) {
        p->fillRect(0, 0, C::SCREEN_W, C::SCREEN_H, Qt::black);
        p->setPen(Qt::white);
        p->setFont(QFont("Courier", 24, QFont::Bold));
        p->drawText(0, C::SCREEN_H / 2 - 80, C::SCREEN_W, 50, Qt::AlignCenter, "WORLD  1-1");
        p->drawPixmap(C::SCREEN_W / 2 - 40, C::SCREEN_H / 2, loadingMario);
        p->drawText(C::SCREEN_W / 2 + 10, C::SCREEN_H / 2 + 25, QString("x %1").arg(lives));
    }
    else if (currentState == GAMEOVER) {
        p->fillRect(0, 0, C::SCREEN_W, C::SCREEN_H, Qt::black);
        p->setPen(Qt::white);
        p->setFont(QFont("Courier", 36, QFont::Bold));
        p->drawText(0, 0, C::SCREEN_W, C::SCREEN_H, Qt::AlignCenter, "GAME OVER");
    }

    // 通关结算文字
    if (currentState == END_SEQUENCE && endPhase == 2) {
        p->setPen(Qt::yellow); p->setFont(QFont("Courier", 36, QFont::Bold));
        p->drawText(this->rect(), Qt::AlignCenter, "COURSE CLEAR!");
    }
    p->restore();
}

void GameWindow::keyPressEvent(QKeyEvent *e) {
    if (!e->isAutoRepeat()) {
        if (e->key() == Qt::Key_Left) keyLeft = true;
        if (e->key() == Qt::Key_Right) keyRight = true;
        if (e->key() == Qt::Key_Up) {
            keyUp = true;
            if (currentState == MENU) menuSelection = 0;
        }
        if (e->key() == Qt::Key_Down) {
            if (currentState == MENU) menuSelection = 1;
        }
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) keyEnter = true;
    }
}

void GameWindow::keyReleaseEvent(QKeyEvent *e) {
    if (!e->isAutoRepeat()) {
        if (e->key() == Qt::Key_Left) keyLeft = false;
        if (e->key() == Qt::Key_Right) keyRight = false;
        if (e->key() == Qt::Key_Up) keyUp = false;
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) keyEnter = false;
    }
}

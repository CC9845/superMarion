#include "gamewindow.h"
#include "constants.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>

// 构造函数：初始化游戏窗口、设置、媒体播放器等
GameWindow::GameWindow(QWidget *parent) : QGraphicsView(parent),
    keyEnter(false), keyLeft(false), keyRight(false), keyUp(false), isLevelFinished(false)
{
    // 基础窗口设置
    this->setFixedSize(C::SCREEN_W, C::SCREEN_H);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setWindowTitle("Super Mario Qt v5.0 - Ultimate Audio Edition");
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    // 初始化游戏状态变量
    currentState = MENU;
    menuSelection = 0;
    lives = 3;
    stateTimer = 0;
    endPhase = 0;
    flagItem = nullptr;

    score = 0; coins = 0; gameTime = 200; timerTickCount = 0; topScore = 0;

    // 加载和裁剪UI图像资源
    QPixmap titleSheet(":/graphics/title_screen.png");
    titleLogo = titleSheet.copy(1, 60, 176, 88).scaled(static_cast<int>(176 * C::BG_MULTI), static_cast<int>(88 * C::BG_MULTI));

    QPixmap marioSheet(":/graphics/mario_bros.png");
    loadingMario = marioSheet.copy(178, 32, 12, 16).scaled(static_cast<int>(12 * C::PLAYER_MULTI), static_cast<int>(16 * C::PLAYER_MULTI));
    loadingLuigi = marioSheet.copy(178, 128, 12, 16).scaled(static_cast<int>(12 * C::PLAYER_MULTI), static_cast<int>(16 * C::PLAYER_MULTI));

    QPixmap itemSheet(":/graphics/item_objects.png");
    menuCursor = itemSheet.copy(24, 160, 8, 8).scaled(static_cast<int>(8 * C::PLAYER_MULTI), static_cast<int>(8 * C::PLAYER_MULTI));

    // 初始化音频系统
    bgmPlaylist = new QMediaPlaylist(this);
    bgmPlaylist->addMedia(QUrl("qrc:/music/main_theme.wav"));
    bgmPlaylist->setPlaybackMode(QMediaPlaylist::Loop);
    bgmPlayer = new QMediaPlayer(this);
    bgmPlayer->setPlaylist(bgmPlaylist);

    sfxJump = new QSoundEffect(this); sfxJump->setSource(QUrl("qrc:/sound/small_jump.wav"));
    sfxCoin = new QSoundEffect(this); sfxCoin->setSource(QUrl("qrc:/sound/coin.wav"));
    sfxStomp = new QSoundEffect(this); sfxStomp->setSource(QUrl("qrc:/sound/stomp.wav"));
    sfxDie = new QSoundEffect(this); sfxDie->setSource(QUrl("qrc:/music/death.wav"));
    sfxClear = new QSoundEffect(this); sfxClear->setSource(QUrl("qrc:/music/stage_clear.wav"));
    sfxFlagpole = new QSoundEffect(this); sfxFlagpole->setSource(QUrl("qrc:/music/flagpole.wav"));
    sfxGameOver = new QSoundEffect(this); sfxGameOver->setSource(QUrl("qrc:/music/game_over.wav"));
    sfxPowerupAppears = new QSoundEffect(this); sfxPowerupAppears->setSource(QUrl("qrc:/sound/powerup_appears.wav"));
    sfxPowerup = new QSoundEffect(this); sfxPowerup->setSource(QUrl("qrc:/sound/powerup.wav"));
    sfxBump = new QSoundEffect(this); sfxBump->setSource(QUrl("qrc:/sound/bump.wav"));
    sfxSmash = new QSoundEffect(this); sfxSmash->setSource(QUrl("qrc:/sound/brick_smash.wav"));
    sfxKick = new QSoundEffect(this); sfxKick->setSource(QUrl("qrc:/sound/kick.wav"));

    scene = new QGraphicsScene(this);
    this->setScene(scene);

    initScene();
    loadMapData();

    // 核心游戏循环定时器)
    timer = new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, &QTimer::timeout, this, &GameWindow::gameLoop);
    timer->start(16);

    bgmPlaylist->setCurrentIndex(0);
    bgmPlayer->play();
}

// 初始化场景
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

// 重置关卡
void GameWindow::resetLevel() {
    // 内存清理
    for (auto it : groundItems) { scene->removeItem(it); delete it; } groundItems.clear();
    for (auto it : solidItems) { scene->removeItem(it); delete it; } solidItems.clear();
    for (auto it : coinsList) { scene->removeItem(it); delete it; } coinsList.clear();
    for (auto it : enemies) { scene->removeItem(it); delete it; } enemies.clear();
    for (auto it : mushrooms) { scene->removeItem(it); delete it; } mushrooms.clear();
    for (auto it : bumpingCoins) { scene->removeItem(it); delete it; } bumpingCoins.clear();
    for (auto it : checkpoints) { scene->removeItem(it); delete it; } checkpoints.clear();
    for (auto it : flagpoleItems) { scene->removeItem(it); delete it; } flagpoleItems.clear();
    inactiveEnemies.clear();

    // 重新从数据加载地图元素
    setupGroundItems();
    setupBricksBoxesAndCoins();
    setupEnemies();
    setupCheckpoints();
    setupFlagpole();

    // 重置玩家
    if (player != nullptr) {
        scene->removeItem(player);
        delete player;
    }

    QString characterName = (menuSelection == 0) ? "mario" : "luigi";
    player = new Player(characterName);
    player->setZValue(10);
    scene->addItem(player);

    // 恢复玩家默认状态
    player->show();
    player->setScale(1.0);
    player->isBig = false;
    player->isDead = false;
    player->state = Player::STAND;
    player->x_vel = 0;
    player->y_vel = 0;
    player->setPos(110, C::GROUND_HEIGHT - player->pixmap().height());

    // 重置游戏时间和计时器
    gameTime = 200;
    timerTickCount = 0;
    endPhase = 0;
    this->centerOn(C::SCREEN_W / 2, C::SCREEN_H / 2); // 镜头居中

    keyLeft = false; keyRight = false; keyUp = false;

    bgmPlaylist->setCurrentIndex(0);
    bgmPlayer->play();
}

// 顶出金币的特效生成
void GameWindow::spawnBumpingCoin(int x, int y) {
    QPixmap is(":/graphics/item_objects.png");
    BumpingCoin* bc = new BumpingCoin(x, y, is, C::BG_MULTI);
    scene->addItem(bc);
    bumpingCoins.append(bc);
    sfxCoin->play();
}

// 顶出蘑菇的生成
void GameWindow::spawnMushroom(int x, int y) {
    QPixmap is(":/graphics/item_objects.png");
    Mushroom* m = new Mushroom(x, y, is, C::BG_MULTI);
    scene->addItem(m);
    mushrooms.append(m);
    sfxPowerupAppears->play();
}

// 核心游戏循环
void GameWindow::gameLoop() {
    if (score > topScore) topScore = score;

    //主菜单
    if (currentState == MENU) {
        stateTimer++;
        if (keyEnter) { currentState = LOADING; stateTimer = 0; lives = 3; score = 0; coins = 0; }
        scene->update(); return;
    }

    //加载界面
    if (currentState == LOADING) {
        stateTimer++;
        if (stateTimer > 120) { currentState = PLAYING; stateTimer = 0; resetLevel(); }
        scene->update(); return;
    }

    //游戏结束界面
    if (currentState == GAMEOVER) {
        stateTimer++;
        if (stateTimer > 250) { currentState = MENU; stateTimer = 0; resetLevel(); bgmPlayer->stop(); }
        scene->update(); return;
    }

    // 过关动画序列
    if (currentState == END_SEQUENCE) {
        // 从旗杆滑下
        if (endPhase == 0) {
            if (player->y() < C::GROUND_HEIGHT - player->pixmap().height()) {
                player->setPos(player->x(), player->y() + 3);
                player->updateLogic(false, false, false);
            }
            if (flagItem && flagItem->y() < 420) { flagItem->setPos(flagItem->x(), flagItem->y() + 3); }
            if (player->y() >= C::GROUND_HEIGHT - player->pixmap().height()) {
                player->setPos(player->x(), C::GROUND_HEIGHT - player->pixmap().height());
                endPhase = 1;
            }
        }
        //走向城堡
        else if (endPhase == 1) {
            player->updateLogic(false, true, false); // 强制向右走
            player->setPos(player->x() + player->x_vel, player->y());

            if (player->x() >= 8760) {
                player->hide(); endPhase = 2; stateTimer = 0; sfxClear->play();
            }
        }
        //时间结算加分
        else if (endPhase == 2) {
            if (gameTime > 0) { gameTime--; score += 50; }
            else {
                stateTimer++;
                if (stateTimer > 350) { currentState = MENU; resetLevel(); bgmPlayer->stop(); }
            }
        }
        updateCamera(); scene->update(); return;
    }

    //游戏进行中：玩家死亡动画
    if (player->isDead) {
        stateTimer++;
        // 经典的马里奥死亡抛物线：先向上弹起，后受重力下落
        player->y_vel += (stateTimer < 30) ? C::ANTI_GRAVITY : C::GRAVITY;
        player->setPos(player->x(), player->y() + player->y_vel);

        if (stateTimer > 180) {
            lives--;
            if (lives > 0) { currentState = LOADING; }
            else { currentState = GAMEOVER; sfxGameOver->play(); }
            stateTimer = 0;
        }
    } else {

        // 终点旗杆判定
        if (player->x() >= 8470 && player->x() < 8500) {
            currentState = END_SEQUENCE; endPhase = 0;
            player->x_vel = 0; player->y_vel = 0;
            player->setPos(8455, player->y());
            bgmPlayer->stop(); sfxFlagpole->play();
            return;
        }

        // 检查点判定：触发生成敌人
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

        // 游戏倒计时
        timerTickCount++;
        if (timerTickCount >= 60) {
            gameTime--; timerTickCount = 0;
            if (gameTime <= 0) { player->goDie(); bgmPlayer->stop(); sfxDie->play(); }
        }

        // 掉入深渊判定
        if (player->y() > C::SCREEN_H) { player->goDie(); bgmPlayer->stop(); sfxDie->play(); }

        // 更新玩家物理参数
        player->updateLogic(keyLeft, keyRight, keyUp);
        double pWidth = player->pixmap().width() * player->scale();
        double pHeight = player->pixmap().height() * player->scale();

        // 玩家 X 轴物理结算与碰撞检测
        player->setPos(player->x() + player->x_vel, player->y());
        auto hX = [&](QGraphicsItem* it) {
            //用于收缩碰撞框的上下边缘，防止在地面平移时产生不必要的 X 轴阻挡
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

        // 玩家 Y 轴物理结算与碰撞检测
        double originalYVel = player->y_vel;
        player->setPos(player->x(), player->y() + player->y_vel);
        bool onGround = false;
        // adjusted(2, 0, -2, 0) 用于收缩碰撞框的左右边缘，使得玩家在贴墙下落时不会判定为踩在墙上
        QRectF yCheckRect = player->sceneBoundingRect().adjusted(2, 0, -2, 0);

        // 处理与静态地面/水管的 Y 轴碰撞
        for (StaticItem* it : groundItems) {
            if (yCheckRect.intersects(it->sceneBoundingRect())) {
                QRectF r = it->sceneBoundingRect();
                if (originalYVel > 0) { // 下落时撞到表面
                    player->setPos(player->x(), r.top() - pHeight);
                    player->y_vel = 0; player->state = Player::WALK; onGround = true;
                } else if (originalYVel < 0) { // 跳跃上升时撞到天花板
                    player->setPos(player->x(), r.bottom());
                    player->y_vel = 2; player->state = Player::FALL;
                }
            }
        }

        // 处理与砖块和问号箱的 Y 轴碰撞
        for (int i = 0; i < solidItems.size(); ++i) {
            TileItem* t = solidItems[i];
            if (yCheckRect.intersects(t->sceneBoundingRect())) {
                QRectF r = t->sceneBoundingRect();
                if (originalYVel > 0) { // 落在砖块上
                    player->setPos(player->x(), r.top() - pHeight);
                    player->y_vel = 0; player->state = Player::WALK; onGround = true;
                } else if (originalYVel < 0) { // 顶砖块
                    if (t->type == TileItem::BRICK && player->isBig) {
                        // 大马里奥碎砖
                        scene->removeItem(t); delete t; solidItems.removeAt(i--);
                        player->y_vel = 1; score += 50;
                        sfxSmash->play();
                        continue;
                    } else {
                        player->setPos(player->x(), r.bottom());
                        player->y_vel = 2; player->state = Player::FALL;

                        if (!t->isUsed && t->type == TileItem::BOX) {
                            // 顶未使用的问号箱
                            t->isUsed = true;
                            if (qrand() % 5 == 0 && !player->isBig) {
                                spawnMushroom(static_cast<int>(t->x()), static_cast<int>(t->y() - 16));
                            } else {
                                spawnBumpingCoin(static_cast<int>(t->x()), static_cast<int>(t->y() - 10));
                                coins++;
                            }
                            score += 100;
                        } else if (t->type == TileItem::BRICK || (t->type == TileItem::BOX && t->isUsed)) {
                            sfxBump->play();
                        }
                        if (t->type == TileItem::BOX) t->bump(); // 触发箱子弹跳动画
                    }
                }
            }
        }

        // 悬空检测：检查脚下1像素是否有支撑物
        if (!onGround && player->state != Player::JUMP) {
            player->setPos(player->x(), player->y() + 1);
            bool touchAnything = false;
            QRectF floatCheckRect = player->sceneBoundingRect().adjusted(2, 0, -2, 0);
            for (StaticItem* it : groundItems) if (floatCheckRect.intersects(it->sceneBoundingRect())) touchAnything = true;
            for (TileItem* it : solidItems) if (floatCheckRect.intersects(it->sceneBoundingRect())) touchAnything = true;
            player->setPos(player->x(), player->y() - 1); // 检测完毕恢复位置

            if (!touchAnything) player->state = Player::FALL;
            else onGround = true;
        }

        // 收集散落金币的判定
        for (int i = 0; i < coinsList.size(); ++i) {
            if (player->collidesWithItem(coinsList[i])) {
                scene->removeItem(coinsList[i]); delete coinsList[i];
                coinsList.removeAt(i--); coins++; score += 100; sfxCoin->play();
            }
        }
    }

    // 更新场景内其他物品的动画和状态逻辑
    for (TileItem* c : coinsList) c->updateLogic();
    for (TileItem* s : solidItems) s->updateLogic();

    //敌人逻辑与物理结算
    for (int i = 0; i < enemies.size(); ++i) {
        Enemy* e = enemies[i];

        e->updateLogic(); // 更新敌人的动画帧

        if (e->isRemovable) { scene->removeItem(e); delete e; enemies.removeAt(i--); continue; }
        if (e->state == Enemy::SQUISHED) continue; // 已经被踩扁的敌人不进行物理判定

        double eScale = (e->scale() == 0 ? 1.0 : e->scale());
        double eActualWidth = e->pixmap().width() * eScale;
        double eActualHeight = e->pixmap().height() * eScale;

        // 敌人 X 轴碰撞检测与阻挡
        // 这里的逻辑确保敌人只有在其坐标确确实实越过了砖块边缘时，才会被修正坐标和反转速度
        auto checkEnemyX = [&](QGraphicsItem* it) {
            QRectF r = it->sceneBoundingRect();
            QRectF eCheckX = e->sceneBoundingRect().adjusted(0, 4, 0, -4);
            if (eCheckX.intersects(r)) {
                if (e->x_vel > 0 && e->x() < r.left()) { // 向右走且碰到了左侧墙壁
                    e->setPos(r.left() - eActualWidth, e->y());
                    e->x_vel *= -1;
                    e->facingRight = !e->facingRight;
                }
                else if (e->x_vel < 0 && (e->x() + eActualWidth) > r.right()) { // 向左走且碰到了右侧墙壁
                    e->setPos(r.right(), e->y());
                    e->x_vel *= -1;
                    e->facingRight = !e->facingRight;
                }
            }
        };
        for (StaticItem* it : groundItems) checkEnemyX(it);
        for (TileItem* it : solidItems) checkEnemyX(it);

        // 敌人施加重力
        e->y_vel += 0.5;
        e->setPos(e->x(), e->y() + e->y_vel);

        // 敌人Y轴落地检测
        bool eOnGround = false;
        auto checkEnemyY = [&](QGraphicsItem* it) {
            QRectF r = it->sceneBoundingRect();
            QRectF eCheckY = e->sceneBoundingRect().adjusted(4, 0, -4, 0);
            if (eCheckY.intersects(r)) {
                if (e->y_vel >= 0 && (e->y() + eActualHeight - e->y_vel) <= r.top() + 16) {
                    // 如果正在下落，且其底部越过了物体表面，则将其托举到表面
                    e->setPos(e->x(), r.top() - eActualHeight);
                    e->y_vel = 0;
                    eOnGround = true;
                }
            }
        };
        for (StaticItem* it : groundItems) checkEnemyY(it);
        for (TileItem* it : solidItems) checkEnemyY(it);

        // 踢出的龟壳击杀其他敌人的逻辑
        if (e->state == Enemy::SHELL_SLIDING) {
            for (int j = 0; j < enemies.size(); ++j) {
                if (i == j) continue; // 不与自己判定
                Enemy* e2 = enemies[j];
                if (e2->state != Enemy::SQUISHED && e->collidesWithItem(e2)) {
                    e2->dieToShell(); score += 500; sfxStomp->play();
                }
            }
        }

        // 玩家与怪物的交互判定
        if (!player->isDead && player->collidesWithItem(e)) {
            double pHeight = player->pixmap().height() * player->scale();
            // 如果玩家处于下落状态，且相对位置高于怪物中心，则判定为成功踩踏
            if (player->y_vel > 0 && (player->y() + pHeight / 2) < (e->y() + eActualHeight / 2)) {
                e->stomped(); player->y_vel = player->jump_vel * 0.5; score += 100; sfxStomp->play();
            } else {
                if (e->state == Enemy::SHELL_IDLE) {
                    // 踢飞静止的龟壳
                    bool fromLeft = player->x() < e->x();
                    e->kicked(fromLeft);
                    e->setPos(e->x() + (fromLeft ? 5 : -5), e->y()); sfxKick->play();
                } else if (e->state == Enemy::SHELL_SLIDING || e->state == Enemy::WALK) {
                    // 受到伤害
                    if (player->isBig) {
                        player->isBig = false; player->setScale(1.0); // 变小
                        player->setPos(player->x(), player->y() + player->pixmap().height() * 0.5); // 调整位置以防穿模
                        e->x_vel *= -1; e->facingRight = !e->facingRight; // 弹开怪物
                        e->setPos(e->x() + (e->x() > player->x() ? 20 : -20), e->y());
                    } else {
                        player->goDie(); bgmPlayer->stop(); sfxDie->play();
                    }
                }
            }
        }
    }

    // 更新顶出的金币特效
    for (int i = 0; i < bumpingCoins.size(); ++i) {
        BumpingCoin* bc = bumpingCoins[i];
        if (bc->updateLogic()) { scene->removeItem(bc); delete bc; bumpingCoins.removeAt(i--); }
    }

    //蘑菇的物理结算
    for (int i = 0; i < mushrooms.size(); ++i) {
        Mushroom* m = mushrooms[i];

        // 蘑菇 X 轴移动与碰撞检测
        m->setPos(m->x() + m->x_vel, m->y());
        auto checkMushroomX = [&](QGraphicsItem* it) {
            QRectF r = it->sceneBoundingRect();
            QRectF mCheckX = m->sceneBoundingRect().adjusted(0, 4, 0, -4);
            if (mCheckX.intersects(r)) {
                if (m->x_vel > 0 && m->x() < r.left()) {
                    m->setPos(r.left() - m->pixmap().width(), m->y());
                    m->x_vel *= -1; // 撞墙反弹
                }
                else if (m->x_vel < 0 && (m->x() + m->pixmap().width()) > r.right()) {
                    m->setPos(r.right(), m->y());
                    m->x_vel *= -1;
                }
            }
        };
        for (StaticItem* g : groundItems) checkMushroomX(g);
        for (TileItem* s : solidItems) checkMushroomX(s);

        // 蘑菇 Y 轴重力与支撑检测
        m->y_vel += C::GRAVITY * 0.5;
        m->setPos(m->x(), m->y() + m->y_vel);
        auto checkMushroomY = [&](QGraphicsItem* it) {
            QRectF r = it->sceneBoundingRect();
            QRectF mCheckY = m->sceneBoundingRect().adjusted(4, 0, -4, 0);
            if (mCheckY.intersects(r)) {
                if (m->y_vel >= 0 && (m->y() + m->pixmap().height() - m->y_vel) <= r.top() + 16) {
                    m->setPos(m->x(), r.top() - m->pixmap().height()); // 落在物体表面
                    m->y_vel = 0;
                }
            }
        };
        for (StaticItem* g : groundItems) checkMushroomY(g);
        for (TileItem* s : solidItems) checkMushroomY(s);

        // 吃蘑菇判定
        if (!player->isDead && player->collidesWithItem(m)) {
            player->becomeBig(); score += 1000;
            scene->removeItem(m); delete m; mushrooms.removeAt(i--);
            sfxPowerup->play(); continue;
        }

        // 蘑菇掉入深渊自动销毁
        if (m->y() > C::SCREEN_H) { scene->removeItem(m); delete m; mushrooms.removeAt(i--); }
    }

    updateCamera();
}

// 解析 JSON 地图数据
void GameWindow::loadMapData() {
    QFile file(":/data/level_1.json");
    if (file.open(QIODevice::ReadOnly)) {
        mapData = QJsonDocument::fromJson(file.readAll()).object(); file.close();
    }
}

// 生成地面、水管和台阶
void GameWindow::setupGroundItems() {
    QStringList names = {"ground", "pipe", "step"};
    for (const QString &name : names) {
        QJsonArray items = mapData[name].toArray();
        for (auto v : items) {
            QJsonObject i = v.toObject();
            StaticItem* si = new StaticItem(i["x"].toInt(), i["y"].toInt(), i["width"].toInt(), i["height"].toInt(), name);
            scene->addItem(si); groundItems.append(si);
        }
    }
}

// 生成砖块、问号箱和散落金币
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
        int startX = b["x"].toInt();
        int startY = b["y"].toInt();
        int num = b.contains("brick_num") ? b["brick_num"].toInt() : 1;
        int dir = b.contains("direction") ? b["direction"].toInt() : 0; // 0=水平 1=垂直

        for (int k = 0; k < num; ++k) {
            int bx = startX + (dir == 0 ? k * 43 : 0);
            int by = startY + (dir == 1 ? k * 43 : 0);

            TileItem* br = new TileItem(bx, by, TileItem::BRICK, ts, C::BG_MULTI);
            scene->addItem(br); solidItems.append(br);
        }
    }
    QJsonArray bxs = mapData["box"].toArray();
    for (auto v : bxs) {
        QJsonObject bx = v.toObject();
        TileItem* box = new TileItem(bx["x"].toInt(), bx["y"].toInt(), TileItem::BOX, ts, C::BG_MULTI);
        scene->addItem(box); solidItems.append(box);
    }
}

// 生成敌人组
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

                double actualHeight = en->boundingRect().height() * (en->scale() == 0 ? 1.0 : en->scale());
                en->setPos(e["x"].toInt(), e["y"].toInt() - actualHeight); // 自动放置在指定 y 坐标上方

                ge.append(en);
            }
            inactiveEnemies[gid] = ge; // 加入待激活列表，等待触发检查点
        }
    }
}

// 生成触发检查点
void GameWindow::setupCheckpoints() {
    QJsonArray cps = mapData["checkpoint"].toArray();
    for (auto v : cps) {
        QJsonObject o = v.toObject();
        if (o["type"].toInt() == 0) { // type 0 为生成敌人的检查点
            StaticItem* cp = new StaticItem(o["x"].toInt(), o["y"].toInt(), o["width"].toInt(), o["height"].toInt(), "checkpoint");
            cp->enemyGroupId = o["enemy_groupid"].toInt();
            scene->addItem(cp); checkpoints.append(cp);
        }
    }
}

// 生成过关旗杆组件
void GameWindow::setupFlagpole() {
    QPixmap ts(":/graphics/tile_set.png");
    QPixmap is(":/graphics/item_objects.png");

    QJsonArray fs = mapData["flagpole"].toArray();
    for (auto v : fs) {
        QJsonObject f = v.toObject();
        int t = f["type"].toInt();
        QPixmap s;

        // 根据类型切割不同部件的贴图
        if (t == 0) s = ts.copy(256, 128, 16, 16); // 旗杆顶球
        else if (t == 1) s = ts.copy(256, 144, 16, 16); // 旗杆躯干
        else s = is.copy(128, 32, 16, 16); // 旗子本身

        QGraphicsPixmapItem* it = new QGraphicsPixmapItem(s.scaled(static_cast<int>(16 * C::BG_MULTI), static_cast<int>(16 * C::BG_MULTI)));

        double posX = f["x"].toInt(); double posY = f["y"].toInt();
        double offsetX = 0; double offsetY = 0;

        // 调整视觉偏移量对齐贴图
        if (t == 0) { offsetX = 13.8; offsetY = -23; }
        else if (t == 1) { offsetX = 0; offsetY = 0; }
        else if (t == 2) { offsetX = -4; offsetY = 0; }

        it->setPos(posX + offsetX * C::BG_MULTI, posY + offsetY * C::BG_MULTI);
        it->setZValue(5); scene->addItem(it); flagpoleItems.append(it);

        if (t == 2) { flagItem = it; flagItem->setZValue(6); } // 记录旗子对象以备动画使用
    }
}

// 镜头跟随逻辑
void GameWindow::updateCamera() {
    if (player->isDead) return;

    double playerScreenPos = C::SCREEN_W / 3.0; // 玩家倾向于在屏幕左侧 1/3 处
    double targetX = player->x() + (C::SCREEN_W / 2.0 - playerScreenPos);

    if (targetX < C::SCREEN_W / 2.0) targetX = C::SCREEN_W / 2.0; // 防止左侧看穿边界
    double maxTargetX = 9086 - (C::SCREEN_W / 2.0); // 关卡末尾最大限制
    if (targetX > maxTargetX) targetX = maxTargetX;

    this->centerOn(targetX, C::SCREEN_H / 2);
}

// 渲染前景UI
void GameWindow::drawForeground(QPainter *p, const QRectF &r) {
    Q_UNUSED(r); p->save(); p->resetTransform();

    // 绘制顶部的HUD数据
    p->setPen(Qt::white); p->setFont(QFont("Courier", 16, QFont::Bold));
    p->drawText(50, 30, "MARIO"); p->drawText(50, 55, QString("%1").arg(score, 6, 10, QChar('0')));
    p->drawText(300, 30, "COINS"); p->drawText(300, 55, QString("x %1").arg(coins, 2, 10, QChar('0')));
    p->drawText(500, 30, "WORLD"); p->drawText(500, 55, "1-1");
    p->drawText(700, 30, "TIME");
    int displayTime = (currentState == PLAYING || currentState == END_SEQUENCE) ? gameTime : 0;
    p->drawText(700, 55, QString("%1").arg(displayTime, 3, 10, QChar('0')));

    // 绘制不同的状态界面覆盖层
    if (currentState == MENU) {
        p->drawPixmap(170, 100, titleLogo);
        p->setPen(Qt::white); p->setFont(QFont("Courier", 18, QFont::Bold));
        p->drawText(QRect(272, 360, 300, 40), Qt::AlignLeft | Qt::AlignTop, "1  PLAYER  GAME");
        p->drawText(QRect(272, 405, 300, 40), Qt::AlignLeft | Qt::AlignTop, "2  PLAYER  GAME");
        p->drawText(QRect(290, 465, 300, 40), Qt::AlignLeft | Qt::AlignTop, QString("TOP - %1").arg(topScore, 6, 10, QChar('0')));
        int cursorY = (menuSelection == 0) ? 362 : 407;
        p->drawPixmap(220, cursorY, menuCursor);
    }
    else if (currentState == LOADING) {
        p->fillRect(0, 0, C::SCREEN_W, C::SCREEN_H, Qt::black);
        p->setPen(Qt::white); p->setFont(QFont("Courier", 24, QFont::Bold));
        p->drawText(0, C::SCREEN_H / 2 - 80, C::SCREEN_W, 50, Qt::AlignCenter, "WORLD  1-1");

        if (menuSelection == 0) p->drawPixmap(C::SCREEN_W / 2 - 40, C::SCREEN_H / 2, loadingMario);
        else p->drawPixmap(C::SCREEN_W / 2 - 40, C::SCREEN_H / 2, loadingLuigi);

        p->drawText(C::SCREEN_W / 2 + 10, C::SCREEN_H / 2 + 25, QString("x %1").arg(lives));
    }
    else if (currentState == GAMEOVER) {
        p->fillRect(0, 0, C::SCREEN_W, C::SCREEN_H, Qt::black);
        p->setPen(Qt::white); p->setFont(QFont("Courier", 36, QFont::Bold));
        p->drawText(0, 0, C::SCREEN_W, C::SCREEN_H, Qt::AlignCenter, "GAME OVER");
    }

    if (currentState == END_SEQUENCE && endPhase == 2) {
        p->setPen(Qt::yellow); p->setFont(QFont("Courier", 36, QFont::Bold));
        p->drawText(this->rect(), Qt::AlignCenter, "COURSE CLEAR!");
    }
    p->restore();
}

// 键盘按下事件捕获
void GameWindow::keyPressEvent(QKeyEvent *e) {
    if (!e->isAutoRepeat()) {
        if (e->key() == Qt::Key_Left || e->key() == Qt::Key_A) keyLeft = true;
        if (e->key() == Qt::Key_Right || e->key() == Qt::Key_D) keyRight = true;

        if (e->key() == Qt::Key_Up || e->key() == Qt::Key_W) {
            keyUp = true;
            if (currentState == MENU) menuSelection = 0;
            if (currentState == PLAYING && player->state != Player::JUMP && player->state != Player::FALL && !player->isDead) {
                sfxJump->play();
            }
        }

        if (e->key() == Qt::Key_Down || e->key() == Qt::Key_S) {
            if (currentState == MENU) menuSelection = 1;
        }
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) keyEnter = true;
    }
}

// 键盘松开事件捕获
void GameWindow::keyReleaseEvent(QKeyEvent *e) {
    if (!e->isAutoRepeat()) {
        if (e->key() == Qt::Key_Left || e->key() == Qt::Key_A) keyLeft = false;
        if (e->key() == Qt::Key_Right || e->key() == Qt::Key_D) keyRight = false;
        if (e->key() == Qt::Key_Up || e->key() == Qt::Key_W) keyUp = false;
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) keyEnter = false;
    }
}

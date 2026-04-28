#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QKeyEvent>
#include <QList>
#include <QJsonObject>
#include <QMap>
#include <QPainter>

#include "player.h"
#include "staticitem.h"
#include "tileitem.h"
#include "enemy.h"
#include "bumpingcoin.h"
#include "mushroom.h"

class GameWindow : public QGraphicsView {
    Q_OBJECT
public:
    explicit GameWindow(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private slots:
    void gameLoop();

private:
    QGraphicsScene *scene;
    QGraphicsPixmapItem *background;
    QTimer *timer;
    Player *player;

    bool keyLeft, keyRight, keyUp;
    bool isLevelFinished;
    int score, coins, gameTime, timerTickCount;

    QJsonObject mapData;

    QList<StaticItem*> groundItems;
    QList<TileItem*> solidItems;
    QList<TileItem*> coinsList;
    QList<Enemy*> enemies;
    QList<QGraphicsPixmapItem*> flagpoleItems;
    QMap<int, QList<Enemy*>> inactiveEnemies;
    QList<StaticItem*> checkpoints;

    // 金币和蘑菇的管理容器
    QList<BumpingCoin*> bumpingCoins;
    QList<Mushroom*> mushrooms;

    void initScene();
    void loadMapData();
    void setupGroundItems();
    void setupBricksBoxesAndCoins();
    void setupEnemies();
    void setupCheckpoints();
    void setupFlagpole();
    void updateCamera();

    // 生成道具的方法
    void spawnBumpingCoin(int x, int y);
    void spawnMushroom(int x, int y);
};

#endif // GAMEWINDOW_H

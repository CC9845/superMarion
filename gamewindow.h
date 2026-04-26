#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QKeyEvent>
#include <QList>
#include <QJsonObject>

#include "player.h"
#include "staticitem.h"
#include "tileitem.h"
#include "enemy.h"

class GameWindow : public QGraphicsView {
    Q_OBJECT
public:
    explicit GameWindow(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();

private:
    QGraphicsScene *scene;
    QGraphicsPixmapItem *background;
    QTimer *timer;

    Player *player;

    bool keyLeft;
    bool keyRight;
    bool keyUp;

    QJsonObject mapData;
    QList<StaticItem*> groundItems;
    QList<TileItem*> solidItems;
    QList<TileItem*> coinsList;
    QList<Enemy*> enemies;

    void initScene();
    void loadMapData();
    void setupGroundItems();
    void setupBricksBoxesAndCoins();
    void setupEnemies();

    void updateCamera();
};

#endif // GAMEWINDOW_H

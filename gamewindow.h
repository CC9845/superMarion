#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QKeyEvent>
#include <QList>
#include <QJsonObject>

// 前置声明
class StaticItem;
class TileItem;
class Enemy;

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
    QGraphicsPixmapItem *mario;
    QTimer *timer;

    double x_vel;
    double y_vel;
    bool isJumping;
    bool keyLeft;
    bool keyRight;

    QJsonObject mapData;
    QList<StaticItem*> groundItems;
    QList<TileItem*> interactiveItems;
    QList<Enemy*> enemies;

    void initScene();
    void loadMapData();
    void setupGroundItems();
    void setupBricksBoxesAndCoins();
    void setupEnemies();
    void updateCamera();
};

#endif // GAMEWINDOW_H

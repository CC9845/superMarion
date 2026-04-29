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

    enum GameState { MENU, LOADING, PLAYING, END_SEQUENCE, GAMEOVER };
    GameState currentState;
    int lives;
    int stateTimer;
    bool keyEnter;

    // 结算动画专用变量
    int endPhase;
    QGraphicsPixmapItem* flagItem;

    QPixmap titleLogo;
    QPixmap loadingMario;
    QPixmap menuCursor;
    int menuSelection;

    void resetLevel();

    bool keyLeft, keyRight, keyUp;
    bool isLevelFinished;

    // 【关键修复】：把中文逗号改成了英文逗号！
    int score, coins, gameTime, timerTickCount, topScore;

    QJsonObject mapData;

    QList<StaticItem*> groundItems;
    QList<TileItem*> solidItems;
    QList<TileItem*> coinsList;
    QList<Enemy*> enemies;
    QList<QGraphicsPixmapItem*> flagpoleItems;
    QMap<int, QList<Enemy*>> inactiveEnemies;
    QList<StaticItem*> checkpoints;

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

    void spawnBumpingCoin(int x, int y);
    void spawnMushroom(int x, int y);
};

#endif // GAMEWINDOW_H

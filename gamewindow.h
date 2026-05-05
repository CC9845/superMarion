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

// 多媒体模块头文件
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QSoundEffect>

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

    int endPhase;
    QGraphicsPixmapItem* flagItem;

    QPixmap titleLogo;
    QPixmap loadingMario;
    QPixmap loadingLuigi; // 新增：用于显示 Luigi 的加载界面贴图
    QPixmap menuCursor;
    int menuSelection;

    QMediaPlayer *bgmPlayer;
    QMediaPlaylist *bgmPlaylist;
    QSoundEffect *sfxJump;
    QSoundEffect *sfxCoin;
    QSoundEffect *sfxStomp;
    QSoundEffect *sfxDie;
    QSoundEffect *sfxClear;
    QSoundEffect *sfxFlagpole;
    QSoundEffect *sfxGameOver;
    QSoundEffect *sfxPowerupAppears;
    QSoundEffect *sfxPowerup;
    QSoundEffect *sfxBump;  // 顶普通砖块/空箱子的声音
    QSoundEffect *sfxSmash; // 顶碎砖块的声音
    QSoundEffect *sfxKick;  // 踢飞龟壳的声音

    void resetLevel();

    bool keyLeft, keyRight, keyUp;
    bool isLevelFinished;
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

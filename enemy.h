#ifndef ENEMY_H
#define ENEMY_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QList>

class Enemy : public QGraphicsPixmapItem {
public:
    enum EnemyType { GOOMBA = 0, KOOPA = 1 }; // 0:蘑菇, 1:乌龟
    enum EnemyState { WALK, SQUISHED, SHELL_IDLE, SHELL_SLIDING }; // 行走, 踩扁, 静止龟壳, 滑行龟壳

    Enemy(int x, int y, int typeFlag, const QPixmap& sheet);
    void updateLogic();
    void stomped();           // 触发被踩的逻辑
    void kicked(bool fromLeft); // 龟壳被踢
    void dieToShell();        // 被龟壳击杀

    EnemyType type;
    EnemyState state;
    bool isRemovable;
    double x_vel;
    double y_vel;
    bool facingRight;

private:
    QList<QPixmap> walkFramesLeft;
    QList<QPixmap> walkFramesRight;
    QPixmap squishedSprite; // 蘑菇踩扁的贴图
    QPixmap shellSprite;    // 龟壳贴图

    int frameIndex;
    int animationTimer;
    int deathTimer;
};

#endif // ENEMY_H

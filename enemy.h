#ifndef ENEMY_H
#define ENEMY_H

#include <QGraphicsPixmapItem>
#include <QPixmap>

class Enemy : public QGraphicsPixmapItem {
public:
    Enemy(int x, int y, int type, const QPixmap& sheet);
    void updateLogic();  // 巡逻和死亡计时更新
    void goDie();        // 触发被踩扁的状态

    bool isDead;         // 是否已被踩扁
    bool isRemovable;    // 是否动画播放完毕

private:
    double x_vel;
    QPixmap normalSprite;
    QPixmap squishedSprite; // 被踩扁的贴图
    int deathTimer;         // 死亡动画计时器
};

#endif // ENEMY_H

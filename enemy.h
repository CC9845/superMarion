#ifndef ENEMY_H
#define ENEMY_H

#include <QGraphicsPixmapItem>
#include <QPixmap>

class Enemy : public QGraphicsPixmapItem {
public:
    Enemy(int x, int y, int type, const QPixmap& sheet);
    void updateLogic();

private:
    double x_vel;
};

#endif // ENEMY_H

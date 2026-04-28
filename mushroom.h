#ifndef MUSHROOM_H
#define MUSHROOM_H

#include <QGraphicsPixmapItem>
#include <QPixmap>

class Mushroom : public QGraphicsPixmapItem {
public:
    Mushroom(int x, int y, const QPixmap& sheet, double scale);
    double x_vel;
    double y_vel;
};

#endif // MUSHROOM_H

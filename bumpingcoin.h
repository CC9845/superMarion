#ifndef BUMPINGCOIN_H
#define BUMPINGCOIN_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QList>

class BumpingCoin : public QGraphicsPixmapItem {
public:

    BumpingCoin(double x, double y, const QPixmap& sheet, double scale);
    bool updateLogic();

private:
    double y_vel;
    double originalY;
    QList<QPixmap> frames;
    int frameIndex;
    int animationTimer;
};

#endif

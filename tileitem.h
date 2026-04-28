#ifndef TILEITEM_H
#define TILEITEM_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QList>

class TileItem : public QGraphicsPixmapItem {
public:
    enum TileType { BRICK, BOX, COIN };

    TileItem(int x, int y, TileType type, const QPixmap& sheet, double scale);

    void bump();
    void updateLogic();

    TileType type;
    bool isUsed;

private:
    bool isBumping;
    double originalY;
    double bumpY_vel;
    QList<QPixmap> frames;
    QPixmap emptyBoxSprite;
    int frameIndex;
    int animationTimer;
};
#endif

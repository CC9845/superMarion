#ifndef TILEITEM_H
#define TILEITEM_H

#include <QGraphicsPixmapItem>
#include <QPixmap>

class TileItem : public QGraphicsPixmapItem {
public:
    TileItem(int x, int y, const QPixmap& sheet, QRect rect, double scale);
};

#endif // TILEITEM_H

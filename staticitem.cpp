#include "staticitem.h"
#include <QBrush>
#include <QPen>

StaticItem::StaticItem(int x, int y, int w, int h, QString name)
    : itemName(name)
{
    // 设置碰撞矩形的大小
    setRect(0, 0, w, h);
    setPos(x, y);

    // 设置为无边框和无填充
    setPen(Qt::NoPen);
    setBrush(Qt::NoBrush);
}

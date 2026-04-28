#include "staticitem.h"
#include <QPen>
#include <QBrush>

StaticItem::StaticItem(int x, int y, int w, int h, QString name)
    : itemName(name), enemyGroupId(-1)
{
    setRect(0, 0, w, h);
    setPos(x, y);
    // 设为透明碰撞盒
    setPen(Qt::NoPen);
    setBrush(Qt::NoBrush);
}

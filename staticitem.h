#ifndef STATICITEM_H
#define STATICITEM_H

#include <QGraphicsRectItem>
#include <QString>

class StaticItem : public QGraphicsRectItem {
public:
    // x, y, w, h 对应 JSON 中的坐标和宽高
    StaticItem(int x, int y, int w, int h, QString name);
    QString itemName;
};

#endif // STATICITEM_H

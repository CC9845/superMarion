#ifndef STATICITEM_H
#define STATICITEM_H

#include <QGraphicsRectItem>
#include <QString>

class StaticItem : public QGraphicsRectItem {
public:
    StaticItem(int x, int y, int w, int h, QString name);
    QString itemName;
    int enemyGroupId; // 存储检查点关联的敌人组 ID

};
#endif // STATICITEM_H

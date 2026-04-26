#ifndef TILEITEM_H
#define TILEITEM_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QList>

class TileItem : public QGraphicsPixmapItem {
public:
    enum TileType { BRICK, BOX, COIN };

    // 构造函数现在只需要传入类型和贴图集，内部自动抠图
    TileItem(int x, int y, TileType type, const QPixmap& sheet, double scale);

    void bump();         // 触发顶撞动画
    void updateLogic();  // 处理动画帧刷新和物理弹跳

    TileType type;
    bool isUsed;         // 宝箱是否已经被顶空

private:
    bool isBumping;      // 是否正在上下弹跳
    double originalY;    // 原始 Y 坐标
    double bumpY_vel;    // 弹跳时的垂直速度

    QList<QPixmap> frames;   // 存储闪烁的动画帧
    QPixmap emptyBoxSprite;  // 灰色空宝箱贴图
    int frameIndex;          // 当前播放到第几帧
    int animationTimer;      // 控制动画播放速度的计时器
};

#endif // TILEITEM_H

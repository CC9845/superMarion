#include "tileitem.h"
#include "constants.h"
#include <QPixmap>

// 构造函数实现
TileItem::TileItem(int x, int y, TileType type, const QPixmap& sheet, double scale)
    : type(type), isUsed(false), isBumping(false),
      bumpY_vel(0), frameIndex(0), animationTimer(0) // 严格按照头文件声明顺序初始化
{
    originalY = y;

    //  static_cast<int> 明确转换
    int sWidth = static_cast<int>(16 * scale);
    int sHeight = static_cast<int>(16 * scale);
    int cWidth = static_cast<int>(8 * scale);
    int cHeight = static_cast<int>(14 * scale);

    if (type == BRICK) {
        QPixmap p = sheet.copy(16, 0, 16, 16).scaled(sWidth, sHeight);
        frames.append(p);
        setPixmap(p);
    }
    else if (type == BOX) {
        // 裁剪并缩放宝箱动画帧
        frames.append(sheet.copy(384, 0, 16, 16).scaled(sWidth, sHeight));
        frames.append(sheet.copy(400, 0, 16, 16).scaled(sWidth, sHeight));
        frames.append(sheet.copy(416, 0, 16, 16).scaled(sWidth, sHeight));
        emptyBoxSprite = sheet.copy(432, 0, 16, 16).scaled(sWidth, sHeight);


        if(!frames.isEmpty()) setPixmap(frames[0]);
    }
    else if (type == COIN) {
        // 裁剪并缩放金币动画帧
        frames.append(sheet.copy(3, 98, 8, 14).scaled(cWidth, cHeight));
        frames.append(sheet.copy(19, 98, 8, 14).scaled(cWidth, cHeight));
        frames.append(sheet.copy(35, 98, 8, 14).scaled(cWidth, cHeight));

        if(!frames.isEmpty()) setPixmap(frames[0]);
    }
    setPos(x, y);
}

// 顶撞动画逻辑
void TileItem::bump() {
    if (type == COIN || isBumping || isUsed) return;
    isBumping = true;
    bumpY_vel = -4.0;
    if (type == BOX) {
        isUsed = true;
        setPixmap(emptyBoxSprite); // 变成空宝箱贴图
    }
}

// 帧更新与物理弹跳更新
void TileItem::updateLogic() {
    // 处理闪烁动画
    animationTimer++;
    if (animationTimer > 10 && !frames.isEmpty()) {
        animationTimer = 0;
        if ((type == BOX && !isUsed) || type == COIN) {
            frameIndex = (frameIndex + 1) % frames.size();
            setPixmap(frames[frameIndex]);
        }
    }

    // 处理弹跳物理逻辑
    if (isBumping) {
        setPos(x(), y() + bumpY_vel);
        bumpY_vel += C::GRAVITY;
        if (y() >= originalY) {
            setPos(x(), originalY);
            isBumping = false;
            bumpY_vel = 0;
        }
    }
}

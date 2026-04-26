#include "tileitem.h"
#include "constants.h"

TileItem::TileItem(int x, int y, TileType type, const QPixmap& sheet, double scale)
    : type(type), isUsed(false), isBumping(false), frameIndex(0), animationTimer(0)
{
    originalY = y;
    bumpY_vel = 0;

    // 1. 普通砖块 (静态)
    if (type == BRICK) {
        // 先把单张图片存入 p1，再设置贴图，
        QPixmap p1 = sheet.copy(16, 0, 16, 16).scaled(16 * scale, 16 * scale);
        frames.append(p1);
        setPixmap(p1);
    }
    // 2. 问号宝箱
    else if (type == BOX) {
        QPixmap p1 = sheet.copy(384, 0, 16, 16).scaled(16 * scale, 16 * scale);
        frames.append(p1);
        frames.append(sheet.copy(400, 0, 16, 16).scaled(16 * scale, 16 * scale));
        frames.append(sheet.copy(416, 0, 16, 16).scaled(16 * scale, 16 * scale));

        emptyBoxSprite = sheet.copy(432, 0, 16, 16).scaled(16 * scale, 16 * scale);
        setPixmap(p1);
    }
    // 3. 金币
    else if (type == COIN) {
        QPixmap p1 = sheet.copy(3, 98, 8, 14).scaled(8 * scale, 14 * scale);
        frames.append(p1);
        frames.append(sheet.copy(19, 98, 8, 14).scaled(8 * scale, 14 * scale));
        frames.append(sheet.copy(35, 98, 8, 14).scaled(8 * scale, 14 * scale));
        setPixmap(p1);
    }

    setPos(x, y);
}

void TileItem::bump() {
    // 如果是金币、正在弹跳中、或者已经空了的宝箱，不能再被顶起
    if (type == COIN || isBumping || isUsed) return;

    isBumping = true;
    bumpY_vel = -4.0; // 给予向上的弹跳初速度

    // 如果是宝箱，顶完后变成死方块
    if (type == BOX) {
        isUsed = true;
        setPixmap(emptyBoxSprite);
    }
}

void TileItem::updateLogic() {
    //处理闪烁动画
    animationTimer++;
    if (animationTimer > 10) { // 调整数值控制闪烁速度
        animationTimer = 0;
        if (type == BOX && !isUsed) {
            frameIndex = (frameIndex + 1) % 3;
            setPixmap(frames.at(frameIndex));
        } else if (type == COIN) {
            frameIndex = (frameIndex + 1) % 3;
            setPixmap(frames.at(frameIndex));
        }
    }

    // 处理弹跳物理逻辑
    if (isBumping) {
        setPos(x(), y() + bumpY_vel);
        bumpY_vel += C::GRAVITY;

        // 如果落回原处，停止弹跳
        if (y() >= originalY) {
            setPos(x(), originalY);
            isBumping = false;
            bumpY_vel = 0;
        }
    }
}

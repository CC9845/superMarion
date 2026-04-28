#include "bumpingcoin.h"
#include "constants.h"

BumpingCoin::BumpingCoin(double x, double y, const QPixmap& sheet, double scale)
    : y_vel(-6.0), frameIndex(0), animationTimer(0)
{
    originalY = y;


    QPixmap f1 = sheet.copy(3, 98, 8, 14).scaled(static_cast<int>(8 * scale), static_cast<int>(14 * scale));
    QPixmap f2 = sheet.copy(19, 98, 8, 14).scaled(static_cast<int>(8 * scale), static_cast<int>(14 * scale));
    QPixmap f3 = sheet.copy(35, 98, 8, 14).scaled(static_cast<int>(8 * scale), static_cast<int>(14 * scale));

    frames << f1 << f2 << f3 << f2;


    if(!frames.isEmpty()) {
        setPixmap(frames[0]);
    }

    setPos(x + 4 * scale, y);
    setZValue(2);
}

bool BumpingCoin::updateLogic() {
    animationTimer++;
    if (animationTimer > 3 && !frames.isEmpty()) {
        animationTimer = 0;
        frameIndex = (frameIndex + 1) % frames.size();
        setPixmap(frames[frameIndex]);
    }

    setPos(x(), y() + y_vel);
    y_vel += C::GRAVITY * 0.5; // 使用物理常量进行重力计算

    // 当金币掉落回起跳点附近时通知主引擎销毁
    if (y_vel > 0 && y() >= originalY - 16) {
        return true;
    }
    return false;
}

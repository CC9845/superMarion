#include "mushroom.h"

Mushroom::Mushroom(int x, int y, const QPixmap& sheet, double scale) {
    QPixmap m = sheet.copy(0, 0, 16, 16).scaled(16 * scale, 16 * scale);
    setPixmap(m);
    setPos(x, y);
    x_vel = 2.0;   // 蘑菇一出生就向右跑
    y_vel = -4.0;  // 从宝箱里弹出的初始向上速度
    setZValue(2);
}

#include "tileitem.h"

TileItem::TileItem(int x, int y, const QPixmap& sheet, QRect rect, double scale) {
    // 裁剪贴图并缩放
    QPixmap sprite = sheet.copy(rect);
    setPixmap(sprite.scaled(sprite.width() * scale, sprite.height() * scale));
    setPos(x, y);
}

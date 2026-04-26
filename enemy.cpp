#include "enemy.h"
#include "constants.h"

Enemy::Enemy(int x, int y, int type, const QPixmap& sheet) {
    // 蘑菇怪 Goomba 在 enemies.png 中的第一帧裁剪区域
    QPixmap sprite = sheet.copy(0, 16, 16, 16);

    // 如果图片为空，可能是资源加载失败
    if (!sprite.isNull()) {
        setPixmap(sprite.scaled(sprite.width() * C::ENEMY_MULTI, sprite.height() * C::ENEMY_MULTI));
    }


    setPos(x, y - pixmap().height());

    // 初始向左巡逻速度
    x_vel = -C::ENEMY_SPEED;
}

void Enemy::updateLogic() {
    setPos(x() + x_vel, y());

}

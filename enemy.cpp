#include "enemy.h"
#include "constants.h"

Enemy::Enemy(int x, int y, int type, const QPixmap& sheet)
    : isDead(false), isRemovable(false), deathTimer(0)
{
    // 正常状态贴图
    normalSprite = sheet.copy(0, 16, 16, 16).scaled(16 * C::ENEMY_MULTI, 16 * C::ENEMY_MULTI);
    // 被踩扁的贴图
    squishedSprite = sheet.copy(32, 16, 16, 16).scaled(16 * C::ENEMY_MULTI, 16 * C::ENEMY_MULTI);

    setPixmap(normalSprite);
    setPos(x, y - pixmap().height());
    x_vel = -C::ENEMY_SPEED;
}

void Enemy::updateLogic() {
    if (!isDead) {
        // 活着的时候正常巡逻
        setPos(x() + x_vel, y());
    } else {
        // 死了之后静止不动，等待约 500ms 后标记为可删除
        deathTimer++;
        if (deathTimer > 30) {
            isRemovable = true;
        }
    }
}

void Enemy::goDie() {
    if (isDead) return;
    isDead = true;
    x_vel = 0; // 停止移动
    setPixmap(squishedSprite); // 切换为踩扁贴图
}

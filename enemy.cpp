#include "enemy.h"
#include "constants.h"
#include <QTransform>

Enemy::Enemy(int x, int y, int typeFlag, const QPixmap& sheet)
    : isRemovable(false), x_vel(-C::ENEMY_SPEED), y_vel(0)
{
    // 把这几个变量移到函数体内初始化
    frameIndex = 0;
    animationTimer = 0;
    deathTimer = 0;
    facingRight = false;

    type = (typeFlag == 1) ? KOOPA : GOOMBA;
    state = WALK;

    if (type == GOOMBA) {
        // 加载蘑菇帧
        QPixmap f1 = sheet.copy(0, 16, 16, 16).scaled(static_cast<int>(16 * C::ENEMY_MULTI), static_cast<int>(16 * C::ENEMY_MULTI));
        QPixmap f2 = sheet.copy(16, 16, 16, 16).scaled(static_cast<int>(16 * C::ENEMY_MULTI), static_cast<int>(16 * C::ENEMY_MULTI));
        walkFramesLeft << f1 << f2;
        walkFramesRight << f1 << f2;
        squishedSprite = sheet.copy(32, 16, 16, 16).scaled(static_cast<int>(16 * C::ENEMY_MULTI), static_cast<int>(16 * C::ENEMY_MULTI));
    } else {
        // 加载乌龟帧
        QPixmap f1 = sheet.copy(96, 9, 16, 22).scaled(static_cast<int>(16 * C::ENEMY_MULTI), static_cast<int>(22 * C::ENEMY_MULTI));
        QPixmap f2 = sheet.copy(112, 9, 16, 22).scaled(static_cast<int>(16 * C::ENEMY_MULTI), static_cast<int>(22 * C::ENEMY_MULTI));
        walkFramesLeft << f1 << f2;

        walkFramesRight << f1.transformed(QTransform().scale(-1, 1))
                        << f2.transformed(QTransform().scale(-1, 1));

        shellSprite = sheet.copy(160, 9, 16, 22).scaled(static_cast<int>(16 * C::ENEMY_MULTI), static_cast<int>(22 * C::ENEMY_MULTI));
    }

    setPixmap(walkFramesLeft[0]);
    setPos(x, y - pixmap().height());
}

void Enemy::updateLogic() {
    if (state == WALK) {
        setPos(x() + x_vel, y());
        animationTimer++;
        if (animationTimer > 10) {
            animationTimer = 0;
            frameIndex = (frameIndex + 1) % 2;
            setPixmap(facingRight ? walkFramesRight[frameIndex] : walkFramesLeft[frameIndex]);
        }
    }
    else if (state == SQUISHED) {
        deathTimer++;
        if (deathTimer > 30) isRemovable = true;
    }
    else if (state == SHELL_IDLE) {
        x_vel = 0; // 静止龟壳
    }
    else if (state == SHELL_SLIDING) {
        setPos(x() + x_vel, y()); // 高速滑动龟壳
    }

    // 基础重力
    if (state != SQUISHED) {
        setPos(x(), y() + y_vel);
        y_vel += C::GRAVITY * 0.5;
    }
}

void Enemy::stomped() {
    if (state == WALK) {
        if (type == GOOMBA) {
            state = SQUISHED;
            x_vel = 0;
            setPixmap(squishedSprite);
        } else if (type == KOOPA) {
            state = SHELL_IDLE; // 乌龟被踩变成静止龟壳
            x_vel = 0;
            setPixmap(shellSprite);
        }
    } else if (state == SHELL_SLIDING) {
        state = SHELL_IDLE; // 滑动的龟壳被踩，强行停下
        x_vel = 0;
    }
}

void Enemy::kicked(bool fromLeft) {
    if (state == SHELL_IDLE) {
        state = SHELL_SLIDING;
        // 龟壳滑动速度极快
        x_vel = fromLeft ? C::ENEMY_SPEED * 6 : -C::ENEMY_SPEED * 6;
    }
}

void Enemy::dieToShell() {
    state = SQUISHED;
    setPixmap(type == GOOMBA ? squishedSprite : shellSprite);
    isRemovable = true;
}

#include "player.h"
#include "constants.h"
#include <QTransform>
#include <QtMath>

Player::Player(const QPixmap& sheet)
    : x_vel(0), y_vel(0), state(STAND), facingRight(true), isDead(false),
      frameIndex(0), animationTimer(0), canJump(true)
{
    max_x_vel = C::MAX_WALK_SPEED;
    x_accel = C::WALK_ACCEL;

    loadFrames(sheet);
    setPixmap(rightFrames[0]);
}

void Player::loadFrames(const QPixmap& sheet) {
    static const struct { int x, y, w, h; } rects[] = {
        {178, 32, 12, 16}, {80, 32, 15, 16}, {96, 32, 16, 16},
        {112, 32, 16, 16}, {144, 32, 16, 16}, {130, 32, 14, 16},
        {160, 32, 15, 16}
    };

    for (int i = 0; i < 7; ++i) {
        QPixmap img = sheet.copy(rects[i].x, rects[i].y, rects[i].w, rects[i].h);
        QPixmap scaled = img.scaled(rects[i].w * C::PLAYER_MULTI, rects[i].h * C::PLAYER_MULTI);

        rightFrames.append(scaled);
        leftFrames.append(scaled.transformed(QTransform().scale(-1, 1)));
    }
}

void Player::updateLogic(bool keyLeft, bool keyRight, bool keyUp) {
    if (isDead) return;

    if (!keyUp) canJump = true;

    handleStates(keyLeft, keyRight, keyUp);

    setPixmap(facingRight ? rightFrames[frameIndex] : leftFrames[frameIndex]);
}

void Player::handleStates(bool keyLeft, bool keyRight, bool keyUp) {
    switch (state) {
        case STAND: stand(keyLeft, keyRight, keyUp); break;
        case WALK:  walk(keyLeft, keyRight, keyUp);  break;
        case JUMP:  jump(keyLeft, keyRight, keyUp);  break;
        case FALL:  fall(keyLeft, keyRight, keyUp);  break;
        default: break;
    }
}

void Player::stand(bool keyLeft, bool keyRight, bool keyUp) {
    frameIndex = 0;
    x_vel = 0;
    y_vel = 0;

    if (keyRight) {
        facingRight = true;
        state = WALK;
    } else if (keyLeft) {
        facingRight = false;
        state = WALK;
    } else if (keyUp && canJump) {
        state = JUMP;
        y_vel = C::JUMP_VELOCITY;
    }
}

void Player::walk(bool keyLeft, bool keyRight, bool keyUp) {
    if (keyUp && canJump) {
        state = JUMP;
        y_vel = C::JUMP_VELOCITY;
        return;
    }

    animationTimer++;
    if (animationTimer > 6) {
        animationTimer = 0;
        frameIndex = (frameIndex < 1 || frameIndex >= 3) ? 1 : frameIndex + 1;
    }

    if (keyRight) {
        facingRight = true;
        if (x_vel < 0) {
            frameIndex = 5;
            x_vel = calcVel(x_vel, C::TURN_ACCEL, max_x_vel, true);
        } else {
            x_vel = calcVel(x_vel, x_accel, max_x_vel, true);
        }
    } else if (keyLeft) {
        facingRight = false;
        if (x_vel > 0) {
            frameIndex = 5;
            x_vel = calcVel(x_vel, C::TURN_ACCEL, max_x_vel, false);
        } else {
            x_vel = calcVel(x_vel, x_accel, max_x_vel, false);
        }
    } else {
        if (x_vel > 0) {
            x_vel = qMax(0.0, x_vel - x_accel);
            if (x_vel == 0) state = STAND;
        } else if (x_vel < 0) {
            x_vel = qMin(0.0, x_vel + x_accel);
            if (x_vel == 0) state = STAND;
        } else {
            state = STAND; // 彻底停下时恢复站立帧
        }
    }
}

void Player::jump(bool keyLeft, bool keyRight, bool keyUp) {
    frameIndex = 4;
    y_vel += C::ANTI_GRAVITY;
    canJump = false;

    if (keyRight) x_vel = calcVel(x_vel, x_accel, max_x_vel, true);
    else if (keyLeft) x_vel = calcVel(x_vel, x_accel, max_x_vel, false);

    if (y_vel >= 0 || !keyUp) state = FALL;
}

void Player::fall(bool keyLeft, bool keyRight, bool /*keyUp*/) {
    frameIndex = 4;
    y_vel = calcVel(y_vel, C::GRAVITY, C::MAX_Y_VELOCITY, true);

    if (keyRight) x_vel = calcVel(x_vel, x_accel, max_x_vel, true);
    else if (keyLeft) x_vel = calcVel(x_vel, x_accel, max_x_vel, false);
}

void Player::goDie() {
    isDead = true;
    state = DIE;
    frameIndex = 6;
    y_vel = C::JUMP_VELOCITY;
}

double Player::calcVel(double vel, double accel, double max_vel, bool isPositive) {
    if (isPositive) return qMin(vel + accel, max_vel);
    return qMax(vel - accel, -max_vel);
}

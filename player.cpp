#include "player.h"
#include "constants.h"
#include <QTransform>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

Player::Player(QString characterName)
    : x_vel(0), y_vel(0), state(STAND), facingRight(true), isDead(false),
      isBig(false), frameIndex(0), animationTimer(0), canJump(true)
{
    loadData(characterName); // 动态加载JSON数据
    if (!rightFrames.isEmpty()) setPixmap(rightFrames[0]);
}

void Player::loadData(QString characterName) {
    QFile file(":/data/" + characterName + ".json");
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonObject data = QJsonDocument::fromJson(file.readAll()).object();
    file.close();

    // 加载物理速度配置
    QJsonObject speed = data["speed"].toObject();
    max_x_vel = speed["max_walk_speed"].toDouble();
    walk_accel = speed["walk_accel"].toDouble();
    x_accel = walk_accel;
    jump_vel = speed["jump_velocity"].toDouble(); // 从json读取跳跃力度

    //加载贴图帧坐标
    QString imgName = data["image_name"].toString();
    QPixmap sheet(":/graphics/" + imgName + ".png");

    QJsonArray framesArr = data["image_frames"].toObject()["right_small_normal"].toArray();
    for (int i = 0; i < framesArr.size(); ++i) {
        QJsonObject f = framesArr[i].toObject();
        int fx = f["x"].toInt();
        int fy = f["y"].toInt();
        int fw = f["width"].toInt();
        int fh = f["height"].toInt();

        QPixmap img = sheet.copy(fx, fy, fw, fh);
        QPixmap scaled = img.scaled(fw * C::PLAYER_MULTI, fh * C::PLAYER_MULTI);

        rightFrames.append(scaled);
        leftFrames.append(scaled.transformed(QTransform().scale(-1, 1))); // 翻转得到左边帧
    }
}

//void Player::becomeBig() {
//    if (!isBig) {
//        isBig = true;
//        setScale(1.5);
//        setPos(x(), y() - 12);
//    }
//}
void Player::becomeBig() {
    if (!isBig) {
        isBig = true;
        setScale(1.5);
        //精确向上偏移增加的身高，确保双脚完美贴合地面，不卡入地底
        setPos(x(), y() - (pixmap().height() * 0.5));
    }
}

void Player::updateLogic(bool keyLeft, bool keyRight, bool keyUp) {
    if (isDead) return;
    if (!keyUp) canJump = true;
    handleStates(keyLeft, keyRight, keyUp);
    setPixmap(facingRight ? rightFrames[frameIndex] : leftFrames[frameIndex]);
}

void Player::handleStates(bool kL, bool kR, bool kU) {
    switch (state) {
        case STAND: stand(kL, kR, kU); break;
        case WALK:  walk(kL, kR, kU);  break;
        case JUMP:  jump(kL, kR, kU);  break;
        case FALL:  fall(kL, kR, kU);  break;
        default: break;
    }
}

void Player::stand(bool kL, bool kR, bool kU) {
    frameIndex = 0; x_vel = 0;
    if (kR) { facingRight = true; state = WALK; }
    else if (kL) { facingRight = false; state = WALK; }

    // 使用读取的跳跃力度
    if (kU && canJump) { state = JUMP; y_vel = jump_vel; }
}

void Player::walk(bool kL, bool kR, bool kU) {
    //  跳跃打断
    if (kU && canJump) { state = JUMP; y_vel = jump_vel; return; }

    // 处理左右移动与急刹车
    if (kR) {
        facingRight = true;
        if (x_vel < 0) { // 急刹车
            frameIndex = 5;
            x_vel = calcVel(x_vel, C::TURN_ACCEL, max_x_vel, true);
        } else {
            x_vel = calcVel(x_vel, x_accel, max_x_vel, true);
        }
    }
    else if (kL) {
        facingRight = false;
        if (x_vel > 0) {
            frameIndex = 5;
            x_vel = calcVel(x_vel, C::TURN_ACCEL, max_x_vel, false);
        } else {
            x_vel = calcVel(x_vel, x_accel, max_x_vel, false);
        }
    }
    else {
        //  惯性减速
        if (x_vel > 0) {
            x_vel -= x_accel;
            if (x_vel <= 0) { x_vel = 0; state = STAND; }
        } else if (x_vel < 0) {
            x_vel += x_accel;
            if (x_vel >= 0) { x_vel = 0; state = STAND; }
        }
    }

    //  步行动画更新
    if (frameIndex != 5) {
        animationTimer++;
        if (animationTimer > 6) {
            animationTimer = 0;
            frameIndex = (frameIndex < 1 || frameIndex >= 3) ? 1 : frameIndex + 1;
        }
    }
}

void Player::jump(bool kL, bool kR, bool kU) {
    // 加入空中左右移动的控制
    if (kR) x_vel = calcVel(x_vel, x_accel, max_x_vel, true);
    else if (kL) x_vel = calcVel(x_vel, x_accel, max_x_vel, false);

    frameIndex = 4; y_vel += C::ANTI_GRAVITY; canJump = false;
    if (y_vel >= 0 || !kU) state = FALL;
}

void Player::fall(bool kL, bool kR, bool kU) {
    // 掉落时可以左右微调
    if (kR) x_vel = calcVel(x_vel, x_accel, max_x_vel, true);
    else if (kL) x_vel = calcVel(x_vel, x_accel, max_x_vel, false);

    frameIndex = 4; y_vel = qMin(y_vel + C::GRAVITY, C::MAX_Y_VELOCITY);
}

void Player::goDie() {
    isDead = true; state = DIE; frameIndex = 6; y_vel = jump_vel;
}

double Player::calcVel(double v, double a, double m, bool p) {
    return p ? qMin(v + a, m) : qMax(v - a, -m);
}

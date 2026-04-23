#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace Constants {
    // 窗口尺寸
    const int SCREEN_W = 800;
    const int SCREEN_H = 600;
    const int GROUND_HEIGHT = SCREEN_H - 62;

    // 缩放倍率 (根据你提供的 Python 代码转换)
    const double BG_MULTI = 2.68;
    const double PLAYER_MULTI = 2.9;
    const double BRICK_MULTI = 2.69;
    const double ENEMY_MULTI = 2.5;
    const int ENEMY_SPEED = 1;

    // 物理参数
    const double GRAVITY = 1.0;
    const double ANTI_GRAVITY = 0.3;

    // 资源路径
    const QString FONT = "FixedSys.ttf";
}

#endif // CONSTANTS_H

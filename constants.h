#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace C {
    // 屏幕尺寸
    const int SCREEN_W = 800;
    const int SCREEN_H = 600;

    // 缩放比例
    const double BG_MULTI = 2.68;
    const double PLAYER_MULTI = 2.9;
    const double BRICK_MULTI = 2.69;
    const double ENEMY_MULTI = 2.5;

    // 马里奥初始帧坐标
    const int MARIO_STAND_X = 178;
    const int MARIO_STAND_Y = 32;
    const int MARIO_WIDTH = 12;
    const int MARIO_HEIGHT = 16;

    // 物理相关常量
    const double GRAVITY = 1.0;
    const double ANTI_GRAVITY = 0.3;
    const double JUMP_VELOCITY = -10.5;
    const double MAX_WALK_SPEED = 6.0;
    const double GROUND_HEIGHT = 538;

    // 马里奥移动加速度
    const double WALK_ACCEL = 0.15;
    const double RUN_ACCEL = 0.3;
    const double TURN_ACCEL = 0.35;
    const double MAX_Y_VELOCITY = 11.0;

    // 敌人相关常量
    const double ENEMY_SPEED = 1.0;

}

#endif // CONSTANTS_H

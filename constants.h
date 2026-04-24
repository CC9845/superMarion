#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace C {
    const int SCREEN_W = 800;
    const int SCREEN_H = 600;

    // 缩放比例
    const double BG_MULTI = 2.68;
    const double PLAYER_MULTI = 2.9;

    // 马里奥站立帧在 mario_bros.png 中的坐标
    const int MARIO_STAND_X = 178;
    const int MARIO_STAND_Y = 32;
    const int MARIO_WIDTH = 12;
    const int MARIO_HEIGHT = 16;

    // 物理相关常量
    const double GRAVITY = 1.0;          // 重力
    const double JUMP_VELOCITY = -10.5;  // 跳跃初速度
    const double MAX_WALK_SPEED = 6.0;   // 最大水平移动速度
    const double GROUND_HEIGHT = 538;    // 地面高度 (用于碰撞拦截)

}

#endif // CONSTANTS_H

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

    // 马里奥站立帧在 mario_bros.png 中的坐标
    const int MARIO_STAND_X = 178;
    const int MARIO_STAND_Y = 32;
    const int MARIO_WIDTH = 12;
    const int MARIO_HEIGHT = 16;

    // 物理相关常量 [cite: 906-907, 1018-1021]
    const double GRAVITY = 1.0;          // 重力
    const double ANTI_GRAVITY = 0.3;     // 跳跃时的反重力/空气阻力
    const double JUMP_VELOCITY = -10.5;  // 跳跃初速度
    const double MAX_WALK_SPEED = 6.0;   // 最大水平移动速度
    const double GROUND_HEIGHT = 538;    // 地面高度

    // 敌人相关常量
    const double ENEMY_SPEED = 1.0;      // 敌人移动速度
}

#endif // CONSTANTS_H

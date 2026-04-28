#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QList>
#include <QString>

class Player : public QGraphicsPixmapItem {
public:
    enum State { STAND, WALK, JUMP, FALL, DIE };

    // 构造函数改为传入角色名字，不再传图片
    Player(QString characterName);
    void updateLogic(bool keyLeft, bool keyRight, bool keyUp);
    void goDie();
    void becomeBig();

    double x_vel;
    double y_vel;
    State state;
    bool facingRight;
    bool isDead;
    bool isBig;

    // 从JSON读取的动态速度参数
    double jump_vel;
    double walk_accel;

private:
    // 解析JSON数据的方法
    void loadData(QString characterName);
    void handleStates(bool keyLeft, bool keyRight, bool keyUp);
    double calcVel(double vel, double accel, double max_vel, bool isPositive);
    void stand(bool keyLeft, bool keyRight, bool keyUp);
    void walk(bool keyLeft, bool keyRight, bool keyUp);
    void jump(bool keyLeft, bool keyRight, bool keyUp);
    void fall(bool keyLeft, bool keyRight, bool keyUp);

    QList<QPixmap> rightFrames;
    QList<QPixmap> leftFrames;
    int frameIndex;
    int animationTimer;
    double x_accel;
    double max_x_vel;
    bool canJump;
};

#endif // PLAYER_H

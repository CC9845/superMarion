#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QList>

class Player : public QGraphicsPixmapItem {
public:
    enum State { STAND, WALK, JUMP, FALL, DIE };

    Player(const QPixmap& sheet);

    void updateLogic(bool keyLeft, bool keyRight, bool keyUp);
    void goDie();

    double x_vel;
    double y_vel;
    State state;
    bool facingRight;
    bool isDead;

private:
    void loadFrames(const QPixmap& sheet);
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

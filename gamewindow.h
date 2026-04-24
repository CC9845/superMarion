#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QKeyEvent>

class GameWindow : public QGraphicsView {
    Q_OBJECT

public:
    GameWindow(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void gameLoop(); // 游戏主循环

private:
    QGraphicsScene *scene;
    QGraphicsPixmapItem *background;
    QGraphicsPixmapItem *mario;

    QTimer *timer;

    // 物理状态变量
    double x_vel;
    double y_vel;
    bool isJumping;

    // 按键状态
    bool keyLeft;
    bool keyRight;

    void initScene();
};

#endif // GAMEWINDOW_H

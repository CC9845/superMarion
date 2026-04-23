#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QGraphicsView>
#include <QGraphicsScene>

class GameWindow : public QGraphicsView {
    Q_OBJECT
public:
    explicit GameWindow(QWidget *parent = nullptr);
    ~GameWindow();

private:
    QGraphicsScene *gameScene; // 游戏场景
    void initUI();             // 初始化界面逻辑
};

#endif // GAMEWINDOW_H

#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

class GameWindow : public QGraphicsView {
    Q_OBJECT

public:
    GameWindow(QWidget *parent = nullptr);

private:
    QGraphicsScene *scene;
    QGraphicsPixmapItem *background;
    QGraphicsPixmapItem *mario;

    void initScene();
};

#endif // GAMEWINDOW_H

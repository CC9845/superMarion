#include "gameWindow.h"
#include "constants.h"

GameWindow::GameWindow(QWidget *parent) : QGraphicsView(parent) {
    initUI();
}

GameWindow::~GameWindow() {
    // 清理资源
}

void GameWindow::initUI() {
    // 1. 创建场景并设置大小
    gameScene = new QGraphicsScene(this);
    gameScene->setSceneRect(0, 0, Constants::SCREEN_W, Constants::SCREEN_H);

    // 2. 将场景绑定到当前的视图窗口
    setScene(gameScene);

    // 3. 设置窗口固定大小，禁止缩放
    setFixedSize(Constants::SCREEN_W, Constants::SCREEN_H);
    setWindowTitle("Super Mario - Qt C++ Version");

    // 4. 游戏细节配置
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 隐藏水平滚动条
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);   // 隐藏垂直滚动条
    setRenderHint(QPainter::Antialiasing);              // 开启抗锯齿
}

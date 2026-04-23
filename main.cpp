#include <QApplication>
#include "gameWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 实例化并显示游戏主窗口
    GameWindow window;
    window.show();

    return app.exec(); // 进入 Qt 事件循环
}

#include <QApplication>
#include "gamewindow.h"

int main(int argc, char *argv[]) {

    qputenv("QT_LOGGING_RULES", "qt.gui.imageio=false");

    QApplication a(argc, argv);
    GameWindow w;
    w.show();
    return a.exec();
}

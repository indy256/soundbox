#include <QApplication>

#include "window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Window window;
    window.show();
    window.resizeColumnToContents();
    return app.exec();
}

#include "window.h"

#include <QApplication>
#include <QStandardItemModel>
#include <QTime>

void addAudio(QAbstractItemModel *model, const QString &file,
             const QString &duration, const QDateTime &date)
{
    model->insertRow(0);
    model->setData(model->index(0, 0), file);
    model->setData(model->index(0, 1), duration);
    model->setData(model->index(0, 2), date);
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Window window;
    window.show();
    window.resizeColumnToContents();
    return app.exec();
}

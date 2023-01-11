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

QAbstractItemModel *createAudioFileModel(QObject *parent)
{
    QStandardItemModel *model = new QStandardItemModel(0, 3, parent);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("File"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Duration"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date"));

    return model;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Window window;
    window.setSourceModel(createAudioFileModel(&window));
    window.show();
    window.resizeColumnToContents();
    return app.exec();
}

#include <QStandardItemModel>
#include <QtWidgets>

#include "window.h"
#include "sdl.h"
#include <string.h>

static inline QColor textColor(const QPalette &palette)
{
    return palette.color(QPalette::Active, QPalette::Text);
}

static void setTextColor(QWidget *w, const QColor &c)
{
    auto palette = w->palette();
    if (textColor(palette) != c) {
        palette.setColor(QPalette::Active, QPalette::Text, c);
        w->setPalette(palette);
    }
}

QAbstractItemModel *createAudioFileModel(QObject *parent)
{
    QStandardItemModel *model = new QStandardItemModel(0, 3, parent);

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("File"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Duration"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date"));

    return model;
}

Window::Window()
{
    sourceView = new QTreeView;
    sourceView->setRootIsDecorated(false);
    sourceView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    sourceView->setAlternatingRowColors(true);
    sourceView->setSortingEnabled(true);

    positionSlider = new QSlider(Qt::Horizontal, this);

    openButton = new QPushButton("Open file", this);

    connect(sourceView, &QTreeView::doubleClicked,
            this, &Window::doubleClicked);

    connect(openButton, &QPushButton::clicked,
            this, &Window::openFile);

    sourceGroupBox = new QGroupBox(tr("File list"));

    QVBoxLayout *sourceLayout = new QVBoxLayout;
    sourceLayout->addWidget(sourceView);
    sourceLayout->addWidget(positionSlider);
    sourceLayout->addWidget(openButton);
    sourceGroupBox->setLayout(sourceLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(sourceGroupBox);

    setLayout(mainLayout);

    setWindowTitle(tr("SoundBox"));
    resize(800, 300);

    setSourceModel(createAudioFileModel(this));

    init_audio();
}

void Window::setSourceModel(QAbstractItemModel *model)
{
    sourceView->setModel(model);
}

void Window::openFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open audio"));
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted) {
        QStringList files = fileDialog.selectedFiles();
        for (const auto& f: files)
        {
            QAbstractItemModel *model = (QAbstractItemModel *) sourceView->model();
            model->insertRow(model->rowCount());
            model->setData(model->index(model->rowCount() - 1, 0), f);
        }
    }
}

void Window::doubleClicked()
{
    QItemSelectionModel *selectionModel  = (QItemSelectionModel  *) sourceView->selectionModel();
    QAbstractItemModel *model = (QAbstractItemModel *) sourceView->model();
    QModelIndexList selected = selectionModel->selection().indexes();
    int row = selected[0].row();
    QString filename = model->data(model->index(row, 0)).toString();
    QByteArray ba = filename.toUtf8();
    const char *name = ba.data();
    ::filename = new char[2000];
    strcpy(::filename, name);
    ::new_file = true;
}

void Window::resizeColumnToContents()
{
    sourceView->setColumnWidth(0, 350);
}


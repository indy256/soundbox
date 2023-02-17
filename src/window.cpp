#include <QStandardItemModel>
#include <QtWidgets>

#include "window.h"
#include "sdl.h"
#include "slider/slider.h"
#include <string.h>

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
    m_settingsFile = QApplication::applicationDirPath() + "/soundbox.ini";
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    audioFiles = settings.value("files", "").toStringList();

    sourceView = new QTreeView;
    sourceView->setRootIsDecorated(false);
    sourceView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    sourceView->setAlternatingRowColors(true);
    sourceView->setSortingEnabled(true);
    sourceView->setModel(createAudioFileModel(this));

    for(const QString &f: audioFiles) {
        QAbstractItemModel *model = (QAbstractItemModel *) sourceView->model();
        model->insertRow(model->rowCount());
        model->setData(model->index(model->rowCount() - 1, 0), f);
    }

    volumeSlider = new Slider(Qt::Horizontal, this);
    volumeSlider->setValue(50);

    positionSlider = new Slider(Qt::Horizontal, this);
    positionSlider->setMaximum(999);

    openButton = new QPushButton("Open file", this);

    connect(sourceView, &QTreeView::doubleClicked,
            this, &Window::doubleClicked);

    connect(volumeSlider, &Slider::valueChanged,
            this, &Window::setVolume);

    connect(positionSlider, &Slider::valueChanged,
            positionSlider, &Slider::valueChangedHandler);
    connect(positionSlider, &Slider::valueChanged2,
            this, &Window::seek);

    connect(this, &Window::requestSliderUpdate,
            this, &Window::updateSlider);

    connect(openButton, &QPushButton::clicked,
            this, &Window::openFile);

    QVBoxLayout *sourceLayout = new QVBoxLayout;
    sourceLayout->addWidget(sourceView);

    sourceGroupBox = new QGroupBox(tr("File list"));
    sourceGroupBox->setLayout(sourceLayout);

    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(volumeSlider);
    controlsLayout->addWidget(positionSlider);
    controlsLayout->addWidget(openButton);

    QGroupBox *controlsGroupBox = new QGroupBox(tr("Controls"));
    controlsGroupBox->setLayout(controlsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(sourceGroupBox);
    mainLayout->addWidget(controlsGroupBox);
    setLayout(mainLayout);

    setWindowTitle(tr("SoundBox"));
    resize(800, 300);

    ::window = this;

    init_audio();
}

void Window::updateSlider(int pos) {
    if (positionSlider->is_mouse_move()) {
        return;
    }
    positionSlider->blockSignals(true);
    positionSlider->setValue(pos * (positionSlider->maximum() - positionSlider->minimum() + 1) / 1000);
    positionSlider->blockSignals(false);
}

void Window::setVolume() {
    audio_state.volume = volumeSlider->sliderPosition() * 100 / (volumeSlider->maximum() - volumeSlider->minimum() + 1);
}

void Window::seek()
{
    int pos = positionSlider->sliderPosition() * 1000 / (positionSlider->maximum() - positionSlider->minimum() + 1);
    fprintf(stderr, "Position: %d\n", pos);

    audio_state.seek_request = true;
    audio_state.seek_pos = pos;
}

void Window::openFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open audio"));
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted) {
        QStringList files = fileDialog.selectedFiles();
        for (const auto& file: files)
        {
            QAbstractItemModel *model = (QAbstractItemModel *) sourceView->model();
            model->insertRow(model->rowCount());
            model->setData(model->index(model->rowCount() - 1, 0), file);
        }
    }
    QStringList files;
    for(int row = 0; row < sourceView->model()->rowCount(); row++) {
        QAbstractItemModel *model = (QAbstractItemModel *) sourceView->model();
        files.append(model->data(model->index(row, 0)).toString());
    }
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.setValue("files", files);
    settings.sync();
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

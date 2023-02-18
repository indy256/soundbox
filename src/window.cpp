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
    sourceView->setContextMenuPolicy(Qt::CustomContextMenu);
    sourceView->setUniformRowHeights(true);

    for(const QString &f: audioFiles) {
        QAbstractItemModel *model = (QAbstractItemModel *) sourceView->model();
        model->insertRow(model->rowCount());
        model->setData(model->index(model->rowCount() - 1, 0), f);
    }

    volumeSlider = new Slider(Qt::Horizontal, this);
    volumeSlider->setValue(50);

    positionSlider = new Slider(Qt::Horizontal, this);
    positionSlider->setMaximum(999);

    addFileButton = new QPushButton("Add file", this);
    addFolderButton = new QPushButton("Add folder", this);

    connect(sourceView, &QTreeView::doubleClicked,
            this, &Window::doubleClicked);
    connect(sourceView, &QTreeView::customContextMenuRequested,
            this, &Window::contextMenu);

    connect(volumeSlider, &Slider::valueChanged,
            this, &Window::setVolume);

    connect(positionSlider, &Slider::valueChanged,
            positionSlider, &Slider::valueChangedHandler);
    connect(positionSlider, &Slider::valueChanged2,
            this, &Window::seek);

    connect(this, &Window::requestSliderUpdate,
            this, &Window::updateSlider);

    connect(addFileButton, &QPushButton::clicked,
            this, &Window::addFile);
    connect(addFolderButton, &QPushButton::clicked,
            this, &Window::addFolder);

    QVBoxLayout *sourceLayout = new QVBoxLayout;
    sourceLayout->addWidget(sourceView);

    sourceGroupBox = new QGroupBox(tr("File list"));
    sourceGroupBox->setLayout(sourceLayout);

    QHBoxLayout *controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(volumeSlider);
    controlsLayout->addWidget(positionSlider);
    controlsLayout->addWidget(addFileButton);
    controlsLayout->addWidget(addFolderButton);

    QGroupBox *controlsGroupBox = new QGroupBox(tr("Controls"));
    controlsGroupBox->setLayout(controlsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(sourceGroupBox);
    mainLayout->addWidget(controlsGroupBox);
    setLayout(mainLayout);

    setWindowTitle(tr("SoundBox"));
    resize(800, 500);

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

void Window::addFile()
{
    QFileDialog fileDialog(this);

    QSettings settings(m_settingsFile, QSettings::IniFormat);
    QString default_path = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0, QDir::homePath());
    QString path = settings.value("current_folder", default_path).toString();

    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open file"));
    fileDialog.setDirectory(path);
    if (fileDialog.exec() == QDialog::Accepted) {
        QStringList files = fileDialog.selectedFiles();
        bool first = true;
        for (const auto& file: files)
        {
            QAbstractItemModel *model = (QAbstractItemModel *) sourceView->model();
            model->insertRow(model->rowCount());
            model->setData(model->index(model->rowCount() - 1, 0), file);

            if (first) {
                QDir d = QFileInfo(file).absoluteDir();
                QSettings settings(m_settingsFile, QSettings::IniFormat);
                settings.setValue("current_folder", d.absolutePath());
                settings.sync();
                first = false;
            }
        }
    }
    saveFiles();
}

void Window::addFolder()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    QString default_path = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0, QDir::homePath());
    QString path = settings.value("current_folder", default_path).toString();
    QString folder = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Open folder"), path));

    if (!folder.isEmpty()) {
        QDirIterator it(folder, QStringList() << "*.mp3", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const auto& file = it.next();
            QAbstractItemModel *model = (QAbstractItemModel *) sourceView->model();
            model->insertRow(model->rowCount());
            model->setData(model->index(model->rowCount() - 1, 0), file);
        }

        saveFiles();
        QSettings settings(m_settingsFile, QSettings::IniFormat);
        settings.setValue("current_folder", folder);
        settings.sync();
    }
}

void Window::saveFiles() {
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

void Window::contextMenu(const QPoint &pos)
{
    const QModelIndex item = sourceView->indexAt(pos);

    int row = item.row();
    QAbstractItemModel *model = (QAbstractItemModel *) sourceView->model();

    QMenu menu;
    QAction *removeAction = menu.addAction("Remove");
    QAction *action = menu.exec(sourceView->mapToGlobal(pos));
    if (!action)
        return;
    if (action == removeAction) {
        model->removeRow(row);
        saveFiles();
    }
}

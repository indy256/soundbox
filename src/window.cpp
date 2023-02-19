#include <QStandardItemModel>
#include <QtWidgets>
#include <algorithm>

#include "window.h"
#include "sdl.h"
#include "slider/slider.h"
#include <string.h>

QAbstractItemModel *createAudioFileModel(QObject *parent)
{
    QStandardItemModel *model = new QStandardItemModel(0, 1, parent);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("File"));
    return model;
}

Window::Window()
{
    QIcon icon =  QIcon(":/images/soundbox.png");
    setWindowIcon(icon);
    m_settingsFile = QApplication::applicationDirPath() + "/soundbox.ini";
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    audioFiles = settings.value("files", QStringList()).toStringList();

    sourceView = new QTreeView;
    sourceView->setRootIsDecorated(false);
    sourceView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    sourceView->setAlternatingRowColors(true);
    sourceView->setSortingEnabled(true);
    sourceView->setModel(createAudioFileModel(this));
    sourceView->setContextMenuPolicy(Qt::CustomContextMenu);
    sourceView->setUniformRowHeights(true);
    sourceView->setSelectionMode(QAbstractItemView::ContiguousSelection);

    for(const QString &f: audioFiles) {
        QAbstractItemModel *model = (QAbstractItemModel *) sourceView->model();
        model->insertRow(model->rowCount());
        model->setData(model->index(model->rowCount() - 1, 0), f);
    }

    volumeSlider = new Slider(Qt::Horizontal, this);
    volumeSlider->setValue(50);

    positionSlider = new Slider(Qt::Horizontal, this);
    positionSlider->setMaximum(999);

    QToolBar *playbackToolBar = addToolBar(tr("Playback"));
    QAction *playAct = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("&Play"), this);
    playAct->setStatusTip(tr("Play"));
    playbackToolBar->addAction(playAct);
    QAction *pauseAct = new QAction(style()->standardIcon(QStyle::SP_MediaPause), tr("&Pause"), this);
    pauseAct->setStatusTip(tr("Pause"));
    playbackToolBar->addAction(pauseAct);
    playbackToolBar->setMovable(false);

    QToolBar *volumeToolBar = addToolBar(tr("Volume"));
    volumeToolBar->addWidget(volumeSlider);

    QToolBar *positionToolBar = addToolBar(tr("Position"));
    positionToolBar->addWidget(positionSlider);

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

    createMenu();

    setCentralWidget(sourceView);
    statusBar()->showMessage(tr("Ready"));

    setWindowTitle(tr("SoundBox v0.0.1"));

    readSettings();

    ::window = this;

    init_audio();
}

void Window::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}


void Window::createMenu()
{
    fileMenu = new QMenu(tr("&File"), this);
    addFilesAction = fileMenu->addAction(tr("Add files..."));
    addFolderAction = fileMenu->addAction(tr("Add folder..."));
    exitAction = fileMenu->addAction(tr("E&xit"));
    exitAction->setShortcuts(QKeySequence::Close);
    menuBar()->addMenu(fileMenu);
    connect(addFilesAction, &QAction::triggered, this, &Window::addFiles);
    connect(addFolderAction, &QAction::triggered, this, &Window::addFolder);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    editMenu = new QMenu(tr("&Edit"), this);
    clearAction = editMenu->addAction(tr("Clear"), this, &Window::clear);
    selectAllAction = editMenu->addAction(tr("Select all"), this, &Window::selectAll);
    selectAllAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
    selectionMenu = editMenu->addMenu("Selection");
    removeAction = selectionMenu->addAction(tr("Remove"), this, &Window::removeSelection);
    removeAction->setShortcut(QKeySequence(Qt::Key_Delete));
    menuBar()->addMenu(editMenu);

    playbackMenu = new QMenu(tr("&Playback"), this);
    pauseAction = playbackMenu->addAction(tr("Pause"), this, &Window::pause);
    playAction = playbackMenu->addAction(tr("Play"), this, &Window::play);
    menuBar()->addMenu(playbackMenu);

    helpMenu = new QMenu(tr("&Help"), this);
    aboutAction = helpMenu->addAction(tr("About"), this, &Window::about);
    menuBar()->addMenu(helpMenu);
}

void Window::clear()
{
    selectAll();
    removeSelection();
}

void Window::selectAll()
{
    sourceView->selectAll();
}

void Window::about()
{
   QMessageBox::about(this, tr("About Application"),
            tr("SoundBox v0.0.1<br/><br/>Cross-platfrom audio player<br/><br/><a href='https://github.com/indy256/soundbox'>https://github.com/indy256/soundbox</a>"));
}

void Window::updateSlider(int pos)
{
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

void Window::addFiles()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    QString default_path = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0, QDir::homePath());
    QString path = settings.value("current_folder", default_path).toString();

    QString selectedFilter;
    QStringList files = QFileDialog::getOpenFileNames(
                                this, tr("Open file"),
                                path,
                                tr("All Files (*);;MPEG layer 3 (*.MP3)"),
                                &selectedFilter);

    if (files.count()) {
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
        saveFiles();
    }
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

void Window::saveFiles()
{
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
    QMenu menu;
    QAction *removeAction = menu.addAction("Remove");
    QAction *action = menu.exec(sourceView->mapToGlobal(pos));
    if (!action)
        return;
    if (action == removeAction) {
        removeSelection();
    }
}

void Window::removeSelection()
{
    QAbstractItemModel *model = (QAbstractItemModel *) sourceView->model();
    QModelIndexList indexList = sourceView->selectionModel()->selection().indexes();
    QList<int> rows;
    for(QModelIndex index: indexList) {
        rows << index.row();
    }
    std::sort(rows.rbegin(), rows.rend());
    for(int row: rows) {
        model->removeRow(row);
    }
    saveFiles();
}

void Window::readSettings()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void Window::writeSettings()
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.setValue("geometry", saveGeometry());
}

void Window::pause()
{
}

void Window::play()
{
}

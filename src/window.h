#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

#include "slider/slider.h"

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QObject;
class QSlider;
class QTreeView;
QT_END_NAMESPACE

class Window : public QMainWindow {
    Q_OBJECT

   public:
    Window();

    void resizeColumnToContents();
    void callUpdateSlider(int pos) { emit requestSliderUpdate(pos); }

   signals:
    void requestSliderUpdate(int pos);

   private slots:
    void setVolume();
    void setPosition();
    void addFiles();
    void addFolder();
    void doubleClicked();
    void updatePositionSlider(int pos);

   private:
    QMenu *fileMenu;
    QAction *exitAction;
    QAction *addFilesAction;
    QAction *addFolderAction;
    QMenu *editMenu;
    QAction *clearAction;
    QAction *selectAllAction;
    QMenu *selectionMenu;
    QAction *removeAction;
    QMenu *playbackMenu;
    QAction *pauseAction;
    QAction *playAction;
    QMenu *helpMenu;
    QAction *aboutAction;

    QSlider *volumeSlider;
    Slider *positionSlider;
    QTreeView *filesView;

    QString settingsFile;

    void contextMenu(const QPoint &pos);
    void createMenu();
    void createToolbar();
    void createFilesView();
    QAbstractItemModel *createAudioFileModel(QObject *parent);
    void about();
    void saveFiles();
    void clear();
    void selectAll();
    void removeSelection();
    void readSettings();
    void writeSettings();
    void closeEvent(QCloseEvent *event);
    void pause();
    void play();
};

#endif  // WINDOW_H

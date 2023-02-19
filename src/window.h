#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include "slider/slider.h"

QT_BEGIN_NAMESPACE
class QGroupBox;
class QLabel;
class QTreeView;
class QPushButton;
class QSlider;
QT_END_NAMESPACE

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window();

    void resizeColumnToContents();
    void callUpdateSlider(int pos) {
        emit requestSliderUpdate(pos);
    }
    void contextMenu(const QPoint &pos);

signals:
    void requestSliderUpdate(int pos);

private slots:
    void setVolume();
    void seek();
    void addFiles();
    void addFolder();
    void doubleClicked();
    void updateSlider(int pos);

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

    QTreeView *filesView;
    QPushButton *addFileButton;
    QPushButton *addFolderButton;
    QSlider *volumeSlider;
    Slider *positionSlider;
    QString m_settingsFile;
    QStringList audioFiles;

    void createMenu();
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

#endif // WINDOW_H

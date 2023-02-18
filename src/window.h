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
    QMenu *playbackMenu;
    QMenu *helpMenu;
    QAction *aboutAction;

    QGroupBox *sourceGroupBox;
    QTreeView *sourceView;
    QPushButton *addFileButton;
    QPushButton *addFolderButton;
    QSlider *volumeSlider;
    Slider *positionSlider;
    QString m_settingsFile;
    QStringList audioFiles;

    void createMenu();
    void saveFiles();
};

#endif // WINDOW_H

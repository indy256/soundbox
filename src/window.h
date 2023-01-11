#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSortFilterProxyModel;
class QTreeView;
class QPushButton;
class QSlider;
QT_END_NAMESPACE

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

    void setSourceModel(QAbstractItemModel *model);
    void resizeColumnToContents();

private slots:
    void openFile();
    void doubleClicked();

private:
    QGroupBox *sourceGroupBox;
    QTreeView *sourceView;
    QPushButton *openButton;
    QSlider *positionSlider;
};

#endif // WINDOW_H

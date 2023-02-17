#ifndef SLIDER_H
#define SLIDER_H

#include <QtWidgets>

QT_BEGIN_NAMESPACE
class QSlider;
QT_END_NAMESPACE


class Slider : public QSlider
{
    Q_OBJECT

private:
    bool mouse_move = false;

public:
    Slider(Qt::Orientation orientation, QWidget *parent = nullptr): QSlider(orientation, parent){}
    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *ev);
    virtual void mouseReleaseEvent(QMouseEvent *ev);
    void valueChangedHandler(int value);
    bool is_mouse_move() {return mouse_move;}

signals:
    void valueChanged2(int value);
};

#endif // SLIDER_H

#include "slider.h"

#include <QtWidgets>

void Slider::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::MouseButton::LeftButton) {
        e->accept();
        int x = e->pos().x();
        int value = (maximum() - minimum()) * x / width() + minimum();
        setValue(value);
    } else {
        QSlider::mousePressEvent(e);
    }
}

void Slider::mouseMoveEvent(QMouseEvent *e) {
    e->accept();
    mouse_move = true;
    int x = e->pos().x();
    int value = (maximum() - minimum()) * x / width() + minimum();
    setValue(value);
    //    QSlider::mouseMoveEvent(e);
}

void Slider::mouseReleaseEvent(QMouseEvent *e) {
    //    e->accept();
    int x = e->pos().x();
    int value = (maximum() - minimum()) * x / width() + minimum();
    setValue(value);
    emit valueChanged2(value);
    mouse_move = false;
}

void Slider::valueChangedHandler(int value) {
    if (!mouse_move) {
        emit valueChanged2(value);
    }
}

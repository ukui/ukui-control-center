#include "myslider.h"
#include <QMouseEvent>
#include <QDebug>
#include <QWidget>


MySlider::MySlider(QWidget *parent):QSlider (parent){}
MySlider::~MySlider(){}

//重写鼠标点击事件
void MySlider::mousePressEvent(QMouseEvent *ev)
{
    isMouseCliked = true;
    QSlider::mousePressEvent(ev);
    int currentx = ev->pos().x();
    int value = (currentx /(double)this->width())*(this->maximum() - this->minimum()) + this->minimum();
    this->setValue(value);
}

void MySlider::wheelEvent(QWheelEvent *ev)
{
    if (isMouseCliked)
        return QSlider::wheelEvent(ev);
}

void MySlider::leaveEvent(QEvent *ev)
{
    isMouseCliked = false;
}

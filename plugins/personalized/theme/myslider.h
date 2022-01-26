#ifndef MYSLIDER_H
#define MYSLIDER_H

#include <QObject>
#include <QSlider>

class MySlider : public QSlider
{
public:
    MySlider(QWidget *parent = nullptr);
    ~MySlider();
protected:
    void mousePressEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *ev);
    void leaveEvent(QEvent *ev);

private:
    bool isMouseCliked = false;

};

#endif // MYSLIDER_H

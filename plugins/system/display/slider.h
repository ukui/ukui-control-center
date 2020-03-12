#ifndef SLIDER_H
#define SLIDER_H

#include <QSlider>
#include <QPaintEvent>
#include <QWidget>
#include <QStringList>

class Slider : public QSlider
{
public:
    Slider();
    void paintEvent(QPaintEvent *ev);
private:
    QStringList scaleList;
};

#endif // SLIDER_H

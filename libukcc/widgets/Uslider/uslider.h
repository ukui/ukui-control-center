#ifndef USLIDER_H
#define USLIDER_H

#include <QSlider>
#include <QPaintEvent>
#include <QWidget>
#include <QStringList>

#include "libukcc_global.h"

class LIBUKCC_EXPORT Uslider : public QSlider
{

public:
    Uslider(QStringList list, int paintValue = 1);
    Uslider(Qt::Orientation orientation, QWidget *parent = nullptr, int paintValue = 0);
    Uslider(QWidget *parent = nullptr, int paintValue = 0);
    QStringList scaleList;

protected:
    void mousePressEvent(QMouseEvent *ev);
    void paintEvent(QPaintEvent *ev);
    void wheelEvent(QWheelEvent *ev);
    void leaveEvent(QEvent *ev);
private:
    int paintValue;
    bool isMouseCliked = false;
};

#endif // USLIDER_H


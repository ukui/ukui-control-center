#ifndef USLIDER_H
#define USLIDER_H

#include <QSlider>
#include <QPaintEvent>
#include <QWidget>
#include <QStringList>

class Uslider : public QSlider
{
public:

    Uslider(QStringList list);
    Uslider(Qt::Orientation orientation, QWidget *parent = nullptr, int paintValue = 0);
    Uslider(QWidget *parent = nullptr, int paintValue = 0);

    QStringList scaleList;
    bool laungeBool;
private:
    const int TOTAL_DELTA = 1200;
    int paintValue;
    bool isMouseCliked = false;
    int oldValue;
protected:
    void mousePressEvent(QMouseEvent *ev);
    void wheelEvent(QWheelEvent *ev);
    void leaveEvent(QEvent *ev);
    void paintEvent(QPaintEvent *ev);
};

#endif // USLIDER_H


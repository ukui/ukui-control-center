#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QtMath>
#include <QTimer>

#define PI 3.14

class Clock : public QWidget
{
    Q_OBJECT

public:
    Clock(QWidget *parent = 0);
    ~Clock();
    void paintEvent(QPaintEvent *);//重写绘制事件函数
    QRectF textRectF(double radius, int pointSize, double angle);

};

#endif // CLOCK_H

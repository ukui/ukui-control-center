#ifndef USLIDER_H
#define USLIDER_H

#include <QSlider>
#include <QPaintEvent>
#include <QWidget>
#include <QStringList>

class Uslider : public QSlider
{

public:
    QStringList scaleList;
public:

    Uslider(QStringList list);
    void paintEvent(QPaintEvent *ev);    
    bool laungeBool;
private:
    const int TOTAL_DELTA = 1200;
protected:
     void wheelEvent(QWheelEvent *e);
};

#endif // USLIDER_H


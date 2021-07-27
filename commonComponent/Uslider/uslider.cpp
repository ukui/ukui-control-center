#include "uslider.h"

#include <QStyle>
#include <QPainter>
#include <QStyleOptionSlider>
#include <QDebug>


Uslider::Uslider(QStringList list) : scaleList(list), QSlider(Qt::Horizontal)
{
    this->setMinimumHeight(50);
    this->setMaximumHeight(100);
    QString locale = QLocale::system().name();
    if ("zh_CN" == locale) {
        laungeBool = true;
    } else if ("en_US" == locale) {
        laungeBool = false;
    }

}

void Uslider::wheelEvent(QWheelEvent *e)
{
    static int touchMoveDistance = 0;
    int oldValue = value();
    int newValue = 0;

    int maxTouchPadMoveMinToMaxDistance = TOTAL_DELTA;
    int totalStep = (maximum() - minimum()) / pageStep();

    int step = pageStep();

    touchMoveDistance += e->delta();

    if(touchMoveDistance > (maxTouchPadMoveMinToMaxDistance / totalStep)){
        newValue = oldValue + step;
        touchMoveDistance = 0;
    }
    else if(touchMoveDistance < (maxTouchPadMoveMinToMaxDistance / totalStep)*-1){
        newValue = oldValue - step;
        touchMoveDistance = 0;
    }
    else{
        goto END;
    }

    newValue = newValue > maximum()? maximum():newValue;
    newValue = newValue < minimum()? minimum():newValue;

    setValue(newValue);
END:
    e->accept();
}

void Uslider::paintEvent(QPaintEvent *e)
{

    QSlider::paintEvent(e);

    auto painter = new QPainter(this);
//    painter->setPen(QPen(Qt::black));
    painter->setBrush(QBrush(QPalette::Text));

    auto rect = this->geometry();

    int numTicks = (maximum() - minimum())/tickInterval();

    QFontMetrics fontMetrics = QFontMetrics(this->font());

    if (this->orientation() == Qt::Horizontal) {
        int fontHeight = fontMetrics.height();

        for (int i=0; i<=numTicks; i++) {
            int tickNum = minimum() + (tickInterval() * i);
            int tickX = 0;
            int tickY = 0;
            if( 0 == i) {
                tickX = 0;
            } else if (numTicks == i){
                if (laungeBool) {
                    //tickX = rect.width()-33;
                    QRect rec = fontMetrics.boundingRect(this->scaleList.at(numTicks));
                    tickX = rect.width()-rec.width() - 2;
                } else {
                    QRect rec = fontMetrics.boundingRect(this->scaleList.at(numTicks));
                    tickX = rect.width()-rec.width() - 2;
                }
            }else{
                QRect rec = fontMetrics.boundingRect(this->scaleList.at(i));
                tickX = (((rect.width()/numTicks) * i) - (rec.width()/2) + 6);
            }
            tickY = rect.height()/2 + fontHeight + 1;
            painter->drawText(QPoint(tickX, tickY),
                                 this->scaleList.at(i));
        }
    }
    painter->end();
}



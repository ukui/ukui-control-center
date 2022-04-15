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

Uslider::Uslider(Qt::Orientation orientation, QWidget *parent, int paintValue) :
    QSlider(orientation, parent)
{
    this->paintValue = paintValue;
    if (paintValue != 0) {   //刻度值显示
        this->setTickPosition(QSlider::TicksBelow);
    }
}

Uslider::Uslider(QWidget *parent, int paintValue) : QSlider(parent)
{
    this->paintValue = paintValue;
    if (paintValue != 0) {   //刻度值显示
        this->setTickPosition(QSlider::TicksBelow);
    }
}

void Uslider::wheelEvent(QWheelEvent *ev)
{
    if (isMouseCliked) {
        static int touchMoveDistance = 0;
            int oldValue = value();
            int newValue = 0;

            int maxTouchPadMoveMinToMaxDistance = TOTAL_DELTA;
            int totalStep = (maximum() - minimum()) / pageStep();

            int step = pageStep();

            touchMoveDistance += ev->delta();

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
            ev->accept();

    }
}

void Uslider::leaveEvent(QEvent *ev)
{
    isMouseCliked = false;
}

//重写鼠标点击事件
void Uslider::mousePressEvent(QMouseEvent *ev)
{
    qDebug() << "-------------";
    int value = 0;
    int currentX = ev->pos().x();
    double per = currentX * 1.0 / this->width();
    if ((this->maximum() - this->minimum()) >= 50) { //减小鼠标点击像素的影响
        value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
        if (value <= (this->maximum() / 2 - this->maximum() / 10 + this->minimum() / 10)) {
            value = qRound(per*(this->maximum() - this->minimum() - 1)) + this->minimum();
        } else if (value > (this->maximum() / 2 + this->maximum() / 10 + this->minimum() / 10)) {
            value = qRound(per*(this->maximum() - this->minimum() + 1)) + this->minimum();
        } else {
            value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
        }
    } else {
        value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
    }

    if (oldValue == 0) {
        oldValue = this->value();
    }

    if (qAbs(oldValue - value) <= 2) {
        isMouseCliked = true;
        QSlider::mousePressEvent(ev);
        return;
    }

    oldValue = value;

    this->setValue(value);
    isMouseCliked = true;
    QSlider::mousePressEvent(ev);  //必须放在后面，否则点击拖动无法使用(待优化)
}


void Uslider::paintEvent(QPaintEvent *e)
{

    QSlider::paintEvent(e);
    if (paintValue == 0)   //不需要刻度值显示
            return;

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



#include "uslider.h"

#include <QStyle>
#include <QPainter>
#include <QStyleOptionSlider>
#include <QDebug>
#include <QPalette>


Uslider::Uslider(QStringList list, int paintValue) : QSlider(Qt::Horizontal), scaleList(list)
{
    this->setMinimumHeight(50);
    this->setMaximumHeight(100);
    this->paintValue = paintValue;
    this->setPageStep(0); //防止qslider的mousePressEvent对坐标造成影响

    if (paintValue != 0) {   //刻度值显示
        this->setTickPosition(QSlider::TicksBelow);
    }
}

Uslider::Uslider(Qt::Orientation orientation, QWidget *parent, int paintValue) :
    QSlider(orientation, parent)
{
    this->paintValue = paintValue;
    this->setPageStep(0); //防止qslider的mousePressEvent对坐标造成影响
    if (paintValue != 0) {   //刻度值显示
        this->setTickPosition(QSlider::TicksBelow);
    }
}

Uslider::Uslider(QWidget *parent, int paintValue) : QSlider(parent)
{
    this->paintValue = paintValue;
    this->setPageStep(0); //防止qslider的mousePressEvent对坐标造成影响
    if (paintValue != 0) {   //刻度值显示
        this->setTickPosition(QSlider::TicksBelow);
    }
}

void Uslider::paintEvent(QPaintEvent *e)
{
    QSlider::paintEvent(e);
    if (paintValue == 0)   //不需要刻度值显示
            return;

    auto painter = new QPainter(this);
    painter->setBrush(QBrush(QColor(QPalette::Base)));

    auto rect = this->geometry();
    int numTicks = (maximum() - minimum()) / tickInterval();
    painter->setFont(this->font());
    int total = 0;
    QFontMetrics fontMetrics = QFontMetrics(painter->font());
    for (int i=0; i <= numTicks; i++) {
        QRect fontRect = fontMetrics.boundingRect(scaleList.at(i));
        total += fontRect.width();
    }
    const float interval = (rect.width() - 16) / float(numTicks);

    if (this->orientation() == Qt::Horizontal) {
        int fontHeight = fontMetrics.height();
        float tickY = rect.height() / 2.0 + fontHeight + 8;
        float preTickEndX = 0.0;
        for (int i = 0; i <= numTicks; i++) {
            float tickX = 8.0 + i * interval;
            tickX = tickX - fontMetrics.boundingRect(scaleList.at(i)).width() / 2;
            if (i == numTicks) {
                while (tickX + fontMetrics.boundingRect(scaleList.at(i)).width() >= this->width()) {
                    tickX = tickX - 1;
                }
                if (tickX < preTickEndX + 4) {
                    QFont fontText;
                    int pointSize = painter->font().pointSize() - 1;
                    if (pointSize < 1) {
                        pointSize = 1;
                    }
                    fontText.setPointSize(pointSize);
                    painter->setFont(fontText);
                    fontMetrics = QFontMetrics(painter->font());
                    if (pointSize > 1) { //避免 == 1死循环
                        i--;
                        continue;
                    }
                }
            } else if (i == 0){
                if (tickX < 0) {
                    tickX = 0;
                }
            }
            preTickEndX = tickX + fontMetrics.boundingRect(scaleList.at(i)).width();
            painter->drawText(QPointF(tickX, tickY),
                              this->scaleList.at(i));
        }
    }
    painter->end();
}

//重写鼠标点击事件
void Uslider::mousePressEvent(QMouseEvent *e)
{
    int value = 0;
    int currentX = e->pos().x();
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
    this->setValue(value);

    QSlider::mousePressEvent(e);  //必须放在后面，否则点击拖动无法使用(待优化)
}



#include "clicklabel.h"

clickLabel::clickLabel()
{
    adjustSize();
}

clickLabel::~clickLabel()
{
}

void clickLabel::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton)
        emit clicked();
//    QLabel::mousePressEvent(event);
}

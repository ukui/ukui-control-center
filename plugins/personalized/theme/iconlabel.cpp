#include "iconlabel.h"
#include <QEvent>

IconLabel::IconLabel(QWidget *parent):
    QLabel(parent)
{

}

void IconLabel::enterEvent(QEvent *event){
    emit enterWidget();
    QLabel::enterEvent(event);
}

void IconLabel::leaveEvent(QEvent *event){
    emit leaveWidget();
    QLabel::leaveEvent(event);
}

#include "drownlabel.h"
#include "deviceframe.h"

DrownLabel::DrownLabel(QWidget * parent) : QLabel(parent)
{
    setFixedSize(36,36);
    loadPixmap(isChecked);
}

DrownLabel::~DrownLabel()
{

}
void DrownLabel::setDropDownStatus(bool status)
{
    isChecked = status;
    loadPixmap(isChecked);
}

void DrownLabel::loadPixmap(bool isChecked)
{
    QPixmap pixmap;
    if (isChecked) {
        pixmap.load(":/img/plugins/netconnect/up.svg");
    } else {
        pixmap.load(":/img/plugins/netconnect/down.svg");
    }
    setPixmap(pixmap);
}

void DrownLabel::mousePressEvent(QMouseEvent *event)
{
    emit labelClicked();
    QWidget::mousePressEvent(event);
}

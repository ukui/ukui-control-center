#include "drownlabel.h"
#include "deviceframe.h"

DrownLabel::DrownLabel(QString devName, QWidget * parent) : QLabel(parent)
{
    m_devName = devName;
    setFixedSize(36,36);
//    setFlat(true);
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
    if (isChecked) {
        setPixmap(QIcon::fromTheme("ukui-up-symbolic", QIcon(":/img/plugins/netconnect/up.svg")).pixmap(12,12));
    } else {
        setPixmap(QIcon::fromTheme("ukui-down-symbolic", QIcon(":/img/plugins/netconnect/down.svg")).pixmap(12,12));
    }
}


void DrownLabel::mouseReleaseEvent(QMouseEvent *event)
{
    emit labelClicked();
    QWidget::mouseReleaseEvent(event);
}

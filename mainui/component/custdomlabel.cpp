#include "custdomlabel.h"

CustdomLabel::CustdomLabel(const QString &text, QWidget * parent) :
    QLabel(parent)
{
    this->setText(text);
    this->adjustSize();
}

CustdomLabel::~CustdomLabel(){

}

void CustdomLabel::mousePressEvent(QMouseEvent * event){
    if (event->button() == Qt::LeftButton){
        emit clicked();
    }
}

#include "custdomframe.h"

CustdomFrame::CustdomFrame(const QString &t, QWidget *parent) :
    QFrame(parent)
{
    text = t;
    this->setFrameShape(QFrame::VLine);
    this->setFrameShadow(QFrame::Sunken);
}

CustdomFrame::~CustdomFrame()
{
}

QString CustdomFrame::getText(){
    return text;
}

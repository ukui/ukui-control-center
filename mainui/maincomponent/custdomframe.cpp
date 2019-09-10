#include "custdomframe.h"

CustdomFrame::CustdomFrame(const QString &t, QWidget *parent) :
    QFrame(parent)
{
    text = t;
    this->setFrameShape(QFrame::VLine);
    this->setFrameShadow(QFrame::Sunken);
//    this->setStyleSheet("color: #bbbbbb");
//    this->setStyleSheet("border-top: 1px solid red; border-bottom: 1px solid red; border-left: 1px solid red; border-right: 1px solid red");
//    this->setStyleSheet("background-color: #bbbbbb");
    this->setStyleSheet("border-left: 1px solid #bbbbbb");
}

CustdomFrame::~CustdomFrame()
{
}

QString CustdomFrame::getText(){
    return text;
}

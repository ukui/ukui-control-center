#include "lightlabel.h"

LightLabel::LightLabel(QWidget *parent):
    QLabel(parent)
{

}

LightLabel::LightLabel(QString text , QWidget *parent):
    QLabel(parent)
{
    this->setText(text);
}

LightLabel::~LightLabel()
{

}

void LightLabel::paintEvent(QPaintEvent *event)
{
    QPalette pal;
    QBrush brush = pal.brightText();  //获取window的色值
    QColor textColor = brush.color();
    QString stringColor = QString("color: rgba(%1,%2,%3,%4)")
           .arg(textColor.red())
           .arg(textColor.green())
           .arg(textColor.blue())
           .arg(0.35);

    this->setStyleSheet(stringColor);

    QLabel::paintEvent(event);
}

#include "lightlabel.h"

LightLabel::LightLabel(QWidget *parent):
    FixLabel(parent)
{

}

LightLabel::LightLabel(QString text , QWidget *parent):
    FixLabel(parent)
{
    this->setText(text);
}

LightLabel::~LightLabel()
{

}

void LightLabel::paintEvent(QPaintEvent *event)
{
    QPalette paltte;
    paltte.setColor(QPalette::WindowText, paltte.placeholderText().color());
    this->setPalette(paltte);
    FixLabel::paintEvent(event);
    return;
}

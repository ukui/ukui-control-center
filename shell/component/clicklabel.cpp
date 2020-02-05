#include "clicklabel.h"

ClickLabel::ClickLabel(const QString &text, QWidget *parent)
{
    setText(text);
    adjustSize();
}

ClickLabel::~ClickLabel()
{
}

void ClickLabel::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton)
        emit clicked();
    QLabel::mousePressEvent(event);
}

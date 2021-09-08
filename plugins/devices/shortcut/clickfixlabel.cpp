#include "clickfixlabel.h"
#include <QDebug>

ClickFixLabel::ClickFixLabel(QWidget *parent):
    FixLabel(parent)
{
    this->setContentsMargins(8,0,0,0);
}

ClickFixLabel::~ClickFixLabel()
{

}

void ClickFixLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    Q_EMIT doubleClicked();
    return;
}

#include "rmenu.h"
#include <QEvent>
#include <QDebug>

RMenu::RMenu(QWidget *parent):
    QMenu(parent)
{

}

void RMenu::showEvent(QShowEvent *event)
{
    int menuXPos = this->pos().x();
    int menuWidth = this->size().width()-4;
    int buttonWidth = 36;
    QPoint pos = QPoint(menuXPos - menuWidth + buttonWidth,
                        this->pos().y());
    this->move(pos);
    QMenu::showEvent(event);
}


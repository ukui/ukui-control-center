#include "customstyle.h"

#include <QDebug>
#include <QPainter>
#include <QStyleOption>

CustomStyle::CustomStyle(QObject *parent) : QProxyStyle("ukui")
{

}

void CustomStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    //qDebug()<<element;
    switch (element) {
    case QStyle::PE_Frame: {

        return;
        break;
    }
    case PE_PanelItemViewItem: {
        return;
        break;
    }
    default:
        break;
    }
    return QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void CustomStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
//    qDebug()<<element;
    switch(element)
    {
    case CE_ShapedFrame:

        break;
    default:
        break;
    }
    return QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomStyle::polish(QPalette &pal)
{
    QProxyStyle::polish(pal);
    pal.setColor(QPalette::Window, pal.base().color());
}

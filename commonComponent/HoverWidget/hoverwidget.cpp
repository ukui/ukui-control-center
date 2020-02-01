#include "hoverwidget.h"

#include <QPainter>
#include <QStyleOption>

#include <QDebug>

HoverWidget::HoverWidget(QString mname, QWidget *parent) :
    QWidget(parent),
    _name(mname)
{
    setAttribute(Qt::WA_DeleteOnClose);
}

HoverWidget::~HoverWidget()
{
}

void HoverWidget::enterEvent(QEvent *event){
    emit enterWidget(_name);

    QWidget::enterEvent(event);
}

void HoverWidget::leaveEvent(QEvent *event){
    emit leaveWidget(_name);

    QWidget::leaveEvent(event);
}

//子类化一个QWidget，为了能够使用样式表，则需要提供paintEvent事件。
//这是因为QWidget的paintEvent()是空的，而样式表要通过paint被绘制到窗口中。
void HoverWidget::paintEvent(QPaintEvent *event){
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

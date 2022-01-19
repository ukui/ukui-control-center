#include "mstackwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QPainterPath>

MStackWidget::MStackWidget(QWidget *parent)
    : QStackedWidget(parent)
{

}

MStackWidget::~MStackWidget()
{

}

void MStackWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    QPainterPath path;
    path.addRoundedRect(this->rect(),6,6);
    path.setFillRule(Qt::WindingFill); // 多块区域组合填充模式
    path.addRect(width() - 6,height() -6 ,6,6);

    status ? painter.setBrush(this->palette().base()) : painter.setBrush(this->palette().window());
    painter.setPen(Qt::transparent);
    painter.drawPath(path);
    QWidget::paintEvent(event);
}

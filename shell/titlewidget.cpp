#include "titlewidget.h"
#include <QPainter>
#include <QDebug>

TitleWidget::TitleWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setAutoFillBackground(true);
}

TitleWidget::~TitleWidget()
{

}

void TitleWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    QPainterPath path;
    path.addRoundedRect(this->rect(),6,6);
    path.setFillRule(Qt::WindingFill); // 多块区域组合填充模式
    path.addRect(width() - 6,height() -6 ,6,6);
    painter.setBrush(this->palette().base());
    status ? painter.setBrush(this->palette().base()) : painter.setBrush(this->palette().window());
    painter.setPen(Qt::transparent);
    painter.drawPath(path);
    QWidget::paintEvent(event);
}

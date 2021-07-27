#include "coustomstyle.h"
#include <QDebug>
#include <QPainter>
#include <QSlider>
#include <QStyleOptionSlider>

CoustoMstyle::CoustoMstyle()
{
}

void CoustoMstyle::drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option,
                                      QPainter *painter, const QWidget *widget) const
{
    if(control == CC_Slider){
        QRect groove = subControlRect(CC_Slider, option, SC_SliderGroove, widget);
        QRect handle = subControlRect(CC_Slider, option, SC_SliderHandle, widget);
        qDebug()<<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>groove"<<groove.x()<<groove.y()<<groove.width()<<groove.height();
        qDebug()<<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>handle"<<handle.x()<<handle.y()<<handle.width()<<handle.height();
        painter->save();

        if(const QSlider *ms = qobject_cast<const QSlider *>(widget))
        {
            if(const QStyleOptionSlider *sl = qstyleoption_cast<const QStyleOptionSlider *>(option))
            {
                QStyleOptionSlider sll = *sl;
                if ((option->subControls & QStyle::SC_SliderGroove) && groove.isValid() && handle.isValid())// 背景
                {
                    //绘制滑槽
                    QPen pen; //画笔。绘制图形边线，由颜色、宽度、线风格等参数组成
                    pen.setColor(QColor(50,100,200,255));
                    QBrush brush;   //画刷。填充几何图形的调色板，由颜色和填充风格组成
                    brush.setColor(QColor(50,100,200,255));
                    brush.setStyle(Qt::SolidPattern);
                    painter->setPen(pen);
                    painter->setBrush(brush);
                    qDebug()<<">>>>>>>>>>>>>>>>>ms->width()"<<ms->width();
                    painter->drawRect(QRect(groove.x()+4, ms->height()/2-4,ms->width()-8,8)); //滑槽中间部分
                    //painter->drawRect(QRect(groove.x(), groove.y()/2+2,ms->width(),8));
                    painter->drawRect(QRect(groove.x(), ms->height()/2-8,4,16));  //滑槽两端矩形部分
                    painter->drawRect(QRect(groove.width()-4, ms->height()/2-8,4,16));
                    QPen pen1;
                    pen1.setColor(QColor(255,255,255,255));
                    QBrush brush1;
                    brush1.setColor(QColor(255,255,255,255));
                    brush1.setStyle(Qt::SolidPattern);
                    painter->setPen(pen1);
                    painter->setBrush(brush1);
                    painter->drawRect(QRect(ms->width()/2-2, ms->height()/2-2,4,4)); //滑槽中间白点标记

                    //绘制滑块
                    QPixmap pix;
                    pix.load(":/new/prefix1/slider.svg");
                    painter->drawPixmap(handle.x(),handle.y()-4,32,24,pix);

                }

            }
        }
        painter->restore();

    }
    //    QPlastiqueStyle::drawComplexControl(cc, opt, p, w);

}

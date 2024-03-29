/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "ukui_custom_style.h"
#include <QStyleOption>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QPaintEvent>
#include <QStylePainter>
#include <QCoreApplication>
#include <QDebug>

UkuiMediaSliderTipLabel::UkuiMediaSliderTipLabel(){
    setAttribute(Qt::WA_TranslucentBackground);
}

UkuiMediaSliderTipLabel::~UkuiMediaSliderTipLabel(){
}

void UkuiMediaSliderTipLabel::paintEvent(QPaintEvent *e)
{
    QStyleOptionFrame opt;
    initStyleOption(&opt);
    QStylePainter p(this);
//    p.setBrush(QBrush(QColor(0x1A,0x1A,0x1A,0x4C)));
    p.setBrush(QBrush(QColor(0xFF,0xFF,0xFF,0x33)));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(this->rect(), 1, 1);
    QPainterPath path;
    path.addRoundedRect(opt.rect,6,6);
    p.setRenderHint(QPainter::Antialiasing);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    this->setProperty("blurRegion", QRegion(QRect(0, 0, 1, 1)));
    QLabel::paintEvent(e);
}

UkmediaVolumeSlider::UkmediaVolumeSlider(QWidget *parent,bool needTip)
{
    Q_UNUSED(parent);
    if (needTip) {
        state = needTip;
        m_pTiplabel = new UkuiMediaSliderTipLabel();
        m_pTiplabel->setWindowFlags(Qt::ToolTip);
    //    qApp->installEventFilter(new AppEventFilter(this));
        m_pTiplabel->setFixedSize(52,30);
        m_pTiplabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    }
}

void UkmediaVolumeSlider::mousePressEvent(QMouseEvent *ev)
{
    int value = 0;
    int currentX = ev->pos().x();
    double per = currentX * 1.0 / this->width();
    if ((this->maximum() - this->minimum()) >= 50) { //减小鼠标点击像素的影响
        value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
        if (value <= (this->maximum() / 2 - this->maximum() / 10 + this->minimum() / 10)) {
            value = qRound(per*(this->maximum() - this->minimum() - 1)) + this->minimum();
        } else if (value > (this->maximum() / 2 + this->maximum() / 10 + this->minimum() / 10)) {
            value = qRound(per*(this->maximum() - this->minimum() + 1)) + this->minimum();
        } else {
            value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
        }
    } else {
        value = qRound(per*(this->maximum() - this->minimum())) + this->minimum();
    }
    this->setValue(value);
    QSlider::mousePressEvent(ev);
}
void UkmediaVolumeSlider::mouseReleaseEvent(QMouseEvent *e)
{
    if(mousePress){
        Q_EMIT silderReleaseSignal();
    }
    mousePress = false;
    QSlider::mouseReleaseEvent(e);
}

void UkmediaVolumeSlider::initStyleOption(QStyleOptionSlider *option)
{
    QSlider::initStyleOption(option);
}

void UkmediaVolumeSlider::leaveEvent(QEvent *e)
{
    if (state) {
        m_pTiplabel->hide();
    }
}

void UkmediaVolumeSlider::enterEvent(QEvent *e)
{
    if (state) {
        m_pTiplabel->show();
    }
}

void UkmediaVolumeSlider::paintEvent(QPaintEvent *e)
{
    QRect rect;
    QStyleOptionSlider m_option;
    QSlider::paintEvent(e);
    if (state) {

        this->initStyleOption(&m_option);
        rect = this->style()->subControlRect(QStyle::CC_Slider, &m_option,QStyle::SC_SliderHandle,this);
        QPoint gPos = this->mapToGlobal(rect.topLeft());
        QString percent;
        percent = QString::number(this->value());
        percent.append("%");
        m_pTiplabel->setText(percent);
        m_pTiplabel->move(gPos.x()-(m_pTiplabel->width()/2)+9,gPos.y()-m_pTiplabel->height()-1);
    }


}

UkmediaVolumeSlider::~UkmediaVolumeSlider()
{
}

void UkuiButtonDrawSvg::init(QImage img, QColor color)
{
    mImage = img;
    mColor = color;
}

void UkuiButtonDrawSvg::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setBrush(QBrush(QColor(0x13,0x13,0x14,0x00)));
    p.setPen(Qt::NoPen);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);
    path.addRoundedRect(opt.rect,6,6);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.drawRoundedRect(opt.rect,6,6);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

QRect UkuiButtonDrawSvg::IconGeometry()
{
    QRect res = QRect(QPoint(0,0),QSize(24,24));
    res.moveCenter(QRect(0,0,width(),height()).center());
    return res;
}

void UkuiButtonDrawSvg::draw(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    QRect iconRect = IconGeometry();
    if (mImage.size() != iconRect.size())
    {
        mImage = mImage.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect r = mImage.rect();
        r.moveCenter(iconRect.center());
        iconRect = r;
    }

    this->setProperty("fillIconSymbolicColor", true);
    filledSymbolicColoredPixmap(mImage,mColor);
    painter.drawImage(iconRect, mImage);
}

bool UkuiButtonDrawSvg::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Paint:
        draw(static_cast<QPaintEvent*>(event));
        break;

    case QEvent::Move:
    case QEvent::Resize:
    {
        QRect rect = IconGeometry();
    }
        break;

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
        event->accept();
        break;

    default:
        break;
    }

    return QPushButton::event(event);
}


UkuiButtonDrawSvg::UkuiButtonDrawSvg(QWidget *parent)
{
    Q_UNUSED(parent);
}
UkuiButtonDrawSvg::~UkuiButtonDrawSvg()
{

}

QPixmap UkuiButtonDrawSvg::filledSymbolicColoredPixmap(QImage &img, QColor &baseColor)
{

    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                int hue = color.hue();
                if (!qAbs(hue - symbolic_color.hue()) < 10) {
                    color.setRed(baseColor.red());
                    color.setGreen(baseColor.green());
                    color.setBlue(baseColor.blue());
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }

    return QPixmap::fromImage(img);
}




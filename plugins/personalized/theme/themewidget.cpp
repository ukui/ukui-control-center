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
#include "themewidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include <QPainter>
#include <QStyleOption>

#include <QDebug>

ThemeWidget::ThemeWidget(QSize iSize, QString name, QStringList iStringList, QWidget *parent) :
    QFrame(parent)
{
    setFixedHeight(66);
    setMinimumWidth(550);
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameShape(QFrame::NoFrame);

    pValue = "";

    QHBoxLayout * mainHorLayout = new QHBoxLayout(this);
    mainHorLayout->setSpacing(16);
    mainHorLayout->setContentsMargins(16, 0, 16, 0);

    //占位Label，解决隐藏选中图标后文字Label位置的变化
    placeHolderLabel = new QLabel(this);
    QSizePolicy phSizePolicy = placeHolderLabel->sizePolicy();
    phSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    phSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    placeHolderLabel->setSizePolicy(phSizePolicy);
    placeHolderLabel->setFixedSize(QSize(16, 16)); //选中图标的大小为16*16

    selectedLabel = new QLabel(this);
    QSizePolicy sSizePolicy = selectedLabel->sizePolicy();
    sSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    sSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    selectedLabel->setSizePolicy(sSizePolicy);
    selectedLabel->setScaledContents(true);
    QIcon selectIcon = QIcon::fromTheme("ukui-selected");
    selectedLabel->setPixmap(selectIcon.pixmap(selectIcon.actualSize(QSize(16, 16))));
    const QByteArray id(THEME_QT_SCHEMA);
    QGSettings *mQtSettings = new QGSettings(id, QByteArray(), this);
    connect(mQtSettings, &QGSettings::changed, this, [=](QString key) {
        if (key == "iconThemeName")
            selectedLabel->setPixmap(selectIcon.pixmap(selectIcon.actualSize(QSize(16, 16))));
    });

    FixLabel * nameLabel = new FixLabel(this);
    QSizePolicy nameSizePolicy = nameLabel->sizePolicy();
    nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    nameLabel->setSizePolicy(nameSizePolicy);
    nameLabel->setMinimumWidth(130);
    nameLabel->setText(name);

    QHBoxLayout * iconHorLayout = new QHBoxLayout;
    iconHorLayout->setSpacing(16);
    iconHorLayout->setMargin(0);
    for (QString icon : iStringList){
        QLabel * label = new QLabel(this);
        label->setFixedSize(iSize);
        label->setPixmap(pixmapToRound(icon, iSize));
//        label->setPixmap(QPixmap(icon));
        iconHorLayout->addWidget(label);
    }


    mainHorLayout->addWidget(nameLabel);
    mainHorLayout->addSpacerItem(new QSpacerItem(38,20));
    mainHorLayout->addLayout(iconHorLayout);
    mainHorLayout->addStretch(2);
    mainHorLayout->addWidget(placeHolderLabel);
    mainHorLayout->addWidget(selectedLabel);

    setLayout(mainHorLayout);
}

ThemeWidget::ThemeWidget(QSize iSize, QString name, const QList<QPixmap> &listMap, QWidget *parent) :
    QFrame(parent)
{
    setFixedHeight(66);
    setMinimumWidth(550);
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameShape(QFrame::NoFrame);

    pValue = "";

    QHBoxLayout * mainHorLayout = new QHBoxLayout(this);
    mainHorLayout->setSpacing(16);
    mainHorLayout->setContentsMargins(16, 0, 16, 0);

    //占位Label，解决隐藏选中图标后文字Label位置的变化
    placeHolderLabel = new QLabel(this);
    QSizePolicy phSizePolicy = placeHolderLabel->sizePolicy();
    phSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    phSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    placeHolderLabel->setSizePolicy(phSizePolicy);
    placeHolderLabel->setFixedSize(QSize(16, 16)); //选中图标的大小为16*16

    selectedLabel = new QLabel(this);
    QSizePolicy sSizePolicy = selectedLabel->sizePolicy();
    sSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    sSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    selectedLabel->setSizePolicy(sSizePolicy);
    selectedLabel->setScaledContents(true);
    QIcon selectIcon = QIcon::fromTheme("ukui-selected");
    selectedLabel->setPixmap(selectIcon.pixmap(selectIcon.actualSize(QSize(16, 16))));
    const QByteArray id(THEME_QT_SCHEMA);
    QGSettings *mQtSettings = new QGSettings(id, QByteArray(), this);
    connect(mQtSettings, &QGSettings::changed, this, [=](QString key) {
        if (key == "iconThemeName")
            selectedLabel->setPixmap(selectIcon.pixmap(selectIcon.actualSize(QSize(16, 16))));
    });

    FixLabel * nameLabel = new FixLabel(this);
    QSizePolicy nameSizePolicy = nameLabel->sizePolicy();
    nameSizePolicy.setHorizontalPolicy(QSizePolicy::Preferred);
    nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    nameLabel->setSizePolicy(nameSizePolicy);
    nameLabel->setMinimumWidth(130);
    nameLabel->setText(name);

    QHBoxLayout * iconHorLayout = new QHBoxLayout;
    iconHorLayout->setSpacing(25);
    iconHorLayout->setMargin(0);
    for (QPixmap icon : listMap){
        QLabel * label = new QLabel(this);
        label->setFixedSize(iSize);
        label->setPixmap(icon);
        iconHorLayout->addWidget(label);
    }

    mainHorLayout->addWidget(nameLabel);
    mainHorLayout->addSpacerItem(new QSpacerItem(38,20));
    mainHorLayout->addLayout(iconHorLayout);
    mainHorLayout->addStretch(2);
    mainHorLayout->addWidget(placeHolderLabel);
    mainHorLayout->addWidget(selectedLabel);

    setLayout(mainHorLayout);

}

ThemeWidget::~ThemeWidget()
{
}

void ThemeWidget::setSelectedStatus(bool status){
    placeHolderLabel->setVisible(!status);
    selectedLabel->setVisible(status);
}

void ThemeWidget::setValue(QString value){
    pValue = value;
}

//新增函数，规避分数缩放锯齿形
QPixmap ThemeWidget::pixmapToRound(const QString &filePath, const QSize &scaledSize)
{

    QPixmap pixmap(filePath);

    qreal dpi = devicePixelRatioF();

    QSize realSize = scaledSize * dpi;
    QPixmap resultPixmap(realSize);

    resultPixmap.fill(Qt::transparent);
    QPainter painter(&resultPixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    //画圆
//    QPainterPath path;
//    path.addEllipse(0, 0, realSize.width(), realSize.height());
//    painter.setClipPath(path);


    QPoint drawLeftTop(0, 0);
    QSize drawSize = realSize;

    drawSize.setWidth(realSize.width());
    drawSize.setHeight(realSize.width() * scaledSize.height() / scaledSize.width());
    drawLeftTop.setX(0);
    drawLeftTop.setY((realSize.height() - drawSize.height()) / 2);

    // scaled draw
    painter.drawPixmap(drawLeftTop.x(), drawLeftTop.y(), drawSize.width(), drawSize.height(), pixmap.scaled(drawSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    resultPixmap.setDevicePixelRatio(dpi);
    return resultPixmap;

}

QString ThemeWidget::getValue(){
    return pValue;
}

void ThemeWidget::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        emit clicked();
    }

    QWidget::mousePressEvent(event);
}

//子类化一个QWidget，为了能够使用样式表，则需要提供paintEvent事件。
//这是因为QWidget的paintEvent()是空的，而样式表要通过paint被绘制到窗口中。
void ThemeWidget::paintEvent(QPaintEvent *event){
    Q_UNUSED(event)
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.save();
    p.setBrush(opt.palette.color(QPalette::Base));
    p.setPen(Qt::transparent);
    p.setOpacity(0.6);
    p.drawRoundedRect(this->rect(), 6, 6);
    p.restore();
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


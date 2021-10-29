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
#include "ukmedia_output_widget.h"
#include "ukui_list_widget_item.h"
#include <QHBoxLayout>
#include <QDebug>
AudioSlider::AudioSlider(QWidget *parent)
{
    Q_UNUSED(parent);
}

AudioSlider::~AudioSlider()
{

}

UkmediaOutputWidget::UkmediaOutputWidget(QWidget *parent) : QWidget(parent)
{
    //加载qss样式文件
    QFile QssFile("://combox.qss");
    QssFile.open(QFile::ReadOnly);

    if (QssFile.isOpen()){
        sliderQss = QLatin1String(QssFile.readAll());
        QssFile.close();
    } else {
        qDebug()<<"combox.qss is not found"<<endl;
    }

    m_pOutputWidget         = new QWidget(this);
    m_pMasterVolumeWidget   = new QFrame(m_pOutputWidget);
    m_pChannelBalanceWidget = new QFrame(m_pOutputWidget);
    m_pOutputSlectWidget    = new QFrame(m_pOutputWidget);

    m_pMasterVolumeWidget->setFrameShape(QFrame::Shape::Box);
    m_pChannelBalanceWidget->setFrameShape(QFrame::Shape::Box);
    m_pOutputSlectWidget->setFrameShape(QFrame::Shape::Box);

    //设置大小
    m_pOutputWidget->setMinimumSize(550,182);
    m_pOutputWidget->setMaximumSize(16777215,182);
    m_pMasterVolumeWidget->setMinimumSize(550,60);
    m_pMasterVolumeWidget->setMaximumSize(16777215,60);
    m_pChannelBalanceWidget->setMinimumSize(550,60);
    m_pChannelBalanceWidget->setMaximumSize(16777215,60);
    m_pOutputSlectWidget->setMinimumSize(550,60);
    m_pOutputSlectWidget->setMaximumSize(16777215,60);

    m_pOutputLabel = new QLabel(this);
    m_pOutputLabel->setText(tr("Output"));
    m_pOutputLabel->setStyleSheet("QLabel{color: palette(windowText);}");

    //~ contents_path /audio/Output Device
    m_pOutputDeviceLabel = new QLabel(tr("Output Device"),m_pOutputSlectWidget);
    m_pDeviceSelectBox = new QComboBox(m_pOutputSlectWidget);
    //~ contents_path /audio/Master Volume
    m_pOpVolumeLabel = new QLabel(tr("Master Volume"),m_pMasterVolumeWidget);
    m_pOutputIconBtn = new UkuiButtonDrawSvg(m_pMasterVolumeWidget);
    m_pOpVolumeSlider = new AudioSlider(m_pMasterVolumeWidget);
    m_pOpVolumeSlider->setStyleSheet( "QSlider::groove:horizontal {"
                                       "border: 0px none;"
                                       "background: palette(button);"
                                       "height: 8px;"
                                       "border-radius: 5px;"
                                       "}"

                                       "QSlider::handle:horizontal {"

                                       "height: 24px;"
                                       "width: 32px;"
                                       "margin: 30px;"
                                       "border-image: url(://img/plugins/mouse/slider.svg);"
                                       "margin: -8 -4px;"
                                       "}"

                                       "QSlider::add-page:horizontal {"
                                       "background: palette(button);"
                                       "border-radius: 20px;"
                                       "}"

                                       "QSlider::sub-page:horizontal {"
                                       "background: #2FB3E8;"
                                       "border-radius: 5px;"
                                       "}");
    m_pOpVolumePercentLabel = new QLabel(m_pMasterVolumeWidget);
    //~ contents_path /audio/Balance
    m_pOpBalanceLabel = new QLabel(tr("Balance"),m_pChannelBalanceWidget);
    m_pLeftBalanceLabel = new QLabel(tr("Left"),m_pChannelBalanceWidget);
    m_pOpBalanceSlider = new UkmediaVolumeSlider(m_pChannelBalanceWidget,true);
    m_pOpBalanceSlider->setStyleSheet( "QSlider::groove:horizontal {"
                                       "border: 0px none;"
                                       "background: palette(button);"
                                       "height: 8px;"
                                       "border-radius: 5px;"
                                       "}"

                                       "QSlider::handle:horizontal {"

                                       "height: 24px;"
                                       "width: 32px;"
                                       "margin: 30px;"
                                       "border-image: url(://img/plugins/mouse/slider.svg);"
                                       "margin: -8 -4px;"
                                       "}"

                                       "QSlider::add-page:horizontal {"
                                       "background: palette(button);"
                                       "border-radius: 20px;"
                                       "}"

                                       "QSlider::sub-page:horizontal {"
                                       "background: palette(button);"
                                       "border-radius: 5px;"
                                       "}");
    m_pRightBalanceLabel = new QLabel(tr("Right"),m_pChannelBalanceWidget);
    m_pBalanceMidPointLabel = new QLabel(m_pChannelBalanceWidget);
    m_pBalanceMidPointLabel->setStyleSheet("min-width: 4px;min-height: 4px;max-width: 4px;max-height: 4px;"
                                           "border-radius:2px;"
                                           "border:1px solid white;"
                                           "background:white");

    m_pOpVolumeSlider->setOrientation(Qt::Horizontal);
    m_pOpBalanceSlider->setOrientation(Qt::Horizontal);
    m_pOpVolumeSlider->setRange(0,100);
    m_pOutputIconBtn->setFocusPolicy(Qt::NoFocus);


    //选择输出设备框添加布局
    m_pOutputDeviceLabel->setFixedSize(140,20);
    m_pDeviceSelectBox->setFixedHeight(40);
    QHBoxLayout *deviceSlectLayout = new QHBoxLayout();
    deviceSlectLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    deviceSlectLayout->addWidget(m_pOutputDeviceLabel);
    deviceSlectLayout->addItem(new QSpacerItem(100,20,QSizePolicy::Fixed));
    deviceSlectLayout->addWidget(m_pDeviceSelectBox);
    deviceSlectLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    deviceSlectLayout->setSpacing(0);
    m_pOutputSlectWidget->setLayout(deviceSlectLayout);
    m_pOutputSlectWidget->layout()->setContentsMargins(0,0,0,0);

    //主音量添加布局
    QHBoxLayout *masterLayout = new QHBoxLayout(m_pMasterVolumeWidget);
    m_pOpVolumeLabel->setFixedSize(140,20);
    m_pOutputIconBtn->setFixedSize(24,24);
    m_pOpVolumeSlider->setFixedHeight(20);
    m_pOpVolumePercentLabel->setFixedSize(55,20);
    m_pOpVolumePercentLabel->setAlignment(Qt::AlignCenter);
    masterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    masterLayout->addWidget(m_pOpVolumeLabel);
    masterLayout->addItem(new QSpacerItem(100,20,QSizePolicy::Fixed));
    masterLayout->addWidget(m_pOutputIconBtn);
    masterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    masterLayout->addWidget(m_pOpVolumeSlider);
    masterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    masterLayout->addWidget(m_pOpVolumePercentLabel);
    masterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    masterLayout->setSpacing(0);
    m_pMasterVolumeWidget->setLayout(masterLayout);
    m_pMasterVolumeWidget->layout()->setContentsMargins(0,0,0,0);

    //声道平衡添加布局
    QHBoxLayout *soundLayout = new QHBoxLayout(m_pChannelBalanceWidget);
    m_pOpBalanceLabel->setFixedSize(140,20);
    m_pLeftBalanceLabel->setFixedSize(35,30);
    m_pOpBalanceSlider->setFixedHeight(20);
    m_pRightBalanceLabel->setFixedSize(55,30);
    m_pRightBalanceLabel->setAlignment(Qt::AlignCenter);
    soundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    soundLayout->addWidget(m_pOpBalanceLabel);
    soundLayout->addItem(new QSpacerItem(100,20,QSizePolicy::Fixed));
    soundLayout->addWidget(m_pLeftBalanceLabel);
    soundLayout->addItem(new QSpacerItem(5,20,QSizePolicy::Fixed));
    soundLayout->addWidget(m_pOpBalanceSlider);
    soundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    soundLayout->addWidget(m_pRightBalanceLabel);
    soundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    soundLayout->setSpacing(0);
    m_pChannelBalanceWidget->setLayout(soundLayout);
    m_pChannelBalanceWidget->layout()->setContentsMargins(0,0,0,0);
    m_pBalanceMidPointLabel->raise();


    //进行整体布局
    m_pVlayout = new QVBoxLayout;
    m_pVlayout->addWidget(m_pOutputSlectWidget);
    m_pVlayout->addWidget(m_pMasterVolumeWidget);
    m_pVlayout->addWidget(m_pChannelBalanceWidget);
    m_pVlayout->setSpacing(1);
    m_pOutputWidget->setLayout(m_pVlayout);
    m_pOutputWidget->layout()->setContentsMargins(0,0,0,0);

    QVBoxLayout *vLayout1 = new QVBoxLayout(this);
    vLayout1->addWidget(m_pOutputLabel);
    vLayout1->addItem(new QSpacerItem(16,0,QSizePolicy::Fixed));
    vLayout1->addWidget(m_pOutputWidget);
    this->setLayout(vLayout1);
    this->layout()->setContentsMargins(0,0,0,0);

    m_pMasterVolumeWidget->setObjectName("masterVolumeWidget");
    //设置样式
    m_pOutputLabel->setObjectName("m_pOutputLabel");

}

void UkmediaOutputWidget::paintEvent(QPaintEvent *e)
{
    m_pBalanceMidPointLabel->move(294+m_pOpBalanceSlider->size().width()/2,m_pOpBalanceSlider->size().height()+7);
    QWidget::paintEvent(e);
}

UkmediaOutputWidget::~UkmediaOutputWidget()
{

}

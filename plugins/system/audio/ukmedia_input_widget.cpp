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
#include "ukmedia_input_widget.h"
#include <QDebug>
#include <QFile>
#include <QApplication>

UkmediaInputWidget::UkmediaInputWidget(QWidget *parent) : QWidget(parent)
{
    m_pInputWidget      = new QWidget(this);
    m_pVolumeWidget     = new QFrame(m_pInputWidget);
    m_pInputLevelWidget = new QFrame(m_pInputWidget);
    m_pInputSlectWidget = new QFrame(m_pInputWidget);
    m_pDnsNoiseReductionWidget = new QFrame(m_pInputWidget);


    m_pVolumeWidget->setFrameShape(QFrame::Shape::Box);
    m_pInputLevelWidget->setFrameShape(QFrame::Shape::Box);
    m_pInputSlectWidget->setFrameShape(QFrame::Shape::Box);
    m_pDnsNoiseReductionWidget->setFrameShape(QFrame::Shape::Box);



    //设置大小
    m_pInputWidget->setMinimumSize(550,244);
    m_pInputWidget->setMaximumSize(16777215,244);
    m_pVolumeWidget->setMinimumSize(550,60);
    m_pVolumeWidget->setMaximumSize(16777215,60);
    m_pInputLevelWidget->setMinimumSize(550,60);
    m_pInputLevelWidget->setMaximumSize(16777215,60);
    m_pInputSlectWidget->setMinimumSize(550,60);
    m_pInputSlectWidget->setMaximumSize(16777215,60);
    m_pDnsNoiseReductionWidget->setMinimumSize(550,60);
    m_pDnsNoiseReductionWidget->setMinimumSize(16777215,60);


    m_pInputLabel = new TitleLabel(this);
    m_pInputLabel->setFixedSize(150,32);
    m_pInputLabel->setText(tr("Input"));
    m_pInputLabel->setStyleSheet("color: palette(windowText);}");


    //~ contents_path /audio/Input Device
    m_pInputDeviceLabel = new QLabel(tr("Input Device"),m_pInputSlectWidget);
    m_pInputDeviceSelectBox = new QComboBox(m_pInputSlectWidget);
    //~ contents_path /audio/Volume
    m_pIpVolumeLabel = new QLabel(tr("Volume"),m_pVolumeWidget);
    m_pInputIconBtn = new UkuiButtonDrawSvg(m_pVolumeWidget);
    m_pIpVolumeSlider = new AudioSlider();
    m_pIpVolumeSlider->setStyleSheet( "QSlider::groove:horizontal {"
                                       "border: 0px none;"
                                       "background: palette(button);"
                                       "height: 8px;"
                                       "border-radius: 5px;"
                                       "}"

                                       "QSlider::handle:horizontal {"

                                       "height: 40px;"
                                       "width: 36px;"
                                       "margin: 30px;"
                                       "border-image: url(://img/plugins/mouse/slider.svg);"
                                       "margin: -20 -4px;"
                                       "}"

//                                       "QSlider::add-page:horizontal {"
//                                       "background: palette(button);"
//                                       "border-radius: 20px;"
//                                       "}"

                                       "QSlider::sub-page:horizontal {"
                                       "background: #2FB3E8;"
                                       "border-radius: 5px;"
                                       "}");

    m_pIpVolumePercentLabel = new QLabel(m_pVolumeWidget);
    //~ contents_path /audio/Input Level
    m_pInputLevelLabel = new QLabel(tr("Input Level"),m_pInputLevelWidget);
    m_pInputLevelProgressBar = new QProgressBar(m_pInputLevelWidget);
    m_pInputLevelProgressBar->setStyle(new CustomStyle);
    m_pInputLevelProgressBar->setTextVisible(false);

    m_pIpVolumeSlider->setOrientation(Qt::Horizontal);
    m_pIpVolumeSlider->setRange(0,100);
    m_pInputIconBtn->setFocusPolicy(Qt::NoFocus);

    //~ contents_path /audio/Noise Reduction
    m_pDnsNoiseReductionLabel = new QLabel(tr("Noise Reduction"),m_pDnsNoiseReductionWidget);
    m_pDnsNoiseReductionButton = new SwitchButton(m_pDnsNoiseReductionWidget);


    //输入设备添加布局
    m_pInputDeviceLabel->setFixedSize(140,20);
    m_pInputDeviceSelectBox->setFixedHeight(40);
    QHBoxLayout *inputdeviceSlectLayout = new QHBoxLayout();
    inputdeviceSlectLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    inputdeviceSlectLayout->addWidget(m_pInputDeviceLabel);
    inputdeviceSlectLayout->addItem(new QSpacerItem(100,20,QSizePolicy::Fixed));
    inputdeviceSlectLayout->addWidget(m_pInputDeviceSelectBox);
    inputdeviceSlectLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    inputdeviceSlectLayout->setSpacing(0);
    m_pInputSlectWidget->setLayout(inputdeviceSlectLayout);
    m_pInputSlectWidget->layout()->setContentsMargins(0,0,0,0);

    //dns降噪
    QHBoxLayout *dnsNoiseReductionLayout = new QHBoxLayout(m_pDnsNoiseReductionWidget);
    dnsNoiseReductionLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    dnsNoiseReductionLayout->addWidget(m_pDnsNoiseReductionLabel);
    dnsNoiseReductionLayout->addWidget(m_pDnsNoiseReductionButton);
    dnsNoiseReductionLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    dnsNoiseReductionLayout->setSpacing(0);
    m_pDnsNoiseReductionWidget->setLayout(dnsNoiseReductionLayout);
    m_pDnsNoiseReductionWidget->layout()->setContentsMargins(0,0,0,0);


    //主音量添加布局
    QHBoxLayout *m_pMasterLayout = new QHBoxLayout(m_pVolumeWidget);
    m_pIpVolumeLabel->setFixedSize(140,20);
    m_pInputIconBtn->setFixedSize(24,24);
    m_pIpVolumeSlider->setFixedHeight(60);
    m_pIpVolumePercentLabel->setFixedSize(55,20);
    m_pIpVolumePercentLabel->setAlignment(Qt::AlignCenter);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pIpVolumeLabel);
    m_pMasterLayout->addItem(new QSpacerItem(100,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pInputIconBtn);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pIpVolumeSlider);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Maximum));
    m_pMasterLayout->addWidget(m_pIpVolumePercentLabel);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Maximum));
    m_pMasterLayout->setSpacing(0);
    m_pVolumeWidget->setLayout(m_pMasterLayout);
    m_pVolumeWidget->layout()->setContentsMargins(0,0,0,0);

    //输入反馈添加布局
    QHBoxLayout *m_pSoundLayout = new QHBoxLayout(m_pInputLevelWidget);
    m_pInputLevelLabel->setFixedSize(140,20);
    m_pSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pInputLevelLabel);
    m_pSoundLayout->addItem(new QSpacerItem(100,20,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pInputLevelProgressBar);
//    m_pSoundLayout->addItem(new QSpacerItem(5,20,QSizePolicy::Fixed));
    m_pSoundLayout->setSpacing(0);
    m_pInputLevelWidget->setLayout(m_pSoundLayout);
    m_pInputLevelWidget->layout()->setContentsMargins(0,0,0,0);

    //进行整体布局
    m_pVlayout = new QVBoxLayout(m_pInputWidget);
    m_pVlayout->addWidget(m_pInputSlectWidget);
    m_pVlayout->addWidget(m_pDnsNoiseReductionWidget);
    m_pVlayout->addWidget(m_pVolumeWidget);
    m_pVlayout->addWidget(m_pInputLevelWidget);
    m_pVlayout->setSpacing(1);
    m_pInputWidget->setLayout(m_pVlayout);
    m_pInputWidget->layout()->setContentsMargins(0,0,0,0);

    QVBoxLayout *m_pVlayout1 = new QVBoxLayout(this);
    m_pVlayout1->addWidget(m_pInputLabel);
    m_pVlayout1->addItem(new QSpacerItem(16,4,QSizePolicy::Fixed));
    m_pVlayout1->addWidget(m_pInputWidget);
    m_pVlayout1->setSpacing(0);
    this->setLayout(m_pVlayout1);
    this->layout()->setContentsMargins(0,0,0,0);

}

UkmediaInputWidget::~UkmediaInputWidget()
{

}

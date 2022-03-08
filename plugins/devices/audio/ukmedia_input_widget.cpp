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
    m_pInputWidget = new QWidget(this);
    m_pInputDeviceWidget = new QFrame(m_pInputWidget);
    m_pVolumeWidget = new QFrame(m_pInputWidget);
    m_pInputLevelWidget = new QFrame(m_pInputWidget);
    m_pNoiseReducteWidget = new QFrame(m_pInputWidget);

    m_pInputListWidget = new QListWidget(this);
    m_pInputListWidget->setFixedHeight(250);

    m_pInputListWidget->setStyleSheet(
                
                "QListWidget{"
                "background-color:palette(base);"
                "padding-left:8;"
                "padding-right:20;"
                "padding-top:8;"
                "padding-bottom:8;}"
                "QListWidget::item{"
                "border-radius:6px;}"
                /**列表项扫过*/
                "QListWidget::item:hover{"
                "background-color:rgba(55,144,250,0.5);}"
                /**列表项选中*/
                "QListWidget::item::selected{"
                "background-color:rgba(55,144,250,1);"
                "border-width:0;}");
    m_pInputDeviceWidget->setFrameShape(QFrame::Shape::Box);
    m_pVolumeWidget->setFrameShape(QFrame::Shape::Box);
    m_pInputLevelWidget->setFrameShape(QFrame::Shape::Box);
    m_pNoiseReducteWidget->setFrameShape(QFrame::Shape::Box);
    //设置大小
    m_pInputWidget->setMinimumSize(550,473);
    m_pInputWidget->setMaximumSize(960,473);
    m_pInputDeviceWidget->setMinimumSize(550,319);
    m_pInputDeviceWidget->setMaximumSize(960,319);
    m_pVolumeWidget->setMinimumSize(550,50);
    m_pVolumeWidget->setMaximumSize(960,50);
    m_pInputLevelWidget->setMinimumSize(550,50);
    m_pInputLevelWidget->setMaximumSize(960,50);
    m_pNoiseReducteWidget->setMinimumSize(550,50);
    m_pNoiseReducteWidget->setMaximumSize(960,50);

    m_pInputLabel = new TitleLabel(this);
    m_pInputLabel->setText(tr("Input"));
    m_pInputLabel->setStyleSheet("color: palette(windowText);}");
    //~ contents_path /audio/Input Device
    m_pInputDeviceLabel = new QLabel(tr("Input Device:"),m_pInputWidget);

    //~ contents_path /audio/Volume
    m_pIpVolumeLabel = new QLabel(tr("Volume"),m_pVolumeWidget);
    m_pInputIconBtn = new UkuiButtonDrawSvg(m_pVolumeWidget);
    m_pIpVolumeSlider = new AudioSlider();
    m_pIpVolumePercentLabel = new QLabel(m_pVolumeWidget);
    //~ contents_path /audio/Input Level
    m_pInputLevelLabel = new QLabel(tr("Input Level"),m_pInputLevelWidget);
    m_pInputLevelProgressBar = new QProgressBar(m_pInputLevelWidget);

    m_pInputLevelProgressBar->setStyle(new CustomStyle);
    m_pInputLevelProgressBar->setTextVisible(false);

    m_pIpVolumeSlider->setOrientation(Qt::Horizontal);
    m_pIpVolumeSlider->setRange(0,100);
    m_pInputIconBtn->setFocusPolicy(Qt::NoFocus);

   //~ contents_path /audio/Noise
    m_pNoiseReducteLabel = new QLabel(tr("Noise Reduction"));
    m_pNoiseReducteButton = new SwitchButton;

    //输入设备添加布局
    QVBoxLayout *m_pInputDeviceLayout = new QVBoxLayout(m_pInputDeviceWidget);
    m_pInputDeviceLabel->setFixedSize(150,32);

    m_pInputDeviceLayout->addWidget(m_pInputDeviceLabel);
    m_pInputDeviceLayout->addWidget(m_pInputListWidget);
    m_pInputDeviceLayout->setSpacing(0);
    m_pInputDeviceWidget->setLayout(m_pInputDeviceLayout);
    m_pInputDeviceLayout->layout()->setContentsMargins(16,14,16,14);

    //主音量添加布局
    QHBoxLayout *m_pMasterLayout = new QHBoxLayout(m_pVolumeWidget);
    m_pIpVolumeLabel->setFixedSize(150,32);
    m_pInputIconBtn->setFixedSize(24,24);
    m_pIpVolumeSlider->setFixedHeight(20);
    m_pIpVolumePercentLabel->setFixedSize(55,24);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pIpVolumeLabel);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pInputIconBtn);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pIpVolumeSlider);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Maximum));
    m_pMasterLayout->addWidget(m_pIpVolumePercentLabel);
    m_pMasterLayout->addItem(new QSpacerItem(10,20,QSizePolicy::Maximum));
    m_pMasterLayout->setSpacing(0);
    m_pVolumeWidget->setLayout(m_pMasterLayout);
    m_pVolumeWidget->layout()->setContentsMargins(0,0,0,0);
    //声道平衡添加布局
    QHBoxLayout *m_pSoundLayout = new QHBoxLayout(m_pInputLevelWidget);
    m_pInputLevelLabel->setFixedSize(150,32);
    m_pSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pInputLevelLabel);
    m_pSoundLayout->addItem(new QSpacerItem(18,20,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pInputLevelProgressBar);
    m_pSoundLayout->setSpacing(0);
    m_pInputLevelWidget->setLayout(m_pSoundLayout);
    m_pInputLevelWidget->layout()->setContentsMargins(0,0,0,0);

    //智能降噪
    QHBoxLayout *noiseReducteLayout = new QHBoxLayout(m_pNoiseReducteWidget);
    noiseReducteLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    noiseReducteLayout->addWidget(m_pNoiseReducteLabel);
    noiseReducteLayout->addWidget(m_pNoiseReducteButton);
    noiseReducteLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    noiseReducteLayout->setSpacing(0);
    m_pNoiseReducteWidget->setLayout(noiseReducteLayout);
    m_pNoiseReducteWidget->layout()->setContentsMargins(0,0,0,0);

    //进行整体布局
    m_pVlayout = new QVBoxLayout(m_pInputWidget);

    m_pVlayout->addWidget(m_pInputDeviceWidget);
    m_pVlayout->addWidget(m_pVolumeWidget);
    m_pVlayout->addWidget(m_pInputLevelWidget);
    m_pVlayout->addWidget(m_pNoiseReducteWidget);
    m_pVlayout->setSpacing(1);
    m_pInputWidget->setLayout(m_pVlayout);
    m_pInputWidget->layout()->setContentsMargins(0,0,0,0);
    QVBoxLayout *m_pVlayout1 = new QVBoxLayout(this);
    m_pVlayout1->addWidget(m_pInputLabel);
    m_pVlayout1->addItem(new QSpacerItem(16,4,QSizePolicy::Fixed));
    m_pVlayout1->addWidget(m_pInputWidget);
    this->setLayout(m_pVlayout1);
    this->layout()->setContentsMargins(0,0,0,0);

}

UkmediaInputWidget::~UkmediaInputWidget()
{

}

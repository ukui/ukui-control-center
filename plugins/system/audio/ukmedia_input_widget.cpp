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
    m_pInputWidget = new QFrame(this);
    m_pInputWidget->setFrameShape(QFrame::Shape::Box);
    m_pInputDeviceWidget = new QFrame(m_pInputWidget);
    m_pVolumeWidget = new QFrame(m_pInputWidget);
    m_pInputLevelWidget = new QFrame(m_pInputWidget);
    m_pInputPortWidget = new QFrame(m_pInputWidget);

//    m_pInputDeviceWidget->setFrameShape(QFrame::Shape::Box);
//    m_pVolumeWidget->setFrameShape(QFrame::Shape::Box);
//    m_pInputLevelWidget->setFrameShape(QFrame::Shape::Box);
//    m_pInputPortWidget->setFrameShape(QFrame::Shape::Box);

    //设置大小
    m_pInputWidget->setMinimumSize(550,0);
    m_pInputWidget->setMaximumSize(16777215,16777215);
    m_pInputDeviceWidget->setMinimumSize(550,64);
    m_pInputDeviceWidget->setMaximumSize(16777215,64);
    m_pVolumeWidget->setMinimumSize(550,64);
    m_pVolumeWidget->setMaximumSize(16777215,64);
    m_pInputLevelWidget->setMinimumSize(550,64);
    m_pInputLevelWidget->setMaximumSize(16777215,64);
    m_pInputPortWidget->setMinimumSize(550,64);
    m_pInputPortWidget->setMaximumSize(16777215,64);

    m_pInputLabel = new QLabel(tr("Input"),this);
    m_pInputLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    m_pInputLabel->setContentsMargins(16,0,0,0);
    //~ contents_path /audio/Input Device
    m_pInputDeviceLabel = new QLabel(tr("Input Device"),m_pInputWidget);
    m_pInputDeviceCombobox = new QComboBox(m_pInputDeviceWidget);

    //~ contents_path /audio/Volume
    m_pIpVolumeLabel = new QLabel(tr("Volume"),m_pVolumeWidget);

    m_pInputIconLabel_off=new QLabel;
    QString filename1(":/new/prefix1/Microphone_off.svg");
    QImage* img1=new QImage;
    img1->load(filename1);
    m_pInputIconLabel_off->setPixmap(QPixmap::fromImage(*img1));


    m_pInputIconLabel_on=new QLabel;
    QString filename2(":/new/prefix1/Microphone_on.svg");
    QImage* img2=new QImage;
    img2->load(filename2);
    m_pInputIconLabel_on->setPixmap(QPixmap::fromImage(*img2));


    m_pInputIconBtn = new UkuiButtonDrawSvg(m_pVolumeWidget);
    m_pIpVolumeSlider = new AudioSlider(m_pVolumeWidget);
    m_pIpVolumePercentLabel = new QLabel(m_pVolumeWidget);
    //~ contents_path /audio/Input Level
    m_pInputLevelLabel = new QLabel(tr("Input Level"),m_pInputLevelWidget);
    m_pInputLevelProgressBar = new QProgressBar(m_pInputLevelWidget);

    m_pInputLevelProgressBar->setStyle(new CustomStyle);
    m_pInputPortCombobox = new QComboBox(m_pInputPortWidget);
    m_pInputPortLabel = new QLabel(tr("Connector"),m_pInputPortWidget);

    m_pInputLevelProgressBar->setTextVisible(false);

    m_pIpVolumeSlider->setOrientation(Qt::Horizontal);
    m_pIpVolumeSlider->setRange(0,100);
    m_pInputIconBtn->setFocusPolicy(Qt::NoFocus);
    //输入设备添加布局
    QHBoxLayout *m_pInputDeviceLayout = new QHBoxLayout(m_pInputDeviceWidget);
    m_pInputLabel->setFixedSize(83,24);
    m_pInputDeviceCombobox->setMinimumSize(50,48);
    m_pInputDeviceCombobox->setMaximumSize(16777215,48);
    m_pInputDeviceLabel->setFixedSize(115,24);
    m_pInputDeviceLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pInputDeviceLayout->addWidget(m_pInputDeviceLabel);
    m_pInputDeviceLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pInputDeviceLayout->addWidget(m_pInputDeviceCombobox);
    m_pInputDeviceLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pInputDeviceLayout->setSpacing(0);
    m_pInputDeviceWidget->setLayout(m_pInputDeviceLayout);
    m_pInputDeviceLayout->layout()->setContentsMargins(0,0,0,0);
    //主音量添加布局
    QHBoxLayout *m_pMasterLayout = new QHBoxLayout(m_pVolumeWidget);
    m_pIpVolumeLabel->setFixedSize(115,24);
    m_pInputIconBtn->setFixedSize(24,24);
    m_pIpVolumeSlider->setFixedHeight(20);
    m_pIpVolumePercentLabel->setFixedSize(40,24);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pIpVolumeLabel);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pInputIconLabel_off);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pIpVolumeSlider);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
//    m_pMasterLayout->addWidget(m_pIpVolumePercentLabel);
//    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pInputIconLabel_on);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pMasterLayout->setSpacing(0);
    m_pVolumeWidget->setLayout(m_pMasterLayout);
    m_pVolumeWidget->layout()->setContentsMargins(0,0,0,0);
    //声道平衡添加布局
    QHBoxLayout *m_pSoundLayout = new QHBoxLayout(m_pInputLevelWidget);
    m_pInputLevelLabel->setFixedSize(115,24);
    m_pSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pInputLevelLabel);
    m_pSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pInputLevelProgressBar);
    m_pSoundLayout->addItem(new QSpacerItem(5,20,QSizePolicy::Fixed));
    m_pSoundLayout->setSpacing(0);
    m_pInputLevelWidget->setLayout(m_pSoundLayout);
    m_pInputLevelWidget->layout()->setContentsMargins(0,0,0,0);

    //连接器添加布局
    QHBoxLayout *pConnectLayout = new QHBoxLayout(m_pInputLevelWidget);
    m_pInputPortLabel->setFixedSize(115,24);
    m_pInputPortCombobox->setMinimumSize(50,48);
    m_pInputPortCombobox->setMaximumSize(16777215,48);
    pConnectLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    pConnectLayout->addWidget(m_pInputPortLabel);
    pConnectLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    pConnectLayout->addWidget(m_pInputPortCombobox);
    pConnectLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    pConnectLayout->setSpacing(0);
    m_pInputPortWidget->setLayout(pConnectLayout);
    pConnectLayout->layout()->setContentsMargins(0,0,0,0);

    line_1 = QSharedPointer<QFrame>(new QFrame);
    line_2 = QSharedPointer<QFrame>(new QFrame);
    line_3 = QSharedPointer<QFrame>(new QFrame);
    line_1.get()->setFrameShape(QFrame::Shape::HLine);
    line_2.get()->setFrameShape(QFrame::Shape::HLine);
    line_3.get()->setFrameShape(QFrame::Shape::HLine);
    line_1.get()->setFixedHeight(1);
    line_2.get()->setFixedHeight(1);
    line_3.get()->setFixedHeight(1);

    //进行整体布局
    m_pVlayout = new QVBoxLayout(m_pInputWidget);
    m_pVlayout->addWidget(m_pInputDeviceWidget);
    m_pVlayout->addWidget(line_1.get());
    m_pVlayout->addWidget(m_pVolumeWidget);
    m_pVlayout->addWidget(line_2.get());
    m_pVlayout->addWidget(m_pInputLevelWidget);
    m_pVlayout->addWidget(line_3.get());

    line_3.get()->hide();
    m_pVlayout->setSpacing(0);
    m_pInputWidget->setLayout(m_pVlayout);
    m_pInputWidget->layout()->setContentsMargins(0,0,0,0);
    m_pInputPortWidget->hide();
    QVBoxLayout *m_pVlayout1 = new QVBoxLayout(this);
    m_pVlayout1->addWidget(m_pInputLabel);
    m_pVlayout1->addItem(new QSpacerItem(16,5,QSizePolicy::Fixed));
    m_pVlayout1->addWidget(m_pInputWidget);
    this->setLayout(m_pVlayout1);
    this->layout()->setContentsMargins(0,0,0,0);

    m_pIpVolumeSlider->setStyleSheet( "QSlider::groove:horizontal {"
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

    m_pIpVolumePercentLabel->setVisible(false);

}

void UkmediaInputWidget::inputWidgetAddPort()
{
    m_pInputWidget->setMinimumSize(550,0);
    m_pInputWidget->setMaximumSize(16777215,16777215);
//    m_pVlayout->addSpacing(1);
//    m_pVlayout->addWidget(m_pInputPortWidget);
//    m_pInputPortWidget->show();
}

void UkmediaInputWidget::inputWidgetRemovePort()
{
//    m_pVlayout->addSpacing(1);
    m_pInputWidget->setMinimumSize(550,0);
    m_pInputWidget->setMaximumSize(16777215,16777215);
//    m_pVlayout->removeWidget(m_pInputPortWidget);
//    m_pInputPortWidget->hide();
}

UkmediaInputWidget::~UkmediaInputWidget()
{

}

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
//    QFILEDEVICE_H

    //加载qss样式文件
    QFile QssFile("://combox.qss");
    QssFile.open(QFile::ReadOnly);

    if (QssFile.isOpen()){
        sliderQss = QLatin1String(QssFile.readAll());
        QssFile.close();
    } else {
        qDebug()<<"combox.qss is not found"<<endl;
    }

    m_pOutputWidget = new QFrame(this);
    m_pOutputWidget->setFrameShape(QFrame::Shape::Box);
    m_pOutputDeviceWidget = new QFrame(m_pOutputWidget);
    m_pMasterVolumeWidget = new QFrame(m_pOutputWidget);
    m_pChannelBalanceWidget = new QFrame(m_pOutputWidget);
    m_pOutputPortWidget = new QFrame(m_pOutputWidget);
    m_pProfileWidget = new QFrame(m_pOutputWidget);
    m_pselectWidget = new QFrame(m_pOutputWidget);

//    m_pselectWidget->setFrameShape(QFrame::Shape::Box);
//    m_pProfileWidget->setFrameShape(QFrame::Shape::Box);
//    m_pOutputDeviceWidget->setFrameShape(QFrame::Shape::Box);
//    m_pMasterVolumeWidget->setFrameShape(QFrame::Shape::Box);
//    m_pChannelBalanceWidget->setFrameShape(QFrame::Shape::Box);
//    m_pOutputPortWidget->setFrameShape(QFrame::Shape::Box);

    //设置大小
    m_pOutputWidget->setMinimumSize(550,0);
    m_pOutputWidget->setMaximumSize(16777215,16777215);
    m_pOutputDeviceWidget->setMinimumSize(550,64);
    m_pOutputDeviceWidget->setMaximumSize(16777215,64);
    m_pMasterVolumeWidget->setMinimumSize(550,64);
    m_pMasterVolumeWidget->setMaximumSize(16777215,64);
    m_pChannelBalanceWidget->setMinimumSize(550,64);
    m_pChannelBalanceWidget->setMaximumSize(16777215,64);
    m_pOutputPortWidget->setMinimumSize(550,64);
    m_pOutputPortWidget->setMaximumSize(16777215,64);
    m_pProfileWidget->setMinimumSize(550,64);
    m_pProfileWidget->setMaximumSize(16777215,64);

    m_pOutputLabel = new QLabel(tr("Output"),this);
    m_pOutputLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    m_pOutputLabel->setContentsMargins(16,0,0,0);
    //~ contents_path /audio/Output Device
    m_pOutputDeviceLabel = new QLabel(tr("Output Device"),m_pOutputWidget);
    m_pOutputDeviceCombobox = new QComboBox(m_pOutputDeviceWidget);
    //~ contents_path /audio/Master Volume
    m_pOpVolumeLabel = new QLabel(tr("Master Volume"),m_pMasterVolumeWidget);


    m_pInputIconLabel_off=new QLabel;
    QString filename1(":/new/prefix1/audio_off.svg");
    QImage* img1=new QImage;
    img1->load(filename1);
    m_pInputIconLabel_off->setPixmap(QPixmap::fromImage(*img1));


    m_pInputIconLabel_on=new QLabel;
    QString filename2(":/new/prefix1/audio_on.svg");
    QImage* img2=new QImage;
    img2->load(filename2);
    m_pInputIconLabel_on->setPixmap(QPixmap::fromImage(*img2));


    m_pOutputIconBtn = new UkuiButtonDrawSvg(m_pMasterVolumeWidget);
    m_pOpVolumeSlider = new AudioSlider(m_pMasterVolumeWidget);
    m_pOpVolumePercentLabel = new QLabel(m_pMasterVolumeWidget);
    //~ contents_path /audio/Balance
    m_pOpBalanceLabel = new QLabel(tr("Balance"),m_pChannelBalanceWidget);
    m_pLeftBalanceLabel = new QLabel(tr("Left"),m_pChannelBalanceWidget);
    m_pOpBalanceSlider = new UkmediaVolumeSlider(m_pChannelBalanceWidget);
    m_pRightBalanceLabel = new QLabel(tr("Right"),m_pChannelBalanceWidget);
    m_pOutputPortLabel = new QLabel(tr("Connector"),m_pOutputPortWidget);
    m_pOutputPortCombobox = new QComboBox(m_pOutputPortWidget);
    //~ contents_path /audio/Profile
    m_pProfileLabel = new QLabel(tr("Profile"),m_pProfileWidget);
    m_pProfileCombobox = new QComboBox(m_pProfileWidget);
    m_pProfileCombobox->setMinimumSize(50,48);
    m_pProfileCombobox->setMaximumSize(16777215,48);
    m_pselectWidget->setMinimumSize(550,64);
    m_pselectWidget->setMaximumSize(16777215,64);
    m_pSelectDeviceLabel = new QLabel(tr("Card"),m_pselectWidget);
    m_pSelectCombobox = new QComboBox(m_pselectWidget);
    m_pProfileLabel->setFixedSize(115,24);

    m_pOpBalanceSlider->setStyle(new CustomStyle());
    m_pOpVolumeSlider->setOrientation(Qt::Horizontal);
    m_pOpBalanceSlider->setOrientation(Qt::Horizontal);
    m_pOpVolumeSlider->setRange(0,100);
    m_pOutputIconBtn->setFocusPolicy(Qt::NoFocus);
    //输出设备添加布局
    QHBoxLayout *outputDeviceLayout = new QHBoxLayout(m_pOutputDeviceWidget);
    m_pOutputLabel->setFixedSize(115,18);
    m_pOutputDeviceCombobox->setMinimumSize(50,48);
    m_pOutputDeviceCombobox->setMaximumSize(16777215,48);

    m_pOutputDeviceLabel->setFixedSize(115,24);
    outputDeviceLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    outputDeviceLayout->addWidget(m_pOutputDeviceLabel);
    outputDeviceLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    outputDeviceLayout->addWidget(m_pOutputDeviceCombobox);
    outputDeviceLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    outputDeviceLayout->setSpacing(0);
    m_pOutputDeviceWidget->setLayout(outputDeviceLayout);
    outputDeviceLayout->layout()->setContentsMargins(0,0,0,0);
    //主音量添加布局
    QHBoxLayout *masterLayout = new QHBoxLayout(m_pMasterVolumeWidget);
    m_pOpVolumeLabel->setFixedSize(115,24);
    m_pOutputIconBtn->setFixedSize(24,24);
    m_pOpVolumeSlider->setFixedHeight(20);
    m_pOpVolumePercentLabel->setFixedSize(40,24);
    masterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    masterLayout->addWidget(m_pOpVolumeLabel);
    masterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    masterLayout->addWidget(m_pInputIconLabel_off);
    masterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    masterLayout->addWidget(m_pOpVolumeSlider);
    masterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    //    masterLayout->addWidget(m_pOpVolumePercentLabel);
    //    masterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    masterLayout->addWidget(m_pInputIconLabel_on);
    masterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    masterLayout->setSpacing(0);
    m_pMasterVolumeWidget->setLayout(masterLayout);
    m_pMasterVolumeWidget->layout()->setContentsMargins(0,0,0,0);
    //声道平衡添加布局
    QHBoxLayout *soundLayout = new QHBoxLayout(m_pChannelBalanceWidget);
    m_pOpBalanceLabel->setFixedSize(115,24);
    m_pLeftBalanceLabel->setFixedSize(36,24);
    m_pOpBalanceSlider->setFixedHeight(20);
    m_pRightBalanceLabel->setFixedSize(55,28);
    soundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    soundLayout->addWidget(m_pOpBalanceLabel);
    soundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    soundLayout->addWidget(m_pLeftBalanceLabel);
    soundLayout->addItem(new QSpacerItem(4,20,QSizePolicy::Fixed));
    soundLayout->addWidget(m_pOpBalanceSlider);
    soundLayout->addItem(new QSpacerItem(4,20,QSizePolicy::Fixed));
    soundLayout->addWidget(m_pRightBalanceLabel);
    soundLayout->addItem(new QSpacerItem(5,20,QSizePolicy::Fixed));
    soundLayout->setSpacing(0);
    m_pChannelBalanceWidget->setLayout(soundLayout);
    m_pChannelBalanceWidget->layout()->setContentsMargins(0,0,0,0);
    //输出连接器布局
    QHBoxLayout *outputPortLayout = new QHBoxLayout(m_pOutputPortWidget);
    m_pOutputPortCombobox->setMinimumSize(50,48);
    m_pOutputPortCombobox->setMaximumSize(16777215,48);

    m_pOutputPortLabel->setFixedSize(115,24);
    outputPortLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    outputPortLayout->addWidget(m_pOutputPortLabel);
    outputPortLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    outputPortLayout->addWidget(m_pOutputPortCombobox);
    outputPortLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    outputPortLayout->setSpacing(0);
    m_pOutputPortWidget->setLayout(outputPortLayout);
    outputPortLayout->layout()->setContentsMargins(0,0,0,0);

    //选择的设备布局
    QHBoxLayout *pSelectDeviceLayout = new QHBoxLayout();
    m_pSelectDeviceLabel->setFixedSize(115,24);
    m_pSelectCombobox->setMinimumSize(50,48);
    m_pSelectCombobox->setMaximumSize(16777215,48);
    pSelectDeviceLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    pSelectDeviceLayout->addWidget(m_pSelectDeviceLabel);
    pSelectDeviceLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    pSelectDeviceLayout->addWidget(m_pSelectCombobox);
    pSelectDeviceLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    pSelectDeviceLayout->setSpacing(0);
    m_pselectWidget->setLayout(pSelectDeviceLayout);
    m_pselectWidget->hide();
    pSelectDeviceLayout->layout()->setContentsMargins(0,0,0,0);

    //配置文件布局
    QHBoxLayout *profileLayout = new QHBoxLayout(m_pProfileWidget);

    profileLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    profileLayout->addWidget(m_pProfileLabel);
    profileLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    profileLayout->addWidget(m_pProfileCombobox);
    profileLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    profileLayout->setSpacing(0);
    m_pProfileWidget->setLayout(profileLayout);
    profileLayout->layout()->setContentsMargins(0,0,0,0);

    line_1 = QSharedPointer<QFrame>(new QFrame);
    line_2 = QSharedPointer<QFrame>(new QFrame);
    line_3 = QSharedPointer<QFrame>(new QFrame);
    line_4 = QSharedPointer<QFrame>(new QFrame);
    line_5 = QSharedPointer<QFrame>(new QFrame);
    line_1.get()->setFrameShape(QFrame::Shape::HLine);
    line_2.get()->setFrameShape(QFrame::Shape::HLine);
    line_3.get()->setFrameShape(QFrame::Shape::HLine);
    line_4.get()->setFrameShape(QFrame::Shape::HLine);
    line_5.get()->setFrameShape(QFrame::Shape::HLine);
    line_1.get()->setFixedHeight(1);
    line_2.get()->setFixedHeight(1);
    line_3.get()->setFixedHeight(1);
    line_4.get()->setFixedHeight(1);
    line_5.get()->setFixedHeight(1);

    //进行整体布局
    m_pVlayout = new QVBoxLayout(m_pOutputWidget);
    m_pVlayout->addWidget(m_pOutputDeviceWidget);
    m_pVlayout->addWidget(line_1.get());
    m_pVlayout->addWidget(m_pMasterVolumeWidget);
    m_pVlayout->addWidget(line_2.get());
    m_pVlayout->addWidget(m_pselectWidget);
    m_pVlayout->addWidget(line_3.get());
    m_pVlayout->addWidget(m_pProfileWidget);
    m_pVlayout->addWidget(line_4.get());
    m_pVlayout->addWidget(m_pChannelBalanceWidget);
    m_pVlayout->setSpacing(0);

    m_pOutputWidget->setLayout(m_pVlayout);
    m_pOutputWidget->layout()->setContentsMargins(0,0,0,0);

    m_pOutputPortWidget->hide();
    QVBoxLayout *vLayout1 = new QVBoxLayout(this);
    vLayout1->addWidget(m_pOutputLabel);
    vLayout1->addItem(new QSpacerItem(16,7,QSizePolicy::Fixed));
    vLayout1->addWidget(m_pOutputWidget);
    this->setLayout(vLayout1);
    this->layout()->setContentsMargins(0,0,0,0);

    m_pOutputDeviceWidget->setObjectName("outputDeviceWidget");
    m_pMasterVolumeWidget->setObjectName("masterVolumeWidget");
    //设置样式
    m_pOutputLabel->setObjectName("m_pOutputLabel");

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
                                       "background: #2FB3E8;"
                                       "border-radius: 5px;"
                                       "}");

    m_pOpVolumePercentLabel->setVisible(false);


}

void UkmediaOutputWidget::outputWidgetAddPort()
{
    m_pOutputWidget->setMinimumSize(550,0);
    m_pOutputWidget->setMaximumSize(16777215,16777215);
    m_pVlayout->insertWidget(3,m_pOutputPortWidget);
    m_pVlayout->insertWidget(3, line_5.get());
    m_pOutputPortWidget->show();
}

void UkmediaOutputWidget::outputWidgetRemovePort()
{
    //    m_pVlayout->addSpacing(1);
    m_pOutputWidget->setMinimumSize(550,0);
    m_pOutputWidget->setMaximumSize(16777215,16777215);
    m_pVlayout->removeWidget(m_pOutputPortWidget);
    m_pOutputPortWidget->hide();
}

UkmediaOutputWidget::~UkmediaOutputWidget()
{

}

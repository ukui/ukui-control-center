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
#include "ukmedia_sound_effects_widget.h"
#include <QHBoxLayout>
#include <QFile>
#include <QVBoxLayout>
#include <QDebug>

UkuiMessageBox::UkuiMessageBox()
{
}

UkmediaSoundEffectsWidget::UkmediaSoundEffectsWidget(QWidget *parent) : QWidget(parent)
{
    m_pSysVolumeWidget = new QFrame(this);
    m_pSysVolumeWidget->setFrameShape(QFrame::Shape::Box);
    m_pThemeWidget = new QFrame(this);
    m_pAlertSoundWidget = new QFrame(this);
    m_pLagoutWidget = new QFrame(this);
    m_pAlertSoundSwitchWidget = new QFrame(this);
    m_pAlertSoundVolumeWidget = new QFrame(this);
    m_pWindowClosedWidget = new QFrame(this);
    m_pVolumeChangeWidget = new QFrame(this);
    m_pSettingSoundWidget = new QFrame(this);

//    m_pThemeWidget->setFrameShape(QFrame::Shape::Box);
//    m_pShutDownWidget->setFrameShape(QFrame::Shape::Box);
//    m_pLagoutWidget->setFrameShape(QFrame::Shape::Box);
//    m_pAlertSoundSwitchWidget->setFrameShape(QFrame::Shape::Box);
//    m_pAlertSoundVolumeWidget->setFrameShape(QFrame::Shape::Box);
//    m_pWindowClosedWidget->setFrameShape(QFrame::Shape::Box);
//    m_pVolumeChangeWidget->setFrameShape(QFrame::Shape::Box);
//    m_pSettingSoundWidget->setFrameShape(QFrame::Shape::Box);

    //~ contents_path /audio/System Sound
    m_pSoundEffectLabel = new QLabel(tr("System Sound"),this);
    m_pSoundEffectLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
//    m_pSoundThemeLabel = new QLabel(tr("Sound Theme"),m_pThemeWidget);
    m_pSoundThemeCombobox = new QComboBox(m_pThemeWidget);
    //~ contents_path /audio/Alert Sound
    m_pShutdownlabel = new QLabel(tr("Alert Sound"),m_pAlertSoundWidget);
    m_pAlertSoundCombobox = new QComboBox(m_pAlertSoundWidget);
    //~ contents_path /audio/Alert Volume
    m_pAlertSoundLabel = new QLabel(tr("Alert Volume"),m_pAlertSoundVolumeWidget);
    m_pAlertVolumeLabel = new QLabel(m_pAlertSoundVolumeWidget);
    m_pAlertSlider = new UkmediaVolumeSlider(m_pAlertSoundVolumeWidget);
    m_pAlertIconLabel_off=new QLabel;
    QString filename1(":/new/prefix1/audio_off.svg");
    QImage* img1=new QImage;
    img1->load(filename1);
    m_pAlertIconLabel_off->setPixmap(QPixmap::fromImage(*img1));

    m_pAlertIconLabel_on=new QLabel;
    QString filename2(":/new/prefix1/audio_on.svg");
    QImage* img2=new QImage;
    img2->load(filename2);
    m_pAlertIconLabel_on->setPixmap(QPixmap::fromImage(*img2));
    m_pAlertSlider->setRange(0,100);
    m_pAlertSlider->setOrientation(Qt::Horizontal);

    //~ contents_path /audio/Boot Music
    m_pLagoutLabel = new QLabel(tr("Boot Music"),m_pLagoutWidget);
    m_pLagoutCombobox = new QComboBox(m_pLagoutWidget);
    m_pBootButton = new SwitchButton(m_pLagoutWidget);
    m_pWindowClosedCombobox = new QComboBox(m_pWindowClosedWidget);
    m_pVolumeChangeCombobox = new QComboBox(m_pVolumeChangeWidget);
    m_pSettingSoundCombobox = new QComboBox(m_pSettingSoundWidget);

    //~ contents_path /audio/Beep Switch
    m_pAlertSoundSwitchLabel = new QLabel(tr("Beep Switch"),m_pAlertSoundSwitchWidget);
    m_pAlertSoundSwitchButton = new SwitchButton(m_pAlertSoundSwitchWidget);

    m_pWindowClosedLabel = new QLabel(tr("Window Closed"),m_pWindowClosedWidget);
    //~ contents_path /audio/Notice Sound
    m_pVolumeChangeLabel = new QLabel(tr("Notice Sound"),m_pVolumeChangeWidget);
    m_pSettingSoundLabel = new QLabel(tr("Setting Menu"),m_pSettingSoundWidget);
    //设置大小
    m_pThemeWidget->setMinimumSize(550,64);
    m_pThemeWidget->setMaximumSize(16777215,64);
    m_pAlertSoundWidget->setMinimumSize(550,64);
    m_pAlertSoundWidget->setMaximumSize(16777215,64);
    m_pLagoutWidget->setMinimumSize(550,64);
    m_pLagoutWidget->setMaximumSize(16777215,64);
    m_pAlertSoundSwitchWidget->setMinimumSize(550,64);
    m_pAlertSoundSwitchWidget->setMaximumSize(16777215,64);
    m_pWindowClosedWidget->setMinimumSize(550,64);
    m_pWindowClosedWidget->setMaximumSize(16777215,64);
    m_pAlertSoundVolumeWidget->setMinimumSize(550,64);
    m_pAlertSoundVolumeWidget->setMaximumSize(16777215,64);
    m_pVolumeChangeWidget->setMinimumSize(550,64);
    m_pVolumeChangeWidget->setMaximumSize(16777215,64);
    m_pSettingSoundWidget->setMinimumSize(550,64);
    m_pSettingSoundWidget->setMaximumSize(16777215,64);

    m_pSoundEffectLabel->setFixedSize(120,24);
    m_pSoundEffectLabel->setContentsMargins(16,0,0,0);
//    m_pSoundThemeLabel->setFixedSize(115,24);
    m_pShutdownlabel->setFixedSize(115,24);
    m_pLagoutLabel->setFixedSize(115,24);
    m_pWindowClosedLabel->setFixedSize(115,24);
    m_pVolumeChangeLabel->setFixedSize(115,24);
    m_pSettingSoundLabel->setFixedSize(115,24);
    m_pAlertSoundLabel->setFixedSize(115,24);
//    m_pAlertVolumeLabel->setFixedSize(115,24);

    m_pSoundThemeCombobox->setMinimumSize(50,48);
    m_pSoundThemeCombobox->setMaximumSize(16777215,48);

    m_pAlertSoundCombobox->setMinimumSize(50,48);
    m_pAlertSoundCombobox->setMaximumSize(16777215,48);
    m_pLagoutCombobox->setMinimumSize(50,48);
    m_pLagoutCombobox->setMaximumSize(16777215,48);
    m_pWindowClosedCombobox->setMinimumSize(50,48);
    m_pWindowClosedCombobox->setMaximumSize(16777215,48);
    m_pVolumeChangeCombobox->setMinimumSize(50,48);
    m_pVolumeChangeCombobox->setMaximumSize(16777215,48);
    m_pSettingSoundCombobox->setMinimumSize(50,48);
    m_pSettingSoundCombobox->setMaximumSize(16777215,48);
    //添加布局
    QHBoxLayout *themeLayout = new QHBoxLayout(m_pThemeWidget);
    themeLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
//    themeLayout->addWidget(m_pSoundThemeLabel);
//    themeLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    themeLayout->addWidget(m_pSoundThemeCombobox);
    themeLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    themeLayout->setSpacing(0);
    m_pThemeWidget->setLayout(themeLayout);
    m_pThemeWidget->layout()->setContentsMargins(0,0,0,0);

    QHBoxLayout *shutdownLayout = new QHBoxLayout(m_pAlertSoundWidget);
    shutdownLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    shutdownLayout->addWidget(m_pShutdownlabel);
    shutdownLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    shutdownLayout->addWidget(m_pAlertSoundCombobox);
    shutdownLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    shutdownLayout->setSpacing(0);
    m_pAlertSoundWidget->setLayout(shutdownLayout);
    m_pAlertSoundWidget->layout()->setContentsMargins(0,0,0,0);

    //开关机音乐设置开关
    QHBoxLayout *lagoutLayout = new QHBoxLayout(m_pLagoutWidget);
    lagoutLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    lagoutLayout->addWidget(m_pLagoutLabel);
    lagoutLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
//    lagoutLayout->addWidget(m_pLagoutCombobox);
    lagoutLayout->addWidget(m_pBootButton);
    lagoutLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    lagoutLayout->setSpacing(0);
    m_pLagoutWidget->setLayout(lagoutLayout);
    m_pLagoutWidget->layout()->setContentsMargins(0,0,0,0);
//    m_pLagoutWidget->setVisible(false);
    m_pLagoutCombobox->setVisible(false);

    //提示音开关布局
    QHBoxLayout *alertSoundLayout = new QHBoxLayout(m_pAlertSoundSwitchWidget);
    alertSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertSoundLayout->addWidget(m_pAlertSoundSwitchLabel);
    alertSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
//    lagoutLayout->addWidget(m_pLagoutCombobox);
    alertSoundLayout->addWidget(m_pAlertSoundSwitchButton);
    alertSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertSoundLayout->setSpacing(0);
    m_pAlertSoundSwitchWidget->setLayout(alertSoundLayout);
    m_pAlertSoundSwitchWidget->layout()->setContentsMargins(0,0,0,0);
    //提示音大小
    QHBoxLayout *alertLayout = new QHBoxLayout(m_pAlertSoundVolumeWidget);
    m_pAlertSoundLabel->setFixedSize(115,24);
    m_pAlertSlider->setFixedHeight(20);
//    m_pAlertVolumeLabel->setFixedSize(40,24);
    alertLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertLayout->addWidget(m_pAlertSoundLabel);
    alertLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertLayout->addWidget(m_pAlertIconLabel_off);
    alertLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertLayout->addWidget(m_pAlertSlider);
    alertLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertLayout->addWidget(m_pAlertIconLabel_on);
    alertLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertLayout->setSpacing(0);
    m_pAlertSoundVolumeWidget->setLayout(alertLayout);
    m_pAlertSoundVolumeWidget->layout()->setContentsMargins(0,0,0,0);

    //窗口关闭提示音
    QHBoxLayout *windowClosedLayout = new QHBoxLayout(m_pWindowClosedWidget);
    windowClosedLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    windowClosedLayout->addWidget(m_pWindowClosedLabel);
    windowClosedLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    windowClosedLayout->addWidget(m_pWindowClosedCombobox);
    windowClosedLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    windowClosedLayout->setSpacing(0);
    m_pWindowClosedWidget->setLayout(windowClosedLayout);
    m_pWindowClosedWidget->layout()->setContentsMargins(0,0,0,0);

    //音量改变提示音
    QHBoxLayout *volumeChangedLayout = new QHBoxLayout(m_pVolumeChangeWidget);
    volumeChangedLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    volumeChangedLayout->addWidget(m_pVolumeChangeLabel);
    volumeChangedLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    volumeChangedLayout->addWidget(m_pVolumeChangeCombobox);
    volumeChangedLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    volumeChangedLayout->setSpacing(0);
    m_pVolumeChangeWidget->setLayout(volumeChangedLayout);
    m_pVolumeChangeWidget->layout()->setContentsMargins(0,0,0,0);
    //设置菜单提示音
    QHBoxLayout *settingMenuLayout = new QHBoxLayout(m_pSettingSoundWidget);
    settingMenuLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    settingMenuLayout->addWidget(m_pSettingSoundLabel);
    settingMenuLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    settingMenuLayout->addWidget(m_pSettingSoundCombobox);
    settingMenuLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    settingMenuLayout->setSpacing(0);
    m_pSettingSoundWidget->setLayout(settingMenuLayout);
    m_pSettingSoundWidget->layout()->setContentsMargins(0,0,0,0);

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

    m_pSoundLayout = new QVBoxLayout(m_pSysVolumeWidget);
//    m_pSoundLayout->addItem(new QSpacerItem(16,7,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pLagoutWidget);
    m_pSoundLayout->addWidget(line_1.get());
//    m_pSoundLayout->addItem(new QSpacerItem(16,6,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pAlertSoundSwitchWidget);
    m_pSoundLayout->addWidget(line_2.get());
//    m_pSoundLayout->addWidget(m_pAlertSoundVolumeWidget);
//    m_pSoundLayout->addItem(new QSpacerItem(16,7,QSizePolicy::Fixed));
//    m_pSoundLayout->addWidget(m_pThemeWidget);
//    m_pSoundLayout->addWidget(line_3.get());
    m_pSoundLayout->addWidget(m_pAlertSoundWidget);
    m_pSoundLayout->addWidget(line_4.get());
//    vLayout->addWidget(m_pWindowClosedWidget);
    m_pSoundLayout->addWidget(m_pVolumeChangeWidget);
//    vLayout->addWidget(m_pSettingSoundWidget);
    m_pSysVolumeWidget->setLayout(m_pSoundLayout);
    m_pSoundLayout->setSpacing(0);
    m_pSoundLayout->setContentsMargins(0, 0, 0, 0);
    m_pThemeWidget->hide();

    //进行整体布局
    m_pSysLayout = new QVBoxLayout(this);
    m_pSysLayout->addWidget(m_pSoundEffectLabel);
    m_pSysLayout->addWidget(m_pSysVolumeWidget);
    this->setLayout(m_pSysLayout);
    m_pSysLayout->setSpacing(8);
    this->layout()->setContentsMargins(0,0,0,0);
    m_pSettingSoundWidget->hide();
    m_pWindowClosedWidget->hide();

    m_pAlertSlider->setStyleSheet( "QSlider::groove:horizontal {"
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
}

UkmediaSoundEffectsWidget::~UkmediaSoundEffectsWidget()
{

}

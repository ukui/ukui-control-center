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
    m_pThemeWidget = new QFrame(this);
    m_pAlertSoundWidget = new QFrame(this);
    m_pAlertSoundSwitchWidget = new QFrame(this);
    m_pStartupMusicWidget = new QFrame(this);
    m_pPoweroffMusicWidget = new QFrame(this);
    m_pLagoutWidget = new QFrame(this);
    m_pWakeupMusicWidget = new QFrame(this);
    m_pVolumeChangeWidget = new QFrame(this);
    m_pAlertSoundChangedWidget = new QFrame(this);

    m_pThemeWidget->setFrameShape(QFrame::Shape::Box);
    m_pAlertSoundWidget->setFrameShape(QFrame::Shape::Box);
    m_pStartupMusicWidget->setFrameShape(QFrame::Shape::Box);
    m_pLagoutWidget->setFrameShape(QFrame::Shape::Box);
    m_pAlertSoundSwitchWidget->setFrameShape(QFrame::Shape::Box);
    m_pWakeupMusicWidget->setFrameShape(QFrame::Shape::Box);
    m_pVolumeChangeWidget->setFrameShape(QFrame::Shape::Box);
    m_pPoweroffMusicWidget->setFrameShape(QFrame::Shape::Box);
    m_pAlertSoundChangedWidget->setFrameShape(QFrame::Shape::Box);

    //~ contents_path /audio/System Sound   系统音效文本框
    m_pSoundEffectLabel = new TitleLabel(this);
    m_pSoundEffectLabel->setText(tr("System Sound"));
    m_pSoundEffectLabel->setStyleSheet("QLabel{color: palette(windowText);}");
    //audio/Sound Theme
    m_pSoundThemeLabel = new QLabel(tr("Sound Theme"),m_pThemeWidget);//提示音下的系统音效主题
    m_pSoundThemeCombobox = new QComboBox(m_pThemeWidget);
    ////~ contents_path /audio/Alert Sound
    m_pShutdownlabel = new QLabel(tr("Alert Sound"),m_pAlertSoundWidget);//提示音下的通知提示
    m_pAlertSoundCombobox = new QComboBox(m_pAlertSoundWidget);
    //~ contents_path /audio/Beep Switch
    m_pAlertSoundSwitchLabel = new QLabel(tr("Beep Switch"),m_pAlertSoundSwitchWidget);//提示音
    m_pPoweroffMusicLabel = new QLabel(tr("Poweroff Music"),m_pPoweroffMusicWidget);//关机
    m_pStartupMusicLabel = new QLabel(tr("Startup Music"),m_pStartupMusicWidget);//开机
    m_pWakeupMusicLabel = new QLabel(tr("Wakeup Music"),m_pWakeupMusicWidget);//唤醒
    m_pVolumeChangeLabel = new QLabel(tr("Volume Change"),m_pVolumeChangeWidget);//音量调节
    m_pLagoutLabel = new QLabel(tr("Logout Music"),m_pLagoutWidget);//注销
    //audio/Alert Volume
    m_pAlertSoundChangedLabel = new QLabel(tr("Alert Volume"),m_pAlertSoundChangedWidget);
    m_pAlertSoundMuteLabel = new UkuiButtonDrawSvg(m_pAlertSoundChangedWidget);
    m_pAlertSoundVolumeLabel = new UkuiButtonDrawSvg(m_pAlertSoundChangedWidget);
    m_pAlertSoundMuteLabel->setCheckable(false);
    m_pAlertSoundVolumeLabel->setCheckable(false);
    m_pAlertSoundChangedSlider = new UkmediaVolumeSlider(m_pAlertSoundChangedWidget,true);

    m_pAlertSoundChangedSlider->setOrientation(Qt::Horizontal);
    m_pAlertSoundChangedSlider->setRange(0,100);

    m_pAlertSoundChangedSlider->setStyleSheet( "QSlider::groove:horizontal {"
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

                                       "QSlider::add-page:horizontal {"
                                       "background: palette(button);"
                                       "border-radius: 20px;"
                                       "}"

                                       "QSlider::sub-page:horizontal {"
                                       "background: #2FB3E8;"
                                       "border-radius: 5px;"
                                       "}");


    m_pLagoutCombobox = new QComboBox(m_pLagoutWidget);
    m_pStartupButton = new SwitchButton(m_pStartupMusicWidget);
    m_pLogoutButton = new SwitchButton(m_pLagoutWidget);
    m_pWakeupMusicButton = new SwitchButton(m_pWakeupMusicWidget);
    m_pPoweroffButton = new SwitchButton(m_pPoweroffMusicWidget);
    m_pVolumeChangeCombobox = new QComboBox(m_pVolumeChangeWidget);
    m_pAlertSoundSwitchButton = new SwitchButton(m_pAlertSoundSwitchWidget);


    //设置大小

    m_pThemeWidget->setMinimumSize(550,60);
    m_pThemeWidget->setMaximumSize(16777215,60);
    m_pAlertSoundWidget->setMinimumSize(550,60);
    m_pAlertSoundWidget->setMaximumSize(16777215,60);
    m_pStartupMusicWidget->setMinimumSize(550,60);
    m_pStartupMusicWidget->setMaximumSize(16777215,60);
    m_pLagoutWidget->setMinimumSize(550,60);
    m_pLagoutWidget->setMaximumSize(16777215,60);
    m_pAlertSoundSwitchWidget->setMinimumSize(550,60);
    m_pAlertSoundSwitchWidget->setMaximumSize(16777215,60);
    m_pWakeupMusicWidget->setMinimumSize(550,60);
    m_pWakeupMusicWidget->setMaximumSize(16777215,60);
    m_pVolumeChangeWidget->setMinimumSize(550,60);
    m_pVolumeChangeWidget->setMaximumSize(16777215,60);
    m_pPoweroffMusicWidget->setMinimumSize(550,60);
    m_pPoweroffMusicWidget->setMaximumSize(16777215,60);
    m_pAlertSoundChangedWidget->setMinimumSize(550,60);
    m_pAlertSoundChangedWidget->setMaximumSize(16777215,60);

    m_pSoundEffectLabel->setFixedSize(150,32);
    m_pSoundThemeLabel->setFixedSize(140,32);
    m_pShutdownlabel->setFixedSize(140,32);
//    m_pLagoutLabel->setFixedSize(150,32);
//    m_pWakeupMusicLabel->setFixedSize(150,32);
    m_pVolumeChangeLabel->setFixedSize(140,32);
    m_pPoweroffMusicLabel->setFixedSize(150,32);
//    m_pDnsNoiseReductionLabel->setFixedSize(150,32);

    m_pSoundThemeCombobox->setFixedHeight(40);
    m_pAlertSoundCombobox->setFixedHeight(40);
    m_pVolumeChangeCombobox->setFixedHeight(40);

    m_pLagoutCombobox->setMinimumSize(50,32);
//    m_pLagoutCombobox->setMaximumSize(900,32);



    //添加布局
    QHBoxLayout *themeLayout = new QHBoxLayout(m_pThemeWidget);
    themeLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    themeLayout->addWidget(m_pSoundThemeLabel);
    themeLayout->addItem(new QSpacerItem(100,20,QSizePolicy::Fixed));
    themeLayout->addWidget(m_pSoundThemeCombobox);
    themeLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    themeLayout->setSpacing(0);
    m_pThemeWidget->setLayout(themeLayout);
    m_pThemeWidget->layout()->setContentsMargins(0,0,0,0);

    QHBoxLayout *AlertLayout = new QHBoxLayout(m_pAlertSoundWidget);
    AlertLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    AlertLayout->addWidget(m_pShutdownlabel);
    AlertLayout->addItem(new QSpacerItem(100,20,QSizePolicy::Fixed));
    AlertLayout->addWidget(m_pAlertSoundCombobox);
    AlertLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    AlertLayout->setSpacing(0);
    m_pAlertSoundWidget->setLayout(AlertLayout);
    m_pAlertSoundWidget->layout()->setContentsMargins(0,0,0,0);

    //开机音乐设置开关
    QHBoxLayout *startupLayout = new QHBoxLayout(m_pStartupMusicWidget);
    startupLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    startupLayout->addWidget(m_pStartupMusicLabel);
    startupLayout->addWidget(m_pStartupButton);
    startupLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    startupLayout->setSpacing(0);
    m_pStartupMusicWidget->setLayout(startupLayout);
    m_pStartupMusicWidget->layout()->setContentsMargins(0,0,0,0);

    //注销提示音布局
    QHBoxLayout *lagoutLayout = new QHBoxLayout(m_pLagoutWidget);
    lagoutLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    lagoutLayout->addWidget(m_pLagoutLabel);
//    lagoutLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    lagoutLayout->addWidget(m_pLogoutButton);
    lagoutLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    lagoutLayout->setSpacing(0);
    m_pLagoutWidget->setLayout(lagoutLayout);
    m_pLagoutWidget->layout()->setContentsMargins(0,0,0,0);
    m_pLagoutCombobox->setVisible(false);

    //提示音开关布局
    QHBoxLayout *alertSoundLayout = new QHBoxLayout(m_pAlertSoundSwitchWidget);
    alertSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertSoundLayout->addWidget(m_pAlertSoundSwitchLabel);
    alertSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    alertSoundLayout->addWidget(m_pAlertSoundSwitchButton);
    alertSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertSoundLayout->setSpacing(0);
    m_pAlertSoundSwitchWidget->setLayout(alertSoundLayout);
    m_pAlertSoundSwitchWidget->layout()->setContentsMargins(0,0,0,0);

    //喚醒提示音
    QHBoxLayout *wakeupMusicLayout = new QHBoxLayout(m_pWakeupMusicWidget);
    wakeupMusicLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    wakeupMusicLayout->addWidget(m_pWakeupMusicLabel);
//    wakeupMusicLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    wakeupMusicLayout->addWidget(m_pWakeupMusicButton);
    wakeupMusicLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    wakeupMusicLayout->setSpacing(0);
    m_pWakeupMusicWidget->setLayout(wakeupMusicLayout);
    m_pWakeupMusicWidget->layout()->setContentsMargins(0,0,0,0);


    //音量改变提示音
    QHBoxLayout *volumeChangedLayout = new QHBoxLayout(m_pVolumeChangeWidget);
    volumeChangedLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    volumeChangedLayout->addWidget(m_pVolumeChangeLabel);
    volumeChangedLayout->addItem(new QSpacerItem(100,20,QSizePolicy::Fixed));
    volumeChangedLayout->addWidget(m_pVolumeChangeCombobox);
    volumeChangedLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    volumeChangedLayout->setSpacing(0);
    m_pVolumeChangeWidget->setLayout(volumeChangedLayout);
    m_pVolumeChangeWidget->layout()->setContentsMargins(0,0,0,0);
    //关机提示音
    QHBoxLayout *poweroffLayout = new QHBoxLayout(m_pPoweroffMusicWidget);
    poweroffLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    poweroffLayout->addWidget(m_pPoweroffMusicLabel);
    poweroffLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    poweroffLayout->addWidget(m_pPoweroffButton);
    poweroffLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    poweroffLayout->setSpacing(0);
    m_pPoweroffMusicWidget->setLayout(poweroffLayout);
    m_pPoweroffMusicWidget->layout()->setContentsMargins(0,0,0,0);
    //提示音量滑动条布局
    QHBoxLayout *alertSoundVolumeLayout = new QHBoxLayout(m_pAlertSoundChangedWidget);
    m_pAlertSoundChangedLabel->setFixedSize(140,20);
    m_pAlertSoundMuteLabel->setFixedSize(24,24);
    m_pAlertSoundChangedSlider->setFixedHeight(60);
    m_pAlertSoundVolumeLabel->setFixedSize(24,24);
    alertSoundVolumeLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertSoundVolumeLayout->addWidget(m_pAlertSoundChangedLabel);
    alertSoundVolumeLayout->addItem(new QSpacerItem(100,20,QSizePolicy::Fixed));
    alertSoundVolumeLayout->addWidget(m_pAlertSoundMuteLabel);
    alertSoundVolumeLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertSoundVolumeLayout->addWidget(m_pAlertSoundChangedSlider);
    alertSoundVolumeLayout->addItem(new QSpacerItem(33,20,QSizePolicy::Fixed));
    alertSoundVolumeLayout->addWidget(m_pAlertSoundVolumeLabel);
    alertSoundVolumeLayout->addItem(new QSpacerItem(30,20,QSizePolicy::Fixed));
    alertSoundVolumeLayout->setSpacing(0);
    m_pAlertSoundChangedWidget->setLayout(alertSoundVolumeLayout);
    m_pAlertSoundChangedWidget->layout()->setContentsMargins(0,0,0,0);




    //进行整体布局
    m_pSoundLayout = new QVBoxLayout(this);
    m_pSoundLayout->addWidget(m_pSoundEffectLabel);
    m_pSoundLayout->addItem(new QSpacerItem(16,4,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pStartupMusicWidget);
    m_pSoundLayout->addItem(new QSpacerItem(16,8,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pAlertSoundSwitchWidget);
    m_pSoundLayout->addItem(new QSpacerItem(16,1,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pAlertSoundChangedWidget);
    m_pSoundLayout->addWidget(m_pPoweroffMusicWidget);
    m_pSoundLayout->addWidget(m_pLagoutWidget);
    m_pSoundLayout->addWidget(m_pWakeupMusicWidget);
    m_pSoundLayout->addWidget(m_pThemeWidget);
    m_pSoundLayout->addWidget(m_pVolumeChangeWidget);
    m_pSoundLayout->addWidget(m_pAlertSoundWidget);

    m_pSoundLayout->setSpacing(0);
    this->setLayout(m_pSoundLayout);
    this->layout()->setContentsMargins(0,0,0,0);

    m_pPoweroffMusicWidget->hide();
    m_pWakeupMusicWidget->hide();
    m_pLagoutWidget->hide();
    m_pAlertSoundWidget->hide();
    m_pVolumeChangeWidget->hide();
    m_pThemeWidget->hide();
    //    m_pStartupMusicWidget->hide();
    //    m_pAlertSoundChangedWidget->hide();



}

UkmediaSoundEffectsWidget::~UkmediaSoundEffectsWidget()
{

}

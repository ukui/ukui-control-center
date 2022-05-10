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
#include "ukmedia_main_widget.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QHeaderView>
#include <QStringList>
#include <QSpacerItem>
#include <QListView>
#include <QScrollBar>
#include <QGSettings>
#include <QPixmap>
#include <qmath.h>
#define MATE_DESKTOP_USE_UNSTABLE_API
#define VERSION "1.12.1"
#define GVC_DIALOG_DBUS_NAME "org.mate.VolumeControl"
#define KEY_SOUNDS_SCHEMA   "org.ukui.sound"
#define GVC_SOUND_SOUND    (xmlChar *) "sound"
#define GVC_SOUND_NAME     (xmlChar *) "name"
#define GVC_SOUND_FILENAME (xmlChar *) "filename"
#define SOUND_SET_DIR "/usr/share/ukui-media/sounds"

#define KEYBINDINGS_CUSTOM_SCHEMA "org.ukui.media.sound"
#define KEYBINDINGS_CUSTOM_DIR "/org/ukui/sound/keybindings/"

#define MAX_CUSTOM_SHORTCUTS 1000

#define FILENAME_KEY "filename"
#define NAME_KEY "name"

guint appnum = 0;
extern bool isCheckBluetoothInput;

enum {
    SOUND_TYPE_UNSET,
    SOUND_TYPE_OFF,
    SOUND_TYPE_DEFAULT_FROM_THEME,
    SOUND_TYPE_BUILTIN,
    SOUND_TYPE_CUSTOM
};

UkmediaMainWidget::UkmediaMainWidget(QWidget *parent)
    : QWidget(parent)
{
    m_pVolumeControl = new UkmediaVolumeControl;
    initWidget();

    m_pSoundList = new QStringList;
    m_pThemeNameList = new QStringList;
    m_pThemeDisplayNameList = new QStringList;
    m_pSoundNameList = new QStringList;
    m_pSoundThemeList = new QStringList;
    m_pSoundThemeDirList = new QStringList;
    m_pSoundThemeXmlNameList = new QStringList;

    initGsettings();
    setupThemeSelector(this);
    updateTheme(this);
    //报警声音,从指定路径获取报警声音文件
    populateModelFromDir(this,SOUND_SET_DIR);
    //初始化combobox的值
    comboboxCurrentTextInit();

    time = new QTimer();
    dealSlot();
}

/*
 * 初始化界面
 */
void UkmediaMainWidget::initWidget()
{
    m_pOutputWidget = new UkmediaOutputWidget();
    m_pInputWidget = new UkmediaInputWidget();
    m_pSoundWidget = new UkmediaSoundEffectsWidget();
    firstEntry = true;
    mThemeName = UKUI_THEME_WHITE;
    QVBoxLayout *m_pvLayout = new QVBoxLayout();
    m_pvLayout->addWidget(m_pOutputWidget);
    m_pvLayout->addWidget(m_pInputWidget);
    m_pvLayout->addWidget(m_pSoundWidget);
    m_pvLayout->addSpacing(32);
    m_pvLayout->addSpacerItem(new QSpacerItem(20,0,QSizePolicy::Fixed,QSizePolicy::Expanding));
    m_pvLayout->setSpacing(40);
    this->setLayout(m_pvLayout);
    this->setMinimumWidth(582);
    this->setMaximumWidth(910);
    this->layout()->setContentsMargins(0,0,31,0);

    //设置滑动条的最大值为100
    m_pInputWidget->m_pIpVolumeSlider->setMaximum(100);
    m_pOutputWidget->m_pOpVolumeSlider->setMaximum(100);
    m_pOutputWidget->m_pOpBalanceSlider->setMaximum(100);
    m_pOutputWidget->m_pOpBalanceSlider->setMinimum(-100);
    m_pOutputWidget->m_pOpBalanceSlider->setSingleStep(100);
    m_pInputWidget->m_pInputLevelProgressBar->setMaximum(101);
}

QList<char *> UkmediaMainWidget::listExistsPath()
{
    char ** childs;
    int len;

    DConfClient * client = dconf_client_new();
    childs = dconf_client_list (client, KEYBINDINGS_CUSTOM_DIR, &len);
    g_object_unref (client);

    QList<char *> vals;

    for (int i = 0; childs[i] != NULL; i++){
        if (dconf_is_rel_dir (childs[i], NULL)){
            char * val = g_strdup (childs[i]);

            vals.append(val);
        }
    }
    g_strfreev (childs);
    return vals;
}

QString UkmediaMainWidget::findFreePath(){
    int i = 0;
    char * dir;
    bool found;
    QList<char *> existsdirs;

    existsdirs = listExistsPath();

    for (; i < MAX_CUSTOM_SHORTCUTS; i++){
        found = true;
        dir = QString("custom%1/").arg(i).toLatin1().data();
        for (int j = 0; j < existsdirs.count(); j++)
            if (!g_strcmp0(dir, existsdirs.at(j))){
                found = false;
                break;
            }
        if (found)
            break;
    }

    if (i == MAX_CUSTOM_SHORTCUTS){
        qDebug() << "Keyboard Shortcuts" << "Too many custom shortcuts";
        return "";
    }

    return QString("%1%2").arg(KEYBINDINGS_CUSTOM_DIR).arg(QString(dir));
}

void UkmediaMainWidget::addValue(QString name,QString filename)
{
    //在创建setting表时，先判断是否存在该设置，存在时不创建
    QList<char *> existsPath = listExistsPath();

    for (char * path : existsPath) {

        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);

        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba))
        {
            QGSettings * settings = new QGSettings(ba, bba);
            QString filenameStr = settings->get(FILENAME_KEY).toString();
            QString nameStr = settings->get(NAME_KEY).toString();

            g_warning("full path: %s", allpath);
            qDebug() << filenameStr << FILENAME_KEY <<NAME_KEY << nameStr;
            if (nameStr == name) {
                return;
            }
            delete settings;
        }
        else {
            continue;
        }

    }
    QString availablepath = findFreePath();

    const QByteArray id(KEYBINDINGS_CUSTOM_SCHEMA);
    const QByteArray idd(availablepath.toUtf8().data());
    if(QGSettings::isSchemaInstalled(id))
    {
        QGSettings * settings = new QGSettings(id, idd);
        settings->set(FILENAME_KEY, filename);
        settings->set(NAME_KEY, name);
    }
}

/*
 * 初始化gsetting
 */
void UkmediaMainWidget::initGsettings()
{
    //获取声音gsettings值
    m_pSoundSettings = g_settings_new (KEY_SOUNDS_SCHEMA);

    g_signal_connect (G_OBJECT (m_pSoundSettings),
                             "changed",
                             G_CALLBACK (onKeyChanged),
                             this);
    //检测系统主题
    if (QGSettings::isSchemaInstalled(UKUI_THEME_SETTING)){
        m_pThemeSetting = new QGSettings(UKUI_THEME_SETTING);
        if (m_pThemeSetting->keys().contains("styleName")) {
            mThemeName = m_pThemeSetting->get(UKUI_THEME_NAME).toString();
        }
        connect(m_pThemeSetting, SIGNAL(changed(const QString &)),this,SLOT(ukuiThemeChangedSlot(const QString &)));
    }

    //检测设计开关机音乐
    if (QGSettings::isSchemaInstalled(UKUI_SWITCH_SETTING)) {
        m_pBootSetting = new QGSettings(UKUI_SWITCH_SETTING);
        if (m_pBootSetting->keys().contains("startupMusic")) {
            bool startup = m_pBootSetting->get(UKUI_STARTUP_MUSIC_KEY).toBool();
            m_pSoundWidget->m_pStartupButton->setChecked(startup);
        }
        if (m_pBootSetting->keys().contains("poweroffMusic")) {
            bool poweroff = m_pBootSetting->get(UKUI_POWEROFF_MUSIC_KEY).toBool();
            m_pSoundWidget->m_pPoweroffButton->setChecked(poweroff);
        }
        if (m_pBootSetting->keys().contains("logoutMusic")) {
            bool logout = m_pBootSetting->get(UKUI_LOGOUT_MUSIC_KEY).toBool();
            m_pSoundWidget->m_pLogoutButton->setChecked(logout);
        }
        if (m_pBootSetting->keys().contains("weakupMusic")) {
            bool m_hasMusic = m_pBootSetting->get(UKUI_WAKEUP_MUSIC_KEY).toBool();
            m_pSoundWidget->m_pWakeupMusicButton->setChecked(m_hasMusic);
        }
        connect(m_pBootSetting,SIGNAL(changed(const QString &)),this,SLOT(bootMusicSettingsChanged()));
    }
    bool status = g_settings_get_boolean(m_pSoundSettings, EVENT_SOUNDS_KEY);
    m_pSoundWidget->m_pAlertSoundSwitchButton->setChecked(status);
    status = g_settings_get_boolean(m_pSoundSettings, DNS_NOISE_REDUCTION);

    m_pInputWidget->m_pNoiseReducteButton->setChecked(status);
}

/*
 * 处理槽函数
 */
void UkmediaMainWidget::dealSlot()
{
    QTimer::singleShot(100, this, SLOT(initVoulmeSlider()));
    connect(m_pInputWidget->m_pInputIconBtn,SIGNAL(clicked()),this,SLOT(inputMuteButtonSlot()));
    connect(m_pOutputWidget->m_pOutputIconBtn,SIGNAL(clicked()),this,SLOT(outputMuteButtonSlot()));
//    connect(m_pSoundWidget->m_pAlertIconBtn,SIGNAL(clicked()),this,SLOT(alertSoundVolumeChangedSlot()));
    connect(m_pSoundWidget->m_pStartupButton,SIGNAL(checkedChanged(bool)),this,SLOT(startupButtonSwitchChangedSlot(bool)));
    connect(m_pSoundWidget->m_pPoweroffButton,SIGNAL(checkedChanged(bool)),this,SLOT(poweroffButtonSwitchChangedSlot(bool)));
    connect(m_pSoundWidget->m_pLogoutButton,SIGNAL(checkedChanged(bool)),this,SLOT(logoutMusicButtonSwitchChangedSlot(bool)));
    connect(m_pSoundWidget->m_pWakeupMusicButton,SIGNAL(checkedChanged(bool)),this,SLOT(wakeButtonSwitchChangedSlot(bool)));
    connect(m_pSoundWidget->m_pAlertSoundSwitchButton,SIGNAL(checkedChanged(bool)),this,SLOT(alertSoundButtonSwitchChangedSlot(bool)));
    //输出音量控制
    //输出滑动条音量控制
    timeSlider = new QTimer(this);
    connect(timeSlider,SIGNAL(timeout()),this,SLOT(timeSliderSlot()));
    //输出滑动条改变
    connect(m_pOutputWidget->m_pOpVolumeSlider,SIGNAL(valueChanged(int)),this,SLOT(outputWidgetSliderChangedSlot(int)));
    //    connect(m_pOutputWidget->m_pOpVolumeSlider,&AudioSlider::silderPressSignal,this,[=](){
    //        mousePress = true;
    //        mouseReleaseState = false;
    //    });
    //    connect(m_pOutputWidget->m_pOpVolumeSlider,&AudioSlider::silderReleaseSignal,this,[=](){
    //        mouseReleaseState = true;
    //    });
    //输入滑动条音量控制
    connect(m_pInputWidget->m_pIpVolumeSlider,SIGNAL(valueChanged(int)),this,SLOT(inputWidgetSliderChangedSlot(int)));
    //输入等级
    connect(m_pVolumeControl,SIGNAL(peakChangedSignal(double)),this,SLOT(peakVolumeChangedSlot(double)));

    connect(m_pVolumeControl,SIGNAL(updatePortSignal()),this,SLOT(updateDevicePort()));
    connect(m_pVolumeControl,SIGNAL(deviceChangedSignal()),this,SLOT(updateListWidgetItemSlot()));
    //切换输出设备或者音量改变时需要同步更新音量
    connect(m_pVolumeControl,&UkmediaVolumeControl::updateVolume,this,[=](int value,bool state){

        QString percent = QString::number(paVolumeToValue(value));
        float balanceVolume = m_pVolumeControl->getBalanceVolume();
        if (!pressOutputListWidget) {

            m_pOutputWidget->m_pOpVolumePercentLabel->setText(percent+"%");
            m_pOutputWidget->m_pOpVolumeSlider->blockSignals(true);
            m_pOutputWidget->m_pOpBalanceSlider->blockSignals(true);
            m_pOutputWidget->m_pOpBalanceSlider->setValue(balanceVolume*100);
            m_pOutputWidget->m_pOpVolumeSlider->setValue(paVolumeToValue(value));
            m_pOutputWidget->m_pOpVolumeSlider->blockSignals(false);
            m_pOutputWidget->m_pOpBalanceSlider->blockSignals(false);
        }
        //fixed bug:110751 宝新创PF215T
        initOutputListWidgetItem();
        themeChangeIcons();
    });
    connect(m_pVolumeControl,&UkmediaVolumeControl::updateSourceVolume,this,[=](int value,bool state){
        QString percent = QString::number(paVolumeToValue(value));

        m_pInputWidget->m_pIpVolumePercentLabel->setText(percent+"%");

        m_pInputWidget->m_pIpVolumeSlider->blockSignals(true);
        m_pInputWidget->m_pIpVolumeSlider->setValue(paVolumeToValue(value));
        m_pInputWidget->m_pIpVolumeSlider->blockSignals(false);

        //当所有可用的输入设备全部移除，台式机才会出现该情况
        if(strstr(m_pVolumeControl->defaultSourceName,"monitor"))
            m_pInputWidget->m_pInputLevelProgressBar->setValue(0);

        initInputListWidgetItem();
        themeChangeIcons();
    });

    connect(m_pOutputWidget->m_pOpBalanceSlider,SIGNAL(valueChanged(int)),this,SLOT(balanceSliderChangedSlot(int)));
    //点击报警音量时播放报警声音
//    connect(m_pSoundWidget->m_pAlertSlider,SIGNAL(valueChanged(int)),this,SLOT(alertVolumeSliderChangedSlot(int)));
    connect(m_pSoundWidget->m_pAlertSoundCombobox,SIGNAL(currentIndexChanged(int)),this,SLOT(comboxIndexChangedSlot(int)));
    connect(m_pSoundWidget->m_pLagoutCombobox ,SIGNAL(currentIndexChanged(int)),this,SLOT(comboxIndexChangedSlot(int)));
    connect(m_pSoundWidget->m_pSoundThemeCombobox,SIGNAL(currentIndexChanged(int)),this,SLOT(themeComboxIndexChangedSlot(int)));
    connect(m_pSoundWidget->m_pVolumeChangeCombobox,SIGNAL(currentIndexChanged (int)),this,SLOT(volumeChangedComboboxChangeSlot(int)));

    connect(m_pOutputWidget->m_pOutputListWidget,SIGNAL(currentRowChanged(int )),this,SLOT(outputListWidgetCurrentRowChangedSlot(int)));
    connect(m_pInputWidget->m_pInputListWidget,SIGNAL(currentRowChanged(int )),this,SLOT(inputListWidgetCurrentRowChangedSlot(int)));
    connect(m_pInputWidget->m_pNoiseReducteButton,SIGNAL(checkedChanged(bool)),this,SLOT(noiseReductionButtonSwitchChangedSlot(bool)));

}

/*
 * 初始化滑动条的值
 */
void UkmediaMainWidget::initVoulmeSlider()
{
    int sinkVolume = paVolumeToValue(m_pVolumeControl->getSinkVolume());
    int sourceVolume = paVolumeToValue(m_pVolumeControl->getSourceVolume());
    QString percent = QString::number(sinkVolume);
    float balanceVolume = m_pVolumeControl->getBalanceVolume();

    m_pOutputWidget->m_pOpVolumePercentLabel->setText(percent+"%");
    percent = QString::number(sourceVolume);
    m_pInputWidget->m_pIpVolumePercentLabel->setText(percent+"%");
    m_pOutputWidget->m_pOpVolumeSlider->blockSignals(true);
    m_pOutputWidget->m_pOpBalanceSlider->blockSignals(true);
    m_pInputWidget->m_pIpVolumeSlider->blockSignals(true);
    m_pOutputWidget->m_pOpBalanceSlider->setValue(balanceVolume*100);
    m_pOutputWidget->m_pOpVolumeSlider->setValue(sinkVolume);
    m_pInputWidget->m_pIpVolumeSlider->setValue(sourceVolume);
    m_pOutputWidget->m_pOpVolumeSlider->blockSignals(false);
    m_pOutputWidget->m_pOpBalanceSlider->blockSignals(false);
    m_pInputWidget->m_pIpVolumeSlider->blockSignals(false);
    initOutputListWidgetItem();
    initInputListWidgetItem();
    themeChangeIcons();
    this->update();
}

/*
 * 初始化output/input list widget的选项
 */
void UkmediaMainWidget::initListWidgetItem()
{
    QString outputCardName = findCardName(m_pVolumeControl->defaultOutputCard,m_pVolumeControl->cardMap);
    QString outputPortLabel = findOutputPortLabel(m_pVolumeControl->defaultOutputCard,m_pVolumeControl->sinkPortName);

    findOutputListWidgetItem(outputCardName,outputPortLabel);

    int vol = m_pVolumeControl->getSinkVolume();
    float balanceVolume = m_pVolumeControl->getBalanceVolume();
    m_pOutputWidget->m_pOpVolumeSlider->blockSignals(true);
    m_pOutputWidget->m_pOpBalanceSlider->blockSignals(true);
    m_pOutputWidget->m_pOpVolumeSlider->setValue(paVolumeToValue(vol));
    m_pOutputWidget->m_pOpBalanceSlider->setValue(balanceVolume*100);
    m_pOutputWidget->m_pOpBalanceSlider->blockSignals(false);
    m_pOutputWidget->m_pOpVolumeSlider->blockSignals(false);
    m_pOutputWidget->m_pOpVolumePercentLabel->setText(QString::number(paVolumeToValue(vol))+"%");


    qDebug() <<"initListWidgetItem" << m_pVolumeControl->defaultOutputCard << outputCardName <<m_pVolumeControl->sinkPortName << outputPortLabel << m_pVolumeControl->defaultSourceName;

    QString inputCardName = findCardName(m_pVolumeControl->defaultInputCard,m_pVolumeControl->cardMap);
    QString inputPortLabel = findInputPortLabel(m_pVolumeControl->defaultInputCard,m_pVolumeControl->sourcePortName);
    if (m_pVolumeControl->defaultSourceName.data() && strstr(m_pVolumeControl->defaultSourceName,"noiseReduceSource")) {
        for (int row=0;row<m_pInputWidget->m_pInputListWidget->count();row++) {
            QListWidgetItem *item = m_pInputWidget->m_pInputListWidget->item(row);
            UkuiListWidgetItem *wid = (UkuiListWidgetItem *)m_pInputWidget->m_pInputListWidget->itemWidget(item);

            if (inputCardName == "" && inputPortLabel == "" && wid->deviceLabel->text().contains("alsa_card") && !wid->deviceLabel->text().contains(".usb")) {
                inputCardName = wid->deviceLabel->text();
                inputPortLabel = wid->portLabel->text();
                break;
            }
        }
    }
    findInputListWidgetItem(inputCardName,inputPortLabel);
}

void UkmediaMainWidget::initOutputListWidgetItem(){
    QString outputCardName = findCardName(m_pVolumeControl->defaultOutputCard,m_pVolumeControl->cardMap);
    QString outputPortLabel = findOutputPortLabel(m_pVolumeControl->defaultOutputCard,m_pVolumeControl->sinkPortName);

    findOutputListWidgetItem(outputCardName,outputPortLabel);

    int vol = m_pVolumeControl->getSinkVolume();
    float balanceVolume = m_pVolumeControl->getBalanceVolume();
    m_pOutputWidget->m_pOpVolumeSlider->blockSignals(true);
    m_pOutputWidget->m_pOpBalanceSlider->blockSignals(true);
    m_pOutputWidget->m_pOpVolumeSlider->setValue(paVolumeToValue(vol));
    m_pOutputWidget->m_pOpBalanceSlider->setValue(balanceVolume*100);
    m_pOutputWidget->m_pOpBalanceSlider->blockSignals(false);
    m_pOutputWidget->m_pOpVolumeSlider->blockSignals(false);
    m_pOutputWidget->m_pOpVolumePercentLabel->setText(QString::number(paVolumeToValue(vol))+"%");

    qDebug() <<"initOutputListWidgetItem" << m_pVolumeControl->defaultOutputCard << outputCardName
            <<m_pVolumeControl->sinkPortName << outputPortLabel;
}


void UkmediaMainWidget::initInputListWidgetItem(){
    QString inputCardName = findCardName(m_pVolumeControl->defaultInputCard,m_pVolumeControl->cardMap);
    QString inputPortLabel = findInputPortLabel(m_pVolumeControl->defaultInputCard,m_pVolumeControl->sourcePortName);
    if (m_pVolumeControl->defaultSourceName.data() && strstr(m_pVolumeControl->defaultSourceName,"noiseReduceSource")) {
        for (int row=0;row<m_pInputWidget->m_pInputListWidget->count();row++) {
            QListWidgetItem *item = m_pInputWidget->m_pInputListWidget->item(row);
            UkuiListWidgetItem *wid = (UkuiListWidgetItem *)m_pInputWidget->m_pInputListWidget->itemWidget(item);

            if (inputCardName == "" && inputPortLabel == "" && wid->deviceLabel->text().contains("alsa_card") && !wid->deviceLabel->text().contains(".usb")) {
                inputCardName = wid->deviceLabel->text();
                inputPortLabel = wid->portLabel->text();
                break;
            }
        }
    }
    findInputListWidgetItem(inputCardName,inputPortLabel);

    //当所有可用的输入设备全部移除，台式机才会出现该情况
    if(strstr(m_pVolumeControl->defaultSourceName,"monitor"))
        m_pInputWidget->m_pInputLevelProgressBar->setValue(0);

    qDebug() <<"initInputListWidgetItem" << m_pVolumeControl->defaultInputCard << inputCardName
            <<m_pVolumeControl->sourcePortName << inputPortLabel;
}

void UkmediaMainWidget::themeChangeIcons()
{
    int nInputValue = paVolumeToValue(m_pVolumeControl->getSourceVolume());
    int nOutputValue = paVolumeToValue(m_pVolumeControl->getSinkVolume());
    bool inputStatus = m_pVolumeControl->getSourceMute();
    bool outputStatus = m_pVolumeControl->getSinkMute();

    inputVolumeDarkThemeImage(nInputValue,inputStatus);
    outputVolumeDarkThemeImage(nOutputValue,outputStatus);
    m_pOutputWidget->m_pOutputIconBtn->repaint();
    m_pInputWidget->m_pInputIconBtn->repaint();

}

/*
 * 滑动条值转换成音量值
 */
int UkmediaMainWidget::valueToPaVolume(int value)
{
    return value / UKMEDIA_VOLUME_NORMAL * PA_VOLUME_NORMAL;
}

/*
 * 音量值转换成滑动条值
 */
int UkmediaMainWidget::paVolumeToValue(int value)
{
    return (value / PA_VOLUME_NORMAL * UKMEDIA_VOLUME_NORMAL) + 0.5;
}


QPixmap UkmediaMainWidget::drawDarkColoredPixmap(const QPixmap &source)
{
//    QColor currentcolor=HighLightEffect::getCurrentSymbolicColor();
    QColor gray(255,255,255);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red()-gray.red())<20 && qAbs(color.green()-gray.green())<20 && qAbs(color.blue()-gray.blue())<20) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                }
                else {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap UkmediaMainWidget::drawLightColoredPixmap(const QPixmap &source)
{
//    QColor currentcolor=HighLightEffect::getCurrentSymbolicColor();
    QColor gray(255,255,255);
    QColor standard (0,0,0);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red()-gray.red())<20 && qAbs(color.green()-gray.green())<20 && qAbs(color.blue()-gray.blue())<20) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                }
                else {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

void UkmediaMainWidget::alertIconButtonSetIcon(bool state,int value)
{
    QImage image;
    QColor color = QColor(0,0,0,216);
    if (mThemeName == UKUI_THEME_WHITE) {
        color = QColor(0,0,0,216);
    }
    else if (mThemeName == UKUI_THEME_BLACK) {
        color = QColor(255,255,255,216);
    }
    m_pSoundWidget->m_pAlertIconBtn->mColor = color;
    if (state) {
        image  = QImage("/usr/share/ukui-media/img/audio-volume-muted.svg");
        m_pSoundWidget->m_pAlertIconBtn->mImage = image;
    }
    else if (value <= 0) {
        image  = QImage("/usr/share/ukui-media/img/audio-volume-muted.svg");
        m_pSoundWidget->m_pAlertIconBtn->mImage = image;
    }
    else if (value > 0 && value <= 33) {
        image = QImage("/usr/share/ukui-media/img/audio-volume-low.svg");
        m_pSoundWidget->m_pAlertIconBtn->mImage = image;
    }
    else if (value >33 && value <= 66) {
        image = QImage("/usr/share/ukui-media/img/audio-volume-medium.svg");
        m_pSoundWidget->m_pAlertIconBtn->mImage = image;
    }
    else {
        image = QImage("/usr/share/ukui-media/img/audio-volume-high.svg");
        m_pSoundWidget->m_pAlertIconBtn->mImage = image;
    }

}

void UkmediaMainWidget::createAlertSound(UkmediaMainWidget *pWidget)
{
    const GList   *list;
    m_pOutputWidget->m_pOutputListWidget->clear();
    m_pInputWidget->m_pInputListWidget->clear();
    cardMap.clear();
    outputPortNameMap.clear();
    outputPortMap.clear();
    inputPortMap.clear();
    inputPortNameMap.clear();
    outputPortLabelMap.clear();
    inputPortLabelMap.clear();

    m_pVolumeControl->inputPortProfileNameMap.clear();
    m_pVolumeControl->cardProfileMap.clear();
    m_pVolumeControl->cardProfilePriorityMap.clear();
    inputCardStreamMap.clear();
    outputCardStreamMap.clear();

}

/*
    初始化combobox的值
*/
void UkmediaMainWidget::comboboxCurrentTextInit()
{
    QList<char *> existsPath = listExistsPath();

    for (char * path : existsPath) {

        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);

        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba))
        {
            QGSettings * settings = new QGSettings(ba, bba);
            QString filenameStr = settings->get(FILENAME_KEY).toString();
            QString nameStr = settings->get(NAME_KEY).toString();
            int index = 0;
            for (int i=0;i<m_pSoundList->count();i++) {
                QString str = m_pSoundList->at(i);
                if (str.contains(filenameStr,Qt::CaseSensitive)) {
                    index = i;
                    break;
                }
            }
            if (nameStr == "alert-sound") {
                QString displayName = m_pSoundNameList->at(index);
                m_pSoundWidget->m_pAlertSoundCombobox->setCurrentText(displayName);
                continue;
            }
            if (nameStr == "window-close") {
                QString displayName = m_pSoundNameList->at(index);
                continue;
            }
            else if (nameStr == "volume-changed") {
                QString displayName = m_pSoundNameList->at(index);
                m_pSoundWidget->m_pVolumeChangeCombobox->setCurrentText(displayName);
                continue;
            }
            else if (nameStr == "system-setting") {
                QString displayName = m_pSoundNameList->at(index);
                continue;
            }
        }
        else {
            continue;
        }
    }
}

/*
    是否播放开机音乐
*/
void UkmediaMainWidget::startupButtonSwitchChangedSlot(bool status)
{
    bool bBootStatus = true;
    if (m_pBootSetting->keys().contains("startupMusic")) {
        bBootStatus = m_pBootSetting->get(UKUI_STARTUP_MUSIC_KEY).toBool();
        if (bBootStatus != status) {
            m_pBootSetting->set(UKUI_STARTUP_MUSIC_KEY,status);
        }
    }
}

/*
    是否播放关机音乐
*/
void UkmediaMainWidget::poweroffButtonSwitchChangedSlot(bool status)
{
    bool bBootStatus = true;
    if (m_pBootSetting->keys().contains("poweroffMusic")) {
        bBootStatus = m_pBootSetting->get(UKUI_POWEROFF_MUSIC_KEY).toBool();
        if (bBootStatus != status) {
            m_pBootSetting->set(UKUI_POWEROFF_MUSIC_KEY,status);
        }
    }
}

/*
    是否播放注销音乐
*/
void UkmediaMainWidget::logoutMusicButtonSwitchChangedSlot(bool status)
{
    bool bBootStatus = true;
    if (m_pBootSetting->keys().contains("logoutMusic")) {
        bBootStatus = m_pBootSetting->get(UKUI_LOGOUT_MUSIC_KEY).toBool();
        if (bBootStatus != status) {
            m_pBootSetting->set(UKUI_LOGOUT_MUSIC_KEY,status);
        }
    }
}

/*
    是否播放唤醒音乐
*/
void UkmediaMainWidget::wakeButtonSwitchChangedSlot(bool status)
{
    bool bBootStatus = true;
    if (m_pBootSetting->keys().contains("weakupMusic")) {
        bBootStatus = m_pBootSetting->get(UKUI_WAKEUP_MUSIC_KEY).toBool();
        if (bBootStatus != status) {
            m_pBootSetting->set(UKUI_WAKEUP_MUSIC_KEY,status);
        }
    }
}

/*
    提示音的开关
*/
void UkmediaMainWidget::alertSoundButtonSwitchChangedSlot(bool status)
{
    g_settings_set_boolean (m_pSoundSettings, EVENT_SOUNDS_KEY, status);
    /*
    if (status == true) {
        m_pSoundWidget->m_pAlertSoundVolumeWidget->show();
        m_pSoundWidget->m_pSoundLayout->insertWidget(5,m_pSoundWidget->m_pAlertSoundVolumeWidget);
    }
    else {
        m_pSoundWidget->m_pAlertSoundVolumeWidget->hide();
        m_pSoundWidget->m_pSoundLayout->removeWidget(m_pSoundWidget->m_pAlertSoundVolumeWidget);
    }
    */
}

void UkmediaMainWidget::noiseReductionButtonSwitchChangedSlot(bool status)
{
    g_settings_set_boolean (m_pSoundSettings, DNS_NOISE_REDUCTION, status);
}

void UkmediaMainWidget::bootMusicSettingsChanged()
{
    bool bBootStatus = true;
    bool status;
    if (m_pBootSetting->keys().contains("startupMusic")) {
        bBootStatus = m_pBootSetting->get(UKUI_STARTUP_MUSIC_KEY).toBool();
        if (status != bBootStatus ) {
            m_pSoundWidget->m_pStartupButton->setChecked(bBootStatus);
        }
    }
    if (m_pBootSetting->keys().contains("poweroffMusic")) {
        bBootStatus = m_pBootSetting->get(UKUI_POWEROFF_MUSIC_KEY).toBool();
        if (status != bBootStatus ) {
            m_pSoundWidget->m_pPoweroffButton->setChecked(bBootStatus);
        }
    }
    if (m_pBootSetting->keys().contains("logoutMusic")) {
        bBootStatus = m_pBootSetting->get(UKUI_LOGOUT_MUSIC_KEY).toBool();
        if (status != bBootStatus ) {
            m_pSoundWidget->m_pLogoutButton->setChecked(bBootStatus);
        }
    }
    if (m_pBootSetting->keys().contains("weakupMusic")) {
        bBootStatus = m_pBootSetting->get(UKUI_WAKEUP_MUSIC_KEY).toBool();
        if (status != bBootStatus ) {
            m_pSoundWidget->m_pWakeupMusicButton->setChecked(bBootStatus);
        }
    }
}


/*
    系统主题更改
*/
void UkmediaMainWidget::ukuiThemeChangedSlot(const QString &themeStr)
{
    if (m_pThemeSetting->keys().contains("styleName")) {
        mThemeName = m_pThemeSetting->get(UKUI_THEME_NAME).toString();
    }
    int nInputValue = getInputVolume();
    int nOutputValue = getOutputVolume();
    bool inputStatus = m_pVolumeControl->getSourceMute();
    bool outputStatus = m_pVolumeControl->getSinkMute();
    inputVolumeDarkThemeImage(nInputValue,inputStatus);
    outputVolumeDarkThemeImage(nOutputValue,outputStatus);
    m_pOutputWidget->m_pOutputIconBtn->repaint();
    m_pSoundWidget->m_pAlertIconBtn->repaint();
    m_pInputWidget->m_pInputIconBtn->repaint();
}

/*
    获取输入音量值
*/
int UkmediaMainWidget::getInputVolume()
{
    return m_pInputWidget->m_pIpVolumeSlider->value();
}

/*
    获取输出音量值
*/
int UkmediaMainWidget::getOutputVolume()
{
    return m_pOutputWidget->m_pOpVolumeSlider->value();
}

/*
    深色主题时输出音量图标
*/
void UkmediaMainWidget::outputVolumeDarkThemeImage(int value,bool status)
{
    QImage image;
    QColor color = QColor(0,0,0,216);
    if (mThemeName == UKUI_THEME_WHITE || mThemeName == "ukui-default") {
        color = QColor(0,0,0,216);
    }
    else if (mThemeName == UKUI_THEME_BLACK || mThemeName == "ukui-dark") {
        color = QColor(255,255,255,216);
    }
    m_pOutputWidget->m_pOutputIconBtn->mColor = color;
    if (status) {
        image = QIcon::fromTheme("audio-volume-muted-symbolic").pixmap(24,24).toImage();
    }
    else if (value <= 0) {
        image = QIcon::fromTheme("audio-volume-muted-symbolic").pixmap(24,24).toImage();
    }
    else if (value > 0 && value <= 33) {
        image = QIcon::fromTheme("audio-volume-low-symbolic").pixmap(24,24).toImage();
    }
    else if (value >33 && value <= 66) {
        image = QIcon::fromTheme("audio-volume-medium-symbolic").pixmap(24,24).toImage();
    }
    else {
        image = QIcon::fromTheme("audio-volume-high-symbolic").pixmap(24,24).toImage();
    }
    m_pOutputWidget->m_pOutputIconBtn->mImage = image;

}

/*
    输入音量图标
*/
void UkmediaMainWidget::inputVolumeDarkThemeImage(int value,bool status)
{
    QImage image;
    QColor color = QColor(0,0,0,190);
    if (mThemeName == UKUI_THEME_WHITE || mThemeName == "ukui-default") {
        color = QColor(0,0,0,190);
    }
    else if (mThemeName == UKUI_THEME_BLACK || mThemeName == "ukui-dark") {
        color = QColor(255,255,255,190);
    }
    m_pInputWidget->m_pInputIconBtn->mColor = color;
    if (status) {
        image = QIcon::fromTheme("microphone-sensitivity-muted-symbolic").pixmap(24,24).toImage();
    }
    else if (value <= 0) {
        image = QIcon::fromTheme("microphone-sensitivity-muted-symbolic").pixmap(24,24).toImage();
    }
    else if (value > 0 && value <= 33) {
        image = QIcon::fromTheme("microphone-sensitivity-low-symbolic").pixmap(24,24).toImage();
    }
    else if (value >33 && value <= 66) {
        image = QIcon::fromTheme("microphone-sensitivity-medium-symbolic").pixmap(24,24).toImage();
    }
    else {
        image = QIcon::fromTheme("microphone-sensitivity-high-symbolic").pixmap(24,24).toImage();
    }
    m_pInputWidget->m_pInputIconBtn->mImage = image;
}


void UkmediaMainWidget::onKeyChanged (GSettings *settings,gchar *key,UkmediaMainWidget *m_pWidget)
{
    Q_UNUSED(settings);
    g_debug("on key changed");
    if (!strcmp (key, EVENT_SOUNDS_KEY) ||
        !strcmp (key, SOUND_THEME_KEY) ||
        !strcmp (key, INPUT_SOUNDS_KEY)) {
        updateTheme (m_pWidget);
    }
}

/*
    更新主题
*/
void UkmediaMainWidget::updateTheme (UkmediaMainWidget *m_pWidget)
{
    g_debug("update theme");
    char *pThemeName;
    gboolean feedBackEnabled;
    gboolean eventsEnabled;
    feedBackEnabled = g_settings_get_boolean(m_pWidget->m_pSoundSettings, INPUT_SOUNDS_KEY);
    eventsEnabled = g_settings_get_boolean(m_pWidget->m_pSoundSettings,EVENT_SOUNDS_KEY);
//    eventsEnabled = FALSE;
    if (eventsEnabled) {
        pThemeName = g_settings_get_string (m_pWidget->m_pSoundSettings, SOUND_THEME_KEY);
    } else {
        pThemeName = g_strdup (NO_SOUNDS_THEME_NAME);
    }
    qDebug() << "updateTheme" << pThemeName;
    //设置combox的主题
    setComboxForThemeName (m_pWidget, pThemeName);
    updateAlertsFromThemeName (m_pWidget, pThemeName);
}

/*
    设置主题名到combox
*/
void UkmediaMainWidget::setupThemeSelector (UkmediaMainWidget *m_pWidget)
{
    g_debug("setup theme selector");
    GHashTable  *hash;
    const char * const *dataDirs;
    const char *m_pDataDir;
    char *dir;
    guint i;

    /* Add the theme names and their display name to a hash table,
     * makes it easy to avoid duplicate themes */
    hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

    dataDirs = g_get_system_data_dirs ();
    for (i = 0; dataDirs[i] != nullptr; i++) {
        dir = g_build_filename (dataDirs[i], "sounds", nullptr);
        soundThemeInDir (m_pWidget,hash, dir);
    }

    m_pDataDir = g_get_user_data_dir ();
    dir = g_build_filename (m_pDataDir, "sounds", nullptr);
    soundThemeInDir (m_pWidget,hash, dir);

    /* If there isn't at least one theme, make everything
     * insensitive, LAME! */
    if (g_hash_table_size (hash) == 0) {
        g_warning ("Bad setup, install the freedesktop sound theme");
        g_hash_table_destroy (hash);
        return;
    }
    /* Add the themes to a combobox */
    g_hash_table_destroy (hash);
}

/*
    主题名所在目录
*/
void UkmediaMainWidget::soundThemeInDir (UkmediaMainWidget *m_pWidget,GHashTable *hash,const char *dir)
{
    Q_UNUSED(hash);
    qDebug() << "sound theme in dir" << dir;
    GDir *d;
    const char *m_pName;
    d = g_dir_open (dir, 0, nullptr);
    if (d == nullptr) {
        return;
    }
    while ((m_pName = g_dir_read_name (d)) != nullptr) {
        char *m_pDirName, *m_pIndex, *m_pIndexName;
        /* Look for directories */
        m_pDirName = g_build_filename (dir, m_pName, nullptr);
        if (g_file_test (m_pDirName, G_FILE_TEST_IS_DIR) == FALSE) {
            continue;
        }

        /* Look for index files */
        m_pIndex = g_build_filename (m_pDirName, "index.theme", nullptr);

        /* Check the name of the theme in the index.theme file */
        m_pIndexName = loadIndexThemeName (m_pIndex, nullptr);
        if (m_pIndexName == nullptr) {
            continue;
        }

        gchar * themeName = g_settings_get_string (m_pWidget->m_pSoundSettings, SOUND_THEME_KEY);
        //设置主题到combox中
        qDebug() << "sound theme in dir" << "displayname:" << m_pIndexName << "theme name:" << m_pName << "theme："<< themeName;
        //屏蔽ubuntu custom 主题
        if (m_pName && !strstr(m_pName,"ubuntu") && !strstr(m_pName,"freedesktop") && !strstr(m_pName,"custom")) {
            m_pWidget->m_pThemeDisplayNameList->append(m_pIndexName);
            m_pWidget->m_pThemeNameList->append(m_pName);
            m_pWidget->m_pSoundWidget->m_pSoundThemeCombobox->addItem(m_pIndexName);

        }
    }
    g_dir_close (d);
}

/*
    加载下标的主题名
*/
char *UkmediaMainWidget::loadIndexThemeName (const char *index,char **parent)
{
    g_debug("load index theme name");
    GKeyFile *file;
    char *indexname = nullptr;
    gboolean hidden;

    file = g_key_file_new ();
    if (g_key_file_load_from_file (file, index, G_KEY_FILE_KEEP_TRANSLATIONS, nullptr) == FALSE) {
        g_key_file_free (file);
        return nullptr;
    }
    /* Don't add hidden themes to the list */
    hidden = g_key_file_get_boolean (file, "Sound Theme", "Hidden", nullptr);
    if (!hidden) {
        indexname = g_key_file_get_locale_string (file,"Sound Theme","Name",nullptr,nullptr);
        /* Save the parent theme, if there's one */
        if (parent != nullptr) {
            *parent = g_key_file_get_string (file,"Sound Theme","Inherits",nullptr);
        }
    }
    g_key_file_free (file);
    return indexname;
}

/*
    设置combox的主题名
*/
void UkmediaMainWidget::setComboxForThemeName (UkmediaMainWidget *m_pWidget,const char *name)
{
    g_debug("set combox for theme name");
    gboolean      found;
    int count = 0;
    /* If the name is empty, use "freedesktop" */
    if (name == nullptr || *name == '\0') {
        name = "freedesktop";
    }
    QString value;
    int index = -1;
    while(!found) {
        value = m_pWidget->m_pThemeNameList->at(count);
        found = (value != "" && value == name);
        count++;
        if( count >= m_pWidget->m_pThemeNameList->size() || found) {
            count = 0;
            break;
        }
    }
    if (m_pWidget->m_pThemeNameList->contains(name)) {
        index = m_pWidget->m_pThemeNameList->indexOf(name);
        value = m_pWidget->m_pThemeNameList->at(index);
        m_pWidget->m_pSoundWidget->m_pSoundThemeCombobox->setCurrentIndex(index);
    }
    /* When we can't find the theme we need to set, try to set the default
     * one "freedesktop" */
    if (found) {
    } else if (strcmp (name, "freedesktop") != 0) {//设置为默认的主题
        qDebug() << "设置为默认的主题" << "freedesktop";
        g_debug ("not found, falling back to fdo");
        setComboxForThemeName (m_pWidget, "freedesktop");
    }
}

/*
    更新报警音
*/
void UkmediaMainWidget::updateAlertsFromThemeName (UkmediaMainWidget *m_pWidget,const gchar *m_pName)
{
    g_debug("update alerts from theme name");
    if (strcmp (m_pName, CUSTOM_THEME_NAME) != 0) {
            /* reset alert to default */
        updateAlert (m_pWidget, DEFAULT_ALERT_ID);
    } else {
        int   sound_type;
        char *linkname;
        linkname = nullptr;
        sound_type = getFileType ("bell-terminal", &linkname);
        g_debug ("Found link: %s", linkname);
        if (sound_type == SOUND_TYPE_CUSTOM) {
            updateAlert (m_pWidget, linkname);
        }
    }
}

/*
    更新报警声音
*/
void UkmediaMainWidget::updateAlert (UkmediaMainWidget *pWidget,const char *alertId)
{
    Q_UNUSED(alertId)
    g_debug("update alert");
    QString themeStr;
    char *theme;
    char *parent;
    gboolean      is_custom;
    gboolean      is_default;
    gboolean add_custom = false;
    gboolean remove_custom = false;
    QString nameStr;
    int index = -1;
    /* Get the current theme's name, and set the parent */
    index = pWidget->m_pSoundWidget->m_pSoundThemeCombobox->currentIndex();
    if (index != -1) {
        themeStr = pWidget->m_pThemeNameList->at(index);
        nameStr = pWidget->m_pThemeNameList->at(index);
    }
    else {
        themeStr = "freedesktop";
        nameStr = "freedesktop";
    }
    QByteArray ba = themeStr.toLatin1();
    theme = ba.data();

    QByteArray baParent = nameStr.toLatin1();
    parent = baParent.data();

    is_custom = strcmp (theme, CUSTOM_THEME_NAME) == 0;
    is_default = strcmp (alertId, DEFAULT_ALERT_ID) == 0;

    if (! is_custom && is_default) {
        /* remove custom just in case */
        remove_custom = TRUE;
    } else if (! is_custom && ! is_default) {
        createCustomTheme (parent);
        saveAlertSounds(pWidget->m_pSoundWidget->m_pSoundThemeCombobox, alertId);
        add_custom = TRUE;
    } else if (is_custom && is_default) {
        saveAlertSounds(pWidget->m_pSoundWidget->m_pSoundThemeCombobox, alertId);
        /* after removing files check if it is empty */
        if (customThemeDirIsEmpty ()) {
            remove_custom = TRUE;
        }
    } else if (is_custom && ! is_default) {
        saveAlertSounds(pWidget->m_pSoundWidget->m_pSoundThemeCombobox, alertId);
    }

    if (add_custom) {
        setComboxForThemeName (pWidget, CUSTOM_THEME_NAME);
    } else if (remove_custom) {
        setComboxForThemeName (pWidget, parent);
    }
}

/*
    获取声音文件类型
*/
int UkmediaMainWidget::getFileType (const char *sound_name,char **linked_name)
{
    g_debug("get file type");
    char *name, *filename;
    *linked_name = nullptr;
    name = g_strdup_printf ("%s.disabled", sound_name);
    filename = customThemeDirPath (name);
    if (g_file_test (filename, G_FILE_TEST_IS_REGULAR) != FALSE) {
        return SOUND_TYPE_OFF;
    }
    /* We only check for .ogg files because those are the
     * only ones we create */
    name = g_strdup_printf ("%s.ogg", sound_name);
    filename = customThemeDirPath (name);
    g_free (name);

    if (g_file_test (filename, G_FILE_TEST_IS_SYMLINK) != FALSE) {
        *linked_name = g_file_read_link (filename, nullptr);
        g_free (filename);
        return SOUND_TYPE_CUSTOM;
    }
    g_free (filename);
    return SOUND_TYPE_BUILTIN;
}

/*
    自定义主题路径
*/
char *UkmediaMainWidget::customThemeDirPath (const char *child)
{
    g_debug("custom theme dir path");
    static char *dir = nullptr;
    const char *data_dir;

    if (dir == nullptr) {
        data_dir = g_get_user_data_dir ();
        dir = g_build_filename (data_dir, "sounds", CUSTOM_THEME_NAME, nullptr);
    }
    if (child == nullptr)
        return g_strdup (dir);

    return g_build_filename (dir, child, nullptr);
}

/*
    获取报警声音文件的路径
*/
void UkmediaMainWidget::populateModelFromDir (UkmediaMainWidget *m_pWidget,const char *dirname)//从目录查找报警声音文件
{
    g_debug("populate model from dir");
    GDir *d;
    const char *name;
    char *path;
    d = g_dir_open (dirname, 0, nullptr);
    if (d == nullptr) {
        return;
    }
    while ((name = g_dir_read_name (d)) != nullptr) {

        if (! g_str_has_suffix (name, ".xml")) {
            continue;
        }
        QString themeName = name;
        QStringList temp = themeName.split("-");
        themeName = temp.at(0);
        if (!m_pWidget->m_pSoundThemeList->contains(themeName)) {
            m_pWidget->m_pSoundThemeList->append(themeName);
            m_pWidget->m_pSoundThemeDirList->append(dirname);
            m_pWidget->m_pSoundThemeXmlNameList->append(name);
        }
        path = g_build_filename (dirname, name, nullptr);
    }
    char *pThemeName = g_settings_get_string (m_pWidget->m_pSoundSettings, SOUND_THEME_KEY);
    int themeIndex;
    if(m_pWidget->m_pSoundThemeList->contains(pThemeName)) {
         themeIndex =  m_pWidget->m_pSoundThemeList->indexOf(pThemeName);
         if (themeIndex < 0 )
             return;

    }
    else {
        themeIndex = 1;
    }

    QString dirName = m_pWidget->m_pSoundThemeDirList->at(themeIndex);
    QString xmlName = m_pWidget->m_pSoundThemeXmlNameList->at(themeIndex);
    path = g_build_filename (dirName.toLatin1().data(), xmlName.toLatin1().data(), nullptr);
    m_pWidget->m_pSoundWidget->m_pAlertSoundCombobox->blockSignals(true);
    m_pWidget->m_pSoundWidget->m_pAlertSoundCombobox->clear();
    m_pWidget->m_pSoundWidget->m_pAlertSoundCombobox->blockSignals(false);

    populateModelFromFile (m_pWidget, path);
    //初始化声音主题

    g_free (path);
    g_dir_close (d);
}

/*
    获取报警声音文件
*/
void UkmediaMainWidget::populateModelFromFile (UkmediaMainWidget *m_pWidget,const char *filename)
{
    g_debug("populate model from file");
    xmlDocPtr  doc;
    xmlNodePtr root;
    xmlNodePtr child;
    gboolean   exists;

    exists = g_file_test (filename, G_FILE_TEST_EXISTS);
    if (! exists) {
        return;
    }
    doc = xmlParseFile (filename);
    if (doc == nullptr) {
        return;
    }
    root = xmlDocGetRootElement (doc);
    for (child = root->children; child; child = child->next) {
        if (xmlNodeIsText (child)) {
                continue;
        }
        if (xmlStrcmp (child->name, GVC_SOUND_SOUND) != 0) {
                continue;
        }
        populateModelFromNode (m_pWidget, child);
    }
    xmlFreeDoc (doc);
}

/*
    从节点查找声音文件并加载到组合框中
*/
void UkmediaMainWidget::populateModelFromNode (UkmediaMainWidget *m_pWidget,xmlNodePtr node)
{
    g_debug("populate model from node");
    xmlNodePtr child;
    xmlChar   *filename;
    xmlChar   *name;

    filename = nullptr;
    name = xmlGetAndTrimNames (node);
    for (child = node->children; child; child = child->next) {
        if (xmlNodeIsText (child)) {
            continue;
        }

        if (xmlStrcmp (child->name, GVC_SOUND_FILENAME) == 0) {
            filename = xmlNodeGetContent (child);
        } else if (xmlStrcmp (child->name, GVC_SOUND_NAME) == 0) {
                /* EH? should have been trimmed */
        }
    }

    gchar * themeName = g_settings_get_string (m_pWidget->m_pSoundSettings, SOUND_THEME_KEY);

    //将找到的声音文件名设置到combox中
    if (filename != nullptr && name != nullptr) {
        m_pWidget->m_pSoundList->append((const char *)filename);
        m_pWidget->m_pSoundNameList->append((const char *)name);
        m_pWidget->m_pSoundWidget->m_pAlertSoundCombobox->addItem((char *)name);
        m_pWidget->m_pSoundWidget->m_pLagoutCombobox->addItem((char *)name);
        m_pWidget->m_pSoundWidget->m_pVolumeChangeCombobox->addItem((char *)name);
    }
    xmlFree (filename);
    xmlFree (name);
}

/* Adapted from yelp-toc-pager.c */
xmlChar *UkmediaMainWidget::xmlGetAndTrimNames (xmlNodePtr node)
{
    g_debug("xml get and trim names");
    xmlNodePtr cur;
    xmlChar *keep_lang = nullptr;
    xmlChar *value;
    int j, keep_pri = INT_MAX;
    const gchar * const * langs = g_get_language_names ();

    value = nullptr;
    for (cur = node->children; cur; cur = cur->next) {
        if (! xmlStrcmp (cur->name, GVC_SOUND_NAME)) {
            xmlChar *cur_lang = nullptr;
            int cur_pri = INT_MAX;
            cur_lang = xmlNodeGetLang (cur);

            if (cur_lang) {
                for (j = 0; langs[j]; j++) {
                    if (g_str_equal (cur_lang, langs[j])) {
                        cur_pri = j;
                        break;
                    }
                }
            } else {
                cur_pri = INT_MAX - 1;
            }

            if (cur_pri <= keep_pri) {
                if (keep_lang)
                    xmlFree (keep_lang);
                if (value)
                    xmlFree (value);

                value = xmlNodeGetContent (cur);

                keep_lang = cur_lang;
                keep_pri = cur_pri;
            } else {
                if (cur_lang)
                    xmlFree (cur_lang);
            }
        }
    }

    /* Delete all GVC_SOUND_NAME nodes */
    cur = node->children;
    while (cur) {
            xmlNodePtr p_this = cur;
            cur = cur->next;
            if (! xmlStrcmp (p_this->name, GVC_SOUND_NAME)) {
                xmlUnlinkNode (p_this);
                xmlFreeNode (p_this);
            }
    }
    return value;
}

/*
 * 播放报警声音
*/
void UkmediaMainWidget::playAlretSoundFromPath (UkmediaMainWidget *w,QString path)
{
    g_debug("play alert sound from path");

   gchar * themeName = g_settings_get_string (w->m_pSoundSettings, SOUND_THEME_KEY);

   if (strcmp (path.toLatin1().data(), DEFAULT_ALERT_ID) == 0) {
       if (themeName != NULL) {
           caPlayForWidget (w, 0,
                            CA_PROP_APPLICATION_NAME, _("Sound Preferences"),
                            CA_PROP_EVENT_ID, "bell-window-system",
                            CA_PROP_CANBERRA_XDG_THEME_NAME, themeName,
                            CA_PROP_EVENT_DESCRIPTION, _("Testing event sound"),
                            CA_PROP_CANBERRA_CACHE_CONTROL, "never",
                            CA_PROP_APPLICATION_ID, "org.mate.VolumeControl",
                 #ifdef CA_PROP_CANBERRA_ENABLE
                            CA_PROP_CANBERRA_ENABLE, "1",
                 #endif
                            NULL);
       } else {
           caPlayForWidget (w, 0,
                            CA_PROP_APPLICATION_NAME, _("Sound Preferences"),
                            CA_PROP_EVENT_ID, "bell-window-system",
                            CA_PROP_EVENT_DESCRIPTION, _("Testing event sound"),
                            CA_PROP_CANBERRA_CACHE_CONTROL, "never",
                            CA_PROP_APPLICATION_ID, "org.mate.VolumeControl",
                 #ifdef CA_PROP_CANBERRA_ENABLE
                            CA_PROP_CANBERRA_ENABLE, "1",
                 #endif
                            NULL);
       }
   } else {
       caPlayForWidget (w, 0,
                        CA_PROP_APPLICATION_NAME, _("Sound Preferences"),
                        CA_PROP_MEDIA_FILENAME, path.toLatin1().data(),
                        CA_PROP_EVENT_DESCRIPTION, _("Testing event sound"),
                        CA_PROP_CANBERRA_CACHE_CONTROL, "never",
                        CA_PROP_APPLICATION_ID, "org.mate.VolumeControl",
                 #ifdef CA_PROP_CANBERRA_ENABLE
                        CA_PROP_CANBERRA_ENABLE, "1",
                 #endif
                        NULL);
   }
}

/*
    点击combox播放声音
*/
void UkmediaMainWidget::comboxIndexChangedSlot(int index)
{
    g_debug("combox index changed slot");
    QString sound_name = m_pSoundList->at(index);
    updateAlert(this,sound_name.toLatin1().data());
    playAlretSoundFromPath(this,sound_name);

    QString fileName = m_pSoundList->at(index);
    QStringList list = fileName.split("/");
    QString soundName = list.at(list.count()-1);
    QStringList eventIdList = soundName.split(".");
    QString eventId = eventIdList.at(0);
    QList<char *> existsPath = listExistsPath();

    for (char * path : existsPath) {

        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);

        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba))
        {
            QGSettings * settings = new QGSettings(ba, bba);
//            QString filenameStr = settings->get(FILENAME_KEY).toString();
            QString nameStr = settings->get(NAME_KEY).toString();
            if (nameStr == "alert-sound") {
                settings->set(FILENAME_KEY,eventId);
                return;
            }
        }
        else {
            continue;
        }
    }

}

/*
    设置窗口关闭的提示音
*/
void UkmediaMainWidget::windowClosedComboboxChangedSlot(int index)
{
    QString fileName = m_pSoundList->at(index);
    QStringList list = fileName.split("/");
    QString soundName = list.at(list.count()-1);
    QStringList eventIdList = soundName.split(".");
    QString eventId = eventIdList.at(0);
    QList<char *> existsPath = listExistsPath();

    for (char * path : existsPath) {

        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);

        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba))
        {
            QGSettings * settings = new QGSettings(ba, bba);
//            QString filenameStr = settings->get(FILENAME_KEY).toString();
            QString nameStr = settings->get(NAME_KEY).toString();
            if (nameStr == "window-close") {
                settings->set(FILENAME_KEY,eventId);
                return;
            }
        }
        else {
            continue;
        }
    }
}

/*
    设置音量改变的提示声音
*/
void UkmediaMainWidget::volumeChangedComboboxChangeSlot(int index)
{
    QString sound_name = m_pSoundList->at(index);
//    updateAlert(this,sound_name.toLatin1().data());
    playAlretSoundFromPath(this,sound_name);

    QString fileName = m_pSoundList->at(index);
    QStringList list = fileName.split("/");
    QString soundName = list.at(list.count()-1);
    QStringList eventIdList = soundName.split(".");
    QString eventId = eventIdList.at(0);
    QList<char *> existsPath = listExistsPath();
    for (char * path : existsPath) {

        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);
        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba))
        {
            QGSettings * settings = new QGSettings(ba, bba);
//            QString filenameStr = settings->get(FILENAME_KEY).toString();
            QString nameStr = settings->get(NAME_KEY).toString();
            if (nameStr == "volume-changed") {
                settings->set(FILENAME_KEY,eventId);
                return;
            }
        }
        else {
            continue;
        }
    }
}

void UkmediaMainWidget::settingMenuComboboxChangedSlot(int index)
{
    QString fileName = m_pSoundList->at(index);
    QStringList list = fileName.split("/");
    QString soundName = list.at(list.count()-1);
    QStringList eventIdList = soundName.split(".");
    QString eventId = eventIdList.at(0);
    QList<char *> existsPath = listExistsPath();
    for (char * path : existsPath) {

        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);
        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba))
        {
            QGSettings * settings = new QGSettings(ba, bba);
            QString nameStr = settings->get(NAME_KEY).toString();
            if (nameStr == "system-setting") {
                settings->set(FILENAME_KEY,eventId);
                return;
            }
        }
        else {
            continue;
        }
    }
}

/*
    点击输入音量按钮静音
*/
void UkmediaMainWidget::inputMuteButtonSlot()
{
     m_pVolumeControl->setSourceMute(!m_pVolumeControl->sourceMuted);
     inputVolumeDarkThemeImage(paVolumeToValue(m_pVolumeControl->sourceVolume),!m_pVolumeControl->sourceMuted);
     m_pOutputWidget->m_pOutputIconBtn->repaint();
}

/*
    点击输出音量按钮静音
*/
void UkmediaMainWidget::outputMuteButtonSlot()
{
     m_pVolumeControl->setSinkMute(!m_pVolumeControl->sinkMuted);

     outputVolumeDarkThemeImage(paVolumeToValue(m_pVolumeControl->sinkVolume),!m_pVolumeControl->sinkMuted);
     m_pOutputWidget->m_pOutputIconBtn->repaint();
}


/*
    点击声音主题实现主题切换
*/
void UkmediaMainWidget::themeComboxIndexChangedSlot(int index)
{
    Q_UNUSED(index);
    g_debug("theme combox index changed slot");
    if (index == -1) {
        return;
    }
    //设置系统主题
    QString theme = m_pThemeNameList->at(index);
    QByteArray ba = theme.toLatin1();
    const char *m_pThemeName = ba.data();

    if (strcmp(m_pThemeName,"freedesktop") == 0) {
        int index = 0;
        for (int i=0;i<m_pSoundList->count();i++) {
            QString str = m_pSoundList->at(i);
            if (str.contains("gudou",Qt::CaseSensitive)) {
                index = i;
                break;
            }
        }

        QString displayName = m_pSoundNameList->at(index);
        m_pSoundWidget->m_pAlertSoundCombobox->setCurrentText(displayName);
    }

    QString dirName = m_pSoundThemeDirList->at(index);
    int themeIndex =  m_pSoundThemeList->indexOf(m_pThemeName);
    if (themeIndex < 0 )
        return;
    //qDebug() << "index changed:" << m_pSoundThemeXmlNameList->at(themeIndex) << m_pThemeNameList->at(index) << m_pThemeName << dirName.toLatin1().data() ;//<< path;
    QString xmlName = m_pSoundThemeXmlNameList->at(themeIndex);
    const gchar *path = g_build_filename (dirName.toLatin1().data(), xmlName.toLatin1().data(), nullptr);
    m_pSoundList->clear();
    m_pSoundNameList->clear();
    m_pSoundWidget->m_pAlertSoundCombobox->blockSignals(true);
    m_pSoundWidget->m_pLagoutCombobox->blockSignals(true);
    m_pSoundWidget->m_pVolumeChangeCombobox->blockSignals(true);
    m_pSoundWidget->m_pAlertSoundCombobox->clear();
    m_pSoundWidget->m_pLagoutCombobox->clear();
    m_pSoundWidget->m_pVolumeChangeCombobox->clear();
    m_pSoundWidget->m_pAlertSoundCombobox->blockSignals(false);
    m_pSoundWidget->m_pLagoutCombobox->blockSignals(false);
    m_pSoundWidget->m_pVolumeChangeCombobox->blockSignals(false);
    populateModelFromFile (this, path);

    /* special case for no sounds */
    if (strcmp (m_pThemeName, NO_SOUNDS_THEME_NAME) == 0) {
        //设置提示音关闭
        g_settings_set_boolean (m_pSoundSettings, EVENT_SOUNDS_KEY, FALSE);
        return;
    } else {
        g_settings_set_boolean (m_pSoundSettings, EVENT_SOUNDS_KEY, TRUE);
    }

}

/*
    滚动输出音量滑动条
*/
void UkmediaMainWidget::outputWidgetSliderChangedSlot(int value)
{
    int volume = valueToPaVolume(value);
    m_pVolumeControl->getDefaultSinkIndex();
    m_pVolumeControl->setSinkVolume(m_pVolumeControl->sinkIndex,volume);
    qDebug() << "outputWidgetSliderChangedSlot" << value <<volume;

    QString percent;
    bool status = false;
    percent = QString::number(value);
    outputVolumeDarkThemeImage(value,status);
    percent.append("%");
    m_pOutputWidget->m_pOpVolumePercentLabel->setText(percent);
    m_pOutputWidget->m_pOutputIconBtn->repaint();

}

void UkmediaMainWidget::timeSliderSlot()
{
    if(mouseReleaseState){
        int value = m_pOutputWidget->m_pOpVolumeSlider->value();

        QString percent;
        bool status = false;
        percent = QString::number(value);
        int volume = value*65536/100;
        if (value <= 0) {
            status = true;

            percent = QString::number(0);
        }
        else {
            if (firstEnterSystem) {

            }
            else {

            }
        }
        firstEnterSystem = false;
        outputVolumeDarkThemeImage(value,status);
        percent.append("%");
        m_pOutputWidget->m_pOpVolumePercentLabel->setText(percent);
        m_pOutputWidget->m_pOutputIconBtn->repaint();

        mouseReleaseState = false;
        mousePress = false;
        timeSlider->stop();
    }
    else{
        timeSlider->start(50);
    }
}

/*
    滚动输入滑动条
*/
void UkmediaMainWidget::inputWidgetSliderChangedSlot(int value)
{
    int volume = valueToPaVolume(value);
    m_pVolumeControl->setSourceVolume(m_pVolumeControl->sourceIndex,volume);
    //输入图标修改成深色主题

    inputVolumeDarkThemeImage(value,m_pVolumeControl->sourceMuted);
    m_pInputWidget->m_pInputIconBtn->repaint();
    QString percent = QString::number(value);
    value = value * 65536 / 100;
    percent.append("%");
    m_pInputWidget->m_pInputIconBtn->repaint();
    m_pInputWidget->m_pIpVolumePercentLabel->setText(percent);
}

/*
 *  平衡值改变
 */
void UkmediaMainWidget::balanceSliderChangedSlot(int value)
{
    gdouble volume = value/100.0;
    value = valueToPaVolume(m_pOutputWidget->m_pOpVolumeSlider->value());
    m_pVolumeControl->setBalanceVolume(m_pVolumeControl->sinkIndex,value,volume);
    qDebug() << "balanceSliderChangedSlot" <<value;
}

/*
 * 输入等级
 */
void UkmediaMainWidget::peakVolumeChangedSlot(double v)
{
    if (v >= 0) {
        m_pInputWidget->m_pInputLevelProgressBar->setEnabled(true);
        int value = qRound(v * m_pInputWidget->m_pInputLevelProgressBar->maximum());
        m_pInputWidget->m_pInputLevelProgressBar->setValue(value);
    } else {
        m_pInputWidget->m_pInputLevelProgressBar->setEnabled(false);
        m_pInputWidget->m_pInputLevelProgressBar->setValue(0);
    }
}

/*
 * 更新设备端口
 */
void UkmediaMainWidget::updateDevicePort()
{
    QMap<int,QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> temp;
    currentInputPortLabelMap.clear();
    currentOutputPortLabelMap.clear();
    if (firstEntry == true) {
        for(it = m_pVolumeControl->outputPortMap.begin();it!=m_pVolumeControl->outputPortMap.end();)
        {
            temp = it.value();
            for (at=temp.begin();at!=temp.end();) {
                qDebug() << "updateDevicePort" << firstEntry << it.key() << at.value();
                QString cardName = findCardName(it.key(),m_pVolumeControl->cardMap);
                addOutputListWidgetItem(at.value(),cardName);
                ++at;
            }
            ++it;
        }
        for(it = m_pVolumeControl->inputPortMap.begin();it!=m_pVolumeControl->inputPortMap.end();)
        {
            temp = it.value();
            for (at=temp.begin();at!=temp.end();) {
                qDebug() << "updateDevicePort" << firstEntry << it.key() << at.value();
                QString cardName = findCardName(it.key(),m_pVolumeControl->cardMap);
                addInputListWidgetItem(at.value(),cardName);
                ++at;
            }
            ++it;
        }

    }
    else {
        //记录上一次output label
        for (int i=0;i<m_pOutputWidget->m_pOutputListWidget->count();i++) {
            QMap<int,QString>::iterator at;
            QListWidgetItem *item = m_pOutputWidget->m_pOutputListWidget->item(i);
            UkuiListWidgetItem *wid = (UkuiListWidgetItem *)m_pOutputWidget->m_pOutputListWidget->itemWidget(item);
            int index;
            for (at=m_pVolumeControl->cardMap.begin();at!=m_pVolumeControl->cardMap.end();) {
                if (wid->deviceLabel->text() == at.value()) {
                    index = at.key();
                    break;
                }
                ++at;
            }
            currentOutputPortLabelMap.insertMulti(index,wid->portLabel->text());
            qDebug() << index << "current output item ************" << wid->deviceLabel->text() <<wid->portLabel->text() ;//<< w->m_pOutputPortLabelList->at(i);
        }

        for (int i=0;i<m_pInputWidget->m_pInputListWidget->count();i++) {

            QListWidgetItem *item = m_pInputWidget->m_pInputListWidget->item(i);
            UkuiListWidgetItem *wid = (UkuiListWidgetItem *)m_pInputWidget->m_pInputListWidget->itemWidget(item);
            int index;
            int count;
            QMap<int,QString>::iterator at;
            for (at=m_pVolumeControl->cardMap.begin();at!=m_pVolumeControl->cardMap.end();) {
                if (wid->deviceLabel->text() == at.value()) {
                    index = at.key();
                    break;
                }
                ++at;
                ++count;
            }
            currentInputPortLabelMap.insertMulti(index,wid->portLabel->text());
        }
        m_pInputWidget->m_pInputListWidget->blockSignals(true);
        deleteNotAvailableOutputPort();
        addAvailableOutputPort();
        deleteNotAvailableInputPort();
        addAvailableInputPort();
        m_pInputWidget->m_pInputListWidget->blockSignals(false);
    }
    if (m_pOutputWidget->m_pOutputListWidget->count() > 0 || m_pInputWidget->m_pInputListWidget->count()) {

        firstEntry = false;
    }
}

void UkmediaMainWidget::updateListWidgetItemSlot()
{
    qDebug() << "updateListWidgetItemSlot---------";
    if (!pressOutputListWidget)
        initOutputListWidgetItem();
    else
        pressOutputListWidget = false;
    initInputListWidgetItem();
    themeChangeIcons();
}

/*
 * output list widget选项改变，设置对应的输出设备
 */
void UkmediaMainWidget::outputListWidgetCurrentRowChangedSlot(int row)
{
    //当所有可用的输出设备全部移除，台式机才会出现该情况
    if (row == -1)
        return;
    QListWidgetItem *item = m_pOutputWidget->m_pOutputListWidget->item(row);
    if (item == nullptr) {
        qDebug() <<"output current item is null";
    }

    if (!pressOutputListWidget)
        pressOutputListWidget = true;
    UkuiListWidgetItem *wid = (UkuiListWidgetItem *)m_pOutputWidget->m_pOutputListWidget->itemWidget(item);
    QListWidgetItem *inputCurrrentItem = m_pInputWidget->m_pInputListWidget->currentItem();
    UkuiListWidgetItem *inputWid = (UkuiListWidgetItem *)m_pInputWidget->m_pInputListWidget->itemWidget(inputCurrrentItem);
    bool isContainBlue = inputDeviceContainBluetooth();

    //当输出设备从蓝牙切换到其他设备时，需将蓝牙声卡的配置文件切换为a2dp-sink
    if (isContainBlue && (strstr(m_pVolumeControl->defaultSourceName,"headset_head_unit") || strstr(m_pVolumeControl->defaultSourceName,"bt_sco_sink"))) {
        QString cardName = blueCardName();
        setCardProfile(cardName,"a2dp_sink");
    }

    QMap<int ,QMap<QString,QString>>::iterator outputProfileMap;
    QMap<int ,QMap<QString,QString>>::iterator inputProfileMap;
    QMap<QString,QString> tempMap;
    QMap<QString,QString>::iterator at;
    QString endOutputProfile = "";
    QString endInputProfile = "";

    int currentCardIndex = findCardIndex(wid->deviceLabel->text(),m_pVolumeControl->cardMap);
    for(outputProfileMap = m_pVolumeControl->profileNameMap.begin();outputProfileMap != m_pVolumeControl->profileNameMap.end();){
        if(outputProfileMap.key() == currentCardIndex){
            tempMap = outputProfileMap.value();
            for(at = tempMap.begin();at != tempMap.end();){
                if(at.key() == wid->portLabel->text())
                    endOutputProfile = at.value();

                ++at;
            }
        }
        ++outputProfileMap;
    }

    if (inputCurrrentItem != nullptr) {
        QMap <QString,QString>::iterator it;
        QMap <QString,QString> temp;
        int index = findCardIndex(inputWid->deviceLabel->text(),m_pVolumeControl->cardMap);
        for (inputProfileMap=m_pVolumeControl->inputPortProfileNameMap.begin();inputProfileMap!= m_pVolumeControl->inputPortProfileNameMap.end();) {
            if (inputProfileMap.key() == index) {
                temp = inputProfileMap.value();
                for(it = temp.begin(); it != temp.end();){
                    if(it.key() == inputWid->portLabel->text()){
                        endInputProfile = it.value();
                    }
                    ++it;
                }

            }
            ++inputProfileMap;
        }
    }
    qDebug() << "outputListWidgetCurrentRowChangedSlot" << row << wid->deviceLabel->text() << endOutputProfile <<endInputProfile;
    //如果选择的输入输出设备为同一个声卡，则追加指定输入输出端口属于的配置文件
    if ((inputCurrrentItem != nullptr && wid->deviceLabel->text() == inputWid->deviceLabel->text()) || \
        wid->deviceLabel->text() == "alsa_card.platform-sound_DA_combine_v5" && inputWid->deviceLabel->text() == "3a.algo") {

        QString setProfile = endOutputProfile;
        if (!endOutputProfile.contains("input:analog-stereo") || !endOutputProfile.contains("HiFi")) {
            setProfile += "+";
            setProfile +=endInputProfile;
        }

        setCardProfile(wid->deviceLabel->text(),setProfile);
        setDefaultOutputPortDevice(wid->deviceLabel->text(),wid->portLabel->text());
    }
    //如果选择的输入输出设备不是同一块声卡，需要设置一个优先级高的配置文件
    else {
        int index = findCardIndex(wid->deviceLabel->text(),m_pVolumeControl->cardMap);
        QMap <int,QList<QString>>::iterator it;
        QString profileName;
        for(it=m_pVolumeControl->cardProfileMap.begin();it!=m_pVolumeControl->cardProfileMap.end();) {
            if (it.key() == index) {
                if (strstr(endOutputProfile.toLatin1().data(),"headset_head_unit"))
                    endOutputProfile = "a2dp_sink";
                profileName = findHighPriorityProfile(index,endOutputProfile);
                break;
            }
            ++it;
        }

        QString setProfile = profileName;
        setCardProfile(wid->deviceLabel->text(),setProfile);
        setDefaultOutputPortDevice(wid->deviceLabel->text(),wid->portLabel->text());
    }
}

/*
 * input list widget选项改变，设置对应的输入设备
 */
void UkmediaMainWidget::inputListWidgetCurrentRowChangedSlot(int row)
{
    //当所有可用的输入设备全部移除，台式机才会出现该情况
    if (row == -1)
        return;
    QListWidgetItem *item = m_pInputWidget->m_pInputListWidget->item(row);
    UkuiListWidgetItem *wid = (UkuiListWidgetItem *)m_pInputWidget->m_pInputListWidget->itemWidget(item);
    QListWidgetItem *outputCurrrentItem = m_pOutputWidget->m_pOutputListWidget->currentItem();
    UkuiListWidgetItem *outputWid = (UkuiListWidgetItem *)m_pOutputWidget->m_pOutputListWidget->itemWidget(outputCurrrentItem);

    bool isContainBlue = inputDeviceContainBluetooth();
    qDebug() << "inputListWidgetCurrentRowChangedSlot" << row << isContainBlue << m_pVolumeControl->defaultSourceName;
    //当输出设备从蓝牙切换到其他设备时，需将蓝牙声卡的配置文件切换为a2dp-sink
    if (isContainBlue && (strstr(m_pVolumeControl->defaultSinkName,"headset_head_unit") || strstr(m_pVolumeControl->defaultSourceName,"bt_sco_source"))) {
        QString cardName = blueCardName();
        setCardProfile(cardName,"a2dp_sink");
    }

    if(wid->deviceLabel->text().contains("bluez_card")) {
        isCheckBluetoothInput = true;
    }
    else {
        isCheckBluetoothInput = false;
    }

    QMap<int, QMap<QString,QString>>::iterator inputProfileMap;
    QMap<int, QMap<QString,QString>>::iterator outputProfileMap;
    QMap <QString,QString> temp;
    QMap<QString,QString>::iterator at;
    QString endOutputProfile = "";
    QString endInputProfile = "";

    int index = findCardIndex(wid->deviceLabel->text(),m_pVolumeControl->cardMap);
    for (inputProfileMap=m_pVolumeControl->inputPortProfileNameMap.begin();inputProfileMap!= m_pVolumeControl->inputPortProfileNameMap.end();) {
        if (inputProfileMap.key() == index) {
            temp = inputProfileMap.value();
            for(at=temp.begin();at!=temp.end();){
                if(at.key() == wid->portLabel->text()){
                    endInputProfile = at.value();
                }
                ++at;
            }
        }
        ++inputProfileMap;
    }
    if (outputCurrrentItem != nullptr) {
        QMap<QString,QString> tempMap;
        QMap<QString,QString>::iterator at;
        int index = findCardIndex(outputWid->deviceLabel->text(),m_pVolumeControl->cardMap);
        for(outputProfileMap = m_pVolumeControl->profileNameMap.begin(); outputProfileMap != m_pVolumeControl->profileNameMap.end();){
            if(outputProfileMap.key() == index){
                tempMap = outputProfileMap.value();
                for(at = tempMap.begin(); at != tempMap.end();){
                    if(outputWid->portLabel->text() == at.key()){
                        endOutputProfile = at.value();
                    }
                    ++at;
                }
            }
            ++outputProfileMap;
        }
    }

    //如果选择的输入输出设备为同一个声卡，则追加指定输入输出端口属于的配置文件
    if (outputCurrrentItem != nullptr && wid->deviceLabel->text() == outputWid->deviceLabel->text()) {
        QString  setProfile;
        //有些声卡的配置文件默认只有输入/输出设备或者配置文件包含了输出输入设备，因此只需要取其中一个配置文件即可
        if (endOutputProfile == "a2dp-sink" || endInputProfile == "headset_head_unit" || endOutputProfile == "HiFi" ) {
            setProfile += endInputProfile;
        }
        else {
            setProfile += endOutputProfile;
            setProfile += "+";
            setProfile +=endInputProfile;
        }
        setCardProfile(wid->deviceLabel->text(),setProfile);
        setDefaultInputPortDevice(wid->deviceLabel->text(),wid->portLabel->text());
    }
    //如果选择的输入输出设备不是同一块声卡，需要设置一个优先级高的配置文件
    else {
        int index = findCardIndex(wid->deviceLabel->text(),m_pVolumeControl->cardMap);
        QMap <int,QList<QString>>::iterator it;
        QString profileName;
        for(it=m_pVolumeControl->cardProfileMap.begin();it!=m_pVolumeControl->cardProfileMap.end();) {

            if (it.key() == index) {
                QStringList list= it.value();
                profileName = findHighPriorityProfile(index,endInputProfile);
                if (list.contains(endOutputProfile)) {

                }
            }
            ++it;
        }
        QString  setProfile = profileName;
        setCardProfile(wid->deviceLabel->text(),setProfile);
        setDefaultInputPortDevice(wid->deviceLabel->text(),wid->portLabel->text());
    }
    m_pVolumeControl->getDefaultSinkIndex();
    QTimer::singleShot(100, this, [=](){

        int vol = m_pVolumeControl->getSourceVolume();
        m_pInputWidget->m_pIpVolumeSlider->blockSignals(true);
        m_pInputWidget->m_pIpVolumeSlider->setValue(paVolumeToValue(vol));
        m_pInputWidget->m_pIpVolumeSlider->blockSignals(false);
        m_pInputWidget->m_pIpVolumePercentLabel->setText(QString::number(paVolumeToValue(vol))+"%");
        qDebug() << "active output port:" << wid->portLabel->text() <<vol;
    });
    qDebug() << "active input port:" << wid->portLabel->text() << isCheckBluetoothInput;
}

gboolean UkmediaMainWidget::saveAlertSounds (QComboBox *combox,const char *id)
{
    const char *sounds[3] = { "bell-terminal", "bell-window-system", NULL };
    char *path;

    if (strcmp (id, DEFAULT_ALERT_ID) == 0) {
        deleteOldFiles (sounds);
        deleteDisabledFiles (sounds);
    } else {
        deleteOldFiles (sounds);
        deleteDisabledFiles (sounds);
        addCustomFile (sounds, id);
    }

    /* And poke the directory so the theme gets updated */
    path = customThemeDirPath(NULL);
    if (utime (path, NULL) != 0) {
        g_warning ("Failed to update mtime for directory '%s': %s",
                   path, g_strerror (errno));
    }
    g_free (path);

    return FALSE;
}

void UkmediaMainWidget::deleteOldFiles (const char **sounds)
{
    guint i;
    for (i = 0; sounds[i] != NULL; i++) {
        deleteOneFile (sounds[i], "%s.ogg");
    }
}

void UkmediaMainWidget::deleteOneFile (const char *sound_name, const char *pattern)
{
        GFile *file;
        char *name, *filename;

        name = g_strdup_printf (pattern, sound_name);
        filename = customThemeDirPath(name);
        g_free (name);
        file = g_file_new_for_path (filename);
        g_free (filename);
        cappletFileDeleteRecursive (file, NULL);
        g_object_unref (file);
}

void UkmediaMainWidget::deleteDisabledFiles (const char **sounds)
{
    guint i;
    for (i = 0; sounds[i] != NULL; i++) {
        deleteOneFile (sounds[i], "%s.disabled");
    }
}

void UkmediaMainWidget::addCustomFile (const char **sounds, const char *filename)
{
    guint i;

    for (i = 0; sounds[i] != NULL; i++) {
        GFile *file;
        char *name, *path;

        /* We use *.ogg because it's the first type of file that
                 * libcanberra looks at */
        name = g_strdup_printf ("%s.ogg", sounds[i]);
        path = customThemeDirPath(name);
        g_free (name);
        /* In case there's already a link there, delete it */
        g_unlink (path);
        file = g_file_new_for_path (path);
        g_free (path);

        /* Create the link */
        g_file_make_symbolic_link (file, filename, NULL, NULL);
        g_object_unref (file);
    }
}

/**
 * capplet_file_delete_recursive :
 * @file :
 * @error  :
 *
 * A utility routine to delete files and/or directories,
 * including non-empty directories.
 **/
gboolean UkmediaMainWidget::cappletFileDeleteRecursive (GFile *file, GError **error)
{
    GFileInfo *info;
    GFileType type;

    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    info = g_file_query_info (file,
                              G_FILE_ATTRIBUTE_STANDARD_TYPE,
                              G_FILE_QUERY_INFO_NONE,
                              NULL, error);
    if (info == NULL) {
        return FALSE;
    }

    type = g_file_info_get_file_type (info);
    g_object_unref (info);

    if (type == G_FILE_TYPE_DIRECTORY) {
        return directoryDeleteRecursive (file, error);
    }
    else {
        return g_file_delete (file, NULL, error);
    }
}

gboolean UkmediaMainWidget::directoryDeleteRecursive (GFile *directory, GError **error)
{
    GFileEnumerator *enumerator;
    GFileInfo *info;
    gboolean success = TRUE;

    enumerator = g_file_enumerate_children (directory,
                                            G_FILE_ATTRIBUTE_STANDARD_NAME ","
                                            G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                            G_FILE_QUERY_INFO_NONE,
                                            NULL, error);
    if (enumerator == NULL)
        return FALSE;

    while (success &&
           (info = g_file_enumerator_next_file (enumerator, NULL, NULL))) {
        GFile *child;

        child = g_file_get_child (directory, g_file_info_get_name (info));

        if (g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY) {
            success = directoryDeleteRecursive (child, error);
        }
        g_object_unref (info);

        if (success)
            success = g_file_delete (child, NULL, error);
    }
    g_file_enumerator_close (enumerator, NULL, NULL);

    if (success)
        success = g_file_delete (directory, NULL, error);

    return success;
}

void UkmediaMainWidget::createCustomTheme (const char *parent)
{
    GKeyFile *keyfile;
    char     *data;
    char     *path;

    /* Create the custom directory */
    path = customThemeDirPath(NULL);
    g_mkdir_with_parents (path, 0755);
    g_free (path);

    /* Set the data for index.theme */
    keyfile = g_key_file_new ();
    g_key_file_set_string (keyfile, "Sound Theme", "Name", _("Custom"));
    g_key_file_set_string (keyfile, "Sound Theme", "Inherits", parent);
    g_key_file_set_string (keyfile, "Sound Theme", "Directories", ".");
    data = g_key_file_to_data (keyfile, NULL, NULL);
    g_key_file_free (keyfile);

    /* Save the index.theme */
    path = customThemeDirPath ("index.theme");
    g_file_set_contents (path, data, -1, NULL);
    g_free (path);
    g_free (data);

    customThemeUpdateTime ();
}

/* This function needs to be called after each individual
 * changeset to the theme */
void UkmediaMainWidget::customThemeUpdateTime (void)
{
    char *path;
    path = customThemeDirPath (NULL);
    utime (path, NULL);
    g_free (path);
}

gboolean UkmediaMainWidget::customThemeDirIsEmpty (void)
{
    char            *dir;
    GFile           *file;
    gboolean         is_empty;
    GFileEnumerator *enumerator;
    GFileInfo       *info;
    GError          *error = NULL;

    dir = customThemeDirPath(NULL);
    file = g_file_new_for_path (dir);
    g_free (dir);

    is_empty = TRUE;

    enumerator = g_file_enumerate_children (file,
                                            G_FILE_ATTRIBUTE_STANDARD_NAME ","
                                            G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                            G_FILE_QUERY_INFO_NONE,
                                            NULL, &error);
    if (enumerator == NULL) {
        g_warning ("Unable to enumerate files: %s", error->message);
        g_error_free (error);
        goto out;
    }

    while (is_empty &&
           (info = g_file_enumerator_next_file (enumerator, NULL, NULL))) {

        if (strcmp ("index.theme", g_file_info_get_name (info)) != 0) {
            is_empty = FALSE;
        }

        g_object_unref (info);
    }
    g_file_enumerator_close (enumerator, NULL, NULL);

out:
    g_object_unref (file);

    return is_empty;
}

int UkmediaMainWidget::caPlayForWidget(UkmediaMainWidget *w, uint32_t id, ...)
{
    va_list ap;
    int ret;
    ca_proplist *p;

    if ((ret = ca_proplist_create(&p)) < 0)
        return ret;

    if ((ret = caProplistSetForWidget(p, w)) < 0)
        return -1;

    va_start(ap, id);
    ret = caProplistMergeAp(p, ap);
    va_end(ap);

    if (ret < 0)
        return -1;
    ca_context *c ;
    ca_context_create(&c);
    ret = ca_context_play_full(c, id, p, NULL, NULL);

    return ret;
}

int UkmediaMainWidget::caProplistMergeAp(ca_proplist *p, va_list ap)
{
    int ret;
    for (;;) {
        const char *key, *value;

        if (!(key = va_arg(ap, const char*)))
            break;

        if (!(value = va_arg(ap, const char*)))
            return CA_ERROR_INVALID;

        if ((ret = ca_proplist_sets(p, key, value)) < 0)
            return ret;
    }

    return CA_SUCCESS;
}

int UkmediaMainWidget::caProplistSetForWidget(ca_proplist *p, UkmediaMainWidget *widget)
{
    int ret;
    const char *t;
    QScreen *screen;
    gint x = -1;
    gint y = -1;
    gint width = -1;
    gint height = -1;
    gint screen_width = -1;
    gint screen_height = -1;

    if ((t = widget->windowTitle().toLatin1().data()))
        if ((ret = ca_proplist_sets(p, CA_PROP_WINDOW_NAME, t)) < 0)
            return ret;

    if (t)
        if ((ret = ca_proplist_sets(p, CA_PROP_WINDOW_ID, t)) < 0)
            return ret;

    if ((t = widget->windowIconText().toLatin1().data()))
        if ((ret = ca_proplist_sets(p, CA_PROP_WINDOW_ICON_NAME, t)) < 0)
            return ret;
    if (screen = qApp->primaryScreen()) {
        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_X11_SCREEN, "%i", 0)) < 0)
            return ret;
    }

    width = widget->size().width();
    height = widget->size().height();

    if (width > 0)
        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_WIDTH, "%i", width)) < 0)
            return ret;
    if (height > 0)
        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_HEIGHT, "%i", height)) < 0)
            return ret;

    if (x >= 0 && width > 0) {
        screen_width = qApp->primaryScreen()->size().width();

        x += width/2;
        x = CA_CLAMP(x, 0, screen_width-1);

        /* We use these strange format strings here to avoid that libc
                         * applies locale information on the formatting of floating
                         * numbers. */

        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_HPOS, "%i.%03i",
                                    (int) (x/(screen_width-1)), (int) (1000.0*x/(screen_width-1)) % 1000)) < 0)
            return ret;
    }

    if (y >= 0 && height > 0) {
        screen_height = qApp->primaryScreen()->size().height();

        y += height/2;
        y = CA_CLAMP(y, 0, screen_height-1);

        if ((ret = ca_proplist_setf(p, CA_PROP_WINDOW_VPOS, "%i.%03i",
                                    (int) (y/(screen_height-1)), (int) (1000.0*y/(screen_height-1)) % 1000)) < 0)
            return ret;
    }
    return CA_SUCCESS;
}

UkmediaMainWidget::~UkmediaMainWidget()
{
}

/*
 * 添加output port到output list widget
 */
void UkmediaMainWidget::addOutputListWidgetItem(QString portName, QString cardName)
{
    UkuiListWidgetItem *itemW = new UkuiListWidgetItem(this);
    int i = m_pOutputWidget->m_pOutputListWidget->count();
    QListWidgetItem * item = new QListWidgetItem(m_pOutputWidget->m_pOutputListWidget);
    item->setSizeHint(QSize(200,64)); //QSize(120, 40) spacing: 12px;
    m_pOutputWidget->m_pOutputListWidget->blockSignals(true);
    m_pOutputWidget->m_pOutputListWidget->setItemWidget(item, itemW);
    m_pOutputWidget->m_pOutputListWidget->blockSignals(false);
    itemW->setLabelText(portName,cardName);
    m_pOutputWidget->m_pOutputListWidget->blockSignals(true);
    m_pOutputWidget->m_pOutputListWidget->insertItem(i,item);
    m_pOutputWidget->m_pOutputListWidget->blockSignals(false);
}

/*
 * 添加input port到input list widget
 */
void UkmediaMainWidget::addInputListWidgetItem(QString portName, QString cardName)
{
    UkuiListWidgetItem *itemW = new UkuiListWidgetItem(this);
    int i = m_pInputWidget->m_pInputListWidget->count();
    QListWidgetItem * item = new QListWidgetItem(m_pInputWidget->m_pInputListWidget);
    item->setSizeHint(QSize(200,64)); //QSize(120, 40) spacing: 12px;
    m_pInputWidget->m_pInputListWidget->setItemWidget(item, itemW);
    itemW->setLabelText(portName,cardName);
    m_pInputWidget->m_pInputListWidget->blockSignals(true);
    m_pInputWidget->m_pInputListWidget->insertItem(i,item);
    m_pInputWidget->m_pInputListWidget->blockSignals(false);
}

/*
 * 移除output list widget上不可用的输出端口
 */
void UkmediaMainWidget::deleteNotAvailableOutputPort()
{
    qDebug() << "deleteNotAvailableOutputPort";
    //删除不可用的输出端口
    QMap<int,QString>::iterator it;
    for(it=currentOutputPortLabelMap.begin();it!=currentOutputPortLabelMap.end();) {
        //没找到，需要删除
        if (outputPortIsNeedDelete(it.key(),it.value())) {
            int index = indexOfOutputPortInOutputListWidget(it.key(),it.value());
            if (index == -1)
                return;
            m_pOutputWidget->m_pOutputListWidget->blockSignals(true);
            QListWidgetItem *item = m_pOutputWidget->m_pOutputListWidget->takeItem(index);
            m_pOutputWidget->m_pOutputListWidget->removeItemWidget(item);
            m_pOutputWidget->m_pOutputListWidget->blockSignals(false);
            it = currentOutputPortLabelMap.erase(it);
            continue;
        }
        ++it;
    }
//    m_pVolumeControl->removeProfileMap();

}

/*
 * 在input list widget删除不可用的端口
 */
void UkmediaMainWidget::deleteNotAvailableInputPort()
{
    //删除不可用的输入端口
    QMap<int,QString>::iterator it;
    for(it=currentInputPortLabelMap.begin();it!=currentInputPortLabelMap.end();) {
        //没找到，需要删除
        if (inputPortIsNeedDelete(it.key(),it.value())) {
            int index = indexOfInputPortInInputListWidget(it.key(),it.value());
            if (index == -1)
                return;
            m_pInputWidget->m_pInputListWidget->blockSignals(true);
            QListWidgetItem *item = m_pInputWidget->m_pInputListWidget->takeItem(index);
            m_pInputWidget->m_pInputListWidget->removeItemWidget(item);
            m_pInputWidget->m_pInputListWidget->blockSignals(false);
            it = currentInputPortLabelMap.erase(it);
            continue;
        }
        ++it;
    }
//    m_pVolumeControl->removeInputProfile();
}

/*
 * 添加可用的输出端口到output list widget
 */
void UkmediaMainWidget::addAvailableOutputPort()
{
    QMap<int,QMap<QString,QString>>::iterator at;
    QMap<QString,QString>::iterator it;
    QMap<QString,QString> tempMap;
    int i = m_pOutputWidget->m_pOutputListWidget->count();
    //增加端口
    for(at=m_pVolumeControl->outputPortMap.begin();at!=m_pVolumeControl->outputPortMap.end();)
    {
        tempMap = at.value();
        for (it=tempMap.begin();it!=tempMap.end();) {
            //需添加到list widget
            if (outputPortIsNeedAdd(at.key(),it.value())) {
                qDebug() << "add output list widget" << at.key()<< it.value();
                UkuiListWidgetItem *itemW = new UkuiListWidgetItem(this);

                QListWidgetItem * item = new QListWidgetItem(m_pOutputWidget->m_pOutputListWidget);
                item->setSizeHint(QSize(200,64)); //QSize(120, 40) spacing: 12px;
                m_pOutputWidget->m_pOutputListWidget->blockSignals(true);
                m_pOutputWidget->m_pOutputListWidget->setItemWidget(item, itemW);
                m_pOutputWidget->m_pOutputListWidget->blockSignals(false);

                itemW->setLabelText(it.value(),findCardName(at.key(),m_pVolumeControl->cardMap));
                currentOutputPortLabelMap.insertMulti(at.key(),it.value());
                m_pOutputWidget->m_pOutputListWidget->blockSignals(true);
                m_pOutputWidget->m_pOutputListWidget->insertItem(i,item);
                m_pOutputWidget->m_pOutputListWidget->blockSignals(false);
            }
            ++it;
        }
        ++at;
    }
}

/*
 *  添加可用的输入端口到input list widget
 */
void UkmediaMainWidget::addAvailableInputPort()
{
    QMap<int,QMap<QString,QString>>::iterator at;
    QMap<QString,QString>::iterator it;
    QMap<QString,QString> tempMap;
    int i = m_pInputWidget->m_pInputListWidget->count();
    //增加端口
    for(at=m_pVolumeControl->inputPortMap.begin();at!=m_pVolumeControl->inputPortMap.end();)
    {
        tempMap = at.value();
        for (it=tempMap.begin();it!=tempMap.end();) {
            //需添加到list widget
            if (inputPortIsNeedAdd(at.key(),it.value())&&it.value()!="电话") {
                qDebug() << "add input list widget" << at.key()<< it.value() <<at.value();
                UkuiListWidgetItem *itemW = new UkuiListWidgetItem(this);
                QListWidgetItem * item = new QListWidgetItem(m_pInputWidget->m_pInputListWidget);
                item->setSizeHint(QSize(200,64)); //QSize(120, 40) spacing: 12px;
                m_pInputWidget->m_pInputListWidget->blockSignals(true);
                m_pInputWidget->m_pInputListWidget->setItemWidget(item, itemW);
                m_pInputWidget->m_pInputListWidget->blockSignals(false);

                itemW->setLabelText(it.value(),findCardName(at.key(),m_pVolumeControl->cardMap));
                currentInputPortLabelMap.insertMulti(at.key(),it.value());
                m_pInputWidget->m_pInputListWidget->blockSignals(true);
                m_pInputWidget->m_pInputListWidget->insertItem(i,item);
                m_pInputWidget->m_pInputListWidget->blockSignals(false);
            }
            ++it;
        }
        ++at;
    }
}

/*
 * 当前的输出端口是否应该在output list widget上删除
 */
bool UkmediaMainWidget::outputPortIsNeedDelete(int index, QString name)
{
    QMap<int,QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> portMap;
    for(it = m_pVolumeControl->outputPortMap.begin();it!=m_pVolumeControl->outputPortMap.end();)
    {
        if (it.key() == index) {
            portMap = it.value();
            for (at=portMap.begin();at!=portMap.end();) {
                if (name == at.value()) {
                    return false;
                }
                ++at;
            }
        }
        ++it;
    }
    return true;
}

/*
 * 当前的输出端口是否应该添加到output list widget上
 */
bool UkmediaMainWidget::outputPortIsNeedAdd(int index, QString name)
{
    QMap<int,QString>::iterator it;
    for(it=currentOutputPortLabelMap.begin();it!=currentOutputPortLabelMap.end();) {
        if ( index == it.key() && name == it.value()) {
            return false;
        }
        ++it;
    }
    return true;
}

/*
 * 当前的输出端口是否应该在input list widget上删除
 */
bool UkmediaMainWidget::inputPortIsNeedDelete(int index, QString name)
{
    QMap<int,QMap<QString,QString>>::iterator it;
    QMap<QString,QString>::iterator at;
    QMap<QString,QString> portMap;
    for(it = m_pVolumeControl->inputPortMap.begin();it!=m_pVolumeControl->inputPortMap.end();)
    {
        if (it.key() == index) {
            portMap = it.value();
            for (at=portMap.begin();at!=portMap.end();) {
                if (name == at.value()) {
                    return false;
                }
                ++at;
            }
        }
        ++it;
    }
    return true;
}

/*
 * 当前的输出端口是否应该添加到input list widget上
 */
bool UkmediaMainWidget::inputPortIsNeedAdd(int index, QString name)
{
    QMap<int,QString>::iterator it;
    for(it=currentInputPortLabelMap.begin();it!=currentInputPortLabelMap.end();) {
        if ( index == it.key() && name == it.value()) {
            return false;
        }
        ++it;
    }
    return true;
}

//查找指定声卡名的索引
int UkmediaMainWidget::findCardIndex(QString cardName, QMap<int,QString> cardMap)
{
    QMap<int, QString>::iterator it;

    for(it=cardMap.begin();it!=cardMap.end();) {
        if (it.value() == cardName) {
            return it.key();
        }
        ++it;
    }
    return -1;
}

/*
 * 根据声卡索引查找声卡名
 */
QString UkmediaMainWidget::findCardName(int index,QMap<int,QString> cardMap)
{
    QMap<int, QString>::iterator it;

    for(it=cardMap.begin();it!=cardMap.end();) {
        if (it.key() == index) {
            return it.value();
        }
        ++it;
    }
    return "";
}

/*
    查找名称为PortLbael 的portName
*/
QString UkmediaMainWidget::findOutputPortName(int index,QString portLabel)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>portMap;
    QMap<QString,QString>::iterator tempMap;
    QString portName = "";
    for (it = m_pVolumeControl->outputPortMap.begin();it != m_pVolumeControl->outputPortMap.end();) {
        if (it.key() == index) {
            portMap = it.value();
            for (tempMap = portMap.begin();tempMap!=portMap.end();) {
                if (tempMap.value() == portLabel) {
                    portName = tempMap.key();
                    break;
                }
                ++tempMap;
            }
        }
        ++it;
    }
    return portName;
}

/*
    查找名称为PortName 的portLabel
*/
QString UkmediaMainWidget::findOutputPortLabel(int index,QString portName)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>portMap;
    QMap<QString,QString>::iterator tempMap;
    QString portLabel = "";
    for (it = m_pVolumeControl->outputPortMap.begin();it != m_pVolumeControl->outputPortMap.end();) {
        if (it.key() == index) {
            portMap = it.value();
            for (tempMap = portMap.begin();tempMap!=portMap.end();) {
                qDebug() <<"findOutputPortLabel" <<portName <<tempMap.key() <<tempMap.value();
                if (tempMap.key() == portName) {
                    portLabel = tempMap.value();
                    break;
                }
                ++tempMap;
            }
        }
        ++it;
    }
    return portLabel;
}

/*
    查找名称为PortLbael 的portName
*/
QString UkmediaMainWidget::findInputPortName(int index,QString portLabel)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>portMap;
    QMap<QString,QString>::iterator tempMap;
    QString portName = "";
    for (it = m_pVolumeControl->inputPortMap.begin();it != m_pVolumeControl->inputPortMap.end();) {
        if (it.key() == index) {
            portMap = it.value();
            for (tempMap = portMap.begin();tempMap!=portMap.end();) {
                if (tempMap.value() == portLabel) {
                    portName = tempMap.key();
                    break;
                }
                ++tempMap;
            }
        }
        ++it;
    }
    return portName;
}

/*
    查找名称为PortName 的portLabel
*/
QString UkmediaMainWidget::findInputPortLabel(int index,QString portName)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString>portMap;
    QMap<QString,QString>::iterator tempMap;
    QString portLabel = "";

    /* fixed bug:112415 宝新创PF215T */
    if(index < 0 || portName == ""){
        index = 1;
    }

    for (it = m_pVolumeControl->inputPortMap.begin();it != m_pVolumeControl->inputPortMap.end();) {
        if (it.key() == index) {
            portMap = it.value();
            for (tempMap = portMap.begin();tempMap!=portMap.end();) {
                if (tempMap.key() == portName) {
                    portLabel = tempMap.value();
                    break;
                }
                ++tempMap;
            }
        }
        ++it;
    }
    return portLabel;
}


QString UkmediaMainWidget::findHighPriorityProfile(int index,QString profile)
{
    QMap<int, QMap<QString,int>>::iterator it;
    int priority = 0;
    QString profileName = "";
    QMap<QString,int> profileNameMap;
    QMap<QString,int>::iterator tempMap;
    QString cardStr = findCardName(index,m_pVolumeControl->cardMap);

    QString profileStr = findCardActiveProfile(index) ;

    QStringList list = profileStr.split("+");
    QString includeProfile = "";

    if (list.count() >1) {
        if (profile.contains("output")) {
            includeProfile = list.at(1);
        }
        else if (profile.contains("input")){
            includeProfile = list.at(0);
        }
        qDebug() << "profile str" <<profile <<"0:"<<list.at(0) <<"1:"<<list.at(1) <<list.count() <<includeProfile;
    }
    else {
        includeProfile = list.at(0);
    }

    for (it=m_pVolumeControl->cardProfilePriorityMap.begin();it!=m_pVolumeControl->cardProfilePriorityMap.end();) {
        if (it.key() == index) {
            profileNameMap = it.value();
            for (tempMap=profileNameMap.begin();tempMap!=profileNameMap.end();) {
                if (includeProfile != "" && tempMap.key().contains(includeProfile)  && !tempMap.key().contains(includeProfile+"-") \
                        && tempMap.key().contains(profile) && !tempMap.key().contains(profile+"-")) {
                    priority = tempMap.value();
                    profileName = tempMap.key();
                    qDebug() << "findHighPriorityProfile" << includeProfile <<tempMap.key() << profile << profileNameMap.count();
                }
                else if ( tempMap.key().contains(profile) && tempMap.value() > priority) {
                    priority = tempMap.value();
                    profileName = tempMap.key();
                    qDebug() << "findHighPriorityProfile" << includeProfile <<tempMap.key() << profile << profileNameMap.count();
                }
                ++tempMap;
            }
        }
        ++it;
    }
    qDebug() << "profile str----------" <<profileStr <<profileName << profile << includeProfile;
    return profileName;

}

void UkmediaMainWidget::findOutputListWidgetItem(QString cardName,QString portLabel)
{

    for (int row=0;row<m_pOutputWidget->m_pOutputListWidget->count();row++) {

        QListWidgetItem *item = m_pOutputWidget->m_pOutputListWidget->item(row);
        UkuiListWidgetItem *wid = (UkuiListWidgetItem *)m_pOutputWidget->m_pOutputListWidget->itemWidget(item);
        if (wid->deviceLabel->text() == cardName && wid->portLabel->text() == portLabel /*&& !wid->isPressed*/) {
            qDebug() << "findOutputListWidgetItem" << "card name:" << cardName << wid->deviceLabel->text() << "portLabel" << portLabel << wid->portLabel->text();
            m_pOutputWidget->m_pOutputListWidget->blockSignals(true);
            m_pOutputWidget->m_pOutputListWidget->setCurrentRow(row);
            m_pOutputWidget->m_pOutputListWidget->blockSignals(false);
            break;
        }

    }
}

void UkmediaMainWidget::findInputListWidgetItem(QString cardName,QString portLabel)
{
//    qDebug() <<"findInputListWidgetItem" << cardName  << m_pInputWidget->m_pInputListWidget->count();
    if(strstr(m_pVolumeControl->defaultSourceName,"monitor"))
        m_pInputWidget->m_pInputLevelProgressBar->setValue(0);
    for (int row=0;row<m_pInputWidget->m_pInputListWidget->count();row++) {
        QListWidgetItem *item = m_pInputWidget->m_pInputListWidget->item(row);
        UkuiListWidgetItem *wid = (UkuiListWidgetItem *)m_pInputWidget->m_pInputListWidget->itemWidget(item);
         qDebug() << "findInputListWidgetItem" << "card name:" << cardName << "portLabel:" << wid->portLabel->text() << "deviceLabel:" << wid->deviceLabel->text() << "port" << portLabel;
        if (wid->deviceLabel->text() == cardName && wid->portLabel->text() == portLabel) {
            m_pInputWidget->m_pInputListWidget->blockSignals(true);
            m_pInputWidget->m_pInputListWidget->setCurrentRow(row);
            m_pInputWidget->m_pInputListWidget->blockSignals(false);

            if (wid->deviceLabel->text().contains("bluez_card"))
                isCheckBluetoothInput = true;
            qDebug() << "set input list widget" << row;
            break;
        }
        else{
            m_pInputWidget->m_pInputListWidget->blockSignals(true);
            m_pInputWidget->m_pInputListWidget->setCurrentRow(SET_NULL_ITEM);
            m_pInputWidget->m_pInputListWidget->blockSignals(false);
        }
    }
}

/*
 * 输入设备中是否包含蓝牙设备
 */
bool UkmediaMainWidget::inputDeviceContainBluetooth()
{
    for (int row=0;row<m_pInputWidget->m_pInputListWidget->count();row++) {

        QListWidgetItem *item = m_pInputWidget->m_pInputListWidget->item(row);
        UkuiListWidgetItem *wid = (UkuiListWidgetItem *)m_pInputWidget->m_pInputListWidget->itemWidget(item);
        if (wid->deviceLabel->text().contains("bluez")) {
            return true;
        }
    }
    return false;
}

QString UkmediaMainWidget::blueCardName()
{
    for (int row=0;row<m_pInputWidget->m_pInputListWidget->count();row++) {

        QListWidgetItem *item = m_pInputWidget->m_pInputListWidget->item(row);
        UkuiListWidgetItem *wid = (UkuiListWidgetItem *)m_pInputWidget->m_pInputListWidget->itemWidget(item);
        if (wid->deviceLabel->text().contains("bluez")) {
            return wid->deviceLabel->text();
        }
    }
    return "";
}

int UkmediaMainWidget::indexOfOutputPortInOutputListWidget(int index,QString portName)
{
    QString cardName = findCardName(index,m_pVolumeControl->cardMap);
    for (int row=0;row<m_pOutputWidget->m_pOutputListWidget->count();row++) {

        QListWidgetItem *item = m_pOutputWidget->m_pOutputListWidget->item(row);
        UkuiListWidgetItem *wid = (UkuiListWidgetItem *)m_pOutputWidget->m_pOutputListWidget->itemWidget(item);
        if (wid->portLabel->text() == portName && wid->deviceLabel->text() == cardName) {
            return row;
        }
    }
    return -1;
}

int UkmediaMainWidget::indexOfInputPortInInputListWidget(int index,QString portName)
{
    QString cardName = findCardName(index,m_pVolumeControl->cardMap);
    qDebug() << "indexOfInputPortInInputListWidget" << cardName << portName;
    for (int row=0;row<m_pInputWidget->m_pInputListWidget->count();row++) {

        QListWidgetItem *item = m_pInputWidget->m_pInputListWidget->item(row);
        UkuiListWidgetItem *wid = (UkuiListWidgetItem *)m_pInputWidget->m_pInputListWidget->itemWidget(item);
        if (wid->portLabel->text() == portName && wid->deviceLabel->text() == cardName) {
            return row;
        }
    }
    return -1;
}

/*
    记录输入stream的card name
*/
void UkmediaMainWidget::inputStreamMapCardName (QString streamName,QString cardName)
{
    if (inputCardStreamMap.count() == 0) {
        inputCardStreamMap.insertMulti(streamName,cardName);
    }
    QMap<QString, QString>::iterator it;

    for (it=inputCardStreamMap.begin();it!=inputCardStreamMap.end();) {
        if (it.value() == cardName) {
            break;
        }
        if (it == inputCardStreamMap.end()-1) {
            qDebug() << "inputCardSreamMap " << streamName << cardName;
            inputCardStreamMap.insertMulti(streamName,cardName);
        }
        ++it;
    }
}

/*
    记录输出stream的card name
*/
void UkmediaMainWidget::outputStreamMapCardName(QString streamName, QString cardName)
{
    if (outputCardStreamMap.count() == 0) {
        outputCardStreamMap.insertMulti(streamName,cardName);
    }
    QMap<QString, QString>::iterator it;

    for (it=outputCardStreamMap.begin();it!=outputCardStreamMap.end();) {
        if (it.value() == cardName) {
            break;
        }
        if (it == outputCardStreamMap.end()-1) {
            qDebug() << "outputCardStreamMap " << streamName << cardName;
            outputCardStreamMap.insertMulti(streamName,cardName);
        }
        ++it;
    }
}

/*
    找输入stream对应的card name
*/
QString UkmediaMainWidget::findInputStreamCardName(QString streamName)
{
    QString cardName;
    QMap<QString, QString>::iterator it;
    for (it=inputCardStreamMap.begin();it!=inputCardStreamMap.end();) {
        if (it.key() == streamName) {
            cardName = it.value();
            qDebug() << "findInputStreamCardName:" << cardName;
            break;
        }
        ++it;
    }
    return cardName;
}

/*
    找输出stream对应的card name
*/
QString UkmediaMainWidget::findOutputStreamCardName(QString streamName)
{
    QString cardName;
    QMap<QString, QString>::iterator it;
    for (it=outputCardStreamMap.begin();it!=outputCardStreamMap.end();) {
        if (it.key() == streamName) {
            cardName = it.value();
            break;
        }
        ++it;
    }
    return cardName;
}

/*
 *  设置声卡的配置文件
 */
void UkmediaMainWidget::setCardProfile(QString name, QString profile)
{
    int index = findCardIndex(name,m_pVolumeControl->cardMap);
    m_pVolumeControl->setCardProfile(index,profile.toLatin1().data());

    qDebug() << "set profile" << profile << index ;
}

/*
 * 设置默认的输出设备端口
 */
void UkmediaMainWidget::setDefaultOutputPortDevice(QString devName, QString portName)
{
    int cardIndex = findCardIndex(devName,m_pVolumeControl->cardMap);
    QString portStr = findOutputPortName(cardIndex,portName);

    QTimer *timer = new QTimer;
    timer->start(300);
    connect(timer,&QTimer::timeout,[=](){
        QString sinkStr = findPortSink(cardIndex,portStr);

        /*默认的stream 和设置的stream相同 需要更新端口*/
        if (strcmp(sinkStr.toLatin1().data(),m_pVolumeControl->defaultSinkName) == 0) {
            m_pVolumeControl->setSinkPort(sinkStr.toLatin1().data(),portStr.toLatin1().data());
        }
        else {
            m_pVolumeControl->setDefaultSink(sinkStr.toLatin1().data());
            m_pVolumeControl->setSinkPort(sinkStr.toLatin1().data(),portStr.toLatin1().data());
        }
        qDebug() << "set default output"  << portName <<cardIndex << portStr <<sinkStr;
        delete timer;
    });

}

/*
 * 设置默认的输入设备端口
 */
void UkmediaMainWidget::setDefaultInputPortDevice(QString devName, QString portName)
{
    int cardIndex = findCardIndex(devName,m_pVolumeControl->cardMap);
    QString portStr = findInputPortName(cardIndex,portName);
    QTimer *timer = new QTimer;
    timer->start(50);
    connect(timer,&QTimer::timeout,[=](){
        QString sourceStr = findPortSource(cardIndex,portStr);

        /*默认的stream 和设置的stream相同 需要更新端口*/
        if (strcmp(sourceStr.toLatin1().data(),m_pVolumeControl->defaultSourceName) == 0) {
            m_pVolumeControl->setSourcePort(sourceStr.toLatin1().data(),portStr.toLatin1().data());
        }
        else {
            m_pVolumeControl->setDefaultSource(sourceStr.toLatin1().data());
            m_pVolumeControl->setSourcePort(sourceStr.toLatin1().data(),portStr.toLatin1().data());
        }
        qDebug() << "set default input"  << portName <<cardIndex << portStr << devName;
        delete timer;
    });

}

/*
 * 查找指定索引声卡的active profile
 */
QString UkmediaMainWidget::findCardActiveProfile(int index)
{
    QString activeProfileName = "";
    QMap<int,QString>::iterator it;
    for (it=m_pVolumeControl->cardActiveProfileMap.begin();it!=m_pVolumeControl->cardActiveProfileMap.end();) {
        if (it.key() == index) {
            activeProfileName = it.value();
            break;
        }
        ++it;
    }
    return activeProfileName;
}

/*
 * Find the corresponding sink according to the port name
 */
QString UkmediaMainWidget::findPortSink(int cardIndex,QString portName)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString> portNameMap;
    QMap<QString,QString>::iterator tempMap;
    QString sinkStr = "";
    for (it=m_pVolumeControl->sinkPortMap.begin();it!=m_pVolumeControl->sinkPortMap.end();) {
//         qDebug() <<"find port sink" << it.value() <<"portname:" << portName << "it.key"<< it.key() << "sink:" <<sinkStr << "card index" << cardIndex;
         if (it.key() == cardIndex) {
             portNameMap = it.value();
             for (tempMap=portNameMap.begin();tempMap!=portNameMap.end();) {
                qDebug() <<"find port sink" << tempMap.value() << portName<< tempMap.key() <<sinkStr;
                 if ( tempMap.value() == portName) {
                     sinkStr = tempMap.key();
                     break;
                 }
                 ++tempMap;
             }
        }
        ++it;
    }
    return sinkStr;
}

/*
 * Find the corresponding source according to the port name
 */
QString UkmediaMainWidget::findPortSource(int cardIndex,QString portName)
{
    QMap<int, QMap<QString,QString>>::iterator it;
    QMap<QString,QString> portNameMap;
    QMap<QString,QString>::iterator tempMap;
    QString sourceStr = "";
    for (it=m_pVolumeControl->sourcePortMap.begin();it!=m_pVolumeControl->sourcePortMap.end();) {
        if (it.key() == cardIndex) {
            portNameMap = it.value();
            for (tempMap=portNameMap.begin();tempMap!=portNameMap.end();) {
                qDebug() << "findportsource ===" <<tempMap.value() << portName;
                if ( tempMap.value() == portName) {
                    sourceStr = tempMap.key();
                    break;
                }
                ++tempMap;
            }
        }
        ++it;
    }
    return sourceStr;
}


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
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "ukmedia_volume_control.h"
#include "ukmedia_output_widget.h"
#include "ukmedia_input_widget.h"
#include "ukmedia_sound_effects_widget.h"
#include "ukui_list_widget_item.h"
#include <QMediaPlayer>
#include <gio/gio.h>
#include <libxml/tree.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gobject/gparamspecs.h>
#include <glib/gstdio.h>
extern "C" {
#include <dconf/dconf.h>
#include <canberra.h>
#include <glib/gmain.h>
#include <pulse/ext-stream-restore.h>
#include <pulse/glib-mainloop.h>
#include <pulse/error.h>
#include <pulse/subscribe.h>
#include <pulse/introspect.h>
}
#include <utime.h>
#include <a.out.h>
#include <QScreen>
#include <QApplication>
#include <QDomDocument>
#include <QGSettings>
#include <QAudioInput>
#include <set>

#define UKUI_THEME_SETTING "org.ukui.style"
#define UKUI_THEME_NAME "style-name"
#define UKUI_THEME_WHITE "ukui-white"
#define UKUI_THEME_BLACK "ukui-black"

#define UKUI_INPUT_REAR_MIC "analog-input-rear-mic"  //后置麦克风
#define UKUI_INPUT_FRONT_MIC "analog-input-front-mic" //前置麦克风
#define UKUI_OUTPUT_HEADPH "analog-output-headphones" //模拟耳机

#define KEYBINDINGS_CUSTOM_SCHEMA "org.ukui.media.sound"
#define KEYBINDINGS_CUSTOM_DIR "/org/ukui/sound/keybindings/"
#define MAX_CUSTOM_SHORTCUTS 1000
#define FILENAME_KEY "filename"
#define NAME_KEY "name"

#define KEY_SOUNDS_SCHEMA "org.ukui.sound"
#define UKUI_SWITCH_SETTING "org.ukui.session"
#define UKUI_STARTUP_MUSIC_KEY "boot-music"
#define UKUI_POWEROFF_MUSIC_KEY "poweroff-music"
#define UKUI_LOGOUT_MUSIC_KEY "logout-music"
#define UKUI_WAKEUP_MUSIC_KEY "weakup-music"

#define DNS_NOISE_REDUCTION "dns-noise-reduction"
#define DNS_NOISE_REDUCTION_VOLUME 65536
#define EVENT_SOUNDS_KEY "event-sounds"
#define INPUT_SOUNDS_KEY "input-feedback-sounds"
#define SOUND_THEME_KEY "theme-name"

#define UKUI_AUDIO_SCHEMA "org.ukui.audio"
#define ALERT_VOLUME "alert-volume"


#define DEFAULT_ALERT_ID "__default"
#define CUSTOM_THEME_NAME "__custom"
#define NO_SOUNDS_THEME_NAME "__no_sounds"

#define PA_VOLUME_NORMAL 65536.0
#define UKMEDIA_VOLUME_NORMAL 100.0

#define INTELDNS_SINK_CARDNAME "alsa_card.pci-0000_00_1f.3-platform-skl_hda_dsp_generic"
#define INTELDNS_SINK_PORTNAME "扬声器（sof-hda-dsp）"


#define INTELDNS_SOURCE_CARDNAME "alsa_card.pci-0000_00_1f.3-platform-skl_hda_dsp_generic"
#define INTELDNS_SOURCE_PORTNAME "内置麦克风（sof-hda-dsp）"

#ifdef __GNUC__
#define CA_CLAMP(x, low, high)                                          \
        __extension__ ({ typeof(x) _x = (x);                            \
                        typeof(low) _low = (low);                       \
                        typeof(high) _high = (high);                    \
                        ((_x > _high) ? _high : ((_x < _low) ? _low : _x)); \
                })
#else
#define CA_CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif

typedef enum
{
    GVC_LEVEL_SCALE_LINEAR,
    GVC_LEVEL_SCALE_LOG
} LevelScale;


class UkmediaMainWidget : public QWidget
{
    Q_OBJECT

public:
    UkmediaMainWidget(QWidget *parent = nullptr);
    ~UkmediaMainWidget();

    void initWidget(); //初始化界面
    void initGsettings(); //初始化gsetting值
    void dealSlot(); //处理槽函数
    int valueToPaVolume(int value); //滑动条值转换成音量
    int paVolumeToValue(int value); //音量值转换成滑动条值
    void themeChangeIcons();
    static int connectContext(gpointer userdata);
    static int caProplistMergeAp(ca_proplist *p, va_list ap);
    static int caPlayForWidget(UkmediaMainWidget *w, uint32_t id, ...);
    static int caProplistSetForWidget(ca_proplist *p, UkmediaMainWidget *widget);

    QPixmap drawDarkColoredPixmap(const QPixmap &source);
    QPixmap drawLightColoredPixmap(const QPixmap &source);
    QPixmap filledSymbolicColoredPixmap(QImage &img, QColor &baseColor);

    void inputVolumeDarkThemeImage(int value,bool status);
    void outputVolumeDarkThemeImage(int value,bool status);
    int getInputVolume();
    int getOutputVolume();
    void comboboxCurrentTextInit();
    QList<char *> listExistsPath();
    QString findFreePath();
    void addValue(QString name,QString filename);

    static void onKeyChanged (GSettings *settings,gchar *key,UkmediaMainWidget *w);
    static void updateTheme (UkmediaMainWidget *w);

    static void setupThemeSelector (UkmediaMainWidget *w);
    static void soundThemeInDir (UkmediaMainWidget *w,GHashTable *hash,const char *dir);
    static char *loadIndexThemeName (const char *index,char **parent);

    static void setComboxForThemeName (UkmediaMainWidget *w,const char *name);
    static void updateAlertsFromThemeName (UkmediaMainWidget *w,const gchar *name);
    static void updateAlert (UkmediaMainWidget *w,const char *alert_id);
    static int getFileType (const char *sound_name,char **linked_name);
    static char *customThemeDirPath (const char *child);

    static void populateModelFromDir (UkmediaMainWidget *w,const char *dirname);
    static void populateModelFromFile (UkmediaMainWidget *w,const char *filename);
    static void populateModelFromNode (UkmediaMainWidget *w,xmlNodePtr node);
    static xmlChar *xmlGetAndTrimNames (xmlNodePtr node);

    static void playAlretSoundFromPath (UkmediaMainWidget *w,QString path);

    static gboolean saveAlertSounds (QComboBox *combox,const char *id);
    static void deleteOldFiles (const char **sounds);
    static void deleteOneFile (const char *sound_name, const char *pattern);
    static void deleteDisabledFiles (const char **sounds);
    static void addCustomFile (const char **sounds, const char *filename);
    static gboolean cappletFileDeleteRecursive (GFile *file, GError **error);
    static gboolean directoryDeleteRecursive (GFile *directory, GError **error);
    static void createCustomTheme (const char *parent);
    static void customThemeUpdateTime (void);
    static gboolean customThemeDirIsEmpty (void);
    void switchNoiseReductionButton(bool status);

    //TEST输出
    void initInputComboboxItem();//初始化输入输出的Combobox选项框
    void initOutputComboboxItem();
    void findOutputComboboxItem(QString cardName,QString portLabel); //初始化Combobox output/input list widget的选项

    void addComboboxAvailableOutputPort();
    void addComboboxOutputListWidgetItem(QString portName, QString cardName);
    void deleteNotAvailableComboboxOutputPort();//删除不可用的端口
    int indexOfOutputPortInOutputCombobox(QString portName);
    bool comboboxOutputPortIsNeedAdd(int index,QString name);//port是否需要在Combobox list中添加
    bool comboboxOutputPortIsNeedDelete(int index,QString name);//port是否需要在Combobox list删除


    //TEST输入

    void findInputComboboxItem(QString cardName,QString portLabel); //初始化Combobox output/input list widget的选项
    void addComboboxAvailableInputPort();
    void addComboboxInputListWidgetItem(QString portName, QString cardName); //添加input listwidget item
    void deleteNotAvailableComboboxInputPort();
    int indexOfInputPortInInputCombobox(QString portName);//获取输入combobox当前的选项框的index
    bool comboboxInputPortIsNeedAdd(int index,QString name);//port是否需要在Combobox list中添加
    bool comboboxInputPortIsNeedDelete(int index,QString name);//port是否需要在Combobox list删除
    //-----------------------------------

    int findCardIndex(QString cardName, QMap<int,QString> cardMap);//查找声卡指定的索引
    QString findCardName(int index,QMap<int,QString> cardMap);
    QString findHighPriorityProfile(int index,QString profile);
    QString findPortSink(int cardIndex,QString portName);
    QString findPortSource(int cardIndex,QString portName);

    bool inputComboboxDeviceContainBluetooth();
    QString blueCardNameInCombobox();//记录蓝牙声卡名称

    void inputStreamMapCardName(QString streamName,QString cardName);
    void outputStreamMapCardName(QString streamName,QString cardName);
    QString findInputStreamCardName(QString streamName);
    QString findOutputStreamCardName(QString streamName);

    bool exitBluetoochDevice();

    QString findOutputPortName(int index,QString portLabel); //找到outputPortLabel对应的portName
    QString findInputPortName(int index,QString portLabel); //找到inputPortLabel对应的portName
    QString findOutputPortLabel(int index,QString portName); //查找名为portName对应的portLabel
    QString findInputPortLabel(int index,QString portName); //查找名为portName对应的portLabel
    void setCardProfile(QString name,QString profile); //设置声卡的配置文件
    void setDefaultOutputPortDevice(QString devName,QString portName); //设置默认的输出端口
    void setDefaultInputPortDevice(QString devName,QString portName); //设置默认的输入端口
    QString findCardActiveProfile(int index); //查找声卡的active profile

private Q_SLOTS:

    void initVoulmeSlider(); //初始化音量滑动条的值

    void themeComboxIndexChangedSlot(int index); //主题下拉框改变
    void comboxIndexChangedSlot(int index);

    void outputWidgetSliderChangedSlot(int v); //输出音量改变
    void inputWidgetSliderChangedSlot(int v); //输入滑动条更改
    void alertSoundSliderChangedSlot(int v);
    void inputMuteButtonSlot(); //输入音量静音控制
    void outputMuteButtonSlot(); //输出音量静音控制
    void balanceSliderChangedSlot(int v); //平衡值改变
    void peakVolumeChangedSlot(double v); //输入等级

    void updateCboxDevicePort(); //更新combobox设备端口
    void updateComboboxListWidgetItemSlot();

    void timeSliderSlot();
    void ukuiThemeChangedSlot(const QString &);
    void startupButtonSwitchChangedSlot(bool status); //开机音乐开关
    void poweroffButtonSwitchChangedSlot(bool status); //关机音乐开关
    void logoutMusicButtonSwitchChangedSlot(bool status); //注销音乐开关
    void wakeButtonSwitchChangedSlot(bool status); //唤醒音乐开关
    void alertSoundButtonSwitchChangedSlot(bool status);
    void dnsNoiseReductionButtonSwitchChangedSlot(bool status);
    void bootMusicSettingsChanged();

    void windowClosedComboboxChangedSlot(int index);
    void volumeChangedComboboxChangeSlot(int index);
    void settingMenuComboboxChangedSlot(int index);

    void cboxoutputListWidgetCurrentRowChangedSlot(int row);//combobox output list widget选项改变
    void cboxinputListWidgetCurrentRowChangedSlot(int row);//combobox input list widget选项改变


protected:
//    void paintEvent(QPaintEvent *event);
//    void mousePressEvent(QMouseEvent *ev);

private:
    UkmediaInputWidget *m_pInputWidget;
    UkmediaOutputWidget *m_pOutputWidget;
    UkmediaSoundEffectsWidget *m_pSoundWidget;

    UkmediaVolumeControl *m_pVolumeControl;

    QStringList *m_pSoundList;
    QStringList *m_pThemeDisplayNameList;
    QStringList *m_pThemeNameList;
    QStringList *m_pSoundThemeList;
    QStringList *m_pSoundThemeDirList;
    QStringList *m_pSoundThemeXmlNameList;
    QStringList *m_pSoundNameList;

    GSettings *m_pSoundSettings;
    QGSettings *m_pBootSetting;
    QGSettings *m_pThemeSetting;
    QGSettings *m_pAlertVolumeSetting;
//    QGSettings *m_pWindowClosedSetting;
    QString mThemeName;
    bool m_hasMusic;
    bool firstEnterSystem = true;

    const gchar* m_privOutputPortLabel = "";

    int callBackCount = 0;
    bool firstEntry = true;

    bool cboxfirstEntry = true;
    QMap<int, QString> currentOutputPortLabelMap;
    QMap<int, QString> currentInputPortLabelMap;
    QMap<QString,QString> inputCardStreamMap;
    QMap<QString,QString> outputCardStreamMap;
    QMap<int, QString> currentCboxOutputPortLabelMap;
    QMap<int, QString> currentCboxInputPortLabelMap;


    bool updatePort = true;
    bool setDefaultstream = true;
    int reconnectTime;
    QTimer *time;

    QTimer *timeSlider;
    bool mousePress = false;
    bool mouseReleaseState = false;

    QTimer *timeSliderBlance;
    bool mousePressBlance = false;
    bool mouseReleaseStateBlance = false;
};

#endif // WIDGET_H

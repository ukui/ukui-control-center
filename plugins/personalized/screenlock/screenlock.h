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
#ifndef SCREENLOCK_H
#define SCREENLOCK_H

#include <QObject>
#include <QtPlugin>
#include <QProcess>
#include <QLabel>
#include <QThread>
#include <QGSettings>
#include <QSettings>
#include "HoverWidget/hoverwidget.h"
#include "ImageUtil/imageutil.h"
#include "shell/interface.h"
#include "SwitchButton/switchbutton.h"
#include "FlowLayout/flowlayout.h"
#include "Uslider/uslider.h"
#include "systemdbusdispatcher.h"
//#include "changepwddialog.h"
#include "userdispatcher.h"
#include "plugins/account/userinfo/changepwddialog.h"
#include "pictureunit.h"

#include "buildpicunitsworker.h"
#define UKUI_SCREENSAVER "org.ukui.screensaver"
#ifdef signals
#undef signals
#endif
extern "C" {
#include <glib.h>
#include <gio/gio.h>
}
enum {
    STANDARDUSER,
    ADMINISTRATOR,
    ROOT
};
typedef struct _UserInfomation {
    QString objpath;
    QString username;
    QString iconfile;
    QString passwd;
    int accounttype;
    int passwdtype;
    bool current;
    bool logined;
    bool autologin;
    bool noPwdLogin;
    qint64 uid;
}UserInfomation;
namespace Ui {
class Screenlock;
}

class Screenlock : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Screenlock();
    ~Screenlock();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    void initSearchText();
    void setupComponent();
    void setupConnect();
    void initScreenlockStatus();
    void showLocalWpDialog();
    void initIdleSliderStatus();
    void slider_released_slot();
    void set_idle_gsettings_value(int value);

private:
    int convertToLocktime(const int value);
    int lockConvertToSlider(const int value);
    void setLockBackground(bool status);

    bool getLockStatus();
    bool writeInit(QString group, QString key, uint32_t value);

    void initPreviewStatus();
    void initPreviewWidget();
    void initButton();

    void startupScreensaver();
    void closeScreensaver();
    void setClickedPic(QString fileName);
private slots:
    void datetime_update_slot();
private:
    Ui::Screenlock *ui;
    QMap<QString, UserInfomation> allUserInfoMap;
    SystemDbusDispatcher * sysdispatcher;

    QWidget              * pluginWidget;
    QWidget              * onMaskWidget;

    QLabel               * timeLabel;
    QLabel               * dateLabel;

    HoverWidget          * localBgd;
    HoverWidget          * resetBgd;

    SwitchButton         * lockSwitchBtn;
    SwitchButton         * showMessageBtn;

    QHBoxLayout          * addLyt_2;
    QHBoxLayout          * addLyt_1;
    QVBoxLayout          * maskLayout;
    FlowLayout           * flowLayout;

    Uslider              * uslider;

    BuildPicUnitsWorker  * pWorker;
    QProcess             * process;
    QThread              * pThread;
    QTimer               * m_itimer = nullptr;

    int pluginType;
    int adminnum;

    GSettings            * session_settings;
    GSettings            * screenlock_settings;
    QSettings            * lockSetting; //锁屏状态QSettings
    QSettings            * lockLoginSettings;

    QGSettings           * qSessionSetting = nullptr;
    QGSettings           * mSettings = nullptr;
    QGSettings           * lSetting = nullptr;
    QGSettings           * powerSettings = nullptr;
    QGSettings           * m_formatsettings = nullptr;

    QSize lockbgSize;

    QStringList  runStringList;
    QString pluginName;
    QString screensaver_bin;
    QString m_initPath;
    bool messageKey = false;

    bool mFirstLoad;
    PictureUnit *prePicUnit = nullptr;
};

#endif // SCREENLOCK_H

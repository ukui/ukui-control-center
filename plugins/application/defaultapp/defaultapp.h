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
#ifndef DEFAULTAPP_H
#define DEFAULTAPP_H

#include <QObject>
#include <QtPlugin>
#include <QComboBox>
#include <QIcon>
#include <QStyledItemDelegate>
#include <QtDBus>
#include <QtConcurrent>
#include <QTime>
#include <QIcon>
#include <QFileSystemWatcher>


#include "shell/interface.h"
#include <ukcc/widgets/fixlabel.h>
#include <ukcc/widgets/titlelabel.h>
#include "shell/utils/mthread.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

typedef struct _Applist
{
    QString strAppid;
}AppList;// 用于存放应用列表

typedef struct _AppInfo
{
    GAppInfo *item;
}Appinfo;// 用于存放应用列表信息

namespace Ui {
class DefaultAppWindow;
}

class DefaultApp : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    DefaultApp();
    ~DefaultApp();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

    void initUi(QWidget *widget);
    void initDefaultUI();
    void initSlots();
    void initSearchText();
    void initDefaultAppInfo(const char* type, QComboBox *combox);
    void resetUi();

    bool setWebBrowsersDefaultProgram(char * appid);
    bool setMailReadersDefaultProgram(char * appid);
    bool setImageViewersDefaultProgram(char * appid);
    bool setAudioPlayersDefaultProgram(char * appid);
    bool setVideoPlayersDefaultProgram(char * appid);
    bool setTextEditorsDefautlProgram(char * appid);

    void connectToServer();
    void watchFileChange();
    QFrame *setLine(QFrame *frame);

private:
    QString getDefaultAppId(const char * contentType);
    QVector<AppList> getAppIdList(const char * contentType);
    static QVector<Appinfo> _getAppList(const char *contentType);

private:
    Ui::DefaultAppWindow * ui;

    QWidget * pluginWidget;

    QFrame *mBrowserFrame;
    QFrame *mMailFrame;
    QFrame *mImageFrame;
    QFrame *mAudioFrame;
    QFrame *mVideoFrame;
    QFrame *mTextFrame;

    TitleLabel *mTitleLabel;
    FixLabel *mBrowserLabel;
    FixLabel *mMailLabel;
    FixLabel *mImageLabel;
    FixLabel *mAudioLabel;
    FixLabel *mVideoLabel;
    FixLabel *mTextLabel;

    QComboBox *mBrowserCombo;
    QComboBox *mMailCombo;
    QComboBox *mImageCombo;
    QComboBox *mAudioCombo;
    QComboBox *mVideoCombo;
    QComboBox *mTextCombo;


    QString pluginName;
    int pluginType;
    QDBusInterface *m_cloudInterface;

    QString mDefaultBrowser;
    QString mDefaultMail;
    QString mDefaultPic;
    QString mDefaultAdudio;
    QString mDefaultVideonum;
    QString mDefaultText;

    QString mDefaultString;

    QStringList browserList;
    QString watchfile;

    bool mFirstLoad;
    bool mComboChange = false;

    QFileSystemWatcher *mConfigFileWatcher;

public slots:
    void browserComBoBox_changed_cb(int index);
    void mailComBoBox_changed_cb(int index);
    void imageComBoBox_changed_cb(int index);
    void audioComBoBox_changed_cb(int index);
    void videoComBoBox_changed_cb(int index);
    void textComBoBox_changed_cb(int index);
    void keyChangedSlot(const QString &key);
Q_SIGNALS:
    void appInitDone(int index,const QString &type);
};

#endif // DEFAULTAPP_H

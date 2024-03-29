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
#include <QWidget>
#include <QDebug>

#include "defaultapp.h"
#include "ui_defaultapp.h"
#include "addappdialog.h"

#define BROWSERTYPE "x-scheme-handler/http"
#define MAILTYPE    "x-scheme-handler/mailto"
#define IMAGETYPE   "image/png"
#define AUDIOTYPE   "audio/x-vorbis+ogg"
#define VIDEOTYPE   "video/x-ogm+ogg"
#define TEXTTYPE    "text/plain"

#define DESKTOPPATH "/usr/share/applications/"

DefaultApp::DefaultApp() : mFirstLoad(true)
{
    pluginName = tr("Default App");
    pluginType = SYSTEM;
}

DefaultApp::~DefaultApp() {
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
    }
}

QString DefaultApp::get_plugin_name() {
    return pluginName;
}

int DefaultApp::get_plugin_type() {
    return pluginType;
}

QWidget *DefaultApp::get_plugin_ui() {
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::DefaultAppWindow;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        mDefaultString = tr("No program available");
        ui->titleLabel->setStyleSheet("QLabel{color: palette(windowText);}");
        initUI();
        initSlots();
        connectToServer();

#ifdef __sw_64__
        ui->ResetBtn->show();
#else
        ui->ResetBtn->hide();
#endif
        connect(ui->ResetBtn, SIGNAL(clicked(bool)), this, SLOT(resetDefaultApp()));
    }
    return pluginWidget;
}

void DefaultApp::plugin_delay_control() {

}

const QString DefaultApp::name() const {

    return QStringLiteral("defaultapp");
}

void DefaultApp::initSlots() {
    connect(ui->browserComBoBox, static_cast<void (QComboBox::*)(int )> (&QComboBox::currentIndexChanged), this, &DefaultApp::browserComBoBox_changed_cb);
    connect(ui->mailComBoBox, static_cast<void (QComboBox::*)(int )> (&QComboBox::currentIndexChanged), this, &DefaultApp::mailComBoBox_changed_cb);
    connect(ui->imageComBoBox, static_cast<void (QComboBox::*)(int )> (&QComboBox::currentIndexChanged), this, &DefaultApp::imageComBoBox_changed_cb);
    connect(ui->audioComBoBox, static_cast<void (QComboBox::*)(int )> (&QComboBox::currentIndexChanged), this, &DefaultApp::audioComBoBox_changed_cb);
    connect(ui->videoComBoBox, static_cast<void (QComboBox::*)(int )> (&QComboBox::currentIndexChanged), this, &DefaultApp::videoComBoBox_changed_cb);
    connect(ui->textComBoBox, static_cast<void (QComboBox::*)(int )> (&QComboBox::currentIndexChanged), this, &DefaultApp::textComBoBox_changed_cb);
}

void DefaultApp::initUI() {

    // 若默认应用被卸载，系统会自动设置某个其它应用为默认应用，若不存在其它应用，则显示"无可用程序"

    // BROWSER
    initDefaultAppInfo(BROWSERTYPE,ui->browserComBoBox);

    // IMAGE
    initDefaultAppInfo(IMAGETYPE,ui->imageComBoBox);

    // MAIL
    initDefaultAppInfo(MAILTYPE,ui->mailComBoBox);

    // AUDIO
    initDefaultAppInfo(AUDIOTYPE,ui->audioComBoBox);

    // VIDEO
    initDefaultAppInfo(VIDEOTYPE,ui->videoComBoBox);

    // TEXT
    initDefaultAppInfo(TEXTTYPE,ui->textComBoBox);

}

void DefaultApp::initSearchText() {
    //~ contents_path /defaultapp/Browser
    ui->browserLabel->setText(tr("Browser"));
    //~ contents_path /defaultapp/Mail
    ui->mailLabel->setText(tr("Mail"));
    //~ contents_path /defaultapp/Image Viewer
    ui->imageLabel->setText(tr("Image Viewer"));
    //~ contents_path /defaultapp/Audio Player
    ui->audioLabel->setText(tr("Audio Player"));
    //~ contents_path /defaultapp/Video Player
    ui->videoLabel->setText(tr("Video Player"));
    //~ contents_path /defaultapp/Text Editor
    ui->textLabel->setText(tr("Text Editor"));
}

void DefaultApp::initDefaultAppInfo(const char *type, QComboBox *combox)
{
    QString currentapp(getDefaultAppId(type));
    QByteArray ba = QString(DESKTOPPATH + currentapp).toUtf8();
    GDesktopAppInfo * textinfo = g_desktop_app_info_new_from_filename(ba.constData());
    QString appname = g_app_info_get_name(G_APP_INFO(textinfo));
    const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(textinfo)));
    QIcon appicon;
    appicon = QIcon::fromTheme(QString(QLatin1String(iconname)),
                               QIcon(QString("/usr/share/pixmaps/"+QString(QLatin1String(iconname))
                                             +".png")));
    if (!appname.isNull()) {
        combox->addItem(appicon, appname, currentapp);

        // 将当前默认应用信息写入~/.config/mimeapps.list
        int i = combox->currentIndex();
    //    combox->setCurrentText(appname);
        if (!strcmp(type , BROWSERTYPE)) {
            browserComBoBox_changed_cb(i);
        } else if (!strcmp(type , IMAGETYPE)) {
            imageComBoBox_changed_cb(i);
        } else if (!strcmp(type , MAILTYPE)) {
            mailComBoBox_changed_cb(i);
        } else if (!strcmp(type , AUDIOTYPE)) {
            audioComBoBox_changed_cb(i);
        } else if (!strcmp(type , VIDEOTYPE)) {
            videoComBoBox_changed_cb(i);
        } else if (!strcmp(type , TEXTTYPE)) {
            textComBoBox_changed_cb(i);
        }
    } else {
        combox->addItem(mDefaultString);
        combox->setCurrentText(mDefaultString);
    }

    // 将可用程序加入列表
    QtConcurrent::run([=]() {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString current(getDefaultAppId(type));
        QVector<AppList> list = getAppIdList(type);
        if (!list.isEmpty()) {
            for (int i = 0; i < list.size(); i++) {
                QString single(list[i].strAppid);
                QByteArray ba = QString(DESKTOPPATH + single).toUtf8();
                GDesktopAppInfo * info = g_desktop_app_info_new_from_filename(ba.constData());
                QString appname = g_app_info_get_name(G_APP_INFO(info));
                const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(info)));
                QIcon appicon;
                appicon = QIcon::fromTheme(QString(QLatin1String(iconname)),
                                           QIcon(QString("/usr/share/pixmaps/"+QString(QLatin1String(iconname))
                                                         +".png")));
                if (current == single) {
                    continue;
                }
                combox->addItem(appicon, appname, single);
            }
        }
        qDebug()<<type<<"  线程耗时: "<<timedebuge.elapsed()<<"ms";
    });

}

void DefaultApp::browserComBoBox_changed_cb(int index) {

    QtConcurrent::run([=] {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString appid = ui->browserComBoBox->itemData(index).toString();
        QByteArray ba = appid.toUtf8(); // QString to char *
        qDebug()<<"browserComBoBox_changed_cb："<<timedebuge.elapsed()<<"ms" << appid;
        setWebBrowsersDefaultProgram(ba.data());

    });

}

void DefaultApp::mailComBoBox_changed_cb(int index) {

    QtConcurrent::run([=] {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString appid = ui->mailComBoBox->itemData(index).toString();
        QByteArray ba = appid.toUtf8(); // QString to char *
        setMailReadersDefaultProgram(ba.data());
        qDebug()<<"mailComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";//输出计时
    });

}

void DefaultApp::imageComBoBox_changed_cb(int index) {

    QtConcurrent::run([=] {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString appid = ui->imageComBoBox->itemData(index).toString();
        QByteArray ba = appid.toUtf8(); // QString to char *
        setImageViewersDefaultProgram(ba.data());
        qDebug()<<"imageComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";//输出计时
    });

}

void DefaultApp::audioComBoBox_changed_cb(int  index) {

    QtConcurrent::run([=] {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString appid = ui->audioComBoBox->itemData(index).toString();
        QByteArray ba = appid.toUtf8(); // QString to char *
        setAudioPlayersDefaultProgram(ba.data());
        qDebug()<<"audioComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";//输出计时
    });

}

void DefaultApp::videoComBoBox_changed_cb(int index) {

    QtConcurrent::run([=] {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString appid = ui->videoComBoBox->itemData(index).toString();
        QByteArray ba = appid.toUtf8(); // QString to char *
        setVideoPlayersDefaultProgram(ba.data());
    });

}

void DefaultApp::textComBoBox_changed_cb(int index) {

    QtConcurrent::run([=] {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString appid = ui->textComBoBox->itemData(index).toString();
        QByteArray ba = appid.toUtf8(); // QString to char *
        setTextEditorsDefautlProgram(ba.data());

    });

}

void DefaultApp::resetDefaultApp() {

    ui->browserComBoBox->setCurrentText(mDefaultBrowser);
    ui->imageComBoBox->setCurrentText(mDefaultPic);
    ui->audioComBoBox->setCurrentText(mDefaultAdudio);
    ui->videoComBoBox->setCurrentText(mDefaultVideo);
    ui->textComBoBox->setCurrentText(mDefaultText);

    if (mDefaultMail.isEmpty()) {
        ui->mailComBoBox->setCurrentIndex(0);
    } else {
        ui->mailComBoBox->setCurrentText(mDefaultMail);
    }
}

QString DefaultApp::getDefaultAppId(const char * contentType) {
    GAppInfo * app = g_app_info_get_default_for_type(contentType, false);
    if(app != NULL){
        const char * id = g_app_info_get_id(app);
        QString strId(id);
        return strId;
    } else {
        return QString("");
    }
}

QVector<AppList> DefaultApp::getAppIdList(const char *contentType) {
    QVector<AppList> appList;
    appList.clear();
    QVector<Appinfo> appinfo = _getAppList(contentType);
    if (!appinfo.isEmpty()) {
        for(int j = 0; j < appinfo.size(); j++) {
            const char *id = g_app_info_get_id(appinfo[j].item);
            if (id != NULL) {
                AppList al;
                al.strAppid = QString(id);
                appList.append(al);
            }
        }
    }
    return appList;
}

QVector<Appinfo> DefaultApp::_getAppList(const char *contentType) {
    GList *applist;
    applist = g_app_info_get_all_for_type(contentType);
    GAppInfo * item;
    QVector<Appinfo> appinfo;
    appinfo.clear();

    if (applist != NULL) {
        int len = g_list_length(applist);

        //获取应用列表
        for (int index=0; index < len; index++) {
            item = (GAppInfo*) g_list_nth_data(applist, index);
            Appinfo ai;
            ai.item = item;
            appinfo.append(ai);
        }
    }
    return appinfo;
}

bool DefaultApp::setWebBrowsersDefaultProgram(char * appid) {
    qDebug() << __func__ << __LINE__;
    const char * content_type = "x-scheme-handler/http";
    QVector<Appinfo> appinfo = _getAppList(content_type);
    bool judge = false;
    if (!appinfo.isEmpty()) {
        for(int i = 0; i < appinfo.size(); i++) {
            const char *id = g_app_info_get_id(appinfo[i].item);
            int result = strcmp(id,appid);
            if (0 == result) {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1=g_app_info_set_as_default_for_type(appitem, "x-scheme-handler/http", NULL);
                gboolean ret2=g_app_info_set_as_default_for_type(appitem, "x-scheme-handler/https", NULL);
                gboolean ret3=g_app_info_set_as_default_for_type(appitem, "x-scheme-handler/about", NULL);
                gboolean ret4=g_app_info_set_as_default_for_type(appitem, "text/html", NULL);
                if(ret1 && ret2 && ret3 && ret4)
                    judge=true;
                break;
            }
        }
    }
    qDebug() << __func__ << __LINE__;
    return judge;
}

bool DefaultApp::setMailReadersDefaultProgram(char *appid) {
    const char *content_type="x-scheme-handler/mailto";
    QVector<Appinfo> appinfo = _getAppList(content_type);

    bool judge = false;
    if (!appinfo.isEmpty()) {
        for (int i = 0; i < appinfo.size(); i++) {
            const char * id = g_app_info_get_id(appinfo[i].item);
            int result=strcmp(id,appid);
            if (0 == result) {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1=g_app_info_set_as_default_for_type(appitem, "x-scheme-handler/mailto", NULL);
                gboolean ret2=g_app_info_set_as_default_for_type(appitem, "application/x-extension-eml", NULL);
                gboolean ret3=g_app_info_set_as_default_for_type(appitem, "message/rfc822", NULL);
                if(ret1 && ret2 && ret3)
                    judge=true;
                break;
            }
        }
    }
    return judge;
}

bool DefaultApp::setImageViewersDefaultProgram(char *appid) {
    const char *content_type="image/png";
    QVector<Appinfo> appinfo = _getAppList(content_type);

    bool judge = false;
    if (!appinfo.isEmpty()) {
        for (int i=0; i < appinfo.size();i++){
            const char *id = g_app_info_get_id(appinfo[i].item);
            int result = strcmp(id, appid);
            if (0 == result) {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1 = g_app_info_set_as_default_for_type(appitem, "image/bmp", NULL);
                gboolean ret2 = g_app_info_set_as_default_for_type(appitem, "image/gif", NULL);
                gboolean ret3 = g_app_info_set_as_default_for_type(appitem, "image/jpeg", NULL);
                gboolean ret4 = g_app_info_set_as_default_for_type(appitem, "image/png", NULL);
                gboolean ret5 = g_app_info_set_as_default_for_type(appitem, "image/tiff", NULL);
                if(ret1 && ret2 && ret3 && ret4 && ret5)
                    judge=true;
                break;
            }
        }
    }

    return judge;
}

bool DefaultApp::setVideoPlayersDefaultProgram(char *appid) {
    const char *content_type = "video/x-ogm+ogg";
    QVector<Appinfo> appinfo = _getAppList(content_type);

    bool judge = false;
    if (!appinfo.isEmpty()) {
        for(int i = 0; i < appinfo.size(); i++) {
            const char *id = g_app_info_get_id(appinfo[i].item);
            int result = strcmp(id,appid);
            if (0 == result) {
                GAppInfo *appitem = appinfo[i].item;
                gboolean ret1 = g_app_info_set_as_default_for_type(appitem, "video/mp4", NULL);
                gboolean ret2 = g_app_info_set_as_default_for_type(appitem, "video/mpeg", NULL);
                gboolean ret3 = g_app_info_set_as_default_for_type(appitem, "video/mp2t", NULL);
                gboolean ret4 = g_app_info_set_as_default_for_type(appitem, "video/msvideo", NULL);
                gboolean ret5 = g_app_info_set_as_default_for_type(appitem, "video/quicktime", NULL);
                gboolean ret6 = g_app_info_set_as_default_for_type(appitem, "video/webm", NULL);
                gboolean ret7 = g_app_info_set_as_default_for_type(appitem, "video/x-avi", NULL);
                gboolean ret8 = g_app_info_set_as_default_for_type(appitem, "video/x-flv", NULL);
                gboolean ret9 = g_app_info_set_as_default_for_type(appitem, "video/x-matroska", NULL);
                gboolean ret10 = g_app_info_set_as_default_for_type(appitem, "video/x-mpeg", NULL);
                gboolean ret11 = g_app_info_set_as_default_for_type(appitem, "video/x-ogm+ogg", NULL);
                gboolean ret12 = g_app_info_set_as_default_for_type(appitem, "video/rm", NULL);
                gboolean ret13 = g_app_info_set_as_default_for_type(appitem, "video/3pg", NULL);
                gboolean ret14 = g_app_info_set_as_default_for_type(appitem, "video/asf", NULL);
                gboolean ret15 = g_app_info_set_as_default_for_type(appitem, "video/3gp", NULL);
                gboolean ret16 = g_app_info_set_as_default_for_type(appitem, "video/3gpp", NULL);
                gboolean ret17 = g_app_info_set_as_default_for_type(appitem, "video/3gpp2", NULL);
                gboolean ret18 = g_app_info_set_as_default_for_type(appitem, "video/x-ms-afs", NULL);
                gboolean ret19 = g_app_info_set_as_default_for_type(appitem, "video/x-ms-asf", NULL);
                gboolean ret20 = g_app_info_set_as_default_for_type(appitem, "video/x-mpeg2", NULL);
                gboolean ret21 = g_app_info_set_as_default_for_type(appitem, "video/x-mpeg3", NULL);
                gboolean ret22 = g_app_info_set_as_default_for_type(appitem, "video/mp4v-es", NULL);
                gboolean ret23 = g_app_info_set_as_default_for_type(appitem, "video/x-m4v", NULL);
                gboolean ret24 = g_app_info_set_as_default_for_type(appitem, "video/divx", NULL);
                gboolean ret25 = g_app_info_set_as_default_for_type(appitem, "video/vnd.divx", NULL);
                gboolean ret26 = g_app_info_set_as_default_for_type(appitem, "video/x-msvideo", NULL);
                gboolean ret27 = g_app_info_set_as_default_for_type(appitem, "video/ogg", NULL);
                gboolean ret28 = g_app_info_set_as_default_for_type(appitem, "video/vnd.rn-realvideo", NULL);
                gboolean ret29 = g_app_info_set_as_default_for_type(appitem, "video/x-ms-wmv", NULL);
                gboolean ret30 = g_app_info_set_as_default_for_type(appitem, "video/x-ms-wmx", NULL);
                gboolean ret31 = g_app_info_set_as_default_for_type(appitem, "video/x-ms-wvxvideo", NULL);
                gboolean ret32 = g_app_info_set_as_default_for_type(appitem, "video/avi", NULL);
                gboolean ret33 = g_app_info_set_as_default_for_type(appitem, "video/x-flic", NULL);
                gboolean ret34 = g_app_info_set_as_default_for_type(appitem, "video/fli", NULL);
                gboolean ret35 = g_app_info_set_as_default_for_type(appitem, "video/x-flc", NULL);
                gboolean ret36 = g_app_info_set_as_default_for_type(appitem, "video/flv", NULL);
                gboolean ret37 = g_app_info_set_as_default_for_type(appitem, "video/x-theora", NULL);
                gboolean ret38 = g_app_info_set_as_default_for_type(appitem, "video/x-theora+ogg", NULL);
                gboolean ret39 = g_app_info_set_as_default_for_type(appitem, "video/mkv", NULL);
                gboolean ret40 = g_app_info_set_as_default_for_type(appitem, "video/x-ogm", NULL);
                gboolean ret41 = g_app_info_set_as_default_for_type(appitem, "video/vnd.mpegurl", NULL);
                gboolean ret42 = g_app_info_set_as_default_for_type(appitem, "video/dv", NULL);
                gboolean ret43 = g_app_info_set_as_default_for_type(appitem, "application/vnd.rn-realmedia", NULL);
                gboolean ret44 = g_app_info_set_as_default_for_type(appitem, "application/vnd.rn-realmedia-vbr", NULL);
                if (ret1 && ret2 && ret3 && ret4 && ret5 && ret6 && ret7 && ret8 && ret9 && ret10 && ret11 && \
                        ret12 && ret13 && ret14 && ret15 && ret16 && ret17 && ret18 && ret19 && ret20 && ret21 && \
                        ret22 && ret23 && ret24 && ret25 && ret26 && ret27 && ret28 && ret29 && ret30 && \
                        ret31 && ret32 && ret33 && ret34 && ret35 && ret36 && ret37 && ret38 && ret39 && \
                        ret40 && ret41 && ret42 && ret43 && ret44)
                    judge=true;
                break;
            }
        }
    }
    return judge;
}

bool DefaultApp::setAudioPlayersDefaultProgram(char *appid) {
    const char *content_type = "audio/x-vorbis+ogg";
    QVector<Appinfo> appinfo = _getAppList(content_type);

    bool judge = false;
    if (!appinfo.isEmpty()) {
        for(int i = 0; i < appinfo.size() ;i++) {
            const char *id = g_app_info_get_id(appinfo[i].item);
            int result = strcmp(id,appid);
            if (0 == result) {
                GAppInfo *appitem=appinfo[i].item;
                gboolean ret1 = g_app_info_set_as_default_for_type(appitem, "audio/mpeg", NULL);
                gboolean ret2 = g_app_info_set_as_default_for_type(appitem, "audio/x-mpegurl", NULL);
                gboolean ret3 = g_app_info_set_as_default_for_type(appitem, "audio/x-scpls", NULL);
                gboolean ret4 = g_app_info_set_as_default_for_type(appitem, "audio/x-vorbis+ogg", NULL);
                gboolean ret5 = g_app_info_set_as_default_for_type(appitem, "audio/x-wav", NULL);
                gboolean ret6 = g_app_info_set_as_default_for_type(appitem, "audio/x-ms-wma", NULL);
                gboolean ret7 = g_app_info_set_as_default_for_type(appitem, "audio/x-flac", NULL);
                gboolean ret8 = g_app_info_set_as_default_for_type(appitem, "audio/ac3", NULL);
                gboolean ret9 = g_app_info_set_as_default_for_type(appitem, "audio/acc", NULL);
                gboolean ret10 = g_app_info_set_as_default_for_type(appitem, "audio/aac", NULL);
                gboolean ret11 = g_app_info_set_as_default_for_type(appitem, "audio/mp4", NULL);
                gboolean ret12 = g_app_info_set_as_default_for_type(appitem, "audio/x-m4r", NULL);
                gboolean ret13 = g_app_info_set_as_default_for_type(appitem, "audio/midi", NULL);
                gboolean ret14 = g_app_info_set_as_default_for_type(appitem, "audio/mp2", NULL);
                gboolean ret15 = g_app_info_set_as_default_for_type(appitem, "audio/x-wavpack", NULL);
                gboolean ret16 = g_app_info_set_as_default_for_type(appitem, "audio/x-ape", NULL);
                gboolean ret17 = g_app_info_set_as_default_for_type(appitem, "audio/x-mmf", NULL);
                gboolean ret18 = g_app_info_set_as_default_for_type(appitem, "audio/mp3", NULL);
                gboolean ret19 = g_app_info_set_as_default_for_type(appitem, "audio/flac", NULL);
                gboolean ret20 = g_app_info_set_as_default_for_type(appitem, "audio/wma", NULL);
                gboolean ret21 = g_app_info_set_as_default_for_type(appitem, "application/x-smaf", NULL);
                if(ret1 && ret2 && ret3 && ret4 && ret5 && ret6 && ret7 && ret8 && ret9 && ret10 &&
                        ret11 && ret12 && ret13 && ret14 && ret15 && ret16 && ret17 && ret18 && ret19 && ret20 && ret21) {
                    judge=true;
                }
                break;
            }
        }
    }
    return judge;
}

bool DefaultApp::setTextEditorsDefautlProgram(char *appid) {
    const char * content_type = "text/plain";
    QVector<Appinfo> appinfo = _getAppList(content_type);

    bool judge = false;
    if (!appinfo.isEmpty()) {
        for (int i = 0;i < appinfo.size(); i++) {
            const char * id = g_app_info_get_id(appinfo[i].item);
            int result = strcmp(id,appid);
            if (0 == result) {
                GAppInfo *appitem = appinfo[i].item;
                gboolean ret1 = g_app_info_set_as_default_for_type(appitem, "text/plain", NULL);
                if(ret1)
                    judge=true;
                break;
            }
        }
    }

    return judge;
}

void DefaultApp::connectToServer(){
    QtConcurrent::run([=]() {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        m_cloudInterface = new QDBusInterface("org.kylinssoclient.dbus",
                                              "/org/kylinssoclient/path",
                                              "org.freedesktop.kylinssoclient.interface",
                                              QDBusConnection::sessionBus());
        if (!m_cloudInterface->isValid())
        {
            qDebug() << "fail to connect to service";
            qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
            return;
        }
        QDBusConnection::sessionBus().connect(QString(), QString("/org/kylinssoclient/path"), QString("org.freedesktop.kylinssoclient.interface"), "keyChanged", this, SLOT(keyChangedSlot(QString)));
        // 将以后所有DBus调用的超时设置为 milliseconds
        m_cloudInterface->setTimeout(2147483647); // -1 为默认的25s超时
        qDebug()<<"NetWorkAcount"<<"  线程耗时: "<<timedebuge.elapsed()<<"ms";


    });

}

void DefaultApp::keyChangedSlot(const QString &key) {
    if(key == "default-open") {
        ui->browserComBoBox->clear();
        ui->audioComBoBox->clear();
        ui->imageComBoBox->clear();
        ui->textComBoBox->clear();
        ui->mailComBoBox->clear();
        ui->videoComBoBox->clear();
        initUI();
    }
}

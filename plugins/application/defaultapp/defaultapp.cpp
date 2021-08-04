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
#include <QVBoxLayout>
#include <QApplication>

#include "defaultapp.h"

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
    pluginType = APPLICATION;
}

DefaultApp::~DefaultApp() {
    if (!mFirstLoad) {

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
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);

        mDefaultString = tr("No program available");
        qDebug()<<mDefaultString;
        initUi(pluginWidget);
        initSearchText();
        initDefaultUI();
        initSlots();
        connectToServer();
    }
    return pluginWidget;
}

void DefaultApp::plugin_delay_control() {

}

const QString DefaultApp::name() const {

    return QStringLiteral("defaultapp");
}

void DefaultApp::initUi(QWidget *widget)
{
    QVBoxLayout *mverticalLayout = new QVBoxLayout(widget);
    mverticalLayout->setSpacing(8);
    mverticalLayout->setContentsMargins(0, 0, 40, 40);

    mTitleLabel = new TitleLabel(widget);

    QFrame *mDefaultFrame = new QFrame(widget);
    mDefaultFrame->setMinimumSize(QSize(550, 0));
    mDefaultFrame->setMaximumSize(QSize(16777215, 16777215));
    mDefaultFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *mDefaultLyt = new QVBoxLayout(mDefaultFrame);
    mDefaultLyt->setContentsMargins(0, 0, 0, 0);
    mDefaultLyt->setSpacing(0);

    mBrowserFrame = new QFrame(mDefaultFrame);
    mBrowserFrame->setMinimumSize(QSize(550, 60));
    mBrowserFrame->setMaximumSize(QSize(16777215, 60));
    mBrowserFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mBrowserLyt = new QHBoxLayout(mBrowserFrame);
    mBrowserLyt->setContentsMargins(16, 0, 16, 0);
    mBrowserLyt->setSpacing(0);

    mBrowserLabel = new FixLabel(mBrowserFrame);
    mBrowserLabel->setFixedSize(180,60);

    mBrowserCombo = new QComboBox(mBrowserFrame);
    mBrowserCombo->setFixedHeight(40);

    mBrowserLyt->addWidget(mBrowserLabel);
    mBrowserLyt->addWidget(mBrowserCombo);

    QFrame *line_1 = new QFrame(mDefaultFrame);
    line_1->setMinimumSize(QSize(0, 1));
    line_1->setMaximumSize(QSize(16777215, 1));
    line_1->setLineWidth(0);
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);

    mMailFrame = new QFrame(mDefaultFrame);
    mMailFrame->setMinimumSize(QSize(550, 60));
    mMailFrame->setMaximumSize(QSize(16777215, 60));
    mMailFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mMailLyt = new QHBoxLayout(mMailFrame);
    mMailLyt->setContentsMargins(16, 0, 16, 0);
    mMailLyt->setSpacing(0);

    mMailLabel = new FixLabel(mMailFrame);
    mMailLabel->setFixedSize(180,60);

    mMailCombo = new QComboBox(mMailFrame);
    mMailCombo->setFixedHeight(40);

    mMailLyt->addWidget(mMailLabel);
    mMailLyt->addWidget(mMailCombo);

    QFrame *line_2 = new QFrame(mDefaultFrame);
    line_2->setMinimumSize(QSize(0, 1));
    line_2->setMaximumSize(QSize(16777215, 1));
    line_2->setLineWidth(0);
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);

    mImageFrame = new QFrame(mDefaultFrame);
    mImageFrame->setMinimumSize(QSize(550, 60));
    mImageFrame->setMaximumSize(QSize(16777215, 60));
    mImageFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mImageLyt = new QHBoxLayout(mImageFrame);
    mImageLyt->setContentsMargins(16, 0, 16, 0);
    mImageLyt->setSpacing(0);

    mImageLabel = new FixLabel(mImageFrame);
    mImageLabel->setFixedSize(180,60);

    mImageCombo = new QComboBox(mImageFrame);
    mImageCombo->setFixedHeight(40);

    mImageLyt->addWidget(mImageLabel);
    mImageLyt->addWidget(mImageCombo);

    QFrame *line_3 = new QFrame(mDefaultFrame);
    line_3->setMinimumSize(QSize(0, 1));
    line_3->setMaximumSize(QSize(16777215, 1));
    line_3->setLineWidth(0);
    line_3->setFrameShape(QFrame::HLine);
    line_3->setFrameShadow(QFrame::Sunken);

    mAudioFrame = new QFrame(mDefaultFrame);
    mAudioFrame->setMinimumSize(QSize(550, 60));
    mAudioFrame->setMaximumSize(QSize(16777215, 60));
    mAudioFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mAudioLyt = new QHBoxLayout(mAudioFrame);
    mAudioLyt->setContentsMargins(16, 0, 16, 0);
    mAudioLyt->setSpacing(0);

    mAudioLabel = new FixLabel(mAudioFrame);
    mAudioLabel->setFixedSize(180,60);

    mAudioCombo = new QComboBox(mAudioFrame);
    mAudioCombo->setFixedHeight(40);

    mAudioLyt->addWidget(mAudioLabel);
    mAudioLyt->addWidget(mAudioCombo);

    QFrame *line_4 = new QFrame(mDefaultFrame);
    line_4->setMinimumSize(QSize(0, 1));
    line_4->setMaximumSize(QSize(16777215, 1));
    line_4->setLineWidth(0);
    line_4->setFrameShape(QFrame::HLine);
    line_4->setFrameShadow(QFrame::Sunken);

    mVideoFrame = new QFrame(mDefaultFrame);
    mVideoFrame->setMinimumSize(QSize(550, 60));
    mVideoFrame->setMaximumSize(QSize(16777215, 60));
    mVideoFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mVideoLyt = new QHBoxLayout(mVideoFrame);
    mVideoLyt->setContentsMargins(16, 0, 16, 0);
    mVideoLyt->setSpacing(0);

    mVideoLabel = new FixLabel(mVideoFrame);
    mVideoLabel->setFixedSize(180,60);

    mVideoCombo = new QComboBox(mVideoFrame);
    mVideoCombo->setFixedHeight(40);

    mVideoLyt->addWidget(mVideoLabel);
    mVideoLyt->addWidget(mVideoCombo);

    QFrame *line_5 = new QFrame(mDefaultFrame);
    line_5->setMinimumSize(QSize(0, 1));
    line_5->setMaximumSize(QSize(16777215, 1));
    line_5->setLineWidth(0);
    line_5->setFrameShape(QFrame::HLine);
    line_5->setFrameShadow(QFrame::Sunken);

    mTextFrame = new QFrame(mDefaultFrame);
    mTextFrame->setMinimumSize(QSize(550, 60));
    mTextFrame->setMaximumSize(QSize(16777215, 60));
    mTextFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mTextLyt = new QHBoxLayout(mTextFrame);
    mTextLyt->setContentsMargins(16, 0, 16, 0);
    mTextLyt->setSpacing(0);

    mTextLabel = new FixLabel(mTextFrame);
    mTextLabel->setFixedSize(180,60);

    mTextCombo = new QComboBox(mTextFrame);
    mTextCombo->setFixedHeight(40);

    mTextLyt->addWidget(mTextLabel);
    mTextLyt->addWidget(mTextCombo);

    mDefaultLyt->addWidget(mBrowserFrame);
    mDefaultLyt->addWidget(line_1);
    mDefaultLyt->addWidget(mMailFrame);
    mDefaultLyt->addWidget(line_2);
    mDefaultLyt->addWidget(mImageFrame);
    mDefaultLyt->addWidget(line_3);
    mDefaultLyt->addWidget(mAudioFrame);
    mDefaultLyt->addWidget(line_4);
    mDefaultLyt->addWidget(mVideoFrame);
    mDefaultLyt->addWidget(line_5);
    mDefaultLyt->addWidget(mTextFrame);

    mverticalLayout->addWidget(mTitleLabel);
    mverticalLayout->addWidget(mDefaultFrame);
    mverticalLayout->addStretch();
}

void DefaultApp::initSlots() {
    connect(mBrowserCombo, static_cast<void (QComboBox::*)(int )> (&QComboBox::currentIndexChanged), this, &DefaultApp::browserComBoBox_changed_cb);
    connect(mMailCombo, static_cast<void (QComboBox::*)(int )> (&QComboBox::currentIndexChanged), this, &DefaultApp::mailComBoBox_changed_cb);
    connect(mImageCombo, static_cast<void (QComboBox::*)(int )> (&QComboBox::currentIndexChanged), this, &DefaultApp::imageComBoBox_changed_cb);
    connect(mAudioCombo, static_cast<void (QComboBox::*)(int )> (&QComboBox::currentIndexChanged), this, &DefaultApp::audioComBoBox_changed_cb);
    connect(mVideoCombo, static_cast<void (QComboBox::*)(int )> (&QComboBox::currentIndexChanged), this, &DefaultApp::videoComBoBox_changed_cb);
    connect(mTextCombo, static_cast<void (QComboBox::*)(int )> (&QComboBox::currentIndexChanged), this, &DefaultApp::textComBoBox_changed_cb);
}

void DefaultApp::initDefaultUI() {

    // 若默认应用被卸载，系统会自动设置某个其它应用为默认应用，若不存在其它应用，则显示"无可用程序"

    QTime timedebuge;//声明一个时钟对象
    timedebuge.start();//开始计时
    // BROWSER  
    QString currentbrowser(getDefaultAppId(BROWSERTYPE)); //获取当前
    QByteArray ba = QString(DESKTOPPATH + currentbrowser).toUtf8();
    GDesktopAppInfo * browserinfo = g_desktop_app_info_new_from_filename(ba.constData());
    QString appname = g_app_info_get_name(G_APP_INFO(browserinfo));
    const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(browserinfo)));
    QIcon appicon;
    appicon = QIcon::fromTheme(QString(iconname));
    if (appname != NULL) {
        mBrowserCombo->addItem(appicon, appname, currentbrowser);
        mBrowserCombo->setCurrentText(appname);
    } else {
        mBrowserCombo->addItem(mDefaultString);
        mBrowserCombo->setCurrentText(mDefaultString);
    }


    QFuture<void> browserfuture = QtConcurrent::run([=]() {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString currentbrowser(getDefaultAppId(BROWSERTYPE)); //获取当前
        QVector<AppList> list = getAppIdList(BROWSERTYPE); //获取可选列表
        if (!list.isEmpty()) {
            for (int i = 0; i < list.size(); i++) {
                QString single(list[i].strAppid);
                QByteArray ba = QString(DESKTOPPATH + single).toUtf8();
                GDesktopAppInfo * browserinfo = g_desktop_app_info_new_from_filename(ba.constData());
                QString appname = g_app_info_get_name(G_APP_INFO(browserinfo));
                const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(browserinfo)));
                QIcon appicon;
                appicon = QIcon::fromTheme(QString(iconname));
                browserList << appname;
                if (currentbrowser == single) {
                    continue;
                }
                mBrowserCombo->addItem(appicon, appname, single);
            }
        }
        qDebug() <<"browser耗时："<<timedebuge.elapsed()<<"ms";//输出计时
        qDebug() << "BROWSER线程" << QThread::currentThreadId() << QThread::currentThread();
    });


    // IMAGE
    {
        QString currentimage(getDefaultAppId(IMAGETYPE));
        QByteArray ba = QString(DESKTOPPATH + currentimage).toUtf8();
        GDesktopAppInfo * imageinfo = g_desktop_app_info_new_from_filename(ba.constData());
        QString appname = g_app_info_get_name(G_APP_INFO(imageinfo));
        const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(imageinfo)));
        QIcon appicon;
        appicon = QIcon::fromTheme(QString(iconname));
        if (appname != NULL) {
            mImageCombo->addItem(appicon, appname, currentbrowser);
            mImageCombo->setCurrentText(appname);
        } else {
            mImageCombo->addItem(mDefaultString);
            mImageCombo->setCurrentText(mDefaultString);
        }
    }

    QFuture<void> imagefuture = QtConcurrent::run([=]() {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString currentimage(getDefaultAppId(IMAGETYPE));
        QVector<AppList> imagelist = getAppIdList(IMAGETYPE);
        if (!imagelist.isEmpty()) {
            for (int i = 0; i < imagelist.size(); i++) {
                QString single(imagelist[i].strAppid);
                QByteArray ba = QString(DESKTOPPATH + single).toUtf8();
                GDesktopAppInfo * imageinfo = g_desktop_app_info_new_from_filename(ba.constData());
                QString appname = g_app_info_get_name(G_APP_INFO(imageinfo));
                const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(imageinfo)));
                QIcon appicon;
                appicon = QIcon::fromTheme(QString(iconname));
                if (currentimage == single) {
                    continue;
                }
                if(!browserList.contains(appname)) {
                    mImageCombo->addItem(appicon, appname, single);
                }
            }
        }
        qDebug() <<"IMAGE耗时："<<timedebuge.elapsed()<<"ms";//输出计时
        qDebug() << "IMAGE线程" << QThread::currentThreadId() << QThread::currentThread();
    });

    // MAIL
    {
        QString currentmail(getDefaultAppId(MAILTYPE));
        QByteArray ba = QString(DESKTOPPATH + currentmail).toUtf8();
        GDesktopAppInfo * mailinfo = g_desktop_app_info_new_from_filename(ba.constData());
        QString appname = g_app_info_get_name(G_APP_INFO(mailinfo));
        const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(mailinfo)));
        QIcon appicon;
        appicon = QIcon::fromTheme(QString(iconname));
        if (appname != NULL) {
            mMailCombo->addItem(appicon, appname, currentbrowser);
            mMailCombo->setCurrentText(appname);
        } else {
            mMailCombo->addItem(mDefaultString);
            mMailCombo->setCurrentText(mDefaultString);
        }
    }

    QFuture<void> mailfuture = QtConcurrent::run([=]() {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString currentmail(getDefaultAppId(MAILTYPE));
        QVector<AppList> maillist = getAppIdList(MAILTYPE);
        if (!maillist.isEmpty()) {
            for (int i = 0; i < maillist.size(); i++) {
                QString single(maillist[i].strAppid);
                QByteArray ba = QString(DESKTOPPATH + single).toUtf8();
                GDesktopAppInfo * mailinfo = g_desktop_app_info_new_from_filename(ba.constData());
                QString appname = g_app_info_get_name(G_APP_INFO(mailinfo));
                const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(mailinfo)));
                QIcon appicon;
                appicon = QIcon::fromTheme(QString(iconname));
                if (currentmail == single) {
                    continue;
                }
              //  mMailCombo->addItem(appicon, appname, single);
            }
        }
        qDebug() <<"MAIL耗时："<<timedebuge.elapsed()<<"ms";//输出计时
        qDebug() << "MAIL线程" << QThread::currentThreadId() << QThread::currentThread();
    });

    // AUDIO
    {
        QString currentaudio(getDefaultAppId(AUDIOTYPE));
        QByteArray ba = QString(DESKTOPPATH + currentaudio).toUtf8();
        GDesktopAppInfo * audioinfo = g_desktop_app_info_new_from_filename(ba.constData());
        QString appname = g_app_info_get_name(G_APP_INFO(audioinfo));
        const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(audioinfo)));
        QIcon appicon;
        appicon = QIcon::fromTheme(QString(iconname));
        if (appname != NULL) {
            mAudioCombo->addItem(appicon, appname, currentbrowser);
            mAudioCombo->setCurrentText(appname);
        } else {
            mAudioCombo->addItem(mDefaultString);
            mAudioCombo->setCurrentText(mDefaultString);
        }
    }

    QFuture<void> audiofuture = QtConcurrent::run([=]() {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString currentaudio(getDefaultAppId(AUDIOTYPE));
        QVector<AppList> audiolist = getAppIdList(AUDIOTYPE);
        if (!audiolist.isEmpty()) {
            for (int i = 0; i < audiolist.size(); i++) {
                QString single(audiolist[i].strAppid);
                QByteArray ba = QString(DESKTOPPATH + single).toUtf8();
                GDesktopAppInfo * audioinfo = g_desktop_app_info_new_from_filename(ba.constData());
                QString appname = g_app_info_get_name(G_APP_INFO(audioinfo));
                const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(audioinfo)));
                QIcon appicon;
                appicon = QIcon::fromTheme(QString(iconname));
                if (currentaudio == single) {
                    continue;
                }
                mAudioCombo->addItem(appicon, appname, single);
            }
        }
        qDebug() <<"AUDIO耗时："<<timedebuge.elapsed()<<"ms";//输出计时
        qDebug() << "AUDIO线程" << QThread::currentThreadId() << QThread::currentThread();
    });

    // VIDEO
    {
        QString currentvideo(getDefaultAppId(VIDEOTYPE));
        QByteArray ba = QString(DESKTOPPATH + currentvideo).toUtf8();
        GDesktopAppInfo * videoinfo = g_desktop_app_info_new_from_filename(ba.constData());
        QString appname = g_app_info_get_name(G_APP_INFO(videoinfo));
        const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(videoinfo)));
        QIcon appicon;
        appicon = QIcon::fromTheme(QString(iconname));
        if (appname != NULL) {
            mVideoCombo->addItem(appicon, appname, currentbrowser);
            mVideoCombo->setCurrentText(appname);
        } else {
            mVideoCombo->addItem(mDefaultString);
            mVideoCombo->setCurrentText(mDefaultString);
        }
    }

    QFuture<void> videofuture = QtConcurrent::run([=]() {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString currentvideo(getDefaultAppId(VIDEOTYPE));
        QVector<AppList> videolist = getAppIdList(VIDEOTYPE);
        if (!videolist.isEmpty()) {
            for (int i = 0; i < videolist.size(); i++) {
                QString single(videolist[i].strAppid);
                QByteArray ba = QString(DESKTOPPATH + single).toUtf8();
                GDesktopAppInfo * videoinfo = g_desktop_app_info_new_from_filename(ba.constData());
                QString appname = g_app_info_get_name(G_APP_INFO(videoinfo));
                const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(videoinfo)));
                QIcon appicon;
                appicon = QIcon::fromTheme(QString(iconname));
                if (currentvideo == single) {
                    continue;
                }
                mVideoCombo->addItem(appicon, appname, single);
            }
        }
        qDebug() <<"VIDEO耗时："<<timedebuge.elapsed()<<"ms";//输出计时
        qDebug() << "VIDEO线程" << QThread::currentThreadId() << QThread::currentThread();
    });

    // TEXT
    {
        QString currenttext(getDefaultAppId(TEXTTYPE));
        QByteArray ba = QString(DESKTOPPATH + currenttext).toUtf8();
        GDesktopAppInfo * textinfo = g_desktop_app_info_new_from_filename(ba.constData());
        QString appname = g_app_info_get_name(G_APP_INFO(textinfo));
        const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(textinfo)));
        QIcon appicon;
        appicon = QIcon::fromTheme(QString(iconname));
        if (appname != NULL) {
            mTextCombo->addItem(appicon, appname, currentbrowser);
            mTextCombo->setCurrentText(appname);
        } else {
            mTextCombo->addItem(mDefaultString);
            mTextCombo->setCurrentText(mDefaultString);
        }
    }

    QFuture<void> textfuture = QtConcurrent::run([=]() {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString currenttext(getDefaultAppId(TEXTTYPE));
        QVector<AppList> textlist = getAppIdList(TEXTTYPE);
        if (!textlist.isEmpty()) {
            for (int i = 0; i < textlist.size(); i++) {
                QString single(textlist[i].strAppid);
                QByteArray ba = QString(DESKTOPPATH + single).toUtf8();
                GDesktopAppInfo * textinfo = g_desktop_app_info_new_from_filename(ba.constData());
                QString appname = g_app_info_get_name(G_APP_INFO(textinfo));
                const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(textinfo)));
                QIcon appicon;
                appicon = QIcon::fromTheme(QString(iconname));
                if (currenttext == single) {
                    continue;
                }
                mTextCombo->addItem(appicon, appname, single);
            }
        }
        qDebug() <<"TEXT耗时："<<timedebuge.elapsed()<<"ms";//输出计时
        qDebug() << "TEXT线程" << QThread::currentThreadId() << QThread::currentThread();
    });

    browserfuture.waitForFinished();
    imagefuture.waitForFinished();
    mailfuture.waitForFinished();
    audiofuture.waitForFinished();
    videofuture.waitForFinished();
    textfuture.waitForFinished();
    qDebug()<<"initUI耗时："<<timedebuge.elapsed()<<"ms";//输出计时
    qDebug() << "主线程" << QThread::currentThreadId() << QThread::currentThread();
}

void DefaultApp::initSearchText() {
    mTitleLabel->setText(QApplication::translate("DefaultAppWindow", "Select Default Application", nullptr));
    //~ contents_path /defaultapp/Browser
    mBrowserLabel->setText(tr("Browser"));
    //~ contents_path /defaultapp/Mail
    mMailLabel->setText(tr("Mail"));
    //~ contents_path /defaultapp/Image Viewer
    mImageLabel->setText(tr("Image Viewer"));
    //~ contents_path /defaultapp/Audio Player
    mAudioLabel->setText(tr("Audio Player"));
    //~ contents_path /defaultapp/Video Player
    mVideoLabel->setText(tr("Video Player"));
    //~ contents_path /defaultapp/Text Editor
    mTextLabel->setText(tr("Text Editor"));
}

void DefaultApp::browserComBoBox_changed_cb(int index) {

    QFuture<void> future = QtConcurrent::run([=] {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString appid = mBrowserCombo->itemData(index).toString();
        QByteArray ba = appid.toUtf8(); // QString to char *
        qDebug()<<"browserComBoBox_changed_cb："<<timedebuge.elapsed()<<"ms" << appid;
        setWebBrowsersDefaultProgram(ba.data());
        qDebug()<<"browserComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms" << appid;//输出计时
    });
    future.waitForFinished();
}

void DefaultApp::mailComBoBox_changed_cb(int index) {

    QFuture<void> future = QtConcurrent::run([=] {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString appid = mMailCombo->itemData(index).toString();
        QByteArray ba = appid.toUtf8(); // QString to char *
        setMailReadersDefaultProgram(ba.data());
        qDebug()<<"mailComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";//输出计时
    });
    future.waitForFinished();
}

void DefaultApp::imageComBoBox_changed_cb(int index) {

    QFuture<void> future = QtConcurrent::run([=] {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString appid = mImageCombo->itemData(index).toString();
        QByteArray ba = appid.toUtf8(); // QString to char *
        setImageViewersDefaultProgram(ba.data());
        qDebug()<<"imageComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";//输出计时
    });
    future.waitForFinished();
}

void DefaultApp::audioComBoBox_changed_cb(int  index) {

    QFuture<void> future = QtConcurrent::run([=] {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString appid = mAudioCombo->itemData(index).toString();
        QByteArray ba = appid.toUtf8(); // QString to char *
        setAudioPlayersDefaultProgram(ba.data());
        qDebug()<<"audioComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";//输出计时
    });
    future.waitForFinished();
}

void DefaultApp::videoComBoBox_changed_cb(int index) {

    QFuture<void> future = QtConcurrent::run([=] {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString appid = mVideoCombo->itemData(index).toString();
        QByteArray ba = appid.toUtf8(); // QString to char *
        setVideoPlayersDefaultProgram(ba.data());
        qDebug() << __FUNCTION__  << QThread::currentThreadId() << QThread::currentThread();
        qDebug()<<"videoComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";//输出计时
    });
    future.waitForFinished();
}

void DefaultApp::textComBoBox_changed_cb(int index) {

    QFuture<void> future = QtConcurrent::run([=] {
        QTime timedebuge;//声明一个时钟对象
        timedebuge.start();//开始计时
        QString appid = mTextCombo->itemData(index).toString();
        QByteArray ba = appid.toUtf8(); // QString to char *
        setTextEditorsDefautlProgram(ba.data());
        qDebug()<<"textComBoBox_changed_cb线程耗时："<<timedebuge.elapsed()<<"ms";//输出计时
    });
    future.waitForFinished();
}

void DefaultApp::resetDefaultApp() {

//    mBrowserCombo->setCurrentText(mDefaultBrowser);
//    mImageCombo->setCurrentText(mDefaultPic);
//    mAudioCombo->setCurrentText(mDefaultAdudio);
//    mVideoCombo->setCurrentText(mDefaultVideo);
//    mTextCombo->setCurrentText(mDefaultText);

//    if (mDefaultMail.isEmpty()) {
//        mMailCombo->setCurrentIndex(0);
//    } else {
//        mMailCombo->setCurrentText(mDefaultMail);
//    }
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
}

void DefaultApp::keyChangedSlot(const QString &key) {
    if(key == "default-open") {
        mBrowserCombo->clear();
        mAudioCombo->clear();
        mImageCombo->clear();
        mTextCombo->clear();
        mMailCombo->clear();
        mVideoCombo->clear();
        initDefaultUI();
    }
}

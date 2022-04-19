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

#include "about.h"
#include "trialdialog.h"

#include <KFormat>
#include <unistd.h>
#include <QFile>
#include <QGridLayout>
#include <QPluginLoader>
#include <QEvent>
#include <QMessageBox>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <QSvgRenderer>

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#elif defined(Q_OS_FREEBSD)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include <QProcess>
#include <QFile>
#include <QDebug>
#include <QStorageInfo>
#include <QtMath>

#define THEME_STYLE_SCHEMA "org.ukui.style"
#define STYLE_NAME_KEY "style-name"
#define CONTAIN_STYLE_NAME_KEY "styleName"
#define UKUI_DEFAULT "ukui-default"
#define UKUI_DARK "ukui-dark"

const QString vTen        = "v10";
const QString vTenEnhance = "v10.1";
const QString vFour = "v4";

About::About() : mFirstLoad(true)
{
    pluginName = tr("About");
    pluginType = SYSTEM;
}

About::~About()
{
    if (!mFirstLoad) {

    }
}

QString About::plugini18nName()
{
    return pluginName;
}

int About::pluginTypes()
{
    return pluginType;
}

QWidget *About::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;

        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        pluginWidget->setObjectName("pluginWidget");

        initUI(pluginWidget);
        initSearchText();
        initActiveDbus();

        setupVersionCompenent();
        setVersionNumCompenent();
        setupDesktopComponent();
        setHostNameCompenet();
        setupSystemVersion();
        setupKernelCompenent();
        setupSerialComponent();
        setPrivacyCompent();

        if (!mDiskFrame->isHidden())
            setupDiskCompenet();
    }
    return pluginWidget;
}

const QString About::name() const
{
    return QStringLiteral("About");
}

bool About::isShowOnHomePage() const
{
    return true;
}

QIcon About::icon() const
{
    return QIcon();
}

bool About::isEnable() const
{
    return true;
}

/* 初始化整体UI布局 */
void About::initUI(QWidget *widget)
{
    QVBoxLayout *mverticalLayout = new QVBoxLayout(widget);
    mverticalLayout->setSpacing(0);
    mverticalLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *Aboutwidget = new QWidget(widget);
    Aboutwidget->setMinimumSize(QSize(550, 0));
    Aboutwidget->setMaximumSize(QSize(16777215, 16777215));

    QVBoxLayout *AboutLayout = new QVBoxLayout(Aboutwidget);
    AboutLayout->setContentsMargins(0, 0, 0, 0);
    AboutLayout->setSpacing(8);

    mTitleLabel = new TitleLabel(Aboutwidget);
    AboutLayout->addWidget(mTitleLabel);

    mInformationFrame = new QFrame(Aboutwidget);
    setFrame_Box(mInformationFrame);

    QVBoxLayout *mInformationLayout = new QVBoxLayout(mInformationFrame);
    mInformationLayout->setContentsMargins(16, 16, 16, 8);

    mLogoLabel = new FixLabel(mInformationFrame);
    mLogoLabel->setFixedSize(130, 50);

    mInformationLayout->addWidget(mLogoLabel);

     /* 版本名称 */
    mVersionFrame = new QFrame(mInformationFrame);
    mVersionFrame->installEventFilter(this);
    setFrame_NoFrame(mVersionFrame);

    QHBoxLayout *mVersionLayout = new QHBoxLayout(mVersionFrame);
    mVersionLayout->setContentsMargins(0, 0, 16, 0);

    mVersionLabel_1 = new FixLabel(mVersionFrame);
    mVersionLabel_1->setFixedSize(80,30);

    mVersionLabel_2 = new QLabel(mVersionFrame);
    mVersionLabel_2->setFixedHeight(30);

    mVersionLayout->addWidget(mVersionLabel_1);
    mVersionLayout->addSpacing(80);
    mVersionLayout->addWidget(mVersionLabel_2);
    mVersionLayout->addStretch();

    mInformationLayout->addWidget(mVersionFrame);

     /* 版本号 */
    mVersionNumberFrame = new QFrame(mInformationFrame);
    setFrame_NoFrame(mVersionNumberFrame);

    QHBoxLayout *mVersionNumberLayout = new QHBoxLayout(mVersionNumberFrame);
    mVersionNumberLayout->setContentsMargins(0, 0, 16, 0);

    mVersionNumberLabel_1 = new FixLabel(tr("Version Number") , mVersionNumberFrame);
    mVersionNumberLabel_1->setFixedSize(80,30);

    mVersionNumberLabel_2 = new FixLabel(mVersionNumberFrame);
    mVersionNumberLabel_2->setFixedHeight(30);

    mVersionNumberLayout->addWidget(mVersionNumberLabel_1);
    mVersionNumberLayout->addSpacing(80);
    mVersionNumberLayout->addWidget(mVersionNumberLabel_2);
    mVersionNumberLayout->addStretch();

    mInformationLayout->addWidget(mVersionNumberFrame);

    /* 内部版本 */
    mInterVersionFrame = new QFrame(mInformationFrame);
    setFrame_NoFrame(mInterVersionFrame);

    QHBoxLayout *mInterVersionLayout = new QHBoxLayout(mInterVersionFrame);
    mInterVersionLayout->setContentsMargins(0, 0, 16, 0);

    mInterVersionLabel_1 = new FixLabel(tr("InterVersion") , mInterVersionFrame);
    mInterVersionLabel_1->setFixedSize(80,30);

    mInterVersionLabel_2 = new FixLabel(mInterVersionFrame);
    mInterVersionLabel_2->setFixedHeight(30);

    mInterVersionLayout->addWidget(mInterVersionLabel_1);
    mInterVersionLayout->addSpacing(80);
    mInterVersionLayout->addWidget(mInterVersionLabel_2);
    mInterVersionLayout->addStretch();

    mInformationLayout->addWidget(mInterVersionFrame);

     /* Intel版本号 */
    mVersionNumFrame = new QFrame(mInformationFrame);
    setFrame_NoFrame(mVersionNumFrame);

    QHBoxLayout *mVersionNumLayout = new QHBoxLayout(mVersionNumFrame);
    mVersionNumLayout->setContentsMargins(0, 0, 16, 0);

    mVersionNumLabel_1 = new FixLabel(mVersionNumFrame);
    mVersionNumLabel_1->setFixedSize(80,30);

    mVersionNumLabel_2 = new FixLabel(mVersionNumFrame);
    mVersionNumLabel_2->setFixedHeight(30);

    mVersionNumLayout->addWidget(mVersionNumLabel_1);
    mVersionNumLayout->addSpacing(80);
    mVersionNumLayout->addWidget(mVersionNumLabel_2);
    mVersionNumLayout->addStretch();

    mInformationLayout->addWidget(mVersionNumFrame);

    mHostNameFrame = new QFrame(mInformationFrame);
    setFrame_NoFrame(mHostNameFrame);

    QHBoxLayout *mHostNameLayout = new QHBoxLayout(mHostNameFrame);
    mHostNameLayout->setContentsMargins(0, 0, 16, 0);

    QHBoxLayout *mHostNameLayout_1 = new QHBoxLayout();
    mHostNameLayout_1->setContentsMargins(0, 0, 0, 0);

    mHostNameLabel_1 = new FixLabel(tr("HostName") , mHostNameFrame);
    mHostNameLabel_1->setFixedSize(80,30);

    mHostNameLabel_2 = new FixLabel(mHostNameFrame );
    mHostNameLabel_2->setFixedHeight(30);

    mHostNameLabel_3 = new FixLabel(mHostNameFrame);
    mHostNameLabel_3->setFixedSize(16, 16);
    mHostNameLabel_3->setProperty("useIconHighlightEffect", 0x8);
    mHostNameLabel_3->setPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(mHostNameLabel_3->size()));
    mHostNameLabel_3->setScaledContents(true);

    mHostNameLabel_2->installEventFilter(this);
    mHostNameLabel_3->installEventFilter(this);

    mHostNameLayout_1->setSpacing(4);
    mHostNameLayout_1->addWidget(mHostNameLabel_2);
    mHostNameLayout_1->addWidget(mHostNameLabel_3);

    mHostNameLayout->addWidget(mHostNameLabel_1);
    mHostNameLayout->addSpacing(80);
    mHostNameLayout->addLayout(mHostNameLayout_1);
    mHostNameLayout->addStretch();

    mInformationLayout->addWidget(mHostNameFrame);

    mKernelFrame = new QFrame(mInformationFrame);
    setFrame_NoFrame(mKernelFrame);

    QHBoxLayout *mKernerLayout = new QHBoxLayout(mKernelFrame);
    mKernerLayout->setContentsMargins(0, 0, 16, 0);

    mKernelLabel_1 = new FixLabel(mKernelFrame);
    mKernelLabel_1->setFixedSize(80,30);

    mKernelLabel_2 = new FixLabel(mKernelFrame);
    mKernelLabel_2->setFixedHeight(30);

    mKernerLayout->addWidget(mKernelLabel_1);
    mKernerLayout->addSpacing(80);
    mKernerLayout->addWidget(mKernelLabel_2);
    mKernerLayout->addStretch();

    mInformationLayout->addWidget(mKernelFrame);

    mCpuFrame = new QFrame(mInformationFrame);
    setFrame_NoFrame(mCpuFrame);

    QHBoxLayout *mCpuLayout = new QHBoxLayout(mCpuFrame);
    mCpuLayout->setContentsMargins(0, 0, 16, 0);

    mCpuLabel_1 = new FixLabel(mCpuFrame);
    mCpuLabel_1->setFixedSize(80,30);

    mCpuLabel_2 = new FixLabel(mCpuFrame);
    mCpuLabel_2->setFixedHeight(30);

    mCpuLayout->addWidget(mCpuLabel_1);
    mCpuLayout->addSpacing(80);
    mCpuLayout->addWidget(mCpuLabel_2);
    mCpuLayout->addStretch();

    mInformationLayout->addWidget(mCpuFrame);

    mMemoryFrame = new QFrame(mInformationFrame);
    setFrame_NoFrame(mMemoryFrame);

    QHBoxLayout *mMemoryLayout = new QHBoxLayout(mMemoryFrame);
    mMemoryLayout->setContentsMargins(0, 0, 16, 0);

    mMemoryLabel_1 = new FixLabel(mMemoryFrame);
    mMemoryLabel_1->setFixedSize(80,30);

    mMemoryLabel_2 = new FixLabel(mMemoryFrame);
    mMemoryLabel_2->setFixedHeight(30);

    mMemoryLayout->addWidget(mMemoryLabel_1);
    mMemoryLayout->addSpacing(80);
    mMemoryLayout->addWidget(mMemoryLabel_2);
    mMemoryLayout->addStretch();

    mInformationLayout->addWidget(mMemoryFrame);

    mDiskFrame = new QFrame(mInformationFrame);
    setFrame_NoFrame(mDiskFrame);

    mDiskLayout = new QHBoxLayout(mDiskFrame);
    mDiskLayout->setContentsMargins(0, 0, 16, 0);

    mDiskLabel_1 = new FixLabel(mDiskFrame);
    mDiskLabel_1->setFixedSize(80,30);

    mDiskLabel_2 = new FixLabel(mDiskFrame);
    mDiskLabel_2->setFixedHeight(30);

    mDiskLayout->addWidget(mDiskLabel_1);
    mDiskLayout->addSpacing(80);
    mDiskLayout->addWidget(mDiskLabel_2);
    mDiskLayout->addStretch();

    mInformationLayout->addWidget(mDiskFrame);

    mDesktopFrame = new QFrame(mInformationFrame);
    setFrame_NoFrame(mDesktopFrame);

    QHBoxLayout *mDesktopLayout = new QHBoxLayout(mDesktopFrame);
    mDesktopLayout->setContentsMargins(0, 0, 16, 0);

    mDesktopLabel_1 = new FixLabel(mDesktopFrame);
    mDesktopLabel_1->setFixedSize(80,30);

    mDesktopLabel_2 = new FixLabel(mDesktopFrame);
    mDesktopLabel_2->setFixedHeight(30);

    mDesktopLayout->addWidget(mDesktopLabel_1);
    mDesktopLayout->addSpacing(80);
    mDesktopLayout->addWidget(mDesktopLabel_2);
    mDesktopLayout->addStretch();

    mInformationLayout->addWidget(mDesktopFrame);

    mUsernameFrame = new QFrame(mInformationFrame);
    setFrame_NoFrame(mUsernameFrame);

    QHBoxLayout *mUsernameLayout = new QHBoxLayout(mUsernameFrame);
    mUsernameLayout->setContentsMargins(0, 0, 16, 0);

    mUsernameLabel_1 = new FixLabel(mUsernameFrame);
    mUsernameLabel_1->setFixedSize(80,30);

    mUsernameLabel_2 = new FixLabel(mUsernameFrame);
    mUsernameLabel_2->setFixedHeight(30);

    mUsernameLayout->addWidget(mUsernameLabel_1);
    mUsernameLayout->addSpacing(80);
    mUsernameLayout->addWidget(mUsernameLabel_2);
    mUsernameLayout->addStretch();

    mInformationLayout->addWidget(mUsernameFrame);

    mPriTitleLabel = new TitleLabel(Aboutwidget);
    mPriTitleLabel->setText(tr("Privacy and agreement"));

    mPrivacyFrame = new QFrame(Aboutwidget);
    setFrame_Box(mPrivacyFrame);

    QHBoxLayout *mPrivacyLayout = new QHBoxLayout(mPrivacyFrame);
    mPrivacyLayout->setContentsMargins(16, 0, 16, 0);
    mPrivacyLayout->setSpacing(48);
    mPriBtn = new SwitchButton(mPrivacyFrame);

    FixLabel *mPriLabel_1 = new FixLabel(tr("Send optional diagnostic data"),  mPrivacyFrame);
    mPriLabel_1->setContentsMargins(0 , 12 , 0 , 0);
    LightLabel *mPriLabel_2 = new LightLabel(tr("By sending us diagnostic data, improve the system experience and solve your problems faster"),  mPrivacyFrame);
    mPriLabel_2->setContentsMargins(0 , 0 , 0 , 12);
    QVBoxLayout *mverticalLayout_2 = new QVBoxLayout;
    mverticalLayout_2->setSpacing(0);
    mverticalLayout_2->setContentsMargins(0 , 0 , 0 , 0);
    mverticalLayout_2->addWidget(mPriLabel_1);
    mverticalLayout_2->addWidget(mPriLabel_2 );

    mPrivacyLayout->addLayout(mverticalLayout_2);
    mPrivacyLayout->addWidget(mPriBtn);

    mActivationFrame = new QFrame(Aboutwidget);
    setFrame_Box(mActivationFrame);

    QHBoxLayout *mActivationLayout_1 = new QHBoxLayout(mActivationFrame);
    mActivationLayout_1->setContentsMargins(16, 16, 16, 8);
    mActivationLayout_1->setSpacing(8);

    QGridLayout *mActivationLayout = new QGridLayout();
    mActivationLayout->setVerticalSpacing(8);

    mStatusLabel_1 = new FixLabel(mActivationFrame);
    mStatusLabel_1->setFixedSize(160,30);
    mStatusLabel_2 = new FixLabel(mActivationFrame);
    mSequenceLabel_1 = new FixLabel(mActivationFrame);
    mSequenceLabel_1->setFixedSize(160,30);
    mSequenceLabel_2 = new FixLabel(mActivationFrame);
    mTimeLabel_1 = new FixLabel(mActivationFrame);
    mTimeLabel_1->setFixedSize(160,30);
    mTimeLabel_2 = new FixLabel(mActivationFrame);
    mActivationBtn = new QPushButton(mActivationFrame);

    mActivationBtn->setFixedSize(120,40);

    mActivationLayout->addWidget(mStatusLabel_1, 0, 0, 1, 1);
    mActivationLayout->addWidget(mStatusLabel_2, 0, 1, 1, 3,Qt::AlignLeft);
    mActivationLayout->addWidget(mSequenceLabel_1, 1, 0, 1, 1);
    mActivationLayout->addWidget(mSequenceLabel_2, 1, 1, 1, 3,Qt::AlignLeft);
    mActivationLayout->addWidget(mTimeLabel_1, 2, 0, 1, 1);
    mActivationLayout->addWidget(mTimeLabel_2, 2, 1, 1, 3,Qt::AlignLeft);

    mActivationLayout_1->addLayout(mActivationLayout);
    mActivationLayout_1->addWidget(mActivationBtn);

    QString currentyear("2022");
    mTipLabel = new FixLabel(QString(tr("Copyright © 2009-%1 KylinSoft. All rights reserved.")).arg(currentyear) , Aboutwidget);
    mTipLabel->setContentsMargins(16 , 0 , 0 , 0);

    mBtnFrame = new QFrame(Aboutwidget);
    mBtnFrame->setMinimumSize(QSize(550, 0));
    mBtnFrame->setMaximumSize(QSize(16777215, 16777215));
    mBtnFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mBtnLyt = new QHBoxLayout(mBtnFrame);
    mBtnLyt->setContentsMargins(16, 0, 0, 0);
    mTrialLabel = new FixLabel(tr("<<Protocol>>") , mBtnFrame);
    mAndLabel = new FixLabel(tr("and") , mBtnFrame);
    mAgreeLabel = new FixLabel(tr("<<Privacy>>") , mBtnFrame);
    mTrialLabel->setStyleSheet("background: transparent;color:#2FB3E8;border-width:1px;"
                     "text-decoration:underline;border-style:none none none;");
    mAgreeLabel->setStyleSheet("background: transparent;color:#2FB3E8;border-width:1px;"
                     "text-decoration:underline;border-style:none none none;");
    mTrialLabel->installEventFilter(this);
    mAgreeLabel->installEventFilter(this);
    mBtnLyt->addWidget(mTrialLabel);
    mBtnLyt->addWidget(mAndLabel);
    mBtnLyt->addWidget(mAgreeLabel);
    mBtnLyt->addStretch();

    mHoldTitleLabel = new TitleLabel(Aboutwidget);

    mHoldWidget = new QFrame(Aboutwidget);
    mHoldWidget->setMinimumSize(QSize(550, 112));
    mHoldWidget->setMaximumSize(QSize(16777215, 112));
    mHoldWidget->setFrameShape(QFrame::Box);

    QHBoxLayout *mHoldLayout = new QHBoxLayout(mHoldWidget);
    mHoldLayout->setSpacing(8);
    mHoldLayout->setContentsMargins(16, 0, 8, 0);

    mQrCodeWidget = new QWidget(mHoldWidget);
    mQrCodeWidget->setFixedSize(96,96);

    mHpLabel = new QLabel(mHoldWidget);
    mEducateIconLabel = new FixLabel(mHoldWidget);
    mEducateIconLabel->setFixedSize(96,96);
    mEducateLabel = new QLabel(mHoldWidget);

    mHpLabel->setWordWrap(true);
    mEducateLabel->setWordWrap(true);

    mHpBtn = new QPushButton(mHoldWidget);
    mEducateBtn = new QPushButton(mHoldWidget);

    QVBoxLayout *Lyt_1 = new QVBoxLayout();
    Lyt_1->setContentsMargins(0, 8, 0, 8);
    Lyt_1->setSpacing(4);
    Lyt_1->addStretch();
    Lyt_1->addWidget(mHpLabel);
    Lyt_1->addWidget(mHpBtn);
    Lyt_1->addStretch();

    QVBoxLayout *Lyt_2 = new QVBoxLayout();
    Lyt_2->setContentsMargins(0, 8, 0, 8);
    Lyt_2->setSpacing(4);
    Lyt_2->addStretch();
    Lyt_2->addWidget(mEducateLabel);
    Lyt_2->addWidget(mEducateBtn);
    Lyt_2->addStretch();

    mHoldLayout->addWidget(mQrCodeWidget);
    mHoldLayout->addLayout(Lyt_1);
    mHoldLayout->addSpacing(16);
    mHoldLayout->addStretch(1);
    mHoldLayout->addWidget(mEducateIconLabel);
    mHoldLayout->addLayout(Lyt_2);
    mHoldLayout->addStretch(1);

    AboutLayout->addWidget(mInformationFrame);
    AboutLayout->addWidget(mActivationFrame);
    AboutLayout->addWidget(mTipLabel);
    item = new QSpacerItem(10 , 32);
    AboutLayout->addSpacerItem(item);
//    AboutLayout->addSpacing(32);
    AboutLayout->addWidget(mPriTitleLabel);
    AboutLayout->addWidget(mPrivacyFrame);
    AboutLayout->addSpacing(0);
    AboutLayout->addWidget(mBtnFrame);
    AboutLayout->addWidget(mHoldTitleLabel);
    AboutLayout->addWidget(mHoldWidget);

    mverticalLayout->addWidget(Aboutwidget);
    mverticalLayout->addStretch();

    retranslateUi();
}

/* 初始化各文本内容 */
void About::retranslateUi()
{
    mTitleLabel->setText(tr("System Summary"));
    mHoldTitleLabel->setText(tr("Support"));
    mVersionNumLabel_1->setText(tr("Version Number"));
    mDiskLabel_1->setText(tr("Disk"));

    mHpLabel->setText(tr("Wechat code scanning obtains HP professional technical support"));
    mEducateLabel->setText(tr("See more about Kylin Tianqi edu platform"));

//    mTrialLabel->setText(tr("<<Protocol>>"));


    //Intel部分
#ifdef WIN32
    QPluginLoader loader("../HpQRCodePlugin/hp-qrcode-plugind.dll");
#else
    QPluginLoader loader("/usr/lib/x86_64-linux-gnu/hp-qrcode-plugin/libhp-qrcode-plugin.so");

#endif
    QObject *plugin = loader.instance();
    if (plugin) {
        app = qobject_cast<hp::QRCodeInterface*>(plugin);
        mQrCodeWidget = app->createWidget(mHoldWidget);
    }
    else{
        qDebug() << "加载插件失败";
    }

    mHpBtn->setText(tr("Learn more HP user manual>>"));
    mHpBtn->setStyleSheet("background: transparent;color:#2FB3E8;font-family:Microsoft YaHei;"
                  "border-width:1px;text-decoration:underline;border-style:none none none;border-color:#2FB3E8;text-align: left");
    connect(mHpBtn,&QPushButton::clicked,this,[=](){
        QString cmd = "/usr/share/hp-document/hp-document";
        QProcess process(this);
        process.startDetached(cmd);
    });

    mEducateIconLabel->setPixmap(QPixmap(":/help-app.png").scaled(mEducateIconLabel->size(), Qt::KeepAspectRatio));
    mEducateBtn->setText(tr("See user manual>>"));
    mEducateBtn->setStyleSheet("background: transparent;color:#2FB3E8;font-size;font-family:Microsoft YaHei;"
                  "border-width:1px;text-decoration:underline;border-style:none none none;border-color:#2FB3E8;text-align: left");
    connect(mEducateBtn,&QPushButton::clicked,this,[=](){
        QString cmd = "/usr/bin/kylin-user-guide";
        QProcess process(this);
        process.startDetached(cmd);
    });
}

/* 添加搜索索引 */
void About::initSearchText()
{
    //~ contents_path /About/version
    mVersionLabel_1->setText(tr("Version"));
    //~ contents_path /About/Kernel
    mKernelLabel_1->setText(tr("Kernel"));
    //~ contents_path /About/CPU
    mCpuLabel_1->setText(tr("CPU"));
    //~ contents_path /About/Memory
    mMemoryLabel_1->setText(tr("Memory"));
    //~ contents_path /About/Desktop
    mDesktopLabel_1->setText(tr("Desktop"));
    //~ contents_path /About/User
    mUsernameLabel_1->setText(tr("User"));
     //~ contents_path /About/Status
    mStatusLabel_1->setText(tr("Status"));
    mSequenceLabel_1->setText(tr("Serial"));
    mTimeLabel_1->setText(tr("DateRes"));
}

/* 初始化DBus对象 */
void About::initActiveDbus()
{
    activeInterface = QSharedPointer<QDBusInterface>(
        new QDBusInterface("org.freedesktop.activation",
                           "/org/freedesktop/activation",
                           "org.freedesktop.activation.interface",
                           QDBusConnection::systemBus()));
    if (activeInterface.get()->isValid()) {
        connect(activeInterface.get(), SIGNAL(activation_result(int)), this, SLOT(activeSlot(int)));
    }
}

/* 获取激活信息 */
void About::setupSerialComponent()
{
    if (!activeInterface.get()->isValid()) {
        qDebug() << "Create active Interface Failed When Get active info: " <<
            QDBusConnection::systemBus().lastError();
        return;
    }

    int status = 0;
    QDBusMessage activeReply = activeInterface.get()->call("status");
    if (activeReply.type() == QDBusMessage::ReplyMessage) {
        status = activeReply.arguments().at(0).toInt();
    }
    int trial_status = 0;
    QDBusMessage trialReply = activeInterface.get()->call("trial_status");
    if (trialReply.type() == QDBusMessage::ReplyMessage) {
        trial_status = trialReply.arguments().at(0).toInt();
    }

    QString serial;
    QDBusReply<QString> serialReply;
    serialReply = activeInterface.get()->call("serial_number");
    if (!serialReply.isValid()) {
        qDebug()<<"serialReply is invalid"<<endl;
    } else {
        serial = serialReply.value();
    }
    QDBusMessage dateReply = activeInterface.get()->call("date");
    QString dateRes;
    if (dateReply.type() == QDBusMessage::ReplyMessage) {
        dateRes = dateReply.arguments().at(0).toString();
    }

    QDBusMessage trial_dateReply = activeInterface.get()->call("trial_date");
    QString trial_dateRes;
    if (trial_dateReply.type() == QDBusMessage::ReplyMessage) {
        trial_dateRes = trial_dateReply.arguments().at(0).toString();
    }
    mSequenceLabel_2->setText(serial);

    if (dateRes.isEmpty()) {  //未激活
         if (!trial_dateRes.isEmpty()) {  //试用期
            mStatusLabel_2->setText(tr("Inactivated"));
            mStatusLabel_2->setStyleSheet("color : red ");
            mTimeLabel_1->setText(tr("Trial expiration time"));
            mTimeLabel_2->setText(trial_dateRes);
            mActivationBtn->setText(tr("Active"));
        } else {
             mTimeLabel_1->hide();
             mTimeLabel_2->hide();
             mStatusLabel_2->setText(tr("Inactivated"));
             mStatusLabel_2->setStyleSheet("color : red ");
             mActivationBtn->setText(tr("Active"));
         }
    }  else {    //已激活
        mActivationBtn->hide();
        mTrialLabel->hide();
        mAndLabel->hide();
        mStatusLabel_2->setStyleSheet("");
        mStatusLabel_2->setText(tr("Activated"));
        mTimeLabel_2->setText(dateRes);
        QTimer::singleShot( 1, this, [=](){
            QString s1(ntpdate());
            s1.remove(QChar('\n'), Qt::CaseInsensitive);
            s1.replace(QRegExp("[\\s]+"), " ");   //把所有的多余的空格转为一个空格
            if (s1.isNull()) {    //未连接上网络
                mTimeLabel_2->setText(dateRes);
            } else {    //获取到网络时间
                qDebug()<<"网络时间 : "<<s1;
                QStringList list_1 = s1.split(" ");
                QStringList list_2 = dateRes.split("-");
                if (QString(list_2.at(0)).toInt() > QString(list_1.at(list_1.count() -1)).toInt() ) { //未到服务到期时间
                    mTimeLabel_2->setText(dateRes);
                } else if (QString(list_2.at(0)).toInt() == QString(list_1.at(list_1.count() -1)).toInt()) {
                    if (QString(list_2.at(1)).toInt() > getMonth(list_1.at(1))) {
                        mTimeLabel_2->setText(dateRes);
                    } else if (QString(list_2.at(1)).toInt() == getMonth(list_1.at(1))) {
                        if (QString(list_2.at(2)).toInt() > QString(list_1.at(2)).toInt()) {
                            mTimeLabel_2->setText(dateRes);
                        } else {   // 已过服务到期时间
                            showExtend(dateRes);
                        }
                    } else {
                        showExtend(dateRes);
                    }
                } else {
                    showExtend(dateRes);
                }
            }
        });
    }
    connect(mActivationBtn, &QPushButton::clicked, this, &About::runActiveWindow);
}

/* 获取内部版本号 */
void About::setVersionNumCompenent()
{
//    QString InterVersion = nullptr;
//    QString VersionNumPath = "/etc/kylin-build";
//    QStringList mCentent = readFile(VersionNumPath);
//     for (QString str : mCentent) {
//         if (str.contains("Build")) {
//             QRegExp rx("^Build (.*)$");
//             int pos = rx.indexIn(str);
//             if (pos > -1) {
//                 InterVersion = rx.cap(1);
//                mInterVersionLabel_2->setText(InterVersion);
//                mVersionNumberLabel_2->setText(InterVersion.mid(2 , 4));
//                mInterVersionFrame->hide();
//             }
//         }
//     }
    mInterVersionFrame->hide();
    QString InfoPath = "/etc/os-release";
    QFile file(InfoPath);
    int pos = -1;
    if (file.exists()) {
         QStringList mCentent = readFile(InfoPath);
          for (QString str : mCentent) {
              if (str.contains("KYLIN_RELEASE_ID=")) {
                  QRegExp rx("^KYLIN_RELEASE_ID=\"(.*)\"$");
                  pos = rx.indexIn(str);
                  if (pos > -1) {
                      mVersionNumberLabel_2->setText(rx.cap(1));
                      break;
                  }
              }
          }
          if (pos == -1)
              mVersionNumberFrame->hide();
     } else {
         mVersionNumberFrame->hide();
     }


}

/* 获取logo图片和版本名称 */
void About::setupVersionCompenent()
{
    QString versionPath = "/etc/os-release";
    QStringList osRes = readFile(versionPath);
    QString versionID;
    QString version;

    if (QGSettings::isSchemaInstalled(THEME_STYLE_SCHEMA)) {
            themeStyleQgsettings = new QGSettings(THEME_STYLE_SCHEMA, QByteArray(), this);
    } else {
        themeStyleQgsettings = nullptr;
        qDebug()<<THEME_STYLE_SCHEMA<<" not installed";
    }

    for (QString str : osRes) {
        if (str.contains("VERSION_ID=")) {
            QRegExp rx("VERSION_ID=\"(.*)\"$");
            int pos = rx.indexIn(str);
            if (pos > -1) {
                versionID = rx.cap(1);
            }
        }

        if (!QLocale::system().name().compare("zh_CN", Qt::CaseInsensitive)) {
            if (str.contains("VERSION=")) {
                QRegExp rx("VERSION=\"(.*)\"$");
                int pos = rx.indexIn(str);
                if (pos > -1) {
                    version = rx.cap(1);
                }
            }
        } else {
            if (str.contains("VERSION_US=")) {
                QRegExp rx("VERSION_US=\"(.*)\"$");
                int pos = rx.indexIn(str);
                if (pos > -1) {
                    version = rx.cap(1);
                }
            }
        }
    }

    if (!version.isEmpty()) {
        setLabelText(mVersionLabel_2,version);
        connect(this,&About::resize,[=](){
           setLabelText(mVersionLabel_2,version);
        });
    }

    if (!versionID.compare(vTen, Qt::CaseInsensitive) ||
            !versionID.compare(vTenEnhance, Qt::CaseInsensitive) ||
            !versionID.compare(vFour, Qt::CaseInsensitive)) {
        mLogoLabel->setPixmap(loadSvg("://img/plugins/about/logo-light.svg", 130, 50)); //默认设置为light
        if (themeStyleQgsettings != nullptr && themeStyleQgsettings->keys().contains(CONTAIN_STYLE_NAME_KEY)) {
            if (themeStyleQgsettings->get(STYLE_NAME_KEY).toString() == UKUI_DARK) { //深色模式改为dark
                mLogoLabel->setPixmap(loadSvg("://img/plugins/about/logo-dark.svg", 130, 50));
            }
            connect(themeStyleQgsettings,&QGSettings::changed,this,[=](QString changedKey) {  //监听主题变化
                if (changedKey == CONTAIN_STYLE_NAME_KEY) {
                    if (themeStyleQgsettings->get(STYLE_NAME_KEY).toString() == UKUI_DARK) {
                        mLogoLabel->setPixmap(loadSvg("://img/plugins/about/logo-dark.svg", 130, 50));
                    } else {
                        mLogoLabel->setPixmap(loadSvg("://img/plugins/about/logo-light.svg", 130, 50));
                    }
                }
            });
       }
    } else {
        mActivationFrame->setVisible(false);
        mTrialLabel->setVisible(false);
        mAndLabel->setVisible(false);
        mLogoLabel->setPixmap(loadSvg("://img/plugins/about/logoukui.svg", 130, 50));
    }

}

/* 获取桌面信息 */
void About::setupDesktopComponent()
{
    // 获取当前桌面环境
    QString dEnv;
    foreach (dEnv, QProcess::systemEnvironment()) {
        if (dEnv.startsWith("XDG_CURRENT_DESKTOP"))
            break;
    }

    // 设置当前桌面环境信息
    if (!dEnv.isEmpty()) {
        QString desktop = dEnv.section("=", -1, -1);
        if (desktop.contains("UKUI", Qt::CaseInsensitive)) {
            mDesktopLabel_2->setText("UKUI");
        } else {
            mDesktopLabel_2->setText(desktop);
        }
    }
    mDesktopLabel_2->setText("UKUI");
    ChangedSlot();

    QDBusConnection::systemBus().connect(QString(), QString("/org/freedesktop/Accounts/User1000"),
                                          QString("org.freedesktop.Accounts.User"), "Changed",this,
                                         SLOT(ChangedSlot()));
}

/* 获取CPU信息 */
void About::setupKernelCompenent()
{
    QString memorySize("0GB");
    QString cpuType;

    QString kernal = QSysInfo::kernelType() + " " + QSysInfo::kernelVersion();
    getTotalMemory();

    QDBusInterface *memoryDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                                             "/",
                                                             "com.control.center.interface",
                                                             QDBusConnection::systemBus(), this);
   if (memoryDbus->isValid()) {
       QDBusReply<QString>  result = memoryDbus->call("getMemory");
       qDebug()<<"memory :"<<result;
       if (!result.value().isEmpty()) {
            memorySize.clear();
            memorySize.append(result + "GB" + mMemAvaliable);
       }
   }
   if (memorySize == "0GB")
       memorySize = mMemtotal + mMemAvaliable;

    qDebug()<<kernal;
    mKernelLabel_2->setText(kernal);
    mMemoryLabel_2->setText(memorySize);

    cpuType = Utils::getCpuInfo();
    mCpuLabel_2->setText(cpuType);
}


/* 获取硬盘信息 */
void About::setupDiskCompenet()
{
    QProcess process_1;
    process_1.start("lsblk");
    process_1.waitForFinished();
    QString diskSize = process_1.readAllStandardOutput();
    foreach (QString diskResult, diskSize.split("\n")) {
        if (diskResult == NULL)
            continue;
        diskResult.replace(QRegExp("[\\s]+"), " ");
        diskInfo = diskResult.split(" ");

        if (diskInfo.count() >= 6 && diskInfo.at(5) == "disk" && (!diskInfo.at(0).contains("fd")) &&
               (diskInfo.at(2) !="1")) { //过滤掉可移动硬盘
            QStringList totalSize;
            totalSize.append(diskInfo.at(3));
            disk2.insert(diskInfo.at(0),totalSize); //硬盘信息分类存储，用以兼容多硬盘电脑
        }
    }
    QProcess process_2;
    process_2.start("df -l ");
    process_2.waitForFinished();
    QString diskSize2 = process_2.readAllStandardOutput();
    double availSize=0;
    QStringList diskInfo2;
    QMap<QString, QStringList>::iterator iter;
    for(iter=disk2.begin();iter!=disk2.end();iter++)
    {
        foreach (QString diskResult, diskSize2.split("\n")) {
            if (diskResult == NULL)
                continue;
            diskResult.replace(QRegExp("[\\s]+"), " ");
            diskInfo2 = diskResult.split(" ");
            if(diskInfo2.at(0).contains(iter.key())){
                availSize += diskInfo2.at(3).toInt();
            }
        }
        QString diskAvailable = QString::number((availSize/1024/1024), 'f', 1) + "G";
        iter.value().append(diskAvailable);
        availSize = 0;
    }

    int count = 0;
    for(iter=disk2.begin();iter!=disk2.end();iter++){
        if (disk2.size() == 1) {
            mDiskLabel_1->show();
            mDiskLabel_2->show();
            mDiskLabel_2->setText(iter.value().at(0) + "B (" + iter.value().at(1) + "B "+ tr("avaliable") +")");
        }
        else {
            mDiskLabel_1->hide();
            mDiskLabel_2->hide();
            QHBoxLayout * layout = new QHBoxLayout;
            QLabel *label = new FixLabel;
            label->setText(tr("Disk") + QString::number(count + 1));
            QLabel *diskLabel = new FixLabel;
            diskLabel->setText(iter.value().at(0) + "B (" + iter.value().at(1) + "B "+ tr("avaliable") +")");

            layout->addWidget(label);
            layout->addWidget(diskLabel);
            layout->addStretch();
            mDiskLayout->addLayout(layout);

        }
    }
}

/* 获取系统版本 */
void About::setupSystemVersion()
{
    QString sysVersion = "/etc/apt/ota_version";
    QFile file(sysVersion);
    if (file.exists() == false) {
        mVersionNumFrame->hide();
        mDiskFrame->hide();
        mHoldWidget->hide();
        mHoldTitleLabel->hide();
        return;
    } else {
        mPrivacyFrame->hide();
        mHostNameFrame->hide();
        mPriTitleLabel->hide();
        mAndLabel->hide();
        mAgreeLabel->hide();
        mActivationFrame->hide();
        mTipLabel->hide();
        mTrialLabel->hide();
    }

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray t = file.readAll();
    file.close();
    QString content = t;
    QStringList versionResult1;
    foreach (QString versionResult, content.split("\n")) {
        if (versionResult == NULL)
            continue;
        versionResult1.append(versionResult);
    }
    QString content1 = versionResult1.at(1);
    content1.replace('"', "");
    QString content2 = content1.split(" = ").at(1);
    mVersionNumLabel_2->setText(content2);
}

void About::setHostNameCompenet()
{
    mHostNameLabel_2->setText(Utils::getHostName());
}

void About::setPrivacyCompent()
{
    if (Utils::isWayland()) {
        mPrivacyFrame->setVisible(false);
        mPriTitleLabel->setVisible(false);
        mAgreeLabel->setVisible(false);
        mAndLabel->setVisible(false);
        item->changeSize(0, 0);
        return;
    }
    QDBusInterface *PriDBus = new QDBusInterface("com.kylin.daq",
                                                             "/com/kylin/daq",
                                                             "com.kylin.daq.interface",
                                                             QDBusConnection::systemBus(), this);
    if (!PriDBus->isValid()) {
        qDebug()<<"create pridbus error";
        return;
    }
    QDBusReply<int> reply = PriDBus->call("GetUploadState");
    mPriBtn->blockSignals(true);
    mPriBtn->setChecked(reply == 0 ? false : true);
    mPriBtn->blockSignals(false);

    connect(mPriBtn,&SwitchButton::checkedChanged ,this ,[=](bool status){
         PriDBus->call("SetUploadState" , (status ? 1 : 0));
    } );
}

void About::showExtend(QString dateres)
{
    mTimeLabel_2->setText(dateres+QString("(%1)").arg(tr("expired")));
    mTimeLabel_2->setStyleSheet("color : red ");
    mActivationBtn->setVisible(true);
    mTrialLabel->setVisible(true);
    mAndLabel->setVisible(true);
    mActivationBtn->setText(tr("Extend"));
}

char *About::ntpdate()
{
    char *hostname=(char *)"162.159.200.123";
    int portno = 123;     //NTP is port 123
    int maxlen = 1024;        //check our buffers
    int i;          // misc var i
    unsigned char msg[48]={(0 << 6) | (3 << 3) | (3 << 0), 0, 4, ((-6) & 0xFF), 0, 0, 0, 0, 0};    // the packet we send
    unsigned long  buf[maxlen]; // the buffer we get back
    struct protoent *proto;
    struct sockaddr_in server_addr;
    int s;  // socket
    long tmit;   // the time -- This is a time_t sort of

    proto = getprotobyname("udp");
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == s) {
        perror("socket");
        return NULL;
    }

    memset( &server_addr, 0, sizeof( server_addr ));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(hostname);

    server_addr.sin_port=htons(portno);

    int on = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST, &on, sizeof(on));
    i=sendto(s,msg,sizeof(msg),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if (-1 == i) {
        perror("sendto");
        return NULL;
    }

    // 设置超时
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;//微秒
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        perror("setsockopt failed:");
        return NULL;
    }


    struct sockaddr saddr;
    socklen_t saddr_l = sizeof (saddr);
    i=recvfrom(s,buf,48,0,&saddr,&saddr_l);
    if (-1 == i) {
        perror("recvfr");
        return NULL;
    }

    tmit=ntohl((time_t)buf[4]);    // get transmit time

    tmit -= 2208988800U;

    return ctime(&tmit);
}

int About::getMonth(QString month)
{
    if (month == "Jan") {
        return 1;
    } else if (month == "Feb") {
        return 2;
    } else if (month == "Mar") {
        return 3;
    } else if (month == "Apr") {
        return 4;
    } else if (month == "May") {
        return 5;
    } else if (month == "Jun") {
        return 6;
    } else if (month == "Jul") {
        return 7;
    } else if (month == "Aug") {
        return 8;
    } else if (month == "Sep" || month == "Sept") {
        return 9;
    } else if (month == "Oct") {
        return 10;
    } else if (month == "Nov") {
        return 11;
    } else if (month == "Dec") {
        return 12;
    }else {
        return 0;
    }
}

void About::reboot()
{
    QDBusInterface *rebootDbus = new QDBusInterface("org.gnome.SessionManager",
                                                             "/org/gnome/SessionManager",
                                                             "org.gnome.SessionManager",
                                                             QDBusConnection::sessionBus());

    rebootDbus->call("reboot");
    delete rebootDbus;
    rebootDbus = nullptr;
}

/* 处理文本宽度 */
void About::setLabelText(QLabel *label, QString text)
{
    QFontMetrics  fontMetrics(label->font());
    int fontSize = fontMetrics.width(text);
    if (fontSize > label->width()) {
        label->setText(fontMetrics.elidedText(text, Qt::ElideRight, label->width()));
        label->setToolTip(text);
    } else {
        label->setText(text);
        label->setToolTip("");
    }
}

/* 处理窗口缩放时的文本显示 */
bool About::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == mVersionFrame ) {
        if (event->type() == QEvent::Resize) {
            mVersionLabel_2->setFixedWidth(mVersionFrame->width()-176);
            emit resize();
        }
        return false;
    } else if (obj == mHostNameLabel_2 || obj == mHostNameLabel_3) {
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton ){
                QString str = Utils::getHostName();
                HostNameDialog *mdialog = new HostNameDialog(pluginWidget);
                QWidget *widget = qApp->activeWindow(); // 记录mainwindow的地址，exec之后，activeWindow会变成空值
                mdialog->exec();
                if (str !=  Utils::getHostName()) {
                    QMessageBox *mReboot = new QMessageBox(widget);
                    mReboot->setIcon(QMessageBox::Warning);
                    mReboot->setText(tr("The system needs to be restarted to set the HostName, whether to reboot"));
                    mReboot->addButton(tr("Reboot Now"), QMessageBox::AcceptRole);
                    mReboot->addButton(tr("Reboot Later"), QMessageBox::RejectRole);
                    int ret = mReboot->exec();
                    switch (ret) {
                    case QMessageBox::AcceptRole:
                        sleep(1);
                        reboot();
                        break;
                    }
                    mHostNameLabel_2->setText(Utils::getHostName());
                }
            }
        }
    } else if (obj == mTrialLabel) {
         if (event->type() == QEvent::MouseButtonPress) {
             TrialDialog *mDialog = new TrialDialog(pluginWidget);
             mDialog->exec();
         }
    } else if (obj == mAgreeLabel) {
        if (event->type() == QEvent::MouseButtonPress) {
            PrivacyDialog *mDialog = new PrivacyDialog(pluginWidget);
            mDialog->exec();
        }
    }
    return false;
}

QStringList About::getUserDefaultLanguage()
{
    QString formats;
    QString language;
    QStringList result;

    unsigned int uid = getuid();
    QString objpath = "/org/freedesktop/Accounts/User"+QString::number(uid);

    QDBusInterface iproperty("org.freedesktop.Accounts",
                             objpath,
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply = iproperty.call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()) {
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        if (propertyMap.keys().contains("FormatsLocale")) {
            formats = propertyMap.find("FormatsLocale").value().toString();
        }
        if(language.isEmpty() && propertyMap.keys().contains("Language")) {
            language = propertyMap.find("Language").value().toString();
        }
    }
    qDebug()<<formats<<"---"<<language;
    result.append(formats);
    result.append(language);
    return result;
}

/* 读取/etc/os-release文件的内容 */
QStringList About::readFile(QString filepath)
{
    QStringList fileCont;
    QFile file(filepath);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "ReadFile() failed to open" << filepath;
            return QStringList();
        }
        QTextStream textStream(&file);
        while (!textStream.atEnd()) {
            QString line = textStream.readLine();
            line.remove('\n');
            fileCont<<line;
        }
        file.close();
        return fileCont;
    } else {
        qWarning() << filepath << " not found"<<endl;
        return QStringList();
    }
}

void About::getTotalMemory()
{
    const QString fileName = "/proc/meminfo";
    QFile meninfoFile(fileName);
    if (!meninfoFile.exists()) {
        printf("/proc/meminfo file not exist \n");
    }
    if (!meninfoFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        printf("open /proc/meminfo fail \n");
    }

    QTextStream in(&meninfoFile);
    QString line = in.readLine();
    float memtotal = 0;
    float memAvaliable = 0;
    int count = 0;

    while (!line.isNull()) {
        if (line.contains("MemTotal")) {
            line.replace(QRegExp("[\\s]+"), " ");

            QStringList lineList = line.split(" ");
            QString mem = lineList.at(1);
            memtotal = mem.toFloat();
            count++;
            if (count ==2) {
               break;
            } else {
                line = in.readLine();
            }
        } else if (line.contains("MemAvailable")) {
            line.replace(QRegExp("[\\s]+"), " ");

            QStringList lineList = line.split(" ");
            QString mem = lineList.at(1);
            memAvaliable = mem.toFloat();
            count++;
            if (count ==2) {
               break;
            }
        }
        else {
            line = in.readLine();
        }
    }

    memtotal = ceil(memtotal / 1024 / 1024);
    memAvaliable /= (1024 * 1024);
    // 向2的n次方取整
//    int nPow = ceil(log(memtotal)/log(2.0));
//    memtotal = pow(2.0, nPow);

    mMemtotal = QString("%1%2").arg(QString::number(memtotal)).arg("GB");
    mMemAvaliable = QString("%1%2%3%4%5").arg("(").arg(QString::number(memAvaliable, 'f', 1)).arg("GB").arg(tr("avaliable")).arg(")");
}

void About::setFrame_Box(QFrame *frame)
{
    frame->setMinimumSize(QSize(550, 0));
    frame->setMaximumSize(QSize(16777215, 16777215));
    frame->setFrameShape(QFrame::Box);
}

void About::setFrame_NoFrame(QFrame *frame)
{
    frame->setMinimumSize(QSize(550, 30));
    frame->setMaximumSize(QSize(16777215, 30));
    frame->setFrameShape(QFrame::NoFrame);
}

void About::activeSlot(int activeSignal)
{
    if (!activeSignal) {
        setupSerialComponent();
    }
}

/* 打开激活窗口 */
void About::runActiveWindow()
{
    QString cmd = "kylin-activation";

    QProcess process(this);
    process.startDetached(cmd);
}

/* 获取用户昵称 */
void About::ChangedSlot()
{
    qlonglong uid = getuid();
    QDBusInterface user("org.freedesktop.Accounts",
                        "/org/freedesktop/Accounts",
                        "org.freedesktop.Accounts",
                        QDBusConnection::systemBus());
    QDBusMessage result = user.call("FindUserById", uid);
    QString userpath = result.arguments().value(0).value<QDBusObjectPath>().path();
    QDBusInterface *userInterface = new QDBusInterface ("org.freedesktop.Accounts",
                                          userpath,
                                        "org.freedesktop.Accounts.User",
                                        QDBusConnection::systemBus());
    QString userName = userInterface->property("RealName").value<QString>();
    mUsernameLabel_2->setText(userName);
}

QPixmap About::loadSvg(const QString &path, int width, int height) {
    const auto ratio = qApp->devicePixelRatio();
    if (ratio >= 2) {
        width += width;
        height += height;
    } else {
        height *= ratio;
        width *= ratio;
    }
    QPixmap pixmap(width, height);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return pixmap;
}

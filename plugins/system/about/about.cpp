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
    //~ contents_path /about/About
    pluginName = tr("About");
    pluginType = SYSTEM;
}

About::~About()
{
    if (!mFirstLoad) {

    }
}

QString About::get_plugin_name()
{
    return pluginName;
}

int About::get_plugin_type()
{
    return pluginType;
}

QWidget *About::get_plugin_ui()
{
    if (mFirstLoad) {
        mFirstLoad = false;

        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);

        initUI(pluginWidget);
        initSearchText();
        initActiveDbus();

        setupVersionCompenent();
        setupSystemVersion();
        setupDesktopComponent();
        setupKernelCompenent();
        setupDiskCompenet();
        setupSerialComponent();

    }

    return pluginWidget;
}

void About::plugin_delay_control()
{
}

const QString About::name() const
{
    return QStringLiteral("about");
}

void About::initUI(QWidget *widget)
{
    QVBoxLayout *mverticalLayout = new QVBoxLayout(widget);
    mverticalLayout->setSpacing(0);
    mverticalLayout->setContentsMargins(0, 0, 32, 100);

    QWidget *Aboutwidget = new QWidget(widget);
    Aboutwidget->setMinimumSize(QSize(550, 0));
    Aboutwidget->setMaximumSize(QSize(960, 16777215));

    QVBoxLayout *AboutLayout = new QVBoxLayout(Aboutwidget);
    AboutLayout->setContentsMargins(0, 0, 0, 0);
    AboutLayout->setSpacing(8);

    mTitleLabel = new TitleLabel(Aboutwidget);
    AboutLayout->addWidget(mTitleLabel);

    mInformationFrame = new QFrame(Aboutwidget);
    mInformationFrame->setMinimumSize(QSize(550, 0));
    mInformationFrame->setMaximumSize(QSize(960, 16777215));
    mInformationFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *mInformationLayout = new QVBoxLayout(mInformationFrame);
    mInformationLayout->setContentsMargins(16, 16, 16, 8);

    mLogoLabel = new QLabel(mInformationFrame);
    mLogoLabel->setFixedSize(140,64);

    mInformationLayout->addWidget(mLogoLabel);

    mVersionFrame = new QFrame(mInformationFrame);
    mVersionFrame->setMinimumSize(QSize(550, 30));
    mVersionFrame->setMaximumSize(QSize(960, 30));
    mVersionFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mVersionLayout = new QHBoxLayout(mVersionFrame);
    mVersionLayout->setContentsMargins(0, 0, 16, 0);

    mVersionLabel_1 = new QLabel(mVersionFrame);
    mVersionLabel_1->setFixedSize(80,30);

    mVersionLabel_2 = new QLabel(mVersionFrame);
    mVersionLabel_2->setFixedHeight(30);

    mVersionLayout->addWidget(mVersionLabel_1);
    mVersionLayout->addSpacing(80);
    mVersionLayout->addWidget(mVersionLabel_2);
    mVersionLayout->addStretch();

    mInformationLayout->addWidget(mVersionFrame);

    mVersionNumFrame = new QFrame(mInformationFrame);
    mVersionNumFrame->setMinimumSize(QSize(550, 30));
    mVersionNumFrame->setMaximumSize(QSize(960, 30));
    mVersionNumFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mVersionNumLayout = new QHBoxLayout(mVersionNumFrame);
    mVersionNumLayout->setContentsMargins(0, 0, 16, 0);

    mVersionNumLabel_1 = new QLabel(mVersionNumFrame);
    mVersionNumLabel_1->setFixedSize(80,30);

    mVersionNumLabel_2 = new QLabel(mVersionNumFrame);
    mVersionNumLabel_2->setFixedHeight(30);

    mVersionNumLayout->addWidget(mVersionNumLabel_1);
    mVersionNumLayout->addSpacing(80);
    mVersionNumLayout->addWidget(mVersionNumLabel_2);
    mVersionNumLayout->addStretch();

    mInformationLayout->addWidget(mVersionNumFrame);

    mKernelFrame = new QFrame(mInformationFrame);
    mKernelFrame->setMinimumSize(QSize(550, 30));
    mKernelFrame->setMaximumSize(QSize(960, 30));
    mKernelFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mKernerLayout = new QHBoxLayout(mKernelFrame);
    mKernerLayout->setContentsMargins(0, 0, 16, 0);

    mKernelLabel_1 = new QLabel(mKernelFrame);
    mKernelLabel_1->setFixedSize(80,30);

    mKernelLabel_2 = new QLabel(mKernelFrame);
    mKernelLabel_2->setFixedHeight(30);

    mKernerLayout->addWidget(mKernelLabel_1);
    mKernerLayout->addSpacing(80);
    mKernerLayout->addWidget(mKernelLabel_2);
    mKernerLayout->addStretch();

    mInformationLayout->addWidget(mKernelFrame);

    mCpuFrame = new QFrame(mInformationFrame);
    mCpuFrame->setMinimumSize(QSize(550, 30));
    mCpuFrame->setMaximumSize(QSize(960, 30));
    mCpuFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mCpuLayout = new QHBoxLayout(mCpuFrame);
    mCpuLayout->setContentsMargins(0, 0, 16, 0);

    mCpuLabel_1 = new QLabel(mCpuFrame);
    mCpuLabel_1->setFixedSize(80,30);

    mCpuLabel_2 = new QLabel(mCpuFrame);
    mCpuLabel_2->setFixedHeight(30);

    mCpuLayout->addWidget(mCpuLabel_1);
    mCpuLayout->addSpacing(80);
    mCpuLayout->addWidget(mCpuLabel_2);
    mCpuLayout->addStretch();

    mInformationLayout->addWidget(mCpuFrame);

    mMemoryFrame = new QFrame(mInformationFrame);
    mMemoryFrame->setMinimumSize(QSize(550, 30));
    mMemoryFrame->setMaximumSize(QSize(960, 30));
    mMemoryFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mMemoryLayout = new QHBoxLayout(mMemoryFrame);
    mMemoryLayout->setContentsMargins(0, 0, 16, 0);

    mMemoryLabel_1 = new QLabel(mMemoryFrame);
    mMemoryLabel_1->setFixedSize(80,30);

    mMemoryLabel_2 = new QLabel(mMemoryFrame);
    mMemoryLabel_2->setFixedHeight(30);

    mMemoryLayout->addWidget(mMemoryLabel_1);
    mMemoryLayout->addSpacing(80);
    mMemoryLayout->addWidget(mMemoryLabel_2);
    mMemoryLayout->addStretch();

    mInformationLayout->addWidget(mMemoryFrame);

    mDiskFrame = new QFrame(mInformationFrame);
    mDiskFrame->setMinimumSize(QSize(550, 30));
    mDiskFrame->setMaximumSize(QSize(960, 30));
    mDiskFrame->setFrameShape(QFrame::NoFrame);

    mDiskLayout = new QHBoxLayout(mDiskFrame);
    mDiskLayout->setContentsMargins(0, 0, 16, 0);

    mDiskLabel_1 = new QLabel(mDiskFrame);
    mDiskLabel_1->setFixedSize(80,30);

    mDiskLabel_2 = new QLabel(mDiskFrame);
    mDiskLabel_2->setFixedHeight(30);

    mDiskLayout->addWidget(mDiskLabel_1);
    mDiskLayout->addSpacing(80);
    mDiskLayout->addWidget(mDiskLabel_2);
    mDiskLayout->addStretch();

    mInformationLayout->addWidget(mDiskFrame);

    mDesktopFrame = new QFrame(mInformationFrame);
    mDesktopFrame->setMinimumSize(QSize(550, 30));
    mDesktopFrame->setMaximumSize(QSize(960, 30));
    mDesktopFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mDesktopLayout = new QHBoxLayout(mDesktopFrame);
    mDesktopLayout->setContentsMargins(0, 0, 16, 0);

    mDesktopLabel_1 = new QLabel(mDesktopFrame);
    mDesktopLabel_1->setFixedSize(80,30);

    mDesktopLabel_2 = new QLabel(mDesktopFrame);
    mDesktopLabel_2->setFixedHeight(30);

    mDesktopLayout->addWidget(mDesktopLabel_1);
    mDesktopLayout->addSpacing(80);
    mDesktopLayout->addWidget(mDesktopLabel_2);
    mDesktopLayout->addStretch();

    mInformationLayout->addWidget(mDesktopFrame);

    mUsernameFrame = new QFrame(mInformationFrame);
    mUsernameFrame->setMinimumSize(QSize(550, 30));
    mUsernameFrame->setMaximumSize(QSize(960, 30));
    mUsernameFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mUsernameLayout = new QHBoxLayout(mUsernameFrame);
    mUsernameLayout->setContentsMargins(0, 0, 16, 0);

    mUsernameLabel_1 = new QLabel(mUsernameFrame);
    mUsernameLabel_1->setFixedSize(80,30);

    mUsernameLabel_2 = new QLabel(mUsernameFrame);
    mUsernameLabel_2->setFixedHeight(30);

    mUsernameLayout->addWidget(mUsernameLabel_1);
    mUsernameLayout->addSpacing(80);
    mUsernameLayout->addWidget(mUsernameLabel_2);
    mUsernameLayout->addStretch();

    mInformationLayout->addWidget(mUsernameFrame);

    mActivationFrame = new QFrame(Aboutwidget);
    mActivationFrame->setMinimumSize(QSize(550, 0));
    mActivationFrame->setMaximumSize(QSize(960, 16777215));
    mActivationFrame->setFrameShape(QFrame::Box);

    QGridLayout *mActivationLayout = new QGridLayout(mActivationFrame);
    mActivationLayout->setVerticalSpacing(8);
    mActivationLayout->setContentsMargins(16, 16, 16, 8);

    mStatusLabel_1 = new QLabel(mActivationFrame);
    mStatusLabel_1->setFixedSize(166,30);
    mStatusLabel_2 = new QLabel(mActivationFrame);
    mSequenceLabel_1 = new QLabel(mActivationFrame);
    mSequenceLabel_1->setFixedSize(166,30);
    mSequenceLabel_2 = new QLabel(mActivationFrame);
    mTimeLabel_1 = new QLabel(mActivationFrame);
    mTimeLabel_1->setFixedSize(166,30);
    mTimeLabel_2 = new QLabel(mActivationFrame);
    mActivationBtn = new QPushButton(mActivationFrame);

    mActivationBtn->setFixedSize(120,40);

    mActivationLayout->addWidget(mStatusLabel_1, 0, 0, 2, 1);
    mActivationLayout->addWidget(mStatusLabel_2, 0, 1, 2, 3,Qt::AlignLeft);
    mActivationLayout->addWidget(mSequenceLabel_1, 2, 0, 2, 1);
    mActivationLayout->addWidget(mSequenceLabel_2, 2, 1, 2, 3,Qt::AlignLeft);
    mActivationLayout->addWidget(mTimeLabel_1, 4, 0, 2, 1);
    mActivationLayout->addWidget(mTimeLabel_2, 4, 1, 2, 3,Qt::AlignLeft);
    mActivationLayout->addWidget(mActivationBtn, 1, 3, 4, 1,Qt::AlignHCenter | Qt::AlignRight);

    mTrialBtn = new QPushButton(Aboutwidget);
    mTrialBtn->setFixedSize(100,40);
    mTrialBtn->setStyleSheet("background: transparent;font-family:Microsoft YaHei;"
                  "border-width:1px;text-decoration:underline;border-style:none none none;");

    QHBoxLayout *mTrialLayout = new QHBoxLayout(mTrialBtn);

    mTrialLabel = new QLabel(mTrialBtn);

    mTrialLayout->addWidget(mTrialLabel);

    mHoldTitleLabel = new TitleLabel(Aboutwidget);

    mHoldWidget = new QFrame(Aboutwidget);
    mHoldWidget->setMinimumSize(QSize(550, 112));
    mHoldWidget->setMaximumSize(QSize(960, 112));
    mHoldWidget->setFrameShape(QFrame::Box);

    QGridLayout *mHoldLayout = new QGridLayout(mHoldWidget);
    mHoldLayout->setVerticalSpacing(0);
    mHoldLayout->setHorizontalSpacing(8);

    mQrCodeWidget = new QWidget(mHoldWidget);
    mQrCodeWidget->setFixedSize(64,64);

    mHpLabel = new QLabel(mHoldWidget);
    mEducateIconLabel = new QLabel(mHoldWidget);    
    mEducateIconLabel->setFixedSize(64,64);
    mEducateLabel = new QLabel(mHoldWidget);

    mHpBtn = new QPushButton(mHoldWidget);
    mEducateBtn = new QPushButton(mHoldWidget);

    mHoldLayout->addWidget(mQrCodeWidget,0,0,2,1);
    mHoldLayout->addWidget(mHpLabel,0,1,1,3);
    mHoldLayout->addWidget(mHpBtn,1,1,1,2,Qt::AlignLeft);
    mHoldLayout->addWidget(mEducateIconLabel,0,4,2,1);
    mHoldLayout->addWidget(mEducateLabel,0,5,1,3);
    mHoldLayout->addWidget(mEducateBtn,1,5,1,2,Qt::AlignLeft);

    AboutLayout->addWidget(mInformationFrame);
    AboutLayout->addWidget(mActivationFrame);
    AboutLayout->addWidget(mTrialBtn);
    AboutLayout->addSpacing(8);
    AboutLayout->addWidget(mHoldTitleLabel);
    AboutLayout->addWidget(mHoldWidget);

    mverticalLayout->addWidget(Aboutwidget);
    mverticalLayout->addStretch();

    retranslateUi();
}

void About::retranslateUi()
{
    mTitleLabel->setText(tr("System Summary"));
    mHoldTitleLabel->setText(tr("Support"));

    mVersionNumLabel_1->setText(tr("Version Number"));
    mDiskLabel_1->setText(tr("Disk"));
    mUsernameLabel_1->setText(tr("User"));
    mStatusLabel_1->setText(tr("Active Status"));
    mSequenceLabel_1->setText(tr("Serial"));
    mTimeLabel_1->setText(tr("DateRes"));

    mHpLabel->setText(tr("Wechat code scanning obtains HP professional technical support"));
    mEducateLabel->setText(tr("See more about Kylin Tianqi edu platform"));

    mTrialBtn->setText(tr("<<Protocol>>"));

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
    mHpBtn->setStyleSheet("background: transparent;color:#2FB3E8;font-size:16px;font-family:Microsoft YaHei;"
                  "border-width:1px;text-decoration:underline;border-style:none none none;border-color:#2FB3E8;");
    //connect(mHpBtn,&QPushButton::clicked,this,&About::on_pushButton_2_clicked);

    mEducateIconLabel->setPixmap(QPixmap(":/help-app.png").scaled(mLogoLabel->size(), Qt::KeepAspectRatio));
    mEducateBtn->setText(tr("See user manual>>"));
    mEducateBtn->setStyleSheet("background: transparent;color:#2FB3E8;font-size:16px;font-family:Microsoft YaHei;"
                  "border-width:1px;text-decoration:underline;border-style:none none none;border-color:#2FB3E8;");
}

void About::initSearchText()
{
    //~ contents_path /about/version
    mVersionLabel_1->setText(tr("Version"));
    //~ contents_path /about/Kernel
    mKernelLabel_1->setText(tr("Kernel"));
    //~ contents_path /about/CPU
    mCpuLabel_1->setText(tr("CPU"));
    //~ contents_path /about/Memory
    mMemoryLabel_1->setText(tr("Memory"));
    //~ contents_path /about/Desktop
    mDesktopLabel_1->setText(tr("Desktop"));
}

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
    if (1 == status) {
        mStatusLabel_2->setText(tr("Activated"));
        mTimeLabel_2->setText(dateRes);
        mActivationBtn->hide();
        mTrialBtn->hide();
    } else {
        mStatusLabel_2->setStyleSheet("color:red;");
        mTimeLabel_2->setStyleSheet("color:red;");
        if (!dateRes.isEmpty()) {
            mStatusLabel_2->setText(tr("Technical service has expired"));
            mTimeLabel_2->setText(dateRes);
            mActivationBtn->setText(tr("Extended"));
        } else {
            mTimeLabel_1->hide();
            mTimeLabel_2->hide();
            mStatusLabel_2->setText(tr("Inactivated"));
        }
    }

    mSequenceLabel_2->setText(serial);

    connect(mActivationBtn, &QPushButton::clicked, this, &About::runActiveWindow);
    connect(mTrialBtn, &QPushButton::clicked, this, &About::showPdf);
//    connect(mTrialBtn, &QPushButton::clicked, this, [=](){
//        TrialDialog *mDialog = new TrialDialog(pluginWidget);
//        mDialog->exec();
//    });
}

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
        mVersionLabel_2->setText(version + "  " + tr("Copyright © 2009-2021 KylinSoft. All rights reserved."));
    }

    if (!versionID.compare(vTen, Qt::CaseInsensitive) ||
            !versionID.compare(vTenEnhance, Qt::CaseInsensitive) ||
            !versionID.compare(vFour, Qt::CaseInsensitive)) {
        mLogoLabel->setPixmap(QPixmap("://img/plugins/about/logo-light.svg").scaled(mLogoLabel->size(), Qt::KeepAspectRatio)); //默认设置为light
        if (themeStyleQgsettings != nullptr && themeStyleQgsettings->keys().contains(CONTAIN_STYLE_NAME_KEY)) {
            if (themeStyleQgsettings->get(STYLE_NAME_KEY).toString() == UKUI_DARK) { //深色模式改为dark
                mLogoLabel->setPixmap(QPixmap("://img/plugins/about/logo-dark.svg").scaled(mLogoLabel->size(), Qt::KeepAspectRatio));
            }
            connect(themeStyleQgsettings,&QGSettings::changed,this,[=](QString changedKey) {  //监听主题变化
                if (changedKey == CONTAIN_STYLE_NAME_KEY) {
                    if (themeStyleQgsettings->get(STYLE_NAME_KEY).toString() == UKUI_DARK) {
                        mLogoLabel->setPixmap(QPixmap("://img/plugins/about/logo-dark.svg").scaled(mLogoLabel->size(), Qt::KeepAspectRatio));
                    } else {
                        mLogoLabel->setPixmap(QPixmap("://img/plugins/about/logo-light.svg").scaled(mLogoLabel->size(), Qt::KeepAspectRatio));
                    }
                }
            });
       }
    } else {
        mActivationFrame->setVisible(false);
        mTrialBtn->setVisible(false);
        mLogoLabel->setPixmap(QPixmap("://img/plugins/about/logoukui.svg"));
    }
}

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

void About::setupKernelCompenent()
{
    QString memorySize;
    QString cpuType;

    QString kernal = QSysInfo::kernelType() + " " + QSysInfo::kernelVersion();
    memorySize = getTotalMemory();

    mKernelLabel_2->setText(kernal);
    mMemoryLabel_2->setText(memorySize);

    QDBusInterface youkerInterface("com.kylin.assistant.systemdaemon",
                                   "/com/kylin/assistant/systemdaemon",
                                   "com.kylin.assistant.systemdaemon",
                                   QDBusConnection::systemBus());
    if (!youkerInterface.isValid()) {
        qCritical() << "Create youker Interface Failed When Get Computer info: " <<
            QDBusConnection::systemBus().lastError();
        return;
    }

    QDBusReply<QMap<QString, QVariant> > cpuinfo;
    cpuinfo = youkerInterface.call("get_cpu_info");
    if (!cpuinfo.isValid()) {
        qDebug() << "cpuinfo is invalid" << endl;
    } else {
        QMap<QString, QVariant> res = cpuinfo.value();
        cpuType = res["CpuVersion"].toString();
    }

    mCpuLabel_2->setText(cpuType);
}

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
        if (diskInfo.at(5) == "disk" && (!diskInfo.at(0).contains("fd")) &&
               (diskInfo.at(2)!="1")) { //过滤掉可移动硬盘
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
            QLabel *label = new QLabel;
            label->setText(tr("Disk") + QString::number(count + 1));
            QLabel *diskLabel = new QLabel;
            diskLabel->setText(iter.value().at(0) + "B (" + iter.value().at(1) + "B "+ tr("avaliable") +")");

            layout->addWidget(label);
            layout->addWidget(diskLabel);
            layout->addStretch();
            mDiskLayout->addLayout(layout);

        }
    }
}

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
    result.append(formats);
    result.append(language);
    return result;
}

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

QString About::getTotalMemory()
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
    int nPow = ceil(log(memtotal)/log(2.0));
    memtotal = pow(2.0, nPow);

    return QString::number(memtotal) + "GB (" + QString::number(memAvaliable, 'f', 1)+ "GB "+tr("avaliable") +")";
}

void About::activeSlot(int activeSignal)
{
    if (!activeSignal) {
        setupSerialComponent();
    }
}

void About::runActiveWindow()
{
    QString cmd = "kylin-activation";

    QProcess process(this);
    process.startDetached(cmd);
}

void About::showPdf()
{
    QStringList res = getUserDefaultLanguage();
    QString lang = res.at(1);
    QString cmd;
    QFile pdfFile_zh("/usr/share/kylin-verify-gui/免责协议.pdf");
    QFile pdfFile_en("/usr/share/kylin-verify-gui/disclaimers.pdf");
    if (lang.split(':').at(0) == "zh_CN") {
        if (pdfFile_zh.exists()) {
            cmd = "atril /usr/share/kylin-verify-gui/免责协议.pdf";
        } else {
            cmd = "atril /usr/share/man/statement.pdf.gz";
        }
    } else {
        if (pdfFile_en.exists()) {
            cmd = "atril /usr/share/kylin-verify-gui/disclaimers.pdf";
        }
        else {
            cmd = "atril /usr/share/man/statement_en.pdf.gz";
        }
    }

    QProcess process(this);
    process.startDetached(cmd);
}
































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
#include "screenlock.h"
#include "ui_screenlock.h"
#include "bgfileparse.h"
#include "pictureunit.h"
#include "MaskWidget/maskwidget.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QDesktopServices>
#include <sys/stat.h>

#define BGPATH                  "/usr/share/backgrounds/"
#define SCREENLOCK_BG_SCHEMA    "org.ukui.screensaver"
#define SCREENLOCK_BG_KEY       "background"
#define SCREENLOCK_DELAY_KEY    "lock-delay"
#define SCREENLOCK_LOCK_KEY     "lock-enabled"
#define SCREENLOCK_ACTIVE_KEY   "idle-activation-enabled"

#define MATE_BACKGROUND_SCHEMAS "org.mate.background"
#define FILENAME                "picture-filename"
const QString kylinUrl        = "https://www.ubuntukylin.com/wallpaper.html";
const QString kylinBackgroundName2 = "/usr/share/backgrounds/aurora.jpg";

Screenlock::Screenlock() : mFirstLoad(true)
{
    pluginName = tr("Screenlock");
    pluginType = PERSONALIZED;
    prePicUnit = nullptr;
}

Screenlock::~Screenlock()
{
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
    }
}

QString Screenlock::get_plugin_name()
{
    return pluginName;
}

int Screenlock::get_plugin_type()
{
    return pluginType;
}

QWidget *Screenlock::get_plugin_ui()
{
    if (mFirstLoad) {
        mFirstLoad = false;

        ui = new Ui::Screenlock;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        ui->titleLabel->setStyleSheet(" QLabel{color: palette(windowText);}");

        const QByteArray id(SCREENLOCK_BG_SCHEMA);
        lSetting = new QGSettings(id, QByteArray(), this);

        connectToServer();
        initSearchText();
        setupComponent();
        setupConnect();
        initScreenlockStatus();

        lockbgSize = QSize(400, 240);
    }
    return pluginWidget;
}

void Screenlock::plugin_delay_control()
{

}

const QString Screenlock::name() const
{
    return QStringLiteral("screenlock");
}

void Screenlock::initSearchText() {
    //~ contents_path /screenlock/Show picture of screenlock on screenlogin
    ui->loginpicLabel->setText(tr("Show picture of screenlock on screenlogin"));
    //~ contents_path /screenlock/Lock screen when screensaver boot
    ui->activepicLabel->setText(tr("Lock screen when screensaver boot"));
    //~ contents_path /screenlock/Browse
    ui->browserLocalwpBtn->setText(tr("Browse"));
    //~ contents_path /screenlock/Online Picture
    ui->onlineBtn->setText(tr("Online Picture"));
    ui->onlineBtn->setFocusPolicy(Qt::NoFocus);
    ui->onlineBtn->setContentsMargins(0,0,0,0);
    ui->onlineBtn->setCursor( QCursor(Qt::PointingHandCursor));
    ui->onlineBtn->setStyleSheet("QPushButton{background: transparent;border-radius: 4px;text-decoration: underline;} ");
    connect( ui->onlineBtn, &QPushButton::clicked, this,[=] {
        QDesktopServices::openUrl(QUrl(QLatin1String("https://www.ubuntukylin.com/wallpaper.html")));
    });
    //~ contents_path /screenlock/Reset To Default
    ui->resetBtn->setText(tr("Reset To Default"));
}

void Screenlock::setupComponent()
{
    mUKCConfig = QDir::homePath() + "/.config/ukui/ukui-control-center.conf";
    lockSetting = new QSettings(mUKCConfig, QSettings::IniFormat, this);

    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    }

    QString lockfilename = "/var/lib/lightdm-data/" + name + "/ukui-greeter.conf";
    lockLoginSettings = new QSettings(lockfilename, QSettings::IniFormat, this);

    QStringList scaleList;
    scaleList<< tr("1m") << tr("5m") << tr("10m") << tr("30m") << tr("45m")
              <<tr("1h") << tr("1.5h") << tr("3h");

    uslider = new Uslider(scaleList);
    uslider->setRange(1,8);
    uslider->setTickInterval(1);
    uslider->setPageStep(1);

    ui->delayFrame->layout()->addWidget(uslider);

    loginbgSwitchBtn = new SwitchButton(pluginWidget);
    ui->loginbgHorLayout->addWidget(loginbgSwitchBtn);
    loginbgSwitchBtn->setChecked(getLockStatus());

    lockSwitchBtn = new SwitchButton(pluginWidget);
    ui->lockHorLayout->addWidget(lockSwitchBtn);

    bool lockKey = false;
    QStringList keys =  lSetting->keys();
    if (keys.contains("lockEnabled")) {
        lockKey = true;
        bool status = lSetting->get(SCREENLOCK_LOCK_KEY).toBool();
        lockSwitchBtn->setChecked(status);
    }

    connect(lockSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked) {
        if (lockKey) {
            lSetting->set(SCREENLOCK_LOCK_KEY,  checked);
        }
    });

    connect(lSetting, &QGSettings::changed, this, [=](QString key) {
        if ("idleActivationEnabled" == key) {
            bool judge = lSetting->get(key).toBool();
            if (!judge) {
                if (lockSwitchBtn->isChecked()) {
                    lockSwitchBtn->setChecked(judge);
                }
            }
        } else if ("lockEnabled" == key) {
            bool status = lSetting->get(key).toBool();
            lockSwitchBtn->setChecked(status);
        } else if ("background" == key) {
            QString filename = lSetting->get(key).toString();
            ui->previewLabel->setPixmap(QPixmap(filename).scaled(ui->previewLabel->size()));
        }
    });

    //设置布局
    flowLayout = new FlowLayout(ui->backgroundsWidget, 16, -1, -1);

    //锁屏延时暂时不可用，屏蔽
    ui->delayFrame->hide();
    ui->line_3->hide();
}

void Screenlock::setupConnect()
{
    connect(loginbgSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked) {
        setLockBackground(checked);
    });

    connect(uslider, &QSlider::valueChanged, [&](int value) {
        QStringList keys = lSetting->keys();
        if (keys.contains("lockDelay")) {
            lSetting->set(SCREENLOCK_DELAY_KEY, convertToLocktime(value));
        }
    });

    QStringList keys = lSetting->keys();
    if (keys.contains("lockDelay")) {
        int value = lockConvertToSlider(lSetting->get(SCREENLOCK_DELAY_KEY).toInt());
        uslider->setValue(value);
    }

    connect(ui->browserLocalwpBtn, &QPushButton::clicked, this, &Screenlock::setScreenLockBgSlot);
    connect(ui->resetBtn, SIGNAL(clicked(bool)), this, SLOT(resetDefaultScreenLockSlot()));
}

void Screenlock::initScreenlockStatus()
{
    // 获取当前锁屏壁纸
    QString bgStr = lSetting->get(SCREENLOCK_BG_KEY).toString();
    if (bgStr.isEmpty()) {
        if (QGSettings::isSchemaInstalled(MATE_BACKGROUND_SCHEMAS)) {
            QGSettings * bgGsetting  = new QGSettings(MATE_BACKGROUND_SCHEMAS, QByteArray(), this);
            bgStr = bgGsetting->get(FILENAME).toString();
        }
    }

    ui->previewLabel->setPixmap(QPixmap(bgStr).scaled(ui->previewLabel->size()));

    // 使用线程解析本地壁纸文件；获取壁纸单元
    pThread = new QThread;
    pWorker = new BuildPicUnitsWorker;
    connect(pWorker, &BuildPicUnitsWorker::pixmapGeneral, this, [=](QPixmap pixmap, BgInfo bgInfo){
        // 线程中构建控件传递会报告event无法install 的警告
        PictureUnit * picUnit = new PictureUnit;
        picUnit->setPixmap(pixmap);
        picUnit->setFilenameText(bgInfo.filename);

        // 选定当前锁屏壁纸
        if (bgInfo.filename == bgStr){
            ui->previewLabel->setPixmap(QPixmap(bgStr).scaled(ui->previewLabel->size()));
            if (prePicUnit != nullptr) {
                prePicUnit->changeClickedFlag(false);
                prePicUnit->setStyleSheet("border-width: 0px;");
            }
            picUnit->changeClickedFlag(true);
            prePicUnit = picUnit;
            picUnit->setFrameShape(QFrame::Box);
            picUnit->setStyleSheet(picUnit->clickedStyleSheet);
        }

        connect(picUnit, &PictureUnit::clicked, [=](QString filename){
            if (prePicUnit != nullptr) {
                prePicUnit->changeClickedFlag(false);
                prePicUnit->setStyleSheet("border-width: 0px;");
            }
            picUnit->changeClickedFlag(true);
            prePicUnit = picUnit;
            picUnit->setFrameShape(QFrame::Box);
            picUnit->setStyleSheet(picUnit->clickedStyleSheet);
            ui->previewLabel->setPixmap(QPixmap(filename).scaled(ui->previewLabel->size()));
            lSetting->set(SCREENLOCK_BG_KEY, filename);
            setLockBackground(loginbgSwitchBtn->isChecked());
        });

        flowLayout->addWidget(picUnit);
    });
    connect(pWorker, &BuildPicUnitsWorker::workerComplete, [=]{
        pThread->quit(); // 退出事件循环
        pThread->wait(); // 释放资源
    });

    pWorker->moveToThread(pThread);
    connect(pThread, &QThread::started, pWorker, &BuildPicUnitsWorker::run);
    connect(pThread, &QThread::finished, this, [=] {

    });
    connect(pThread, &QThread::finished, pWorker, &BuildPicUnitsWorker::deleteLater);

    pThread->start();

    // 设置锁屏时间，屏保激活后多久锁定屏幕
    int lDelay = lSetting->get(SCREENLOCK_DELAY_KEY).toInt();

    uslider->blockSignals(true);
    uslider->setValue(lockConvertToSlider(lDelay));
    uslider->blockSignals(false);
}

int Screenlock::convertToLocktime(const int value)
{
    switch (value) {
    case 1:
        return 1;
        break;
    case 2:
        return 5;
        break;
    case 3:
        return 10;
        break;
    case 4:
        return 30;
        break;
    case 5:
        return 45;
        break;
    case 6:
        return 60;
        break;
    case 7:
        return 90;
        break;
    case 8:
        return 180;
        break;
    default:
        return 1;
        break;
    }
}

int Screenlock::lockConvertToSlider(const int value)
{
    switch (value) {
    case 1:
        return 1;
        break;
    case 5:
        return 2;
        break;
    case 10:
        return 3;
        break;
    case 30:
        return 4;
        break;
    case 45:
        return 5;
        break;
    case 60:
        return 6;
        break;
    case 90:
        return 7;
        break;
    case 180:
        return 8;
        break;
    default:
        return 1;
        break;
    }
}

//设置登录界面显示锁屏壁纸
void Screenlock::setLockBackground(bool status)
{
    QString bgStr;
    struct stat fileStat;
    if (lSetting && status) {
        bgStr = lSetting->get(SCREENLOCK_BG_KEY).toString();
        stat(bgStr.toStdString().c_str(),&fileStat);
        if (fileStat.st_uid != 0) {  //在普通用户下
            bgStr = copyLoginFile(bgStr);
        }
    } else if (!status) {
        bgStr = "";
    }
    lockSetting->beginGroup("ScreenLock");
    lockSetting->setValue("lockStatus", status);
    lockSetting->endGroup();

    lockLoginSettings->beginGroup("greeter");
    lockLoginSettings->setValue("backgroundPath", bgStr);
    lockLoginSettings->endGroup();
}

bool Screenlock::getLockStatus()
{
    if (!QFile::exists(mUKCConfig)) {
        setLockBackground(true);
    }

    lockSetting->beginGroup("ScreenLock");
    lockSetting->sync();
    bool status = lockSetting->value("lockStatus").toBool();
    lockSetting->endGroup();
    return  status;
}

void Screenlock::connectToServer()
{
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker,&MThread::keychangedsignal,this,&Screenlock::keyChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
     NetThread->start();
}

void Screenlock::keyChangedSlot(const QString &key)
{
    if(key == "ukui-screensaver") {
        if(!bIsCloudService)
            bIsCloudService = true;
        QString bgStr = lSetting->get(SCREENLOCK_BG_KEY).toString();
        if (bgStr.isEmpty()) {
            if (QGSettings::isSchemaInstalled(MATE_BACKGROUND_SCHEMAS)) {
                QGSettings * bgGsetting  = new QGSettings(MATE_BACKGROUND_SCHEMAS, QByteArray(), this);
                bgStr = bgGsetting->get(FILENAME).toString();
            }
        }

        ui->previewLabel->setPixmap(QPixmap(bgStr).scaled(ui->previewLabel->size()));
        QStringList keys =  lSetting->keys();
        if (keys.contains("lockEnabled")) {
            bool status = lSetting->get(SCREENLOCK_LOCK_KEY).toBool();
            lockSwitchBtn->setChecked(status);
        }
        loginbgSwitchBtn->setChecked(getLockStatus());
    }
}

void Screenlock::setScreenLockBgSlot()
{
    QStringList filters;
    filters<<tr("Wallpaper files(*.jpg *.jpeg *.bmp *.dib *.png *.jfif *.jpe *.gif *.tif *.tiff *.wdp)")<<tr("allFiles(*.*)");
    QFileDialog fd(pluginWidget);

    QList<QUrl> usb_list = fd.sidebarUrls();
    int sidebarNum = 8;// 最大添加U盘数，可以自己定义
    QString home_path = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home_path + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList file_list = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < file_list.size(); ++i) {
        QFileInfo fi = file_list.at(i);
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher m_fileSystemWatcher(&fd);
    m_fileSystemWatcher.addPath("/media/" + home_path + "/");
    connect(&m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, &fd,
            [=, &sidebarNum, &mntUrlList, &usb_list, &fd](const QString path) {
        QDir m_wmntDir(path);
        m_wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList m_wfilist = m_wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < m_wfilist.size(); ++i) {
            QFileInfo m_fi = m_wfilist.at(i);
            mntUrlList << QUrl("file://" + m_fi.filePath());
        }
        fd.setSidebarUrls(usb_list + mntUrlList);
        fd.update();
    });

    connect(&fd, &QFileDialog::finished, &fd, [=, &usb_list, &fd]() {
        fd.setSidebarUrls(usb_list);
    });


    fd.setDirectory(QString(const_cast<char *>(g_get_user_special_dir(G_USER_DIRECTORY_PICTURES))));
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilters(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("select custom wallpaper file"));
    fd.setLabelText(QFileDialog::Accept, tr("Select"));
    fd.setLabelText(QFileDialog::LookIn, tr("Position: "));
    fd.setLabelText(QFileDialog::FileName, tr("FileName: "));
    fd.setLabelText(QFileDialog::FileType, tr("FileType: "));
    fd.setLabelText(QFileDialog::Reject, tr("Cancel"));

    fd.setSidebarUrls(usb_list + mntUrlList);

    if (fd.exec() != QDialog::Accepted)
        return;
    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    QStringList fileRes = selectedfile.split("/");

    QProcess * process = new QProcess(this);
    QString program("cp");
    QStringList arguments;
    arguments << selectedfile ;
    arguments << "/tmp";
    process->start(program, arguments);

    lSetting->set(SCREENLOCK_BG_KEY, selectedfile);
    setLockBackground(loginbgSwitchBtn->isChecked());
    if (prePicUnit != nullptr) {  //去掉选定标记
        prePicUnit->changeClickedFlag(false);
        prePicUnit->setStyleSheet("border-width: 0px;");
    }
}

QString Screenlock::copyLoginFile(QString fileName) {
    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    }
    QString loginFilename = "/var/lib/lightdm-data/" + name + "/" + "loginBackground";
    QProcess process;
    QString loginCmd = QString("cp %1 %2").arg(fileName).arg(loginFilename);
    process.startDetached(loginCmd);
    return loginFilename;
}

void Screenlock::resetDefaultScreenLockSlot(){
    GSettings * wpgsettings;
    wpgsettings = g_settings_new(SCREENLOCK_BG_SCHEMA);
    GVariant * variant = g_settings_get_default_value(wpgsettings, SCREENLOCK_BG_KEY);
    gsize size = g_variant_get_size(variant);
    const char * dwp = g_variant_get_string(variant, &size);
    g_object_unref(wpgsettings);
    lSetting->set(SCREENLOCK_BG_KEY, QVariant(QString(dwp)));
    setClickedPic(kylinBackgroundName2);//默认背景图片和aurora.jpg一样，暂时特殊标记
}

void Screenlock::setClickedPic(QString fileName) {
    for (int i = flowLayout->count() - 1; i >= 0; --i) {
        QLayoutItem *it      = flowLayout->itemAt(i);
        PictureUnit *picUnit = static_cast<PictureUnit*>(it->widget());
        if (fileName == picUnit->filenameText()) {
            if (prePicUnit != nullptr) {
                prePicUnit->changeClickedFlag(false);
                prePicUnit->setStyleSheet("border-width: 0px;");
            }
            picUnit->changeClickedFlag(true);
            prePicUnit = picUnit;
            picUnit->setFrameShape(QFrame::Box);
            picUnit->setStyleSheet(picUnit->clickedStyleSheet);
        }
    }
}

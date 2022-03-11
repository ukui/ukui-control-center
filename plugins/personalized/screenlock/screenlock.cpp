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
#include <QDBusReply>
#include <QDBusInterface>
#include <QDesktopServices>
#include <QProcess>
#include <QDateTime>

#define BGPATH                           "/usr/share/backgrounds/"
#define SCREENLOCK_BG_SCHEMA             "org.ukui.screensaver"
#define SCREENLOCK_BG_KEY                "background"
#define SCREENLOCK_DELAY_KEY             "lock-delay"
#define SCREENLOCK_LOCK_KEY              "lock-enabled"
#define SCREENLOCK_MESSAGE_KEY           "show-message-enabled"
#define SCREENLOCK_ACTIVE_KEY            "idle-activation-enabled"

#define SESSION_SCHEMA                   "org.ukui.session"
#define IDLE_DELAY_KEY                   "idle-delay"

#define MATE_BACKGROUND_SCHEMAS          "org.mate.background"
#define FILENAME                         "picture-filename"
#define LOCK_BACKGROUND                  "background"

#define FORMAT_SCHEMA                    "org.ukui.control-center.panel.plugins"
#define TIME_FORMAT_KEY                  "hoursystem"
#define DATE_FORMATE_KEY                 "date"

#define UKUI_CONTROL_CENTER_PERSONALISE  "org.ukui.control-center.personalise"
#define POWER_MODE                       "power-mode"

#define POWERMANAGER_SCHEMA              "org.ukui.power-manager"
#define SLEEP_COMPUTER_AC_KEY            "sleep-computer-ac"
#define SLEEP_COMPUTER_BATT_KEY          "sleep-computer-battery"
#define SLEEP_DISPLAY_AC_KEY             "sleep-display-ac"
#define SLEEP_DISPLAY_BATT_KEY           "sleep-display-battery"

#define DISPLAY_BALANCE_BA 5 * 60
#define DISPLAY_BALANCE_AC 10 * 60
#define COMPUTER_BALANCE_BA 5 * 60
#define COMPUTER_BALANCE_AC 10 * 60

#define DISPLAY_SAVING 60
#define COMPUTER_SAVING 2 * 60
#define BRIGHTNESS_SAVING 20
Screenlock::Screenlock() : mFirstLoad(true)
{
    pluginName = tr("Lockscreen");
    pluginType = PERSONALIZED;
}

Screenlock::~Screenlock()
{
    if (!mFirstLoad) {
        delete ui;
        delete lSetting;
        delete lockSetting;
        delete lockLoginSettings;
        delete powerSettings;
        if (m_formatsettings)
            delete m_formatsettings;
    }
}
void Screenlock::datetime_update_slot(){
    QString dateformat;
    QDateTime cur_time = QDateTime::currentDateTime();
    dateformat =  m_formatsettings->get(TIME_FORMAT_KEY).toString();
    if (dateformat == "12") {
        QString currentsecStr ;
        currentsecStr = cur_time.toString("hh:mm:ss AP");
        timeLabel->setText(currentsecStr);
    } else if (dateformat == "24") {
        QString currentsecStr ;
        currentsecStr = cur_time.toString("hh:mm:ss");
        timeLabel->setText(currentsecStr);
    }
    QString dateFormat = m_formatsettings->get(DATE_FORMATE_KEY).toString();
    if (dateFormat == "cn") {
        QString currentdateStr ;
        currentdateStr = cur_time.toString("yyyy/MM/dd ddd");
        dateLabel->setText(currentdateStr);
    } else if (dateFormat == "en") {
        QString currentdateStr ;
        currentdateStr = cur_time.toString("yyyy-MM-dd ddd");
        dateLabel->setText(currentdateStr);
    }
    connect(m_formatsettings,&QGSettings::changed,this,[=](const QString &keys){
        if (keys == "hoursystem") {
            QString timeformat =  m_formatsettings->get(TIME_FORMAT_KEY).toString();
            if (timeformat == "12") {
                QString currentsecStr ;
                currentsecStr = cur_time.toString("hh:mm:ss AP");
                timeLabel->setText(currentsecStr);
            } else if (timeformat == "24") {
                QString currentsecStr ;
                currentsecStr = cur_time.toString("hh:mm:ss");
                timeLabel->setText(currentsecStr);
            }
        }
        if (keys == "date") {
            QString dateFormat = m_formatsettings->get(DATE_FORMATE_KEY).toString();
            if (dateFormat == "cn") {
                QString currentdateStr ;
                currentdateStr = cur_time.toString("yyyy/MM/dd ");
                dateLabel->setText(currentdateStr);
            } else if (dateFormat == "en") {
                QString currentdateStr ;
                currentdateStr = cur_time.toString("yyyy-MM-dd ");
                dateLabel->setText(currentdateStr);
            }
        }
    });
}
QString Screenlock::get_plugin_name(){
    return pluginName;
}

int Screenlock::get_plugin_type(){
    return pluginType;
}

QWidget *Screenlock::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Screenlock;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        m_initPath = QString("%1/%2/%3").arg(QDir::homePath()).arg(".config/ukui").arg("config.ini");//构造函数中初始化设置信息

        if(QGSettings::isSchemaInstalled(FORMAT_SCHEMA)
        && QGSettings::isSchemaInstalled(SCREENLOCK_BG_SCHEMA)
        && QGSettings::isSchemaInstalled(POWERMANAGER_SCHEMA)
        && QGSettings::isSchemaInstalled(UKUI_CONTROL_CENTER_PERSONALISE)) {
            m_formatsettings = new QGSettings(FORMAT_SCHEMA);
            lSetting = new QGSettings(SCREENLOCK_BG_SCHEMA);
            powerSettings = new QGSettings(POWERMANAGER_SCHEMA);
            mSettings = new QGSettings(UKUI_CONTROL_CENTER_PERSONALISE);
        }
        process = new QProcess();
        ui->previewWidget->setAutoFillBackground(true);
        m_itimer = new QTimer();
        m_itimer->start(1000);
        connect(m_itimer,SIGNAL(timeout()), this, SLOT(datetime_update_slot()));
        //预览遮罩
        MaskWidget * maskWidget = new MaskWidget(ui->previewLabel);
        maskWidget->setGeometry(0, 0, ui->previewLabel->width(), ui->previewLabel->height());
        onMaskWidget = new QWidget(maskWidget);
        onMaskWidget->setGeometry(0, 0, maskWidget->width(), maskWidget->height());
        maskLayout = new QVBoxLayout(onMaskWidget);
        timeLabel = new QLabel(onMaskWidget);
        dateLabel = new QLabel(onMaskWidget);
        maskLayout->setContentsMargins(16,16,16,16);
        maskLayout->setSpacing(0);
        timeLabel->setAlignment(Qt::AlignCenter);

        timeLabel->setStyleSheet("QLabel{font-size: 25px; background-color: transparent; color: white;}");
        dateLabel->setStyleSheet("QLabel{font-size: 11px; background-color: transparent; color: white;}");
        onMaskWidget->setStyleSheet("QWidget{border-radius: 6px;background-color: rgba(0,0,0,0.25);}");
        ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
        ui->previewWidget->setStyleSheet("#previewWidget{background: black; border-radius: 8px;}");
        ui->frame->setStyleSheet("QFrame{background: palette(base); border-top-left-radius: 12px;border-top-right-radius: 12px;}");
        ui->showMessageFrame->setStyleSheet("QFrame{background: palette(base); border-radius: 0px;}");
        ui->enableFrame->setStyleSheet("QFrame{background: palette(base); border-radius: 0px;}");
        ui->frame_2->setStyleSheet("QFrame{background: palette(base); border-bottom-left-radius: 12px;border-bottom-right-radius: 12px;}");

        dateLabel->setAlignment(Qt::AlignCenter);
        maskLayout->addWidget(timeLabel);
        maskLayout->addWidget(dateLabel);
        maskLayout->addStretch();

        //屏蔽锁屏预览
        ui->previewWidget->hide();

        screensaver_bin = "/usr/lib/ukui-screensaver/ukui-screensaver-default";

        sysdispatcher = new SystemDbusDispatcher;

        initSearchText();
        initButton();
        setupComponent();
        setupConnect();
        initScreenlockStatus();
        initIdleSliderStatus();
        lockbgSize = QSize(400, 240);
    }
    return pluginWidget;
}

void Screenlock::plugin_delay_control(){
}

const QString Screenlock::name() const {

    return QStringLiteral("screenlock");
}
void Screenlock::initSearchText() {
    //~ contents_path /screenlock/Number of unread messages displayed when locking screen
    ui->showMessageLabel->setText(tr("Number of unread messages displayed when locking screen"));
    //~ contents_path /screenlock/Auto-Lock
    ui->label_5->setText(tr("Auto-Lock"));
    //~ contents_path /screenlock/Screenlock Interface
    ui->titleLabel->setText(tr("Screenlock Interface"));
}

void Screenlock::setupComponent(){
    QString filename = QDir::homePath() + "/.config/ukui/ukui-control-center.conf";
    lockSetting = new QSettings(filename, QSettings::IniFormat);

    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    }
    if(QGSettings::isSchemaInstalled(SESSION_SCHEMA)) {
        qSessionSetting = new QGSettings(SESSION_SCHEMA, QByteArray(), this);
    }
    QString lockfilename = "/var/lib/lightdm-data/" + name + "/ukui-greeter.conf";
    lockLoginSettings = new QSettings(lockfilename, QSettings::IniFormat);

    QStringList scaleList;
    scaleList<< tr("1min") << tr("5min") << tr("10min") << tr("30min") << tr("45min")
             <<tr("1hour") << tr("1.5hour") << tr("2hour") << tr("3hour") << tr("Never");
    uslider = new Uslider(scaleList);
    uslider->setStyleSheet( "QSlider::groove:horizontal {"
                                   "border: 0px none;"
                                   "background: palette(button);"
                                   "height: 8px;"
                                   "border-radius: 5px;"
                                   "}"

                                   "QSlider::handle:horizontal {"

                                   "height: 24px;"
                                   "width: 32px;"
                                   "margin: 30px;"
                                   "border-image: url(://img/plugins/mouse/slider.svg);"
                                   "margin: -8 -4px;"
                                   "}"

                                   "QSlider::add-page:horizontal {"
                                   "background: palette(button);"
                                   "border-radius: 20px;"
                                   "}"

                                   "QSlider::sub-page:horizontal {"
                                   "background: #2FB3E8;"
                                   "border-radius: 5px;"
                                   "}");
    uslider->setRange(10,100);
    uslider->setTickInterval(10);
    uslider->setPageStep(10);
    uslider->setTracking(false);
    uslider->setSingleStep(10);
    uslider->setMinimumWidth(495);
    ui->lockhorizontalLayout->addWidget(uslider);
    //隐藏激活屏保时锁定屏幕设置按钮

    showMessageBtn = new SwitchButton(pluginWidget);
    ui->messageShowHorLayout->addWidget(showMessageBtn);


    bool lockKey = false;
    QStringList keys =  lSetting->keys();

    if (keys.contains("showMessageEnabled")) {
        lockKey = true;
        messageKey = true;
        bool status = lSetting->get(SCREENLOCK_MESSAGE_KEY).toBool();
        showMessageBtn->setChecked(status);
    }
    connect(showMessageBtn, &SwitchButton::checkedChanged, this, [=](bool checked) {
        if (messageKey) {
            lSetting->set(SCREENLOCK_MESSAGE_KEY,  checked);
        }
    });

    connect(lSetting, &QGSettings::changed, this, [=](QString key) {
        if ("idleActivationEnabled" == key) {
            bool judge = lSetting->get(key).toBool();
        } else if ("showMessageEnabled" == key){
            bool status = lSetting->get(key).toBool();
            showMessageBtn->setChecked(status);
        } else if ("background" == key) {
            initPreviewStatus();
            QString filename = lSetting->get(key).toString();
            setClickedPic(filename);
        }
    });
    //设置布局
    flowLayout = new FlowLayout;
    flowLayout->setContentsMargins(11, 13, 11, 13);
    ui->backgroundsWidget->setLayout(flowLayout);
}

void Screenlock::initButton(){
    localBgd = new HoverWidget("");
    localBgd->setObjectName("localBgd");
    localBgd->setToolTip(tr("Local wallpaper"));
    localBgd->setFixedSize(136,56);
    localBgd->setStyleSheet("HoverWidget#localBgd{background: palette(base); border-radius: 12px;}HoverWidget:hover:!pressed#localBgd{background: #2FB3E8; border-radius: 12px;}");
    addLyt_1 = new QHBoxLayout(pluginWidget);
    QLabel * iconLabel_1 = new QLabel();
    //~ contents_path /screenlock/Local wallpaper
    QLabel * textLabel_1 = new QLabel(tr("Local wall..."));
    textLabel_1->setStyleSheet("color: #2FB3E8;font-weight: 400;");
    QPixmap pixgray_1 = ImageUtil::loadSvg(":/img/titlebar/openLocal.svg", "default", 24);
    iconLabel_1->setPixmap(pixgray_1);
    addLyt_1->addStretch();
    addLyt_1->addWidget(iconLabel_1);
    addLyt_1->addWidget(textLabel_1);
    addLyt_1->addStretch();
    localBgd->setLayout(addLyt_1);
    ui->horizontalLayout_2->addWidget(localBgd);

    // 悬浮改变Widget状态
    connect(localBgd, &HoverWidget::enterWidget, this, [=](QString mname){
        Q_UNUSED(mname);
        QPixmap pixgray_1 = ImageUtil::loadSvg(":/img/titlebar/openLocal.svg", "white", 24);
        iconLabel_1->setPixmap(pixgray_1);
        textLabel_1->setStyleSheet("color: palette(base);");
    });
    // 还原状态
    connect(localBgd, &HoverWidget::leaveWidget, this, [=](QString mname){
        Q_UNUSED(mname);
        QPixmap pixgray_1 = ImageUtil::loadSvg(":/img/titlebar/openLocal.svg", "default", 24);
        iconLabel_1->setPixmap(pixgray_1);
        textLabel_1->setStyleSheet("color: #2FB3E8;font-weight: 400;");
    });
    connect(localBgd, &HoverWidget::widgetClicked,[=]{
        showLocalWpDialog();
    });

    resetBgd = new HoverWidget("");
    resetBgd->setObjectName("resetBgd");
    resetBgd->setFixedSize(136,56);
    resetBgd->setStyleSheet("HoverWidget#resetBgd{background: palette(base); border-radius: 12px;}HoverWidget:hover:!pressed#resetBgd{background: #2FB3E8; border-radius: 12px;}");
    addLyt_2 = new QHBoxLayout(pluginWidget);
    QLabel * iconLabel_2 = new QLabel();
    //~ contents_path /screenlock/Reset
    QLabel * textLabel_2 = new QLabel(tr("Reset"));
    textLabel_2->setStyleSheet("color: #2FB3E8;font-weight: 400;");
    QPixmap pixgray_2 = ImageUtil::loadSvg(":/img/titlebar/resetBgd.svg", "default", 24);
    iconLabel_2->setPixmap(pixgray_2);
    addLyt_2->addStretch();
    addLyt_2->addWidget(iconLabel_2);
    addLyt_2->addWidget(textLabel_2);
    addLyt_2->addStretch();
    resetBgd->setLayout(addLyt_2);
    ui->horizontalLayout_2->addWidget(resetBgd);
    ui->horizontalLayout_2->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    // 悬浮改变Widget状态
    connect(resetBgd, &HoverWidget::enterWidget, this, [=](QString mname){
        Q_UNUSED(mname);
        QPixmap pixgray_2 = ImageUtil::loadSvg(":/img/titlebar/resetBgd.svg", "white", 24);
        iconLabel_2->setPixmap(pixgray_2);
        textLabel_2->setStyleSheet("color: palette(base);");

    });
    // 还原状态
    connect(resetBgd, &HoverWidget::leaveWidget, this, [=](QString mname){
        Q_UNUSED(mname);
        QPixmap pixgray_2 = ImageUtil::loadSvg(":/img/titlebar/resetBgd.svg", "default", 24);
        iconLabel_2->setPixmap(pixgray_2);
        textLabel_2->setStyleSheet("color: #2FB3E8;font-weight: 400;");
    });
    connect(resetBgd, &HoverWidget::widgetClicked,[=]{
        GSettings * wpgsettings;
        wpgsettings = g_settings_new(UKUI_SCREENSAVER);
        GVariant * variant = g_settings_get_default_value(wpgsettings, LOCK_BACKGROUND);
        gsize size = g_variant_get_size(variant);
        //        const char * dwp = g_variant_get_string(variant, &size);
        g_object_unref(wpgsettings);
        QString dwp = "/usr/share/backgrounds/warty-final-ubuntukylin.jpg";
        lSetting->set(LOCK_BACKGROUND, dwp);
        plugin_delay_control();
        uslider->setValue(20);
        showMessageBtn->setChecked(true);
    });

}

void Screenlock::setupConnect(){
    //屏保时间
    session_settings = g_settings_new(SESSION_SCHEMA);
    connect(uslider, &QSlider::valueChanged, this, [=]{
        int value = convertToLocktime(qRound(uslider->value()/10.00)*10);
        screenlock_settings=g_settings_new(SCREENLOCK_BG_SCHEMA);
        if(value==-1){
            //qDebug()<<"never";           
            g_settings_set_boolean(screenlock_settings,SCREENLOCK_ACTIVE_KEY,false);
            //当设置为从不时，禁止系统进入睡眠
            //设置显示器关闭
            powerSettings->set(SLEEP_DISPLAY_AC_KEY, -1);
            powerSettings->set(SLEEP_DISPLAY_BATT_KEY,-1);
            //设置计算机睡眠
            powerSettings->set(SLEEP_COMPUTER_AC_KEY, -1);
            powerSettings->set(SLEEP_COMPUTER_BATT_KEY, -1);
            g_object_unref(screenlock_settings);
            uslider->setValue(lockConvertToSlider(value));
        }else{
            QString powerMode = mSettings->get(POWER_MODE).toString();
            //设置显示器关闭
            powerSettings->set(SLEEP_DISPLAY_AC_KEY, (value + 5) * 60);
            powerSettings->set(SLEEP_DISPLAY_BATT_KEY, (value + 5) * 60);
            //设置计算机睡眠
            powerSettings->set(SLEEP_COMPUTER_AC_KEY, (value + 5) * 60);
            powerSettings->set(SLEEP_COMPUTER_BATT_KEY, (value + 5) * 60);
            g_settings_set_boolean(screenlock_settings,SCREENLOCK_ACTIVE_KEY,true);
            g_settings_set_int(screenlock_settings, IDLE_DELAY_KEY, value);
            g_object_unref(screenlock_settings);
            //qDebug()<<"value"<<value;
        }
    });
	    g_settings_set_int(session_settings, IDLE_DELAY_KEY, 1);
            g_object_unref(session_settings);
    connect(lSetting, &QGSettings::changed, this,[=](const QString& key){
        if ("idleDelay" == key) {
            screenlock_settings=g_settings_new(SCREENLOCK_BG_SCHEMA);
            auto lock_value=g_settings_get_boolean(screenlock_settings,SCREENLOCK_ACTIVE_KEY);
            if(!lock_value){
                uslider->setValue(lockConvertToSlider(-1));
                return;
            }
            auto value = lSetting->get(key).toInt();
            uslider->setValue(lockConvertToSlider(value));
        }
    });
}

bool Screenlock::writeInit(QString group, QString key, uint32_t value)
{
    if(group.isEmpty() || key.isEmpty())
    {
        return false;
    }else
    {
        //创建配置文件操作对象,Iniformat存储数据到INI文件中
        QSettings config(m_initPath, QSettings::IniFormat);

        //将信息写入配置文件
        config.beginGroup(group);
        config.setValue(key, value);
        config.endGroup();

        return true;
    }
}

void Screenlock::initIdleSliderStatus(){
    int minutes;
    session_settings = g_settings_new(SESSION_SCHEMA);
    //minutes = g_settings_get_int(session_settings, IDLE_DELAY_KEY);

    screenlock_settings=g_settings_new(SCREENLOCK_BG_SCHEMA);
    minutes = g_settings_get_int(screenlock_settings, IDLE_DELAY_KEY);
    bool lock_value=g_settings_get_boolean(screenlock_settings,SCREENLOCK_ACTIVE_KEY);
    qDebug()<<"lock"<<lock_value;
    if(!lock_value){
        minutes=-1;
    }
    //qDebug()<<"minutes"<<minutes;
    uslider->blockSignals(true);
    uslider->setValue(lockConvertToSlider(minutes));
    uslider->blockSignals(false);
    g_object_unref(session_settings);

}

void Screenlock::set_idle_gsettings_value(int value){
    g_settings_set_int(session_settings, IDLE_DELAY_KEY, value);
}

void Screenlock::slider_released_slot(){
    int minutes;
    //    minutes = ui->idleSlider->value();
    minutes = convertToLocktime(uslider->value());
    set_idle_gsettings_value(minutes);
}

void Screenlock::initScreenlockStatus(){
    // 获取当前锁屏壁纸
    initPreviewStatus();

    QString bgStr = lSetting->get(SCREENLOCK_BG_KEY).toString();
    if (bgStr.isEmpty()) {
        if (QGSettings::isSchemaInstalled(MATE_BACKGROUND_SCHEMAS)) {
            QGSettings * bgGsetting  = new QGSettings(MATE_BACKGROUND_SCHEMAS, QByteArray(), this);
            bgStr = bgGsetting->get(FILENAME).toString();
            lSetting->set(LOCK_BACKGROUND, bgStr);
        }
    }
    // 使用线程解析本地壁纸文件；获取壁纸单元
    pThread = new QThread;
    pWorker = new BuildPicUnitsWorker;
    connect(pWorker, &BuildPicUnitsWorker::pixmapGeneral, this, [=](QPixmap pixmap, BgInfo bgInfo){

        // 线程中构建控件传递会报告event无法install 的警告
        PictureUnit * picUnit = new PictureUnit;
        picUnit->setPixmap(pixmap);
        picUnit->setFilenameText(bgInfo.filename);

        // 设置当前锁屏壁纸的预览
        if (bgInfo.filename == bgStr){
            initPreviewStatus();
            if (prePicUnit != nullptr) {
                prePicUnit->changeClickedFlag(false);
                prePicUnit->setStyleSheet("border-width: 0px");
            }
            picUnit->changeClickedFlag(true);
            prePicUnit = picUnit;
            picUnit->setFrameShape(QFrame::Box);
            picUnit->setStyleSheet(picUnit->clickedStyleSheet);
        }

        connect(picUnit, &PictureUnit::clicked, [=](QString filename){
            if (prePicUnit != nullptr) {
                prePicUnit->changeClickedFlag(false);
                prePicUnit->setStyleSheet("border-width: 0px");
            }
            picUnit->changeClickedFlag(true);
            prePicUnit = picUnit;
            picUnit->setFrameShape(QFrame::Box);
            picUnit->setStyleSheet(picUnit->clickedStyleSheet);
            initPreviewStatus();
            plugin_delay_control();
            qDebug()<<filename;
            lSetting->set(SCREENLOCK_BG_KEY, filename);
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

}
void Screenlock::showLocalWpDialog(){
    QStringList filters;
    //由于锁屏无法解析.tiff格式文件，故去掉此类文件的选择
    filters<<tr("Wallpaper files(*.jpg *.jpeg *.bmp *.dib *.png *.jfif *.jpe *.gif *.wdp)");
    QFileDialog fd(pluginWidget);

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

    if (fd.exec() != QDialog::Accepted)
        return;
    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    QStringList fileRes = selectedfile.split("/");

    QProcess * process = new QProcess();
    QString program("cp");
    QStringList arguments;
    arguments << selectedfile ;
    arguments << QDir::homePath() + "/.config";
    process->start(program, arguments);
    QString bgfile = QDir::homePath() + "/.config/"  + fileRes.at(fileRes.length() - 1);
    lSetting->set(SCREENLOCK_BG_KEY, bgfile);
    plugin_delay_control();
}
void Screenlock::initPreviewStatus(){
    //设置图片背景的预览效果
    QString filename = lSetting->get(SCREENLOCK_BG_KEY).toString();
    //    qDebug()<<"preview pic is---------->"<<filename<<endl;
    //装机后第一次打开控制面板，未设置锁屏壁纸时，锁屏预览壁纸是空的
    if (filename == "") {
        filename = "/usr/share/backgrounds/warty-final-ubuntukylin.jpg";
    }
    ui->previewLabel->setPixmap(QPixmap(filename).scaled(ui->previewLabel->size()));
}

void Screenlock::initPreviewWidget(){
    startupScreensaver();
}
void Screenlock::startupScreensaver(){
    //关闭屏保
    closeScreensaver();
    qDebug() << Q_FUNC_INFO << ui->previewWidget->winId();
        QStringList args;
        args << "-window-id" << QString::number(ui->previewWidget->winId());
        //启动屏保
        process->startDetached(screensaver_bin, args);
        runStringList.append(screensaver_bin);
}
void Screenlock::closeScreensaver(){
    //杀死分离启动的屏保预览程序
    if (!runStringList.isEmpty()){
        process->start(QString("killall"), runStringList);
        process->waitForStarted();
        process->waitForFinished(2000);

        runStringList.clear();
    }
}
int Screenlock::convertToLocktime(const int value) {
    switch (value) {
    case 10:
        return 1;
        break;
    case 20:
        return 5;
        break;
    case 30:
        return 10;
        break;
    case 40:
        return 30;
        break;
    case 50:
        return 45;
        break;
    case 60:
        return 60;
        break;
    case 70:
        return 90;
        break;
    case 80:
        return 120;
        break;
    case 90:
        return 180;
        break;
    case 100:
        return -1;
        break;
    default:
        return 1;
        break;
    }
}

int Screenlock::lockConvertToSlider(const int value) {
    switch (value) {
    case 1:
        return 10;
        break;
    case 5:
        return 20;
        break;
    case 10:
        return 30;
        break;
    case 30:
        return 40;
        break;
    case 45:
        return 50;
        break;
    case 60:
        return 60;
        break;
    case 90:
        return 70;
        break;
    case 120:
        return 80;
        break;
    case 180:
        return 90;
        break;
    case -1:
        return 100;
        break;
    default:
        return 10;
        break;
    }
}

void Screenlock::setLockBackground(bool status)
{
    QString bgStr;
    if (lSetting && status) {
        bgStr= lSetting->get(SCREENLOCK_BG_KEY).toString();
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
    lockSetting->beginGroup("ScreenLock");
    bool status = lockSetting->value("lockStatus").toBool();
    lockSetting->endGroup();
    return  status;
}

void Screenlock::setClickedPic(QString fileName)
{
    if (prePicUnit != nullptr) {
        prePicUnit->changeClickedFlag(false);
        prePicUnit->setStyleSheet("border-width:0px;");
    }
    for (int i = flowLayout->count()-1; i>= 0; --i) {
        QLayoutItem *it = flowLayout->itemAt(i);
        PictureUnit *picUnit = static_cast<PictureUnit*>(it->widget());
        if (fileName == picUnit->filenameText()) {
            picUnit->changeClickedFlag(true);
            prePicUnit = picUnit;
            picUnit->setFrameShape(QFrame::Box);
            picUnit->setStyleSheet(picUnit->clickedStyleSheet);
        }
    }
}

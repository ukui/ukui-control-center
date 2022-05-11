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
#include "screensaver.h"
#include "ui_screensaver.h"

#include <QDebug>
#include <QBoxLayout>
#include <QTextBrowser>
#include <QFileDialog>
#include <QLineEdit>
#include <ukcc/widgets/combobox.h>
#include <QListView>
#include <ukcc/widgets/hlineframe.h>
#include <QRadioButton>
#include <QButtonGroup>
#include <ukcc/widgets/fixlabel.h>

#define SSTHEMEPATH                 "/usr/share/applications/screensavers/"
#define ID_PREFIX                   "screensavers-ukui-"

#define SCREENSAVER_SCHEMA          "org.ukui.screensaver"
#define MODE_KEY                    "mode"
#define THEMES_KEY                  "themes"
#define LOCK_KEY                    "lock-enabled"
#define AUTO_SWITCH_KEY             "automatic-switching-enabled"
#define MYTEXT_KEY                  "mytext"
#define TEXT_CENTER_KEY             "text-is-center"
#define SHOW_REST_TIME_KEY          "show-rest-time"
#define SHOW_CUSTOM_REST_TIME_KEY   "show-custom-rest-time"
#define SHOW_UKUI_REST_TIME_KEY     "show-ukui-rest-time"
#define CONTAIN_AUTO_SWITCH_KEY     "automaticSwitchingEnabled"
#define CONTAIN_MYTEXT_KEY          "mytext"
#define CONTAIN_TEXT_CENTER_KEY     "textIsCenter"
#define CONTAIN_SHOW_REST_TIME_KEY  "showRestTime"
#define CONTAIN_SHOW_CUSTOM_REST_TIME_KEY  "showCustomRestTime"
#define CONTAIN_SHOW_UKUI_REST_TIME_KEY    "showUkuiRestTime"
#define IDLE_DELAY_KEY              "idle-delay"

#define SCREENSAVER_DEFAULT_SCHEMA  "org.ukui.screensaver-default"
#define BACKGROUND_PATH_KEY         "background-path"
#define CYCLE_TIME_KEY              "cycle-time"
#define CONTAIN_BACKGROUND_PATH_KEY "backgroundPath"
#define CONTAIN_CYCLE_TIME_KEY      "cycleTime"

const int silderNeverValue = -1;

#define IDLEMIN 1
#define IDLEMAX 120
#define IDLESTEP 1

typedef enum
{
    MODE_BLANK_ONLY,   // 纯黑屏保
    MODE_RANDOM,       // 暂无
    MODE_SINGLE,       // 单独的屏保设置(多用于其它屏保)
    MODE_IMAGE,        // 暂无
    MODE_DEFAULT_UKUI, // UKUI
    MODE_CUSTOMIZE,    // 自定义
}SaverMode;


const QStringList screensaverList = {
                            "BinaryRing",
                            "FuzzyFlakes",
                            "Galaxy"};

/*
 选择框中的序号     
*/
typedef enum
{
    INDEX_MODE_DEFAULT_UKUI,
    INDEX_MODE_BLANK_ONLY,
}SaverModeIndex;
static int INDEX_MODE_CUSTOMIZE = -1;

Screensaver::Screensaver() : mFirstLoad(true)
{
    pluginName = tr("Screensaver");
    pluginType = PERSONALIZED;
}

Screensaver::~Screensaver()
{
    if (!mFirstLoad) {
        closeScreensaver();
        delete ui;
        ui = nullptr;
        delete process;
        process = nullptr;
    }
}

QString Screensaver::plugini18nName()
{
    return pluginName;
}

int Screensaver::pluginTypes()
{
    return pluginType;
}

QWidget *Screensaver::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;

        ui = new Ui::Screensaver;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
  
        process = new QProcess();
        
        previewWind = new PreviewWindow();
        previewWind->setWidth(ui->previewWidget->width());
        previewWind->setHeight(ui->previewWidget->height());

        initSearchText();
        _acquireThemeinfoList();
        initComponent();
        initShowTimeBtnStatus();
        initThemeStatus();
        initIdleSliderStatus();
    }
    QTimer::singleShot(10, [&]{
        initPreviewWidget();
    });

    return pluginWidget;
}

const QString Screensaver::name() const
{
    return QStringLiteral("Screensaver");
}

void Screensaver::plugin_leave()
{
    closeScreensaver();
    return;
}

bool Screensaver::isShowOnHomePage() const
{
    return true;
}

QIcon Screensaver::icon() const
{
    return QIcon::fromTheme("ukui-screensaver-symbolic");
}

bool Screensaver::isEnable() const
{
    return true;
}

void Screensaver::initPreviewLabel()
{
    if (previewLabel == nullptr) {
        previewLabel = new PressLabel(ui->previewWidget->parentWidget()->parentWidget());
        previewLabel->setStyleSheet("background-color: rgb(38,38,38); border-radius: 0px; color:white;");
        previewLabel->setContentsMargins(0,0,0,4);
        previewLabel->setText(tr("View"));//预览
        previewLabel->setAlignment(Qt::AlignCenter);
    }
    QPoint pos = ui->previewWidget->mapToParent(ui->previewWidget->parentWidget()->pos());
    previewLabel->setGeometry(pos.x() + 120, pos.y() + 142, 60, 30);
    previewLabel->setVisible(true);
    previewLabel->raise();
}

void Screensaver::initSearchText()
{
    //~ contents_path /Screensaver/Screensaver program
    ui->programLabel->setText(tr("Screensaver program"));
    //~ contents_path /Screensaver/Idle time
    ui->idleLabel->setText(tr("Idle time"));
}

void Screensaver::initComponent()
{
    if (QGSettings::isSchemaInstalled(SCREENSAVER_SCHEMA)) {
        qScreenSaverSetting = new QGSettings(SCREENSAVER_SCHEMA, QByteArray(), this);
        mScreenSaverKeies = qScreenSaverSetting->keys();
    }

    if (QGSettings::isSchemaInstalled(SCREENSAVER_DEFAULT_SCHEMA)) {
        qScreensaverDefaultSetting = new QGSettings(SCREENSAVER_DEFAULT_SCHEMA, QByteArray(), this);
    } else {
        qDebug()<<"org.ukui.screensaver-default not installed"<<endl;
        qScreensaverDefaultSetting = nullptr;
    }

    screensaver_bin = "/usr/lib/ukui-screensaver/ukui-screensaver-default";

    //添加开启屏保按钮
    ui->enableFrame->setVisible(false); //屏保激活锁屏(原打开屏保按钮)，在锁屏界面实现，这里暂时隐藏处理，保留原打开功能
    enableSwitchBtn = new SwitchButton(ui->enableFrame);
    ui->enableHorLayout->addStretch();
    ui->enableHorLayout->addWidget(enableSwitchBtn);

    initCustomizeFrame();
    //初始化屏保程序下拉列表
    ui->programCombox->addItem(tr("UKUI"));
    ui->programCombox->addItem(tr("Blank_Only"));
    // ui->programCombox->addItem(tr("Random"));
    QMap<QString, SSThemeInfo>::iterator it = infoMap.begin();
    for (int index = 2; it != infoMap.end(); it++) {
        SSThemeInfo info = (SSThemeInfo)it.value();
        // if (!screensaverList.contains(info.name))  //开放所有下载的第三方屏保，接口保留
        //     continue;
        ui->programCombox->addItem(info.name);
        ui->programCombox->setItemData(index, QVariant::fromValue(info));
        index++;
    }
    ui->programCombox->addItem(tr("Customize"));
    INDEX_MODE_CUSTOMIZE = ui->programCombox->count() - 1;  //得到【自定义】在滑动栏中的位置

    QListView* view = qobject_cast<QListView *>(ui->programCombox->view());
    Q_ASSERT(view != nullptr);
    view->setRowHidden(1, true);  //隐藏纯黑，使用隐藏，防止以后会需要纯黑
    //初始化滑动条
    QStringList scaleList;
    scaleList<< tr("5min") << tr("10min") << tr("15min") << tr("30min") << tr("1hour")
              << tr("Never");

    ui->idleCombox->addItems(scaleList);

    connect(ui->idleCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](){
        int value = ui->idleCombox->currentIndex();
        screensaver_settings = g_settings_new(SCREENSAVER_SCHEMA);
        int setValue = convertToLocktime(value + 1);
        qScreenSaverSetting->set(IDLE_DELAY_KEY, setValue);
    });
    connectToServer();
    connect(qScreenSaverSetting, &QGSettings::changed, this,[=](const QString& key) {
       if ("idleDelay" == key) {
            int value = qScreenSaverSetting->get(key).toInt();
            ui->idleCombox->setCurrentIndex(lockConvertToSlider(value) - 1);
       }
    });
    connect(ui->programCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(themesComboxChanged(int)));
    connect(ui->previewWidget, &QWidget::destroyed, this, [=] {
        closeScreensaver();
    });

    qApp->installEventFilter(this);
}

void Screensaver::initPreviewWidget()
{
    previewWind->setVisible(false);
    ui->previewWidget->update();
    qApp->processEvents();
    startupScreensaver();
}

/*显示时间的设置*/
void Screensaver::initShowTimeBtnStatus()
{
    if (qScreensaverDefaultSetting != nullptr) {
        if (qScreensaverDefaultSetting->keys().contains(CONTAIN_SHOW_CUSTOM_REST_TIME_KEY,Qt::CaseSensitive)) {
            showCustomTimeBtn->setChecked(qScreensaverDefaultSetting->get(SHOW_CUSTOM_REST_TIME_KEY).toBool());
            connect(showCustomTimeBtn,&SwitchButton::checkedChanged,this,[=]{
                qScreensaverDefaultSetting->set(SHOW_CUSTOM_REST_TIME_KEY,showCustomTimeBtn->isChecked());
            });
        } else {
            showCustomTimeBtn->setChecked(false);
        }
        if (qScreensaverDefaultSetting->keys().contains(CONTAIN_SHOW_UKUI_REST_TIME_KEY,Qt::CaseSensitive)) {
            showUkuiTimeBtn->setChecked(qScreensaverDefaultSetting->get(SHOW_UKUI_REST_TIME_KEY).toBool());
            connect(showUkuiTimeBtn,&SwitchButton::checkedChanged,this,[=]{
                qScreensaverDefaultSetting->set(SHOW_UKUI_REST_TIME_KEY,showUkuiTimeBtn->isChecked());
            });
        } else {
            showUkuiTimeBtn->setChecked(false);
        }
    } else {
        showCustomTimeBtn->setEnabled(false);
        showUkuiTimeBtn->setChecked(false);
    }
}

void Screensaver::initThemeStatus()
{
    int mode;
    char * name;

    ui->programCombox->blockSignals(true);

    QByteArray id(SCREENSAVER_SCHEMA);
    if (QGSettings::isSchemaInstalled(id)) {
        screensaver_settings = g_settings_new(SCREENSAVER_SCHEMA);
    } else {
        qDebug()<<"org.ukui.screensaver not installed"<<endl;
        return ;
    }

    mode = g_settings_get_enum(screensaver_settings, MODE_KEY);
    if (mode == MODE_BLANK_ONLY) {  //如果是黑屏，则设置为UKUI
        g_settings_set_enum(screensaver_settings, MODE_KEY, MODE_DEFAULT_UKUI);
        mode = MODE_DEFAULT_UKUI;
    } 

    if (mode == MODE_DEFAULT_UKUI) {
        ui->programCombox->setCurrentIndex(INDEX_MODE_DEFAULT_UKUI); //UKUI
        hideCustomizeFrame();
        showUkuiTimeBtn->show();
        showCustomTimeBtn->hide();
    }
    else if (mode == MODE_BLANK_ONLY) {
        ui->programCombox->setCurrentIndex(INDEX_MODE_BLANK_ONLY); //Black_Only
        hideCustomizeFrame();
    } else if (mode == MODE_CUSTOMIZE) {
        ui->programCombox->setCurrentIndex(INDEX_MODE_CUSTOMIZE); //CUSTOMIZE
        showCustomizeFrame();
        showUkuiTimeBtn->hide();
        showCustomTimeBtn->show();
    } else {
        hideCustomizeFrame();
        if (showTimeFrame)
            showTimeFrame->hide();
        gchar ** strv;
        strv = g_settings_get_strv(screensaver_settings, THEMES_KEY);
        if (strv != NULL) {
            name = g_strdup(strv[0]);

            QString dest =  (infoMap.find(name) != infoMap.end()) ? infoMap.value(name).name : "";

            if (dest == "") {
                ui->programCombox->setCurrentIndex(INDEX_MODE_DEFAULT_UKUI);
            } else {
                ui->programCombox->setCurrentText(dest);
            }
        } else {
            ui->programCombox->setCurrentIndex(INDEX_MODE_DEFAULT_UKUI); //no data, default
        }
        g_strfreev(strv);
    }
    g_object_unref(screensaver_settings);

    ui->programCombox->blockSignals(false);
}

void Screensaver::initIdleSliderStatus()
{
    int minutes = 0;
    //判断是否存在该设置项
    if (!mScreenSaverKeies.contains("idleDelay")) {
        ui->idleCombox->blockSignals(true);
        ui->idleCombox->setCurrentIndex(5);
        ui->idleCombox->blockSignals(false);
    } else {
        minutes = qScreenSaverSetting->get(IDLE_DELAY_KEY).toInt();
        ui->idleCombox->blockSignals(true);
        ui->idleCombox->setCurrentIndex(lockConvertToSlider(minutes) - 1);
        ui->idleCombox->blockSignals(false);
    }
}

void Screensaver::startupScreensaver()
{
    //关闭屏保
    closeScreensaver();

    QWindow *preWidFromWin = QWindow::fromWinId(ui->previewWidget->winId());
    previewWind->winId();
    previewWind->setParent(preWidFromWin);
    previewWind->show();

    qDebug() << Q_FUNC_INFO << ui->previewWidget->winId();
    if (ui->programCombox->currentIndex() == INDEX_MODE_DEFAULT_UKUI || \
        ui->programCombox->currentIndex() == INDEX_MODE_CUSTOMIZE) { //UKUI && 自定义
         QStringList args;
         args << "-window-id" << QString::number(previewWind->winId());
        //启动屏保
        process->startDetached(screensaver_bin, args);
        runStringList.append(screensaver_bin);
    } else if (ui->programCombox->currentIndex() == INDEX_MODE_BLANK_ONLY) {//黑屏
        previewWind->setVisible(false);
        ui->previewWidget->update();
    } else {// 屏保
        SSThemeInfo info = ui->programCombox->currentData().value<SSThemeInfo>();
        QStringList args;
        args << "-window-id" << QString::number(previewWind->winId());
        // 启动屏保
        process->startDetached(info.exec, args);
        runStringList.append(info.exec);
    }
}

void Screensaver::closeScreensaver() {
    //杀死分离启动的屏保预览程序
    if (!runStringList.isEmpty()) {
        QString cmd = "killall";
        for(int i = 0; i < runStringList.count(); i++) {
            cmd = cmd + " " + runStringList.at(i);
        }
        qDebug()<<"cmd = "<<cmd;
        system(cmd.toLatin1().data());
        runStringList.clear();
    }
}

void Screensaver::kill_and_start()
{
    emit kill_signals(); //如果有屏保先杀死
    if (ui->programCombox->currentIndex() == INDEX_MODE_DEFAULT_UKUI || \
        ui->programCombox->currentIndex() == INDEX_MODE_CUSTOMIZE) { //UKUI && 自定义
        QStringList args;
        args << "-window-id" << QString::number(mPreviewWidget->winId());
        //启动屏保
        process->startDetached(screensaver_bin, args);
        runStringList.append(screensaver_bin);
    } else if (ui->programCombox->currentIndex() == INDEX_MODE_BLANK_ONLY) { //黑屏
        mPreviewWidget->update();
    } else if (ui->programCombox->currentIndex() == 2) {//随机
        mPreviewWidget->update();
    } else {//屏保
        SSThemeInfo info = ui->programCombox->currentData().value<SSThemeInfo>();
        QStringList args;
        args << "-window-id" << QString::number(mPreviewWidget->winId());
        //启动屏保
        process->startDetached(info.exec, args);
        killList.clear(); killList.append(info.exec);
    }
}

int Screensaver::convertToLocktime(const int value)
{
    switch (value) {
    case 1:
        return 5;
        break;
    case 2:
        return 10;
        break;
    case 3:
        return 15;
        break;
    case 4:
        return 30;
        break;
    case 5:
        return 60;
        break;
    case 6:
        return silderNeverValue;
        break;
    default:
        return silderNeverValue;
        break;
    }
}

int Screensaver::lockConvertToSlider(const int value)
{
    switch (value) {
    case silderNeverValue:
        return 6;
        break;
    case 5:
        return 1;
        break;
    case 10:
        return 2;
        break;
    case 15:
        return 3;
        break;
    case 30:
        return 4;
        break;
    case 60:
        return 5;
        break;
    default:
        return 6;
        break;
    }
}

void Screensaver::themesComboxChanged(int index)
{
    char ** strv = NULL;

    //设置屏保
    screensaver_settings = g_settings_new(SCREENSAVER_SCHEMA);
    if (index == INDEX_MODE_DEFAULT_UKUI) {
        g_settings_set_enum(screensaver_settings, MODE_KEY, MODE_DEFAULT_UKUI);
        hideCustomizeFrame();
        if (showTimeFrame) {
            showTimeFrame->show();
            showUkuiTimeBtn->show();
            showCustomTimeBtn->hide();
        }
    } else if (index == INDEX_MODE_BLANK_ONLY) {
        hideCustomizeFrame();
        g_settings_set_enum(screensaver_settings, MODE_KEY, MODE_BLANK_ONLY);
    } else if (index == INDEX_MODE_CUSTOMIZE) {
        showCustomizeFrame();
        g_settings_set_enum(screensaver_settings, MODE_KEY, MODE_CUSTOMIZE);
        if (showTimeFrame) {
            showTimeFrame->show();
            showUkuiTimeBtn->hide();
            showCustomTimeBtn->show();
        }
    } else {
        hideCustomizeFrame();
        if (showTimeFrame)
            showTimeFrame->hide();
        g_settings_set_enum(screensaver_settings, MODE_KEY, MODE_SINGLE);
        //获取当前屏保的id
        QVariant variant = ui->programCombox->itemData(index);
        SSThemeInfo info = variant.value<SSThemeInfo>();
        QByteArray ba = info.id.toLatin1();
        strv = g_strsplit(ba.data(), "%%%", 1);
        //qDebug() << Q_FUNC_INFO << "wxy-----------" <<strv;
        g_settings_set_strv(screensaver_settings, THEMES_KEY, (const gchar * const*)strv);
    }
    g_object_unref(screensaver_settings);
    g_strfreev(strv);
    g_settings_sync(); //同步gsetting
    //刷新预览
    startupScreensaver();
}

void Screensaver::kill_screensaver_preview()
{

}

SSThemeInfo Screensaver::_newThemeinfo(const char * path)
{
    SSThemeInfo info;
    GKeyFile * keyfile;
    char * name, * exec;

    keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, path, G_KEY_FILE_NONE, NULL)) {
        g_key_file_free (keyfile);
        return info;
    }

    name = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
    exec = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TRY_EXEC, NULL);


    info.name = QString::fromUtf8(name);
    info.exec = QString::fromUtf8(exec);
    info.id = ID_PREFIX + info.name.toLower();

    g_key_file_free(keyfile);

    return info;
}

void Screensaver::_acquireThemeinfoList()
{
    GDir * dir;
    const char * name;

    infoMap.clear();

    dir = g_dir_open(SSTHEMEPATH, 0, NULL);
    if (!dir)
        return;
    while ((name = g_dir_read_name(dir))) {
        SSThemeInfo info;
        char * desktopfilepath;
        if (!g_str_has_suffix(name, ".desktop"))
            continue;
        desktopfilepath = g_build_filename(SSTHEMEPATH, name, NULL);
        info = _newThemeinfo(desktopfilepath);
        infoMap.insert(info.id, info);
        g_free (desktopfilepath);
    }
    g_dir_close(dir);
}

void Screensaver::connectToServer()
{
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker,&MThread::keychangedsignal,this,&Screensaver::keyChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
     NetThread->start();
}

void Screensaver::keyChangedSlot(const QString &key) {
    if(key == "ukui-screensaver") {
        initThemeStatus();
    }
}

void Screensaver::showCustomizeFrame() {
    ui->customizeFrame->setVisible(true);
}

void Screensaver::hideCustomizeFrame() {
    ui->customizeFrame->setVisible(false);
}

void Screensaver::initCustomizeFrame() {
    ui->customizeFrame->setFrameShape(QFrame::NoFrame);
    ui->customizeFrame->setFixedHeight(306);
    ui->customizeFrame->adjustSize();
    ui->customizeLayout->setMargin(0);
    initScreensaverSourceFrame();
    initPictureSwitchFrame();
    initTimeSetFrame();
    initShowTextFrame();
    initShowTextSetFrame();
    initShowtimeFrame();
}

void Screensaver::initScreensaverSourceFrame()
{
    QFrame *screensaverSourceFrame = new QFrame();
    QHBoxLayout *sourceLayout = new QHBoxLayout();
    FixLabel *sourceLabel = new FixLabel();
    sourcePathLine = new QLineEdit();
    QPushButton *sourceBtn = new QPushButton();
    screensaverSourceFrame->setFixedHeight(60);
    screensaverSourceFrame->setLayout(sourceLayout);
    sourceLayout->setContentsMargins(16,0,16,0);
    sourceLayout->addWidget(sourceLabel);
    sourceLayout->addWidget(sourcePathLine);
    sourceLayout->addWidget(sourceBtn);
    sourceLabel->setText(tr("Screensaver source"));
    sourceLabel->setFixedWidth(220);
    sourcePathLine->setFixedHeight(36);
    sourcePathLine->setMinimumWidth(252);
    sourcePathLine->setReadOnly(true);
    sourceBtn->setFixedSize(80,36);
    sourceBtn->setText(tr("Select"));
    sourceBtn->raise();
    if (qScreensaverDefaultSetting != nullptr && \
        qScreensaverDefaultSetting->keys().contains(CONTAIN_BACKGROUND_PATH_KEY, Qt::CaseSensitive)) {   //存在【设置路径】
        sourcePathText = qScreensaverDefaultSetting->get(BACKGROUND_PATH_KEY).toString();
        setSourcePathText();
        connect(sourceBtn,&QPushButton::clicked, this, [=]{
            QString filter;
            filter = tr("Wallpaper files(*.jpg *.jpeg *.bmp *.dib *.png *.jfif *.jpe *.gif *.tif *.tiff *.wdp *.svg)");
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

            fd.setDirectory(QString(qScreensaverDefaultSetting->get(BACKGROUND_PATH_KEY).toString()));
            fd.setAcceptMode(QFileDialog::AcceptOpen);
            fd.setViewMode(QFileDialog::List);
            fd.setNameFilter(filter);
            fd.setFileMode(QFileDialog::Directory);
            fd.setWindowTitle(tr("select custom screensaver dir"));
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
            if(selectedfile != "") {  //非点击【取消】时
                sourcePathText = selectedfile;
                setSourcePathText();
                qScreensaverDefaultSetting->set(BACKGROUND_PATH_KEY, selectedfile);
            }
        });
    } else {  //不存在【设置路径】
        sourceBtn->setEnabled(false);
    }

    ui->customizeLayout->addWidget(screensaverSourceFrame);
    HLineFrame *line = new HLineFrame();
    ui->customizeLayout->addWidget(line);
}

void Screensaver::initTimeSetFrame() {
    QFrame *timeSetFrame = new QFrame();
    QHBoxLayout *timeSetLayout = new QHBoxLayout();
    FixLabel *timeSetLabel = new FixLabel();
    QComboBox *timeCom = new QComboBox();
    timeSetFrame->setFixedHeight(60);
    timeSetFrame->setLayout(timeSetLayout);
    timeSetLayout->setContentsMargins(16,0,16,0);
    timeSetLayout->addWidget(timeSetLabel);
    timeSetLayout->addWidget(timeCom);
    timeSetLabel->setText(tr("Switching time"));
    timeSetLabel->setFixedWidth(220);
    timeCom->setFixedHeight(36);
    timeCom->setMinimumWidth(340);
    timeCom->addItem(tr("1min"));
    timeCom->addItem(tr("5min"));
    timeCom->addItem(tr("10min"));
    timeCom->addItem(tr("30min"));
    if (qScreensaverDefaultSetting != nullptr && \
        qScreensaverDefaultSetting->keys().contains(CONTAIN_CYCLE_TIME_KEY, Qt::CaseSensitive)) { //存在【间隔时间设置】
        int timeComIndex = qScreensaverDefaultSetting->get(CYCLE_TIME_KEY).toInt();
        if (timeComIndex == 60) {
            timeCom->setCurrentIndex(0);
        } else if (timeComIndex == 300) {
            timeCom->setCurrentIndex(1);
        } else if (timeComIndex == 600) {
            timeCom->setCurrentIndex(2);
        } else if (timeComIndex == 1800) {
            timeCom->setCurrentIndex(3);
        }
        connect(timeCom,QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](){
            if (timeCom->currentIndex() == 0) {
                qScreensaverDefaultSetting->set(CYCLE_TIME_KEY,60);
            } else if (timeCom->currentIndex() == 1) {
                qScreensaverDefaultSetting->set(CYCLE_TIME_KEY,300);
            } else if (timeCom->currentIndex() == 2) {
                qScreensaverDefaultSetting->set(CYCLE_TIME_KEY,600);
            } else if (timeCom->currentIndex() == 3) {
                qScreensaverDefaultSetting->set(CYCLE_TIME_KEY,1800);
            }
        });
    }
    ui->customizeLayout->addWidget(timeSetFrame);
    HLineFrame *line = new HLineFrame();
    ui->customizeLayout->addWidget(line);
}



void Screensaver::initPictureSwitchFrame() {
    QFrame *pictureSwitchFrame = new QFrame();
    QHBoxLayout *randomLayout = new QHBoxLayout();
    FixLabel *randomLabel = new FixLabel();
    SwitchButton *randomBtn = new SwitchButton();
    QRadioButton *radioOrder = new QRadioButton();
    QRadioButton *radioRand = new QRadioButton();
    QButtonGroup *modeBtn   = new QButtonGroup();
    modeBtn->addButton(radioOrder, 0);
    modeBtn->addButton(radioRand, 1);

    radioOrder->setFixedWidth(200);
    radioRand->setFixedWidth(100);
    radioOrder->setText(tr("Ordinal"));
    radioRand->setText(tr("Random"));

    pictureSwitchFrame->setFixedHeight(60);
    pictureSwitchFrame->setLayout(randomLayout);
    randomLayout->setContentsMargins(16,0,16,0);
    randomLayout->addWidget(randomLabel);
    randomLayout->addWidget(radioOrder);
    randomLayout->addWidget(radioRand);
    randomLabel->setText(tr("Random switching"));
    randomLabel->setFixedWidth(220);
    randomLayout->addStretch();

    if (qScreensaverDefaultSetting != nullptr && \
        qScreensaverDefaultSetting->keys().contains(CONTAIN_AUTO_SWITCH_KEY, Qt::CaseSensitive)) { //存在【随机切换】
        randomBtn->setChecked(qScreensaverDefaultSetting->get(AUTO_SWITCH_KEY).toBool());
        if (qScreensaverDefaultSetting->get(AUTO_SWITCH_KEY).toBool()) {
             radioRand->setChecked(true);
             radioOrder->setChecked(false);
        } else {
             radioRand->setChecked(false);
             radioOrder->setChecked(true);
        }
        connect(modeBtn, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [=](int id){
            if (id == 0) {
                qScreensaverDefaultSetting->set(AUTO_SWITCH_KEY, false);
            } else {
                qScreensaverDefaultSetting->set(AUTO_SWITCH_KEY, true);
            }
        });
    } else { //不存在【随机切换】
        radioRand->setChecked(true);
        radioOrder->setChecked(false);
    }
    ui->customizeLayout->addWidget(pictureSwitchFrame);
    HLineFrame *line = new HLineFrame();
    ui->customizeLayout->addWidget(line);
}

void Screensaver::initShowTextFrame() {
    QFrame *showTextFrame = new QFrame();
    QHBoxLayout *showTextLayout = new QHBoxLayout();
    FixLabel *showLabel = new FixLabel();
    inputText = new QLineEdit(); //用户输入文字
    showTextFrame->setFixedHeight(60);
    showTextFrame->setLayout(showTextLayout);
    showTextLayout->setContentsMargins(16,6,15,8);
    showTextLayout->addWidget(showLabel);
    showTextLayout->addWidget(inputText);

    showLabel->setText(tr("Text(up to 30 characters):"));
    showLabel->setFixedWidth(220);
    if (qScreensaverDefaultSetting != nullptr && \
        qScreensaverDefaultSetting->keys().contains(CONTAIN_MYTEXT_KEY, Qt::CaseSensitive)) { //存在【文本设置】
        inputText->setMaxLength(30);
        inputText->setText(qScreensaverDefaultSetting->get(MYTEXT_KEY).toString());     //初始化文本
        connect(inputText,&QLineEdit::textChanged, this, [=]{
            qScreensaverDefaultSetting->set(MYTEXT_KEY, inputText->text());
        });
    } else {//不存在【文本设置】
        inputText->setEnabled(false);
    }
    ui->customizeLayout->addWidget(showTextFrame);
    HLineFrame *line = new HLineFrame();
    ui->customizeLayout->addWidget(line);
}

void Screensaver::initShowtimeFrame()
{
    showTimeFrame = new QFrame();
    QHBoxLayout *showTimeLayout = new QHBoxLayout(showTimeFrame);
    FixLabel *showTimeLabel = new FixLabel();

    showTimeFrame->setFixedHeight(60);
    showTimeLayout->setContentsMargins(16,0,16,0);
    showTimeLayout->addWidget(showTimeLabel);
    //添加休息时间显示按钮
    showCustomTimeBtn = new SwitchButton(showTimeFrame);
    showUkuiTimeBtn   = new SwitchButton(showTimeFrame);
    showTimeLayout->addStretch();
    showTimeLayout->addWidget(showCustomTimeBtn);
    showTimeLayout->addWidget(showUkuiTimeBtn);
    showTimeLabel->setFixedWidth(220);

    showTimeLabel->setText(tr("Show rest time"));
    ui->verticalLayout_4->addWidget(showTimeFrame);
}

void Screensaver::initShowTextSetFrame() {
    QFrame *showTextSetFrame = new QFrame();
    QHBoxLayout *textSetLayout = new QHBoxLayout();
    FixLabel *textSetLabel = new FixLabel();
    QButtonGroup *groupBtn = new QButtonGroup();
    QRadioButton *radioRandom = new QRadioButton(this);
    QRadioButton *radioCenter = new QRadioButton(this);

    groupBtn->addButton(radioRandom, 0);
    groupBtn->addButton(radioCenter, 1);

    showTextSetFrame->setFixedHeight(60);
    showTextSetFrame->setLayout(textSetLayout);
    textSetLayout->addWidget(textSetLabel);
    textSetLayout->setContentsMargins(16,0,16,0);
    textSetLabel->setText(tr("Text position"));
    textSetLabel->setFixedWidth(220);
    radioRandom->setFixedWidth(200);
    radioCenter->setFixedWidth(100);

    textSetLayout->addWidget(radioRandom);
    textSetLayout->addWidget(radioCenter);
    textSetLayout->addStretch();
    radioCenter->setText(tr("Centered"));
    radioRandom->setText(tr("Randow(Bubble text)"));

    if (qScreensaverDefaultSetting != nullptr && \
        qScreensaverDefaultSetting->keys().contains(CONTAIN_TEXT_CENTER_KEY, Qt::CaseSensitive)) { //存在【文本设置】
        if (qScreensaverDefaultSetting->get(TEXT_CENTER_KEY).toBool() == true) {
            radioCenter->setChecked(true);
        } else {
            radioRandom->setChecked(true);
        }

        connect(groupBtn, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [=](int id){
            if (id == 0) {
                qScreensaverDefaultSetting->set(TEXT_CENTER_KEY, false);
            } else if (id == 1) {
                qScreensaverDefaultSetting->set(TEXT_CENTER_KEY, true);
            }
        });
    } else { //不存在【文本设置】
        radioRandom->setEnabled(false);
        radioCenter->setEnabled(false);
    }

    ui->customizeLayout->addWidget(showTextSetFrame);
    HLineFrame *line = new HLineFrame();
    ui->customizeLayout->addWidget(line);
}

void Screensaver::setSourcePathText()
{
    if (sourcePathLine == nullptr)
        return;

    QFontMetrics fontMetrics(sourcePathLine->font());
    int fontSize = fontMetrics.width(sourcePathText);
    if (fontSize > sourcePathLine->width()) {
       sourcePathLine->setText(fontMetrics.elidedText(sourcePathText, Qt::ElideRight, sourcePathLine->width()));
       sourcePathLine->setToolTip(sourcePathText);
    } else {
       sourcePathLine->setText(sourcePathText);
       sourcePathLine->setToolTip("");
    }
}

PreviewWidget::PreviewWidget(QWidget *parent):QWidget(parent)
{

}

PreviewWidget::~PreviewWidget()
{

}

void PreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawRect(rect());
}

void PreviewWidget::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton) {
        PreviewWindow::previewScreensaver();
    }
}

bool Screensaver::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QMouseEvent::Enter && watched == previewWind) {
        initPreviewLabel();
    } else if(event->type() == QMouseEvent::Leave) {
        QWidget *widParent = static_cast<QWidget*>(ui->previewWidget->parent());
        QPoint  globalPoint   = widParent->mapFromGlobal(this->cursor().pos());//鼠标位置
        if (!ui->previewWidget->geometry().contains(globalPoint) && previewLabel != nullptr) {
            previewLabel->setVisible(false);
        }
    }

    if (event->type() == QEvent::Resize && watched == sourcePathLine) {
        setSourcePathText();
    }
    return QObject::eventFilter(watched, event);
}

PreviewWindow::PreviewWindow()
{

}
PreviewWindow::~PreviewWindow()
{

}

void PreviewWindow::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton) {
        previewScreensaver();
    }
}

void PreviewWindow::previewScreensaver()
{
    static QProcess *viewProcess = new QProcess();
    viewProcess->start("ukui-screensaver-command --screensaver");
}

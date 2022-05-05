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
#include "autoboot.h"
#include <ukcc/widgets/switchbutton.h>
#include <ukcc/widgets/hoverwidget.h>
#include <ukcc/widgets/imageutil.h>
#include "shell/utils/mthread.h"

#include <QThread>
#include <QSignalMapper>
#include <QDebug>
#include <QFont>
#include <QMouseEvent>
#include <QPushButton>
#include <QGSettings>
#include <QToolButton>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include "rmenu.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#define ITEMWIDTH 522
#define ITEMHEIGHT 62
#define HEADHEIGHT 38

#define THEME_QT_SCHEMA  "org.ukui.style"
#define THEME_GTK_SCHEMA "org.mate.interface"

#define ICON_QT_KEY      "icon-theme-name"
#define ICON_GTK_KEY     "icon-theme"

#define LOCAL_CONFIG_DIR           "/.config/autostart/"
#define SYSTEM_CONFIG_DIR          "/etc/xdg/autostart/"
#define USR_CONFIG_DIR             "/usr/share/applications/"

class ukFileDialog : public QFileDialog
{
public:
    explicit ukFileDialog(QWidget *parent = nullptr,
                         const QString &caption = QString(),
                         const QString &directory = QString(),
                         const QString &filter = QString())
        : QFileDialog(parent, caption, directory, filter)
    {
    }
protected:
   void accept() override;

};

AutoBoot::AutoBoot() : mFirstLoad(true)
{
    pluginName = tr("Auto Boot");
    pluginType = APPLICATION;
}

AutoBoot::~AutoBoot()
{
    if (!mFirstLoad) {
    }
}

QString AutoBoot::plugini18nName()
{
    return pluginName;
}

int AutoBoot::pluginTypes()
{
    return pluginType;
}

QWidget *AutoBoot::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;

        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);

        whitelist.append("sogouImeService.desktop");
        whitelist.append("kylin-weather.desktop");
        initConfig();
        connectToServer();
        initUI(pluginWidget);
        initStyle();
        setupGSettings();
        initConnection();
    }
    return pluginWidget;
}

const QString AutoBoot::name() const
{
    return QStringLiteral("Autoboot");
}

bool AutoBoot::isShowOnHomePage() const
{
    return true;
}

QIcon AutoBoot::icon() const
{
    return QIcon();
}

bool AutoBoot::isEnable() const
{
    return true;
}

/* 初始化整体UI布局 */
void AutoBoot::initUI(QWidget *widget)
{
    QVBoxLayout *mverticalLayout = new QVBoxLayout(widget);
    mverticalLayout->setSpacing(0);
    mverticalLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *AutobootWidget = new QWidget(widget);
    AutobootWidget->setMinimumSize(QSize(550, 0));
    AutobootWidget->setMaximumSize(QSize(16777215, 16777215));

    QVBoxLayout *AutobootLayout = new QVBoxLayout(AutobootWidget);
    AutobootLayout->setContentsMargins(0, 0, 0, 0);
    AutobootLayout->setSpacing(0);

    mTitleLabel = new TitleLabel(AutobootWidget);

    mAutoBootFrame = new QFrame(AutobootWidget);
    mAutoBootFrame->setMinimumSize(QSize(550, 0));
    mAutoBootFrame->setMaximumSize(QSize(16777215, 16777215));
    mAutoBootFrame->setFrameShape(QFrame::Box);

    mAutoBootLayout = new QVBoxLayout(mAutoBootFrame);
    mAutoBootLayout->setContentsMargins(0, 0, 0, 0);
    mAutoBootLayout->setSpacing(0);

    initAddBtn();

    AutobootLayout->addWidget(mTitleLabel);
    AutobootLayout->addSpacing(8);
    AutobootLayout->addWidget(mAutoBootFrame);
    AutobootLayout->addWidget(addWgt);

    mverticalLayout->addWidget(AutobootWidget);
    mverticalLayout->addStretch();

    initAutoUI();

}


void AutoBoot::initAutoUI()
{
    initStatus();
    appgroupMultiMaps.clear();
    QSignalMapper *checkSignalMapper = new QSignalMapper(this);
    // 构建每个启动项
    QMap<QString, AutoApp>::iterator it = statusMaps.begin();
    for (int index = 0; it != statusMaps.end(); it++, index++) {
        QString bname = it.value().bname;
        QString appName = it.value().name;

        QFrame *baseWidget = new QFrame(pluginWidget);
        baseWidget->setMinimumWidth(550);
        baseWidget->setMaximumWidth(16777215);
        baseWidget->setFrameShape(QFrame::NoFrame);
        baseWidget->setAttribute(Qt::WA_DeleteOnClose);

        QVBoxLayout *baseVerLayout = new QVBoxLayout(baseWidget);
        baseVerLayout->setSpacing(0);
        baseVerLayout->setContentsMargins(0, 0, 0, 2);

        HoverWidget *widget = new HoverWidget(bname);
        widget->setMinimumWidth(550);
        widget->setMaximumWidth(16777215);

        widget->setMinimumHeight(60);
        widget->setMaximumHeight(60);

        widget->setAttribute(Qt::WA_DeleteOnClose);

        QHBoxLayout *mainHLayout = new QHBoxLayout(widget);
        mainHLayout->setContentsMargins(16, 0, 16, 0);
        mainHLayout->setSpacing(16);

        QLabel *iconLabel = new QLabel(widget);
        iconLabel->setFixedSize(32, 32);
        iconLabel->setPixmap(it.value().pixmap);

        QLabel *textLabel = new QLabel(widget);
        textLabel->setFixedWidth(500);
        textLabel->setText(appName);

        SwitchButton *button = new SwitchButton(widget);
        button->setAttribute(Qt::WA_DeleteOnClose);
        button->setChecked(!it.value().hidden);
        checkSignalMapper->setMapping(button, it.key());
        connect(button, SIGNAL(checkedChanged(bool)), checkSignalMapper, SLOT(map()));
        appgroupMultiMaps.insert(it.key(), button);

        QToolButton *deBtn = new QToolButton(widget);
        deBtn->setStyleSheet("QToolButton:!checked{background-color: palette(base)}");
        deBtn->setProperty("useButtonPalette", true);
        deBtn->setPopupMode(QToolButton::InstantPopup);
        deBtn->setFixedSize(QSize(36, 36));
        deBtn->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));

        RMenu *pMenu = new RMenu(deBtn);

        deBtn->setMenu(pMenu);
        QAction* mDel = new QAction(tr("Delete"),this);
        pMenu->addAction(mDel);
        connect(mDel, &QAction::triggered, this, [=](){
            QMap<QString, AutoApp>::iterator it = statusMaps.find(bname);
            if (it == statusMaps.end()) {
                qDebug() << "AutoBoot Data Error";
                return;
            }
            deleteLocalAutoapp(bname);
            baseWidget->close();
        });

        mainHLayout->addWidget(iconLabel);
        mainHLayout->addWidget(textLabel);
        mainHLayout->addStretch();
        if (it.value().xdg_position == LOCALPOS) {
            mainHLayout->addWidget(deBtn);
        } else {
            deBtn->hide();
        }

        mainHLayout->addWidget(button);
        widget->setLayout(mainHLayout);

        QFrame *line = new QFrame(pluginWidget);
        line->setMinimumSize(QSize(0, 1));
        line->setMaximumSize(QSize(16777215, 1));
        line->setLineWidth(0);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        baseVerLayout->addWidget(widget);

        baseVerLayout->addWidget(line);

        baseWidget->setLayout(baseVerLayout);

        mAutoBootLayout->addWidget(baseWidget);
    }
    connect(checkSignalMapper, SIGNAL(mapped(QString)), this, SLOT(checkbox_changed_cb(QString)));
}

void AutoBoot::setupGSettings()
{
    const QByteArray id(THEME_QT_SCHEMA);
    mQtSettings = new QGSettings(id, QByteArray(), this);
}

AutoApp AutoBoot::setInformation(QString filepath)
{
    AutoApp app;
    QSettings* desktopFile = new QSettings(filepath, QSettings::IniFormat);
    QString icon, only_showin, not_show_in;
    if (desktopFile) {
       desktopFile->setIniCodec("utf-8");

       QFileInfo file = QFileInfo(filepath);
       app.bname = file.fileName();
       app.path = filepath;
       app.exec = desktopFile->value(QString("Desktop Entry/Exec")).toString();
       icon = desktopFile->value(QString("Desktop Entry/Icon")).toString();
       app.hidden = desktopFile->value(QString("Desktop Entry/Hidden")).toBool();
       app.no_display = desktopFile->value(QString("Desktop Entry/NoDisplay")).toBool();
       only_showin = desktopFile->value(QString("Desktop Entry/OnlyShowIn")).toString();
       not_show_in = desktopFile->value(QString("Desktop Entry/NotShowIn")).toString();
       bool mshow = true;
       if (app.bname == "sogouImeService.desktop") {
           icon = "/opt/sogouimebs/files/share/resources/skin/logo/logo.png";
       }
       if (only_showin != nullptr) {
           if (!only_showin.contains("UKUI")) {
               mshow = false;
           }
       }
       if (not_show_in != nullptr) {
           if (not_show_in.contains("UKUI")) {
               mshow = false;
           }
       }
       app.shown = mshow;

       QFileInfo iconfile(icon);

       if (!QString(icon).isEmpty()) {
           QIcon currenticon
               = QIcon::fromTheme(icon,
                                  QIcon(QString("/usr/share/pixmaps/"+icon
                                                +".png")));
           app.pixmap = currenticon.pixmap(QSize(32, 32));
       } else if (iconfile.exists()) {
           app.pixmap = QPixmap(iconfile.filePath()).scaled(32, 32);
       } else {
           app.pixmap = QPixmap(QString(":/img/plugins/autoboot/desktop.png"));
       }

       delete desktopFile;
       desktopFile = nullptr;
    }
    //通过glib库函数获取Name字段，防止特殊情况（含有字段X-Ubuntu-Gettext-Domain）
    GKeyFile *keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, filepath.toLatin1().data(), G_KEY_FILE_NONE, NULL)) {
        g_key_file_free(keyfile);
        return app;
    }
    app.name = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                        G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
    g_key_file_free(keyfile);

    return app;

}

bool AutoBoot::copyFileToLocal(QString bname)
{
    QString srcPath;
    QString dstPath;

    QMap<QString, AutoApp>::iterator it = appMaps.find(bname);

    srcPath = it.value().path;
    dstPath = QDir::homePath()+LOCAL_CONFIG_DIR+bname;

    if (!QFile::copy(srcPath, dstPath))
        return false;

    //将复制的文件权限改为可读写
    QFile(dstPath).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    // 更新数据，将新加入该目录下的应用信息读取，加入到localappMaps，更新statusMaps对应应用的信息
    AutoApp addapp;
    addapp = setInformation(dstPath);
    addapp.xdg_position = ALLPOS;

    localappMaps.insert(addapp.bname, addapp);

    QMap<QString, AutoApp>::iterator updateit = statusMaps.find(bname);
    updateit.value().xdg_position = ALLPOS;
    updateit.value().path = dstPath;

    return true;
}

bool AutoBoot::deleteLocalAutoapp(QString bname)
{
    QString dstpath = QDir::homePath()+LOCAL_CONFIG_DIR+bname;

    if (dstpath.isEmpty() || !QDir().exists(dstpath))//是否传入了空的路径||路径是否存在
            return false;

    // 更新数据
    localappMaps.remove(bname);
    QMap<QString, AutoApp>::iterator updateit = statusMaps.find(bname);
    if (updateit == statusMaps.end())
        qDebug() << "statusMaps Data Error when delete local file";
    else {
        if (updateit.value().xdg_position == LOCALPOS) {
            statusMaps.remove(bname);
        } else if (updateit.value().xdg_position == ALLPOS) {
            //当系统应用的自启动开关打开时，将其属性变为SYSTEMPOS，将其从~/.config/autostart目录下删除
            QMap<QString, AutoApp>::iterator appit = appMaps.find(bname);
            if (appit == appMaps.end())
                qDebug() << "appMaps Data Error when delete local file";
            else {
                updateit.value().hidden = appit.value().hidden;
                updateit.value().path = appit.value().path;
            }
            updateit.value().xdg_position = SYSTEMPOS;
        }
    }

    QFile::remove(dstpath);
    return true;
}

bool AutoBoot::setAutoAppStatus(QString bname, bool status)
{
    QString dstpath = QDir::homePath()+LOCAL_CONFIG_DIR+bname;
    //修改hidden字段
    GKeyFile *keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, dstpath.toUtf8().data(), G_KEY_FILE_NONE, NULL)) {
        g_key_file_free(keyfile);

        return false;
    }
    g_key_file_set_boolean(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                        G_KEY_FILE_DESKTOP_KEY_HIDDEN, !status);
    if (!_key_file_to_file(keyfile, dstpath.toUtf8().data())) {
        qDebug() << "Stop autoboot failed because could not save desktop file";
        g_free(dstpath.toUtf8().data());
        return false;
    }

    g_key_file_free(keyfile);
   // 更新数据
   QMap<QString, AutoApp>::iterator updateit = statusMaps.find(bname);
   if (updateit == statusMaps.end()) {
       qDebug() << "Start autoboot failed because autoBoot Data Error";

   }
   else {
       updateit.value().hidden = !status;
   }
    return true;
}

void AutoBoot::clearAutoItem()
{
    if (mAutoBootLayout->layout() != NULL) {
        QLayoutItem *item;
        while ((item = mAutoBootLayout->layout()->takeAt(0)) != NULL)
        {
            if(item->widget()) {
               item->widget()->setParent(NULL);
            }
            delete item;
            item = nullptr;
        }
    }
}

void AutoBoot::open_desktop_dir_slots()
{
    QString filters = tr("Desktop files(*.desktop)");
    ukFileDialog *fd = new ukFileDialog(pluginWidget);
    fd->setDirectory(USR_CONFIG_DIR);
    fd->setModal(true);
    fd->setAcceptMode(QFileDialog::AcceptOpen);
    fd->setViewMode(QFileDialog::List);
    fd->setNameFilter(filters);
    fd->setFileMode(QFileDialog::ExistingFile);
    fd->setWindowTitle(tr("select autoboot desktop"));
    fd->setLabelText(QFileDialog::Accept, tr("Select"));
    fd->setLabelText(QFileDialog::Reject, tr("Cancel"));
    if (fd->exec() != QDialog::Accepted)
        return;

    QString selectedfile;
    selectedfile = fd->selectedFiles().first();

    QByteArray ba;
    ba = selectedfile.toUtf8();
    // 解析desktop文件
    GKeyFile *keyfile;
    char *name, *comment, *mname, *exec, *icon;

    keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, ba.data(), G_KEY_FILE_NONE, NULL)) {
        g_key_file_free(keyfile);
        return;
    }
    name = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                        G_KEY_FILE_DESKTOP_KEY_NAME, NULL);
    mname = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                        G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
    comment = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                           G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL, NULL);
    exec = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                         G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);
    icon = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                         G_KEY_FILE_DESKTOP_KEY_ICON, NULL);

    g_key_file_free(keyfile);
    emit autoboot_adding_signals(selectedfile, QString(mname), QString(exec),
                                 QString(comment), QString(icon));
}

gboolean AutoBoot::_key_file_to_file(GKeyFile *keyfile, const gchar *path)
{
    GError *werror;
    gchar *data;
    gsize length;
    gboolean res;

    werror = NULL;
    data = g_key_file_to_data(keyfile, &length, &werror);

    if (werror)
        return FALSE;

    res = g_file_set_contents(path, data, length, &werror);
    g_free(data);

    if (werror)
        return FALSE;

    return res;
}

void AutoBoot::checkbox_changed_cb(QString bname)
{
    foreach (QString key, appgroupMultiMaps.keys()) {
        if (key == bname) {
            QMap<QString, AutoApp>::iterator it = statusMaps.find(bname);
            if (it == statusMaps.end()) {
                qDebug() << "AutoBoot Data Error";
                return;
            }

            if (((SwitchButton *)appgroupMultiMaps.value(key))->isChecked()) { // 开启开机启动
                if (it.value().xdg_position == SYSTEMPOS) { //
                } else if (it.value().xdg_position == ALLPOS) { // 从~/.config/autostart目录下删除
                    QMap<QString, AutoApp>::iterator appit = appMaps.find(bname);
                    if (!appit.value().hidden) { // 直接删除
                        deleteLocalAutoapp(bname);
                        // 更新状态
                        QMap<QString, AutoApp>::iterator updateit = statusMaps.find(bname);
                        if (updateit != statusMaps.end()) {
                            updateit.value().hidden = false;
                            updateit.value().xdg_position = SYSTEMPOS;
                            //路径改为/etc/xdg/autostart
                            updateit.value().path = appit.value().path;
                        } else
                            qDebug() << "Update status failed when start autoboot";
                    }
                } else if (it.value().xdg_position == LOCALPOS) {// 改值("hidden"字段->false)
                    setAutoAppStatus(bname, true);
                }
            } else { // 关闭
                if (it.value().xdg_position == SYSTEMPOS) { // 复制后改值，将对应应用的desktop文件从/etc/xdg/autostart目录下复制到~/.config/autostart目录下
                    if (copyFileToLocal(bname)) {
                        //更新数据（"hidden"字段->true）
                         setAutoAppStatus(bname, false);
                    }
                } else if (it.value().xdg_position == ALLPOS) {// 正常逻辑不应存在该情况,预防处理
                    QMap<QString, AutoApp>::iterator appit = appMaps.find(bname);
                    if (appit.value().hidden)
                        deleteLocalAutoapp(bname);
                } else if (it.value().xdg_position == LOCALPOS) {// 改值
                    //更新数据（"hidden"字段->true）
                   setAutoAppStatus(bname, false);
                }
            }
        }
    }
}

void AutoBoot::keyChangedSlot(const QString &key)
{
    if (key == "boot") {
        clearAutoItem();
        initAutoUI();
    }
}

void AutoBoot::add_autoboot_realize_slot(QString path, QString name, QString exec, QString comment, QString icon)
{
    if (exec.contains("kylin-screenshot")) {
        QStringList screenshotExec = exec.split(" ");
        exec = screenshotExec.at(0);
    }
    if (path.isEmpty())
        return;

    // 判断是否有重复项，重复则不加入对应列表
    QMap<QString, AutoApp>::iterator it = statusMaps.begin();
    for (int index = 0; it != statusMaps.end(); it++, index++) {
        if (it.value().name == name) {
            return;
        }
    }

    QFileInfo file = QFileInfo(path);
    QString mFileName = file.fileName();
    QString filepath = QDir::homePath()+LOCAL_CONFIG_DIR+mFileName;
    if(!QFile::copy(path,filepath))
        return;
    clearAutoItem();
    initAutoUI();
}

void AutoBoot::initAddBtn()
{
    addWgt = new AddBtn(pluginWidget);
    //~ contents_path /autoboot/Add
    tr("Add");       // 用于添加搜索索引
    connect(addWgt, &AddBtn::clicked, this, &AutoBoot::open_desktop_dir_slots);
}

void AutoBoot::initStyle()
{
    //~ contents_path /autoboot/Autoboot Settings
    mTitleLabel->setText(tr("Autoboot Settings"));
}



void AutoBoot::initStatus()
{
    QDir localdir(QString(QDir::homePath()+LOCAL_CONFIG_DIR).toUtf8());
    QDir systemdir(QString(SYSTEM_CONFIG_DIR).toUtf8());
    QDir usrdir(QString(USR_CONFIG_DIR).toUtf8());

    QStringList filters;
    filters<<QString("*.desktop");
    localdir.setFilter(QDir::Files | QDir::NoSymLinks); // 设置类型过滤器，只为文件格式
    systemdir.setFilter(QDir::Files | QDir::NoSymLinks);

    localdir.setNameFilters(filters);  // 设置文件名称过滤器，只为filters格式
    systemdir.setNameFilters(filters);

    //将系统目录下的应用加入appMaps
    appMaps.clear();

    for( QString file_name : whitelist) {
        AutoApp app;
        app = setInformation(SYSTEM_CONFIG_DIR+file_name);
        if (app.name.isEmpty())
            continue;
        app.xdg_position = SYSTEMPOS;
        appMaps.insert(app.bname, app);
    }
//    for (int i = 0; i < systemdir.count(); i++) {
//        QString file_name = systemdir[i];  // 文件名称
//        AutoApp app;
//        app = setInformation(SYSTEM_CONFIG_DIR+file_name);
//        app.xdg_position = SYSTEMPOS;
//        appMaps.insert(app.bname, app);
//    }

    //将本地配置目录下的应用加入localappMaps
    localappMaps.clear();
    QStringList usrlist;
    for(uint i = 0 ; i < usrdir.count() ; i++)
        usrlist.append(usrdir[i]);
    for (uint i = 0; i < localdir.count(); i++) {
        QString file_name = localdir[i];  // 文件名称
        if (!usrlist.contains(file_name) && !whitelist.contains(file_name)) { //过滤掉不存在于白名单和/usr/share/applications/目录下的应用，解决bug#101357
            QFile::remove(QString(QDir::homePath() + LOCAL_CONFIG_DIR + file_name));
            continue;
        }
        AutoApp app;
        app = setInformation(QDir::homePath()+LOCAL_CONFIG_DIR+file_name);
        app.xdg_position = LOCALPOS;
        localappMaps.insert(app.bname, app);
    }

    //将localappMaps和appMaps中的应用加入statusMaps
    statusMaps.clear();

    QMap<QString, AutoApp>::iterator it = appMaps.begin();
    for (; it != appMaps.end(); it++) {
        statusMaps.insert(it.key(), it.value());
    }

    QMap<QString, AutoApp>::iterator localit = localappMaps.begin();
    for (; localit != localappMaps.end(); localit++) {
//        if (/*localit.value().hidden || */ localit.value().no_display || !localit.value().shown) {
//            statusMaps.remove(localit.key());
//            continue;
//        }

        if (statusMaps.contains(localit.key())) {
            // 整合状态
            QMap<QString, AutoApp>::iterator updateit = statusMaps.find(localit.key());

            if (localit.value().hidden != updateit.value().hidden) {
                //将statusMaps中的应用状态与localappMaps中对应的应用状态同步
                updateit.value().hidden = localit.value().hidden;
                updateit.value().path = localit.value().path;
                //appMaps中hidden属性为false的应用进入下面这个if语句
                if (appMaps.contains(localit.key())) {
                    //ALLPOS代表系统目录下没被过滤掉的应用，自启动按钮关闭（hidden = false）
                    updateit.value().xdg_position = ALLPOS;
                }
            }
        } else {
            statusMaps.insert(localit.key(), localit.value());
        }
    }
}

void AutoBoot::initConnection()
{
    connect(mQtSettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "iconThemeName") {
            clearAutoItem();
            initAutoUI();
        }
    });

    connect(this, &AutoBoot::autoboot_adding_signals, this, &AutoBoot::add_autoboot_realize_slot);
}

void AutoBoot::connectToServer()
{
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker,&MThread::keychangedsignal,this,&AutoBoot::keyChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
     NetThread->start();
}

bool AutoBoot::initConfig()
{
    QDir localdir(QString(QDir::homePath()+LOCAL_CONFIG_DIR).toUtf8());
    if(localdir.exists()) {
      return true;
    } else {
       return localdir.mkdir(QDir::homePath()+LOCAL_CONFIG_DIR);
    }
}


void ukFileDialog::accept()
{
    QString selectedfile;
    selectedfile = this->selectedFiles().first();

    QByteArray ba;
    ba = selectedfile.toUtf8();

    // 解析desktop文件
    GKeyFile *keyfile;
    bool no_display;

    keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, ba.data(), G_KEY_FILE_NONE, NULL)) {
        g_key_file_free(keyfile);
        return;
    }
    no_display = g_key_file_get_boolean(keyfile, G_KEY_FILE_DESKTOP_GROUP,
                                        G_KEY_FILE_DESKTOP_KEY_NO_DISPLAY, FALSE);

    g_key_file_free(keyfile);

    if (no_display) {
        QMessageBox msg(qApp->activeWindow());
        msg.setIcon(QMessageBox::Warning);
        msg.setText(QObject::tr("Programs are not allowed to be added."));
        msg.exec();
    } else {
        QFileDialog::accept();
    }
}

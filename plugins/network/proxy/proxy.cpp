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
#include "proxy.h"
#include "ui_proxy.h"
#include "aptinfo.h"

#include <QDebug>

#define PROXY_SCHEMA              "org.gnome.system.proxy"
#define PROXY_MODE_KEY            "mode"
#define PROXY_AUTOCONFIG_URL_KEY  "autoconfig-url"
#define IGNORE_HOSTS_KEY          "ignore-hosts"

#define HTTP_PROXY_SCHEMA         "org.gnome.system.proxy.http"
#define HTTP_USE_AUTH_KEY         "use-authentication"
#define HTTP_AUTH_USER_KEY        "authentication-user"
#define HTTP_AUTH_PASSWD_KEY      "authentication-password"

#define HTTPS_PROXY_SCHEMA        "org.gnome.system.proxy.https"

#define FTP_PROXY_SCHEMA          "org.gnome.system.proxy.ftp"

#define SOCKS_PROXY_SCHEMA        "org.gnome.system.proxy.socks"

#define PROXY_HOST_KEY       "host"
#define PROXY_PORT_KEY       "port"

Proxy::Proxy() : mFirstLoad(true)
{
    ui = new Ui::Proxy;
    pluginName = tr("Proxy");
    pluginType = NETWORK;
}

Proxy::~Proxy()
{
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;

        if (settingsCreate){
            delete proxysettings;
            proxysettings = nullptr;
            delete httpsettings;
            httpsettings = nullptr;
            delete securesettings;
            securesettings = nullptr;
            delete ftpsettings;
            ftpsettings = nullptr;
            delete sockssettings;
            sockssettings = nullptr;
        }
    }
}

QString Proxy::get_plugin_name() {
    return pluginName;
}

int Proxy::get_plugin_type() {
    return pluginType;
}

QWidget *Proxy::get_plugin_ui() {
    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        settingsCreate = false;
        mfileWatch_1 = new QFileSystemWatcher(this);
        mfileWatch_2 = new QFileSystemWatcher(this);

        QString dir_1("/etc/apt/apt.conf.d");
        QString dir_2("/etc/profile.d");
        mfileWatch_1->addPath(dir_1);
        mfileWatch_2->addPath(dir_2);

        const QByteArray id(PROXY_SCHEMA);
        const QByteArray idd(HTTP_PROXY_SCHEMA);
        const QByteArray iddd(HTTPS_PROXY_SCHEMA);
        const QByteArray iid(FTP_PROXY_SCHEMA);
        const QByteArray iiid(SOCKS_PROXY_SCHEMA);

        initTitleLabel();
        initSearchText();
        setupStylesheet();
        setupComponent();

        if (QGSettings::isSchemaInstalled(id) && QGSettings::isSchemaInstalled(idd) &&
                QGSettings::isSchemaInstalled(iddd) && QGSettings::isSchemaInstalled(iid) &&
                QGSettings::isSchemaInstalled(iiid)){

            settingsCreate = true;
            proxysettings = new QGSettings(id);
            httpsettings = new QGSettings(idd);
            securesettings = new QGSettings(iddd);
            ftpsettings = new QGSettings(iid);
            sockssettings = new QGSettings(iiid);

            setupConnect();
            initProxyModeStatus();
            initAutoProxyStatus();
            initManualProxyStatus();
            initIgnoreHostStatus();
        } else {
            qCritical() << "Xml needed by Proxy is not installed";
        }
    }
    return pluginWidget;
}

void Proxy::plugin_delay_control(){

}

const QString Proxy::name() const {

    return QStringLiteral("proxy");
}

void Proxy::initTitleLabel() {

}

void Proxy::initSearchText() {
    //~ contents_path /proxy/Auto proxy
    ui->autoLabel->setText(tr("Auto proxy"));
    //~ contents_path /proxy/Manual proxy
    ui->manualLabel->setText(tr("Manual proxy"));
}

void Proxy::setupStylesheet(){
}

void Proxy::setupComponent(){
    //添加自动配置代理开关按钮
    autoSwitchBtn = new SwitchButton(ui->autoFrame);
    autoSwitchBtn->setObjectName("auto");
    ui->autoHorLayout->addWidget(autoSwitchBtn);

    //添加手动配置代理开关按钮
    manualSwitchBtn = new SwitchButton(ui->manualFrame);
    manualSwitchBtn->setObjectName("manual");
    ui->manualHorLayout->addWidget(manualSwitchBtn);

    ui->cetificationBtn->hide();

    //添加配置Apt代理开关按钮
    mAptBtn = new SwitchButton(ui->mAPTFrame_1);
    ui->mAPTFrame_1->layout()->addWidget(mAptBtn);

    //QLineEdit 设置数据
    GSData httpHostData;
    httpHostData.schema = HTTP_PROXY_SCHEMA;
    httpHostData.key = PROXY_HOST_KEY;
    ui->httpHostLineEdit->setProperty("gData", QVariant::fromValue(httpHostData));

    GSData httpsHostData;
    httpsHostData.schema = HTTPS_PROXY_SCHEMA;
    httpsHostData.key = PROXY_HOST_KEY;
    ui->httpsHostLineEdit->setProperty("gData", QVariant::fromValue(httpsHostData));

    GSData ftpHostData;
    ftpHostData.schema = FTP_PROXY_SCHEMA;
    ftpHostData.key = PROXY_HOST_KEY;
    ui->ftpHostLineEdit->setProperty("gData", QVariant::fromValue(ftpHostData));

    GSData socksHostData;
    socksHostData.schema = SOCKS_PROXY_SCHEMA;
    socksHostData.key = PROXY_HOST_KEY;
    ui->socksHostLineEdit->setProperty("gData", QVariant::fromValue(socksHostData));

    GSData httpPortData;
    httpPortData.schema = HTTP_PROXY_SCHEMA;
    httpPortData.key = PROXY_PORT_KEY;
    ui->httpPortLineEdit->setProperty("gData", QVariant::fromValue(httpPortData));

    GSData httpsPortData;
    httpsPortData.schema = HTTPS_PROXY_SCHEMA;
    httpsPortData.key = PROXY_PORT_KEY;
    ui->httpsPortLineEdit->setProperty("gData", QVariant::fromValue(httpsPortData));

    GSData ftpPortData;
    ftpPortData.schema = FTP_PROXY_SCHEMA;
    ftpPortData.key = PROXY_PORT_KEY;
    ui->ftpPortLineEdit->setProperty("gData", QVariant::fromValue(ftpPortData));

    GSData socksPortData;
    socksPortData.schema = SOCKS_PROXY_SCHEMA;
    socksPortData.key = PROXY_PORT_KEY;
    ui->socksPortLineEdit->setProperty("gData", QVariant::fromValue(socksPortData));
}

void Proxy::setupConnect(){
    connect(autoSwitchBtn, SIGNAL(checkedChanged(bool)), this, SLOT(proxyModeChangedSlot(bool)));
    connect(manualSwitchBtn, SIGNAL(checkedChanged(bool)), this, SLOT(proxyModeChangedSlot(bool)));
    connect(ui->urlLineEdit, &QLineEdit::textChanged, this, [=](const QString &txt){proxysettings->set(PROXY_AUTOCONFIG_URL_KEY, QVariant(txt));});

    connect(ui->httpHostLineEdit, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(ui->httpsHostLineEdit, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(ui->ftpHostLineEdit, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(ui->socksHostLineEdit, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(ui->httpPortLineEdit, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(ui->httpsPortLineEdit, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(ui->ftpPortLineEdit, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(ui->socksPortLineEdit, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});

//    connect(ui->cetificationBtn, &QPushButton::clicked, [=](bool checked){
//        Q_UNUSED(checked)
//        showCertificationDialog();
//    });
    connect(ui->ignoreHostTextEdit, &QTextEdit::textChanged, this, [=](){
        QString text = ui->ignoreHostTextEdit->toPlainText();
        QStringList hostStringList = text.split(";");
        proxysettings->set(IGNORE_HOSTS_KEY, QVariant(hostStringList));
    });

    connect(mfileWatch_1, &QFileSystemWatcher::directoryChanged, this, [=](){
        QFile file("/etc/apt/apt.conf.d/80apt-proxy");
        if (mAptBtn->isChecked()) {
            if (!file.exists()) {
                mAptBtn->setChecked(false);
                ui->mAPTFrame_2->hide();
                setAptProxy("" ,0 ,false);
            }
        }
    });

    connect(mfileWatch_2, &QFileSystemWatcher::directoryChanged, this, [=](){
        QFile file("/etc/profile.d/80apt-proxy.sh");
        if (mAptBtn->isChecked()) {
            if (!file.exists()) {
                mAptBtn->setChecked(false);
                ui->mAPTFrame_2->hide();
                setAptProxy("" ,0 ,false);
            }
        }
    });

    connect(ui->mEditBtn ,&QPushButton::clicked, this, &Proxy::setAptProxySlot);

    connect(mAptBtn, &SwitchButton::checkedChanged ,this ,[=](bool status) {
       if (status) {
           emit ui->mEditBtn->click();
       } else {  // 关闭APT代理，删除对应的配置文件
           if (QString(qgetenv("http_proxy").data()).isEmpty()) {
               ui->mAPTFrame_2->hide();
               setAptProxy("" ,0 ,false);
           } else {
               QMessageBox *mReboot = new QMessageBox(pluginWidget->topLevelWidget());
               mReboot->setIcon(QMessageBox::Warning);
               mReboot->setText(tr("The apt proxy  has been turned off and needs to be restarted to take effect"));
               QPushButton *laterbtn =  mReboot->addButton(tr("Reboot Later"), QMessageBox::RejectRole);
               QPushButton *nowbtn =   mReboot->addButton(tr("Reboot Now"), QMessageBox::AcceptRole);
               mReboot->exec();
               if (mReboot->clickedButton() == nowbtn) {  //选择了立即重启，一秒后系统会重启
                   ui->mAPTFrame_2->hide();
                   setAptProxy("" ,0 ,false);
                   sleep(1);
                   reboot();
               } else {  //选择了稍后重启,删掉对应文件，但删不了已生效的环境变量
                   ui->mAPTFrame_2->hide();
                   setAptProxy("" ,0 ,false);
               }
           }
       }
    });
}

void Proxy::initProxyModeStatus(){
    int mode = _getCurrentProxyMode();
    QHash<QString, QVariant> mAptinfo = getAptProxy();

    autoSwitchBtn->blockSignals(true);
    manualSwitchBtn->blockSignals(true);
    mAptBtn->blockSignals(true);

    if (mode == AUTO){
        autoSwitchBtn->setChecked(true);
    } else if (mode == MANUAL){
        manualSwitchBtn->setChecked(true);
    } else{
        autoSwitchBtn->setChecked(false);
        manualSwitchBtn->setChecked(false);
    }

    if (mAptinfo["open"].toBool()) {
        mAptBtn->setChecked(true);
        ui->mAPTHostLabel_2->setText(mAptinfo["ip"].toString());
        ui->mAPTPortLabel_2->setText(mAptinfo["port"].toString());
    } else {
        mAptBtn->setChecked(false);
        ui->mAPTFrame_2->setVisible(false);
    }

    autoSwitchBtn->blockSignals(false);
    manualSwitchBtn->blockSignals(false);
    mAptBtn->blockSignals(false);

    _setSensitivity();
}

void Proxy::initAutoProxyStatus(){

    ui->urlLineEdit->blockSignals(true);
    //设置当前url
    QString urlString = proxysettings->get(PROXY_AUTOCONFIG_URL_KEY).toString();
    ui->urlLineEdit->setText(urlString);

    ui->urlLineEdit->blockSignals(false);
}

void Proxy::initManualProxyStatus(){
    //信号阻塞
    ui->httpHostLineEdit->blockSignals(true);
    ui->httpsHostLineEdit->blockSignals(true);
    ui->ftpHostLineEdit->blockSignals(true);
    ui->socksHostLineEdit->blockSignals(true);

    ui->httpPortLineEdit->blockSignals(true);
    ui->httpsPortLineEdit->blockSignals(true);
    ui->ftpPortLineEdit->blockSignals(true);
    ui->socksPortLineEdit->blockSignals(true);

    //HTTP
    QString httphost = httpsettings->get(PROXY_HOST_KEY).toString();
    ui->httpHostLineEdit->setText(httphost);
    int httpport = httpsettings->get(PROXY_PORT_KEY).toInt();
    ui->httpPortLineEdit->setText(QString::number(httpport));

    //HTTPS
    QString httpshost = securesettings->get(PROXY_HOST_KEY).toString();
    ui->httpsHostLineEdit->setText(httpshost);
    int httpsport = securesettings->get(PROXY_PORT_KEY).toInt();
    ui->httpsPortLineEdit->setText(QString::number(httpsport));

    //FTP
    QString ftphost = ftpsettings->get(PROXY_HOST_KEY).toString();
    ui->ftpHostLineEdit->setText(ftphost);
    int ftppost = ftpsettings->get(PROXY_PORT_KEY).toInt();
    ui->ftpPortLineEdit->setText(QString::number(ftppost));

    //SOCKS
    QString sockshost = sockssettings->get(PROXY_HOST_KEY).toString();
    ui->socksHostLineEdit->setText(sockshost);
    int socksport = sockssettings->get(PROXY_PORT_KEY).toInt();
    ui->socksPortLineEdit->setText(QString::number(socksport));

    //解除信号阻塞
    ui->httpHostLineEdit->blockSignals(false);
    ui->httpsHostLineEdit->blockSignals(false);
    ui->ftpHostLineEdit->blockSignals(false);
    ui->socksHostLineEdit->blockSignals(false);

    ui->httpPortLineEdit->blockSignals(false);
    ui->httpsPortLineEdit->blockSignals(false);
    ui->ftpPortLineEdit->blockSignals(false);
    ui->socksPortLineEdit->blockSignals(false);
}

void Proxy::initIgnoreHostStatus(){
    ui->ignoreHostTextEdit->blockSignals(true);

    //设置当前ignore host
    QStringList ignorehost = proxysettings->get(IGNORE_HOSTS_KEY).toStringList();
    ui->ignoreHostTextEdit->setPlainText(ignorehost.join(";"));

    ui->ignoreHostTextEdit->blockSignals(false);
}

int Proxy::_getCurrentProxyMode(){
    GSettings * proxygsettings;
    proxygsettings = g_settings_new(PROXY_SCHEMA);
    int mode = g_settings_get_enum(proxygsettings, PROXY_MODE_KEY);
    g_object_unref(proxygsettings);

    return mode;
}

void Proxy::_setSensitivity(){
    //自动配置代理界面敏感性
    bool autoChecked = autoSwitchBtn->isChecked();
    ui->urlFrame->setVisible(autoChecked);


    //手动配置代理界面敏感性
    bool manualChecked = manualSwitchBtn->isChecked();
    ui->httpFrame->setVisible(manualChecked);
    ui->httpsFrame->setVisible(manualChecked);
    ui->ftpFrame->setVisible(manualChecked);
    ui->socksFrame->setVisible(manualChecked);

}

void Proxy::setAptProxy(QString host, QString port, bool status)
{
    QDBusInterface *mAptproxyDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                                             "/",
                                                             "com.control.center.interface",
                                                             QDBusConnection::systemBus());
    if (mAptproxyDbus->isValid())
        QDBusReply<bool> reply = mAptproxyDbus->call("setaptproxy", host, port , status);
    delete mAptproxyDbus;
    mAptproxyDbus = nullptr;
}

QHash<QString, QVariant> Proxy::getAptProxy()
{
    QHash<QString, QVariant> mAptInfo;
    QDBusInterface *mAptproxyDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                                             "/",
                                                             "com.control.center.interface",
                                                             QDBusConnection::systemBus());
   if (mAptproxyDbus->isValid()) {
       QDBusMessage result = mAptproxyDbus->call("getaptproxy");

       QList<QVariant> outArgs = result.arguments();
       QVariant first = outArgs.at(0);
       QDBusArgument dbvFirst = first.value<QDBusArgument>();
       QVariant vFirst = dbvFirst.asVariant();
       const QDBusArgument &dbusArgs = vFirst.value<QDBusArgument>();

       QVector<AptInfo> aptinfo;

       dbusArgs.beginArray();
       while (!dbusArgs.atEnd()) {
           AptInfo info;
           dbusArgs >> info;
           aptinfo.push_back(info);
       }
       dbusArgs.endArray();

       for (AptInfo it : aptinfo) {
           mAptInfo.insert(it.arg, it.out.variant());
       }
   }
   delete mAptproxyDbus;
   mAptproxyDbus = nullptr;
   return mAptInfo;
}

void Proxy::setAptInfo()
{
    QMessageBox *mReboot = new QMessageBox(pluginWidget->topLevelWidget());
    mReboot->setIcon(QMessageBox::Warning);
    mReboot->setText(tr("The system needs to be restarted to set the Apt proxy, whether to reboot"));
    QPushButton *laterbtn =  mReboot->addButton(tr("Reboot Later"), QMessageBox::RejectRole);
    QPushButton *nowbtn =   mReboot->addButton(tr("Reboot Now"), QMessageBox::AcceptRole);
    mReboot->exec();
    if (mReboot->clickedButton() == nowbtn) {  //选择了立即重启，一秒后系统会重启
        sleep(1);
        reboot();
    } else  {  //选择了稍后重启或点击了关闭按钮，配置文件已写入，但是/etc/profile.d目录下新增的脚本文件未执行
        ui->mAPTFrame_2->show();
        ui->mAPTHostLabel_2->setText(getAptProxy()["ip"].toString());
        ui->mAPTPortLabel_2->setText(getAptProxy()["port"].toString());
    }
}

void Proxy::reboot()
{
    QDBusInterface *rebootDbus = new QDBusInterface("org.gnome.SessionManager",
                                                             "/org/gnome/SessionManager",
                                                             "org.gnome.SessionManager",
                                                             QDBusConnection::sessionBus());

    rebootDbus->call("reboot");
    delete rebootDbus;
    rebootDbus = nullptr;
}

void Proxy::setAptProxySlot()
{
    mAptBtn->blockSignals(true);
    QHash<QString, QVariant> preaptinfo = getAptProxy();
    bool prestatus = preaptinfo["open"].toBool();
    AptProxyDialog *mwindow = new AptProxyDialog(pluginWidget);
    mwindow->exec();
    if (getAptProxy()["open"].toBool() && !prestatus) { // open值为true，用户点击了确定按钮，进行配置文件的写入，提示用户重启系统
        setAptInfo();
    }
    if (getAptProxy()["open"].toBool() && prestatus) {
        if (getAptProxy()["ip"].toString() == preaptinfo["ip"].toString() && getAptProxy()["port"].toString() == preaptinfo["port"].toString() && prestatus){  //点击了编辑按钮，且在设置IP和端口号的弹窗中，点击了取消或者关闭按钮
            ui->mAPTFrame_2->show();
        } else {
             setAptInfo();
        }
    }
    if(!getAptProxy()["open"].toBool() && !prestatus){ // 点击了APT开关按钮，但是在设置IP和端口号的弹窗中，点击了取消或者关闭按钮
       mAptBtn->setChecked(false);
    }
    mAptBtn->blockSignals(false);
}

void Proxy::showCertificationDialog(){
    QDialog * certificationDialog = new CertificationDialog();
    certificationDialog->setAttribute(Qt::WA_DeleteOnClose);
    certificationDialog->show();
}

void Proxy::manualProxyTextChanged(QString txt){
    //获取被修改控件
    QObject * pobject = this->sender();
    QLineEdit * who = dynamic_cast<QLineEdit *>(pobject);

    //获取控件保存的用户数据
    GSData currentData = who->property("gData").value<GSData>();
    QString schema = currentData.schema; QString key = currentData.key;

    //构建临时QGSettings
    const QByteArray id = schema.toUtf8();
    const QByteArray iidd(id.data());
    QGSettings * setting = new QGSettings(iidd);

    setting->set(key, QVariant(txt));

    delete setting;
    setting = nullptr;
}

void Proxy::proxyModeChangedSlot(bool checked){
    GSettings * proxygsettings;
    proxygsettings = g_settings_new(PROXY_SCHEMA);

    //两个switchbutton要达到互斥的效果，自定义按钮暂时未支持添加buttongroup
    QObject * object = QObject::sender();
    if (object->objectName() == "auto"){ //区分哪个switchbutton
        if (checked){
            if (manualSwitchBtn->isChecked())
                manualSwitchBtn->setChecked(false);
            g_settings_set_enum(proxygsettings, PROXY_MODE_KEY, AUTO);
        }
        else{
            if (!manualSwitchBtn->isChecked())
                g_settings_set_enum(proxygsettings, PROXY_MODE_KEY, NONE);
        }
    }
    else{
        if (checked){
            if (autoSwitchBtn->isChecked())
                autoSwitchBtn->setChecked(false);
            g_settings_set_enum(proxygsettings, PROXY_MODE_KEY, MANUAL);
        }
        else{
            if (!autoSwitchBtn->isChecked())
                g_settings_set_enum(proxygsettings, PROXY_MODE_KEY, NONE);
        }
    }
    g_object_unref(proxygsettings);

    _setSensitivity();
}

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

#include <QDebug>

Proxy::Proxy()
{
    ui = new Ui::Proxy;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("proxy");
    pluginType = NETWORK;

    const QByteArray id(PROXY_SCHEMA);
    proxysettings = new QGSettings(id);
    const QByteArray idd(HTTP_PROXY_SCHEMA);
    httpsettings = new QGSettings(idd);
    const QByteArray iddd(SECURE_PROXY_SCHEMA);
    securesettings = new QGSettings(iddd);
    const QByteArray iid(FTP_PROXY_SCHEMA);
    ftpsettings = new QGSettings(iid);
    const QByteArray iiid(SOCKS_PROXY_SCHEMA);
    sockssettings = new QGSettings(iiid);

//    //设置了一个隐藏按钮，实现其他两个按钮互斥，但是可以都不选中
//    ui->hideCheckBox->setHidden(true);

    component_init();
    status_init();
}

Proxy::~Proxy()
{
    delete ui;
    delete proxysettings;
    delete httpsettings;
    delete securesettings;
    delete ftpsettings;
    delete sockssettings;
}

QString Proxy::get_plugin_name(){
    return pluginName;
}

int Proxy::get_plugin_type(){
    return pluginType;
}

CustomWidget *Proxy::get_plugin_ui(){
    return pluginWidget;
}

void Proxy::plugin_delay_control(){

}

void Proxy::component_init(){

    autoSwitchBtn = new SwitchButton();
    autoSwitchBtn->setAttribute(Qt::WA_DeleteOnClose);
    autoSwitchBtn->setObjectName("auto");
    ui->autoHLayout->addWidget(autoSwitchBtn);
    ui->autoHLayout->addStretch();
    manualSwitchBtn = new SwitchButton();
    manualSwitchBtn->setAttribute(Qt::WA_DeleteOnClose);
    manualSwitchBtn->setObjectName("manual");
    ui->manualHLayout->addWidget(manualSwitchBtn);
    ui->manualHLayout->addStretch();

//    ui->buttonGroup->setId(ui->hideCheckBox,0);
//    ui->buttonGroup->setId(ui->manualCheckBox,1);
//    ui->buttonGroup->setId(ui->autoCheckBox,2);

    //设置组件数据，方便绑定一个槽函数，注意控件的添加顺序与hostStringList对应
    QStringList schemaStringList;
    schemaStringList << HTTP_PROXY_SCHEMA << SECURE_PROXY_SCHEMA << FTP_PROXY_SCHEMA << SOCKS_PROXY_SCHEMA;

    QStringList hostStringList;
    hostStringList << HTTP_PROXY_HOST_KEY << SECURE_PROXY_HOST_KEY << FTP_PROXY_HOST_KEY << SOCKS_PROXY_HOST_KEY;
    for (int num = 0; num < hostStringList.count() && num < ui->hostVLayout->count(); num++){
        KeyValue * value = new KeyValue();
        value->key = hostStringList.at(num);
        value->schema = schemaStringList.at(num);
        QLayoutItem * it = ui->hostVLayout->itemAt(num);
        QLineEdit * current = qobject_cast<QLineEdit *>(it->widget());
        current->setUserData(Qt::UserRole, value);
    }
    QStringList portStringList;
    portStringList << HTTP_PROXY_PORT_KEY << SECURE_PROXY_PORT_KEY << FTP_PROXY_PORT_KEY << SOCKS_PROXY_PORT_KEY;
    for (int num = 0; num < portStringList.count() && num < ui->portVLayout->count(); num++){
        KeyValue * value = new KeyValue();
        value->key = hostStringList.at(num);
        value->schema = schemaStringList.at(num);
        QLayoutItem * it = ui->portVLayout->itemAt(num);
        QLineEdit * current = qobject_cast<QLineEdit *>(it->widget());
        current->setUserData(Qt::UserRole, value);
    }
}

void Proxy::status_init(){
    //设置当前mode
    int mode = _get_current_proxy_mode();
    if (mode == AUTO){
//        ui->autoCheckBox->setChecked(true);
        autoSwitchBtn->setChecked(true);
    }
    else if (mode == MANUAL){
//        ui->manualCheckBox->setChecked(true);
        manualSwitchBtn->setChecked(true);
    }
    else{
//        ui->hideCheckBox->setChecked(true);
        autoSwitchBtn->setChecked(false);
        manualSwitchBtn->setChecked(false);
    }

    //设置当前url
    QString urlString = proxysettings->get(PROXY_AUTOCONFIG_URL_KEY).toString();
    ui->urlLineEdit->setText(urlString);

    //设置当前ignore host
    QStringList ignorehost = proxysettings->get(IGNORE_HOSTS_KEY).toStringList();
    ui->ignorehostTextEdit->setPlainText(ignorehost.join(";"));

    //HTTP
    QString httphost = httpsettings->get(HTTP_PROXY_HOST_KEY).toString();
    ui->httpLineEdit->setText(httphost);
    int httpport = httpsettings->get(HTTP_PROXY_PORT_KEY).toInt();
    ui->httpportLineEdit->setText(QString::number(httpport));

    //HTTPS
    QString securehost = securesettings->get(SECURE_PROXY_HOST_KEY).toString();
    ui->httpsLineEdit->setText(securehost);
    int secureport = securesettings->get(SECURE_PROXY_PORT_KEY).toInt();
    ui->httpsportLineEdit->setText(QString::number(secureport));

    //FTP
    QString ftphost = ftpsettings->get(FTP_PROXY_HOST_KEY).toString();
    ui->ftpLineEdit->setText(ftphost);
    int ftppost = ftpsettings->get(FTP_PROXY_PORT_KEY).toInt();
    ui->ftpportLineEdit->setText(QString::number(ftppost));

    //SOCKS
    QString sockshost = sockssettings->get(SOCKS_PROXY_HOST_KEY).toString();
    ui->socksLineEdit->setText(sockshost);
    int socksport = sockssettings->get(SOCKS_PROXY_PORT_KEY).toInt();
    ui->socksportLineEdit->setText(QString::number(socksport));

//    connect(ui->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(mode_changed_slot(int)));
    connect(autoSwitchBtn, SIGNAL(checkedChanged(bool)), this, SLOT(mode_changed_slot(bool)));
    connect(manualSwitchBtn, SIGNAL(checkedChanged(bool)), this, SLOT(mode_changed_slot(bool)));

    connect(ui->urlLineEdit, SIGNAL(textChanged(QString)), this, SLOT(url_edit_changed_slot(QString)));
    connect(ui->ignorehostTextEdit, SIGNAL(textChanged()), this, SLOT(ignore_host_edit_changed_slot()));

    connect(ui->httpLineEdit, SIGNAL(textChanged(QString)), this, SLOT(manual_component_changed_slot(QString)));
    connect(ui->httpsportLineEdit, SIGNAL(textChanged(QString)), this, SLOT(manual_component_changed_slot(QString)));
    connect(ui->httpsLineEdit, SIGNAL(textChanged(QString)), this, SLOT(manual_component_changed_slot(QString)));
    connect(ui->httpsportLineEdit, SIGNAL(textChanged(QString)), this, SLOT(manual_component_changed_slot(QString)));
    connect(ui->ftpLineEdit, SIGNAL(textChanged(QString)), this, SLOT(manual_component_changed_slot(QString)));
    connect(ui->ftpportLineEdit, SIGNAL(textChanged(QString)), this, SLOT(manual_component_changed_slot(QString)));
    connect(ui->socksLineEdit, SIGNAL(textChanged(QString)), this, SLOT(manual_component_changed_slot(QString)));
    connect(ui->socksportLineEdit, SIGNAL(textChanged(QString)), this, SLOT(manual_component_changed_slot(QString)));

    connect(ui->pushButton, SIGNAL(released()), this, SLOT(certification_dialog_show_slot()));

    _refreshUI();
}

int Proxy::_get_current_proxy_mode(){
    GSettings * proxygsettings;
    proxygsettings = g_settings_new(PROXY_SCHEMA);
    int mode = g_settings_get_enum(proxygsettings, PROXY_MODE_KEY);
    g_object_unref(proxygsettings);

    return mode;
}

void Proxy::_refreshUI(){
//    int id = ui->buttonGroup->checkedId();
//    if (id == 1){
//        ui->autoWidget->setEnabled(false);
//        ui->manualWidget->setEnabled(true);
//    }
//    else if (id == 2){
//        ui->manualWidget->setEnabled(false);
//        ui->autoWidget->setEnabled(true);
//    }
//    else{
//        ui->autoWidget->setEnabled(false);
//        ui->manualWidget->setEnabled(false);
//    }

    if (autoSwitchBtn->isChecked())
        ui->autoWidget->setEnabled(true);
    else
        ui->autoWidget->setEnabled(false);

    if (manualSwitchBtn->isChecked()){
        ui->manualWidget->setEnabled(true);
    }
    else
        ui->manualWidget->setEnabled(false);
}

void Proxy::certification_dialog_show_slot(){
    QDialog * certificationDialog = new CertificationDialog();
    certificationDialog->setAttribute(Qt::WA_DeleteOnClose);
    certificationDialog->show();
}

void Proxy::manual_component_changed_slot(QString edit){
    //获取被修改控件
    QObject * pobject = this->sender();
    QLineEdit * who = dynamic_cast<QLineEdit *>(pobject);

    //获取控件保存的用户数据
    KeyValue * current = (KeyValue *)(who->userData(Qt::UserRole));

    if (current->schema.endsWith("http"))
        httpsettings->set(current->key, QVariant(edit));
    else if (current->schema.endsWith("https"))
        securesettings->set(current->key, QVariant(edit));
    else if (current->schema.endsWith("ftp"))
        ftpsettings->set(current->key, QVariant(edit));
    else
        sockssettings->set(current->key, QVariant(edit));
}

void Proxy::ignore_host_edit_changed_slot(){
    QString text = ui->ignorehostTextEdit->toPlainText();
    QStringList hostStringList = text.split(";");
    proxysettings->set(IGNORE_HOSTS_KEY, QVariant(hostStringList));
}

void Proxy::url_edit_changed_slot(QString edit){
    proxysettings->set(PROXY_AUTOCONFIG_URL_KEY, QVariant(edit));
}

void Proxy::mode_changed_slot(bool checked){
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

    _refreshUI();

}

//void Proxy::mode_changed_slot(int id){
//    GSettings * proxygsettings;
//    proxygsettings = g_settings_new(PROXY_SCHEMA);
//    int mode = g_settings_get_enum(proxygsettings, PROXY_MODE_KEY);
//    if (mode == id){ //说明重复点击，将隐藏按钮点亮。实现了两个按钮互斥但是可以同时不点亮。
//        ui->hideCheckBox->setChecked(true);
//        g_settings_set_enum(proxygsettings, PROXY_MODE_KEY, 0);
//    }
//    else
//        g_settings_set_enum(proxygsettings, PROXY_MODE_KEY, id);

//    g_object_unref(proxygsettings);

//    _refreshUI();
//}

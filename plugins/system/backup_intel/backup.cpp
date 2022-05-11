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
#include "backup.h"
#include "ui_backup.h"

#include <QProcess>
#include <QFile>
#include <QDebug>
#include <QPixmap>
#include <QSvgRenderer>
#include <QPainter>
#include <QDBusInterface>
#include <QDBusReply>
#include <QMessageBox>
#ifdef signals
#undef signals
#endif

extern "C" {
#include <gio/gdesktopappinfo.h>
}

Backup::Backup() : mFirstLoad(true)
{
    pluginName = tr("System Recovery");
    pluginType = SYSTEM;
}

Backup::~Backup()
{
    if (!mFirstLoad)
        delete ui;
}

QString Backup::plugini18nName(){
    return pluginName;
}

int Backup::pluginTypes(){
    return pluginType;
}

QWidget *Backup::pluginUi(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Backup;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        messageBox = new MessageBox;

        QPixmap pixmap=loadSvg(QString("://img/plugins/backup/setting-reset.svg"),"default");
        ui->label->setStyleSheet("background:transparent");
        ui->label->setFixedSize(164,164);
        ui->label->setPixmap(pixmap);
        ui->backBtn->setFixedSize(184,56);
        //~ contents_path /Backup/Clear and restore
        ui->backBtn->setText(tr("Clear and restore"));

        const QByteArray id("org.ukui.style");
        if (QGSettings::isSchemaInstalled(id)){
            themeSetting = new QGSettings(id);
            QString m_themeMode = themeSetting->get("style-name").toString();
            if ("ukui-white" == m_themeMode || "ukui-default" == m_themeMode || "ukui-light" == m_themeMode || "ukui-white-unity" == m_themeMode) {
                ui->backBtn->setStyleSheet("QPushButton#backBtn{background-color:#FFFFFF;border: 1px;"
                                          "border-radius: 12px;font-size: 18px; color: palette(windowText);}"
                                          "QPushButton:hover:!pressed#backBtn:hover{background: #2FB3E8;"
                                          "border: 1px;border-radius: 12px;font-size: 18px; color: palette(windowText);}");
            } else if ("ukui-dark" == m_themeMode || "ukui-black" == m_themeMode || "ukui-black-unity" == m_themeMode){
                ui->backBtn->setStyleSheet("QPushButton#backBtn{background-color:#2C2F33;border: 1px;"
                                          "border-radius: 12px;font-size: 18px; color: palette(windowText);}"
                                          "QPushButton:hover:!pressed#backBtn:hover{background: #2FB3E8;"
                                          "border: 1px;border-radius: 12px;font-size: 18px; color: palette(windowText);}");
            }
            connect(themeSetting, &QGSettings::changed, this, [=](const QString &key){
                QString m_themeMode = themeSetting->get("style-name").toString();
                if ("ukui-white" == m_themeMode || "ukui-default" == m_themeMode || "ukui-light" == m_themeMode || "ukui-white-unity" == m_themeMode) {
                    ui->backBtn->setStyleSheet("QPushButton#backBtn{background-color:#FFFFFF;border: 1px;"
                                              "border-radius: 12px;font-size: 18px; color: palette(windowText);}"
                                              "QPushButton:hover:!pressed#backBtn:hover{background: #2FB3E8;"
                                              "border: 1px;border-radius: 12px;font-size: 18px; color: palette(windowText);}");
                } else if ("ukui-dark" == m_themeMode || "ukui-black" == m_themeMode || "ukui-black-unity" == m_themeMode){
                    ui->backBtn->setStyleSheet("QPushButton#backBtn{background-color:#2C2F33;border: 1px;"
                                              "border-radius: 12px;font-size: 18px; color: palette(windowText);}"
                                              "QPushButton:hover:!pressed#backBtn:hover{background: #2FB3E8;"
                                              "border: 1px;border-radius: 12px;font-size: 18px; color: palette(windowText);}");
                }
            });
        }

        connect(ui->backBtn, &QPushButton::clicked, this, [=](bool checked){
            Q_UNUSED(checked)
            btnClicked();
        });
    }
    return pluginWidget;
}

const QString Backup::name() const {

    return QStringLiteral("Backup");
}

QIcon Backup::icon() const
{
    return QIcon::fromTheme("ukui-backup-restore-symbolic");
}

bool Backup::isShowOnHomePage() const
{
    return false;
}

bool Backup::isEnable() const
{
    return false;
}

void Backup::btnClicked(){
    //监测当前电量
    QStringList users;
    QDBusInterface m_interface1( "org.freedesktop.UPower",
                                "/org/freedesktop/UPower",
                                "org.freedesktop.UPower",
                                QDBusConnection::systemBus() );
    if (!m_interface1.isValid()) {
        qDebug() << "dbus接口初始化失败";
        return;
    }

    QDBusReply<QList<QDBusObjectPath>> obj_reply = m_interface1.call("EnumerateDevices");

     QString powerpath = NULL;
     if (obj_reply.isValid()) {
         for (QDBusObjectPath op : obj_reply.value())
              users << op.path();
         if(users.size()==1 || users.isEmpty()){
             return ;
         }
         powerpath=users.at(1);
         qDebug()<<users.at(1);

     }

     QDBusInterface m_interface( "org.freedesktop.UPower",
                                 powerpath,
                                 "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus() );

     if (!m_interface.isValid()) {
         qDebug() << "dbus接口初始化失败~~~~~~~~~~~~~~~~~~~~~~~";
         return;
     }
     QDBusReply<QVariant> obj_reply1 = m_interface.call("Get","org.freedesktop.UPower.Device","Percentage");

     QDBusReply<QVariant> obj_state = m_interface.call("Get","org.freedesktop.UPower.Device","State");

     if (obj_reply1.isValid() && obj_state.isValid()) {

         if(obj_state.value().toUInt() != 1)
         {
             int Ele_surplus_int = obj_reply1.value().toInt();
             if (Ele_surplus_int < 25) {
                 MessageBoxPower *messageBoxpower = new MessageBoxPower;
                 messageBoxpower->exec();
                 return;
             }
         }
    }

    QDBusInterface * interface = new QDBusInterface("org.freedesktop.Accounts",
                                                    "/org/freedesktop/Accounts",
                                                    "org.freedesktop.Accounts",
                                                    QDBusConnection::systemBus());
    if (!interface->isValid()){
        qCritical() << "Get /org/freedesktop/Accounts Client Interface Failed " << QDBusConnection::systemBus().lastError();
        return;
    }

    QDBusReply<QDBusObjectPath> reply = interface->call("FindUserByName", g_get_user_name());
    QString userPath;
    if (reply.isValid()){
        userPath = reply.value().path();
        //qDebug() << "userPath: "<<userPath;
    } else {
        qCritical() << "Call 'UserInfo' Failed!" << reply.error().message();
        return;
    }
    QDBusInterface * useriFace = new QDBusInterface("org.freedesktop.Accounts",
                                                    userPath,
                                                    "org.freedesktop.DBus.Properties",
                                                    QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply2 = useriFace->call("GetAll", "org.freedesktop.Accounts.User");
    int userrights = -1;
    if (reply2.isValid()) {
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply2.value();
        userrights= propertyMap.find("AccountType").value().toInt();
        qDebug() <<userrights;
    }
    if (userrights == 1) {
        QDBusInterface *resetinterface;
        resetinterface = new QDBusInterface("org.kylin.mdm.reset",
                                            "/org/kylin/mdm/reset",
                                            "org.kylin.mdm.reset",
                                            QDBusConnection::sessionBus());
        if(!resetinterface->isValid()) {
            qCritical() << "Create Client Interface Failed : " << QDBusConnection::sessionBus().lastError().message();
            return;
        }
        QDBusReply<int> reply1=resetinterface->call("ResetFactory");
        if(reply1.isValid()) {
            if(reply1.value()) {
                return;
            } else {
                qCritical() <<"failed";
                messageBox->exec();
            }
        }
    } else {
        messagedialog = new MessageBoxDialog;
        messagedialog->exec();
//         QMessageBox::critical(NULL,"warning","您没有管理员权限！不能恢复出厂设置。",QMessageBox::Ok);
    }
}

QStringList Backup::readFile(QString filepath)
{
    QStringList fileCont;
    QFile file(filepath);
    if(file.exists()) {
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "ReadFile() failed to open" << filepath;
            return QStringList();
        }
        QTextStream textStream(&file);
        while(!textStream.atEnd()) {
            QString line= textStream.readLine();
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
const QPixmap Backup::loadSvg(const QString &fileName, QString color)
{
    const auto ratio = qApp->devicePixelRatio();
    QPixmap pixmap(164, 164);
    QSvgRenderer renderer(fileName);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap, color);
}
QPixmap Backup::drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ("white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if ("black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
//                    color.setAlpha(0.1);
                    color.setAlphaF(0.9);
                    img.setPixelColor(x, y, color);
                } else if ("gray" == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor){
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                } else {
                    return source;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

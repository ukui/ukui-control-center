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
#include "area.h"
#include "ui_area.h"

#include <QDebug>
#include <QFile>

const QVector<QString> CFormats{"zh_SG.UTF-8", "zh_CN.UTF-8", "lt_LT.UTF-8", "en_ZW.UTF-8", "en_ZM.UTF-8",
                               "en_ZA.UTF-8", "en_US.UTF-8", "en_SG.UTF-8", "en_PH.UTF-8", "en_NZ.UTF-8",
                               "en_NG.UTF-8", "en_IN.UTF-8", "en_IL.UTF-8", "en_IE.UTF-8", "en_HK.UTF-8",
                               "en_GB.UTF-8", "en_DK.UTF-8", "en_CA.UTF-8", "en_BW.UTF-8", "en_AU.UTF-8",
                               "en_AG.UTF-8", "af_ZA.UTF-8"};

Area::Area()
{
    ui = new Ui::Area;
    itemDelege = new QStyledItemDelegate();
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    pluginWidget->setStyleSheet("background: #ffffff;");
    ui->setupUi(pluginWidget);

    pluginName = tr("area");
    pluginType = DATETIME;

    unsigned int uid = getuid();
    objpath = objpath +"/org/freedesktop/Accounts/User"+QString::number(uid);


    m_areaInterface = new QDBusInterface("org.freedesktop.Accounts",
                                         objpath,
                                         "org.freedesktop.Accounts.User",
                                         QDBusConnection::systemBus());
    initUI();
    connect(ui->langcomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_language_slot(int)));
    connect(ui->countrycomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_area_slot(int)));
}

Area::~Area()
{
    delete ui;
}

QString Area::get_plugin_name(){
    return pluginName;
}

int Area::get_plugin_type(){
    return pluginType;
}

QWidget *Area::get_plugin_ui(){
    return pluginWidget;
}

void Area::plugin_delay_control(){

}

void Area::run_external_app_slot(){
    QString cmd = "gnome-language-selector";

    QProcess process(this);
    process.startDetached(cmd);
}


void Area::initUI(){
    ui->arealabel->setText(tr("current area"));

    ui->countrylabel->setText(tr("display format area"));
    ui->showformwidget->setStyleSheet("QWidget#showformwidget{background-color:#E5E7E9;border-radius:6px}");
    ui->countrylabel->setStyleSheet("QLabel#countrylabel{background: #E5E7E9;}");


    ui->countrycomboBox->setItemDelegate(itemDelege);
    ui->countrycomboBox->setMaxVisibleItems(5);

    ui->countrycomboBox->addItem(tr("US"));
    ui->countrycomboBox->addItem(tr("CN"));

    ui->areaformlabel->setText(tr("format of area"));
    ui->calendarlabel->setText(tr("calendar"));
    ui->weeklabel->setText(tr("first day of week"));
    ui->datelabel->setText(tr("date"));
    ui->timelabel->setText(tr("time"));

    ui->chgformButton->setStyleSheet("QPushButton{background-color:#E5E7E9;border-radius:4px}"
                                   "QPushButton:hover{background-color: #3D6BE5;};border-radius:4px");

    ui->chgformButton->setText(tr("change format of data"));

    ui->formwidget->setStyleSheet("background-color:#E5E7E9;border-radius:6px");

    ui->mainLagLabel->setText(tr("first language"));
    ui->languagewidget->setStyleSheet("QWidget#languagewidget{background-color:#E5E7E9;border-radius:6px}");
    ui->languagelabel->setText(tr("system language"));
    ui->languagelabel->setStyleSheet("QLabel#languagelabel{background: #E5E7E9;}");

    ui->addlanwidget->setStyleSheet("QWidget#addlanwidget{background-color:#E5E7E9;border-radius:6px}");

    ui->langcomboBox->setItemDelegate(itemDelege);
    ui->langcomboBox->setMaxVisibleItems(5);
    ui->langcomboBox->addItem(tr("English"));
    ui->langcomboBox->addItem(tr("Chinese"));

    ui->addlanlabel->setStyleSheet("QLabel{background-color:#E5E7E9}");
    ui->addlanlabel->setText(tr("add main language"));

    ui->addlanBtn->setIcon(QIcon("://img/plugins/printer/add.png"));
    ui->addlanBtn->setIconSize(QSize(48, 48));
}

void Area::initComponent() {

}

void Area::change_language_slot(int index){
    QDBusReply<bool> res;
    switch (index) {
    case 0:
        res = m_areaInterface->call("SetLanguage","en_US");
        break;
    case 1:
        res = m_areaInterface->call("SetLanguage","zh_CN");
        break;
    }
}

void Area::change_area_slot(int index){
    qDebug()<<"area----------->"<<endl;
    QDBusReply<bool> res;
    switch (index) {
    case 0:
        res = m_areaInterface->call("SetFormatsLocale","en_US");
        break;
    case 1:
        res = m_areaInterface->call("SetFormatsLocale","zh_CN");
        break;
    }
}

QStringList Area::readFile(const QString& filepath) {
    QStringList res;
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
            res<<line;
        }
        file.close();
        return res;
    } else {
        qWarning() << filepath << " not found"<<endl;
        return QStringList();
    }
}

QStringList Area::getUserDefaultLanguage() {
    int pos = 0;
    QString formats;
    QString language;
    QStringList filestr;
    QStringList result;
    QString fname = getenv("HOME");
    fname += "/.pam_environment";

    filestr = this->readFile(fname);
    qDebug()<<"result is------>"<<filestr<<endl;
    QRegExp re("LANGUAGE(\t+DEFAULT)?=(.*)$");
    for(int i = 0; i < filestr.length(); i++) {
        while((pos = re.indexIn(filestr.at(i), pos)) != -1) {
            language = re.cap(2);
            pos += re.matchedLength();
        }
    }

    QDBusInterface * iproperty = new QDBusInterface("org.freedesktop.Accounts",
                                            objpath,
                                            "org.freedesktop.DBus.Properties",
                                            QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply = iproperty->call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()){
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        formats = propertyMap.find("FormatsLocale").value().toString();
//        qDebug()<<"formats is----------->"<<formats<<endl;
    }
    else {
        qDebug() << "reply failed";
    }
    result.append(formats);
    result.append(language);
    qDebug()<<"result is---------->"<<result<<endl;
    return result;
}






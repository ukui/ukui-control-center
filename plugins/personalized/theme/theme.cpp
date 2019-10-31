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
#include "theme.h"
#include "ui_theme.h"

#include <QDebug>

#include "../../pluginsComponent/publicdata.h"

#define DESKTOP_SCHEMA "org.ukui.peony.desktop"

#define COMPUTER_ICON "computer-icon-visible"
#define HOME_ICON "home-icon-visible"
#define NETWORK_ICON "network-icon-visible"
#define TRASH_ICON "trash-icon-visible"
#define VOLUMES_ICON "volumes-visible"

#define ICONTHEMEPATH "/usr/share/icons/"
#define SYSTHEMEPATH "/usr/share/themes/"

Theme::Theme()
{
    ui = new Ui::Theme;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("theme");
    pluginType = PERSONALIZED;

    //初始化gsettings
    const QByteArray id(INTERFACE_SCHEMA);
    ifsettings = new QGSettings(id);
    const QByteArray idd(MARCO_SCHEMA);
    marcosettings = new QGSettings(idd);
    const QByteArray iid(DESKTOP_SCHEMA);
    desktopsettings = new QGSettings(iid);

    component_init();
    status_init();
}

Theme::~Theme()
{
    delete ui;
    delete ifsettings;
    delete marcosettings;
    delete desktopsettings;

//    QMap<QString, QToolButton *>::iterator it = delbtnMap.begin();
//    for (; it != delbtnMap.end(); it++){
//        delete it.value();
//    }
}

QString Theme::get_plugin_name(){
    return pluginName;
}

int Theme::get_plugin_type(){
    return pluginType;
}

CustomWidget *Theme::get_plugin_ui(){
    return pluginWidget;
}

void Theme::plugin_delay_control(){

}

void Theme::component_init(){
    QStringList themeList;

    QDir themesDir = QDir(SYSTHEMEPATH);
    foreach (QString themedir, themesDir.entryList(QDir::Dirs)) {
        if (themedir.startsWith("ukui-") && (themedir != "ukui-theme")){
            themeList.append(themedir);
        }
    }

    QSize themesize(52, 52);
    QSignalMapper * setSignalMapper = new QSignalMapper(this);
    for (int num = 0; num < themeList.length(); num++){
        QToolButton * button = new QToolButton();
        button->setAttribute(Qt::WA_DeleteOnClose);
        button->setFixedSize(themesize);
//        button->setText(themeList[num]);
        //获取背景色
        QString bgcolor = themeList[num].split("-")[1].trimmed();

        if (bgcolor == "blue")
            bgcolor = QString("darkblue");

        button->setStyleSheet(QString("background-color: %1").arg(bgcolor));
        connect(button, SIGNAL(released()), setSignalMapper, SLOT(map()));
        setSignalMapper->setMapping(button, themeList[num]);
        delbtnMap.insert(themeList[num], button);
        ui->themeHLayout->addWidget(button);
    }
    ui->themeHLayout->addStretch();
    connect(setSignalMapper, SIGNAL(mapped(QString)), this, SLOT(set_theme_slots(QString)));

    //
    QSize iconsize(48, 48);
    ui->wallpaperBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->wallpaperBtn->setIcon(QIcon(":/theme/background.png"));
    ui->wallpaperBtn->setIconSize(iconsize);

    ui->audioToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->audioToolBtn->setIcon(QIcon(":/theme/audio.png"));
    ui->audioToolBtn->setIconSize(iconsize);

    ui->mouseToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->mouseToolBtn->setIcon(QIcon(":/theme/cursor.png"));
    ui->mouseToolBtn->setIconSize(iconsize);

    ui->iconToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->iconToolBtn->setIcon(QIcon(":/theme/icon.png"));
    ui->iconToolBtn->setIconSize(iconsize);


    ui->computerIconLabel->setPixmap(QPixmap("://theme/computer.png"));
    ui->homefolderIconLabel->setPixmap(QPixmap("://theme/homefolder.png"));
    ui->trashIconLabel->setPixmap(QPixmap("://theme/trash.png"));
    ui->networkIconLabel->setPixmap(QPixmap("://theme//default.png"));
    ui->volumesIconLabel->setPixmap(QPixmap("://theme/default.png"));

    QMap<QString, QString> desktopiconMap;
    desktopiconMap.insert("computerHLayout", COMPUTER_ICON);
    desktopiconMap.insert("homefolderHLayout", HOME_ICON);
    desktopiconMap.insert("networkHLayout", NETWORK_ICON);
    desktopiconMap.insert("trashHLayout", TRASH_ICON);
    desktopiconMap.insert("volumesHLayout", VOLUMES_ICON);

    QSignalMapper * desktopSignalMapper = new QSignalMapper(this);
    QList<QHBoxLayout *>allHLayout = ui->page->findChildren<QHBoxLayout *>();
    for (int i = 0; i < allHLayout.count(); i++){
        QString key = desktopiconMap.find(allHLayout.at(i)->objectName()).value();

        SwitchButton * button = new SwitchButton();
        button->setAttribute(Qt::WA_DeleteOnClose);
        button->setChecked(desktopsettings->get(key).toBool());
        connect(button, SIGNAL(checkedChanged(bool)), desktopSignalMapper, SLOT(map()));
        desktopSignalMapper->setMapping(button, key);
        delsbMap.insert(key, button);
        allHLayout.at(i)->addWidget(button);
        allHLayout.at(i)->addStretch();
//        qDebug() << allHLayout.at(i)->objectName();
    }
    connect(desktopSignalMapper, SIGNAL(mapped(QString)), this, SLOT(desktop_icon_settings_slots(QString)));

    refresh_icon_theme();

}

void Theme::status_init(){
    refresh_btn_select_status();

    //设置当前主题预览图
    QSize size(300, 170);
    ui->previewLabel->setPixmap(QPixmap("://theme/preview.png").scaled(size));

    PublicData * publicdata = new PublicData();

    //
    QList<QStringList> tmpList = publicdata->subfuncList;
    connect(ui->wallpaperBtn, &QToolButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList[PERSONALIZED].at(0), PERSONALIZED, 0);});
    connect(ui->audioToolBtn, &QToolButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList[DEVICES].at(3), DEVICES, 0);});
    connect(ui->mouseToolBtn, &QToolButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList[DEVICES].at(1), DEVICES, 2);});
    connect(ui->iconToolBtn, &QToolButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList[PERSONALIZED].at(1), PERSONALIZED, 1);});

    connect(ui->desktopiconBtn, &QPushButton::clicked, this, [=]{ui->StackedWidget->setCurrentIndex(2);});

    delete publicdata;
}

void Theme::refresh_btn_select_status(){
    //获取当前主题
    QString current_theme;
    current_theme = marcosettings->get(MARCO_THEME_KEY).toString();

    QMap<QString, QToolButton *>::iterator it = delbtnMap.begin();
    for (; it != delbtnMap.end(); it++){
        QString key = QString(it.key());
        QToolButton * tmpBtn = (QToolButton *)it.value();
        if (key == current_theme)
            tmpBtn->setIcon(QIcon("://theme/select.png"));
        else
            tmpBtn->setIcon(QIcon(""));
    }
}

void Theme::refresh_icon_theme(){
    //清空layout
    for ( int i = 0; i < ui->themesVBoxLayout->count(); i++){
        QLayoutItem * item = ui->themesVBoxLayout->itemAt(i);
        ui->themesVBoxLayout->removeItem(item);
    }
    //删除new obj
    QMap<QString, IconThemeWidget *>::iterator it = delframeMap.begin();

    for (; it != delframeMap.end(); it++){
        delete it.value();
    }

    delframeMap.clear();

    //图标主题
    QDir themesDir = QDir(ICONTHEMEPATH);

    foreach (QString themedir, themesDir.entryList(QDir::Dirs)) {
        if (themedir.startsWith("ukui-icon")){
            QDir appsDir = QDir(ICONTHEMEPATH + themedir + "/48x48/apps/");
            appsDir.setFilter(QDir::Files | QDir::NoSymLinks);
            //图标主题中apps图标
            QStringList first, second;
            QStringList appiconsList = appsDir.entryList();
            for (int i = 0; i < appiconsList.size(); i++){
                if (i%32 == 0 && i < 8 * 32)
                    first.append(appsDir.path() + "/" + appiconsList.at(i));
                if ((i+1)%61 == 0 && i <= 8 * 61)
                    second.append(appsDir.path() + "/" + appiconsList.at(i));
            }

            IconThemeWidget * widget = new IconThemeWidget();
            widget->set_icontheme_name(themedir.section("-", 2, -1));
            widget->set_icontheme_fullname(themedir);
            widget->set_icontheme_selected(false);
            widget->set_icontheme_example(first, second);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(icon_theme_changed_slot(QString)));
            ui->themesVBoxLayout->addWidget(widget);

            //append data
            delframeMap.insert(themedir, widget);
        }
    }
    ui->themesVBoxLayout->addStretch();

    //设置当前图标主题
    QString icontheme = ifsettings->get(ICON_THEME_KEY).toString();
    delframeMap.value(icontheme)->set_icontheme_selected(true);
}

void Theme::set_theme_slots(QString value){
    ifsettings->set(GTK_THEME_KEY, QVariant(value));
    marcosettings->set(MARCO_THEME_KEY, QVariant(value));
//    if (value.contains("blue"))
//        ifsettings->set(ICON_THEME_KEY, "ukui-icon-theme-one");
//    else
//        ifsettings->set(ICON_THEME_KEY, "ukui-icon-theme");

    refresh_btn_select_status();
}

void Theme::desktop_icon_settings_slots(QString key){
    SwitchButton * button = reinterpret_cast<SwitchButton *>(delsbMap.find(key).value());
//    QString key;
//    if (flag == "computerHLayout"){
//        key = COMPUTER_ICON;
//    }
//    else if (flag == "homefolderHLayout"){
//        key = HOME_ICON;
//    }
//    else if (flag == "networkHLayout"){
//        key = NETWORK_ICON;
//    }
//    else if (flag == "trashHLayout"){
//        key = TRASH_ICON;
//    }
//    else if (flag == "volumesHLayout"){
//        key = VOLUMES_ICON;
//    }
    desktopsettings->set(key, button->isChecked());
}

void Theme::icon_theme_changed_slot(QString name){
    ifsettings->set(ICON_THEME_KEY, name);
    QMap<QString, IconThemeWidget *>::iterator it = delframeMap.begin();
    for (; it != delframeMap.end(); it++){
        if (it.key() == name)
            it.value()->set_icontheme_selected(true);
        else
            it.value()->set_icontheme_selected(false);
    }
    refresh_icon_theme();
}

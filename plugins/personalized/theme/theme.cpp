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

#include <QToolButton>
#include <QSignalMapper>
#include <QGSettings/QGSettings>

#include "SwitchButton/switchbutton.h"
#include "themewidget.h"

#include <QDebug>

#define DESKTOP_SCHEMA "org.ukui.peony.desktop"

#define COMPUTER_ICON "computer-icon-visible"
#define HOME_ICON "home-icon-visible"
#define NETWORK_ICON "network-icon-visible"
#define TRASH_ICON "trash-icon-visible"
#define VOLUMES_ICON "volumes-visible"

#define INTERFACE_SCHEMA "org.mate.interface"
#define MARCO_SCHEMA "org.gnome.desktop.wm.preferences"

#define GTK_THEME_KEY "gtk-theme"
#define ICON_THEME_KEY "icon-theme"
#define MARCO_THEME_KEY "theme"

#define ICONTHEMEPATH "/usr/share/icons/"
#define SYSTHEMEPATH "/usr/share/themes/"
#define CURSORS_THEMES_PATH "/usr/share/icons/"

#define ICONWIDGETHEIGH 74

Theme::Theme()
{
    ui = new Ui::Theme;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("theme");
    pluginType = PERSONALIZED;


    pluginWidget->setStyleSheet("background: #ffffff;");

    ui->defaultBtn->setStyleSheet("QPushButton{border-image: url('://img/plugins/theme/default.png')}");
    ui->lightBtn->setStyleSheet("QPushButton{border-image: url('://img/plugins/theme/light.png')}");
    ui->darkBtn->setStyleSheet("QPushButton{border-image: url('://img/plugins/theme/dark.png')}");
    ui->defaultSelectedLabel->setPixmap(QPixmap("://img/plugins/theme/selected.png"));
    ui->lightSelectedLabel->setPixmap(QPixmap("://img/plugins/theme/selected.png"));
    ui->darkSelectedLabel->setPixmap(QPixmap("://img/plugins/theme/selected.png"));

    ui->iconListWidget->setStyleSheet("QListWidget#iconListWidget{border: none;}");
    ui->cursorListWidget->setStyleSheet("QListWidget#cursorListWidget{border: none;}");

    ui->iconListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->iconListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->cursorListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->cursorListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->iconListWidget->setSpacing(0);
    ui->cursorListWidget->setSpacing(0);
    ui->iconListWidget->setFocusPolicy(Qt::NoFocus);
    ui->cursorListWidget->setFocusPolicy(Qt::NoFocus);
    ui->iconListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->cursorListWidget->setSelectionMode(QAbstractItemView::NoSelection);

    ui->controlWidget->setStyleSheet("QWidget#controlWidget{background: #F4F4F4; border-radius: 6px;}");

    ui->effectWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->line->setStyleSheet("QFrame{border-top: 1px solid gray; border-left: none; border-right: none; border-bottom: none;}");

    ui->resetBtn->setStyleSheet("QPushButton#resetBtn{border: none;}"
                                "QPushButton:hover:!pressed#resetBtn{border: none; background: #3D6BE5; border-radius: 2px;}"
                                "QPushButton:hover:pressed#resetBtn{border: none; background: #2C5AD6; border-radius: 2px;}");

    //初始化gsettings
    const QByteArray id(INTERFACE_SCHEMA);
    ifsettings = new QGSettings(id);
    const QByteArray idd(MARCO_SCHEMA);
    marcosettings = new QGSettings(idd);
    const QByteArray iid(DESKTOP_SCHEMA);
    desktopsettings = new QGSettings(iid);

    initComponent();
    initIconTheme();
    initControlTheme();
    initCursorTheme();
    initEffectSettings();

//    component_init();
//    status_init();
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

QWidget *Theme::get_plugin_ui(){
    return pluginWidget;
}

void Theme::plugin_delay_control(){

}

void Theme::initComponent(){

    //设置选中图标的显示状态
    ui->defaultSelectedLabel->setVisible(ui->defaultBtn->isChecked());
    ui->lightSelectedLabel->setVisible(ui->lightBtn->isChecked());
    ui->darkSelectedLabel->setVisible(ui->darkBtn->isChecked());
    //反向设置占位Labe的显示状态，放置选中图标隐藏后文字Label位置变化
    ui->defaultPlaceHolderLabel->setHidden(ui->defaultBtn->isChecked());
    ui->lightPlaceHolderLabel->setHidden(ui->lightBtn->isChecked());
    ui->darkPlaceHolderLabel->setHidden(ui->darkBtn->isChecked());

    connect(ui->themeModeBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, [=]{
        ui->defaultSelectedLabel->setVisible(ui->defaultBtn->isChecked());
        ui->lightSelectedLabel->setVisible(ui->lightBtn->isChecked());
        ui->darkSelectedLabel->setVisible(ui->darkBtn->isChecked());
        ui->defaultPlaceHolderLabel->setHidden(ui->defaultBtn->isChecked());
        ui->lightPlaceHolderLabel->setHidden(ui->lightBtn->isChecked());
        ui->darkPlaceHolderLabel->setHidden(ui->darkBtn->isChecked());
    });

}

void Theme::initIconTheme(){

    QStringList themeNameStringList;
    themeNameStringList << QString("hicolor") << QString("ukui-icon-theme") << QString("ukui-icon-theme-one");
    QStringList i18nthemeNameStringList;
    i18nthemeNameStringList << QObject::tr("hicolor") << QObject::tr("theme") << QObject::tr("theme-one");

    ui->iconListWidget->setFixedHeight(themeNameStringList.count() * ICONWIDGETHEIGH);

    //获取当前图标主题
    QString currentIconTheme = "ukui-icon-theme";

    QDir themesDir = QDir(ICONTHEMEPATH);

    foreach (QString themedir, themesDir.entryList(QDir::Dirs)) {
        if (themeNameStringList.contains(themedir)){
            int index = themeNameStringList.indexOf(themedir);
            QDir appsDir = QDir(ICONTHEMEPATH + themedir + "/48x48/apps/");
            appsDir.setFilter(QDir::Files | QDir::NoSymLinks);
            QStringList appIconsList = appsDir.entryList();

            QStringList showIconsList;
            for (int i = 0; i < appIconsList.size(); i++){
                if (i%64 == 0 && i < 5 * 64){
                    showIconsList.append(appsDir.path() + "/" + appIconsList.at(i));
                }
            }

            ThemeWidget * widget = new ThemeWidget(QSize(48, 48), i18nthemeNameStringList.at(index), showIconsList);
            widget->setAttribute(Qt::WA_DeleteOnClose);

            QListWidgetItem * item = new QListWidgetItem(ui->iconListWidget);
            item->setData(Qt::UserRole, QVariant(themedir));
            item->setSizeHint(QSize(554, ICONWIDGETHEIGH));

            if (themedir == currentIconTheme){
                ui->iconListWidget->setCurrentItem(item);
                widget->setSelectedStatus(true);
            } else {
                widget->setSelectedStatus(false);
            }

            ui->iconListWidget->setItemWidget(item, widget);
        }
    }
    connect(ui->iconListWidget, &QListWidget::currentItemChanged, this, [=](QListWidgetItem * current, QListWidgetItem * previous){
        ThemeWidget * currentWidget = dynamic_cast<ThemeWidget *>(ui->iconListWidget->itemWidget(current));
        currentWidget->setSelectedStatus(true);
        ThemeWidget * previousWidget = dynamic_cast<ThemeWidget *>(ui->iconListWidget->itemWidget(previous));
        previousWidget->setSelectedStatus(false);
    });
}

void Theme::initControlTheme(){
    QStringList colorStringList;
    colorStringList << QString("#3D6BE5");
    colorStringList << QString("#FA6C63");
    colorStringList << QString("#6cd472");
    colorStringList << QString("#f9a959");
    colorStringList << QString("#BA7Bd8");
    colorStringList << QString("#F8D15D");
    colorStringList << QString("#E7BBB0");
    colorStringList << QString("#176F57");

    QButtonGroup * colorBtnGroup = new QButtonGroup();

    for (QString color : colorStringList){
        QVBoxLayout * colorVerLayout = new QVBoxLayout();
        colorVerLayout->setSpacing(8);
        colorVerLayout->setMargin(0);

        QPushButton * button = new QPushButton(ui->controlWidget);
        button->setFixedSize(QSize(24, 24));
        button->setCheckable(true);
        QString btnStyle = QString("QPushButton{background: %1; border-radius: 4px;}").arg(color);
        button->setStyleSheet(btnStyle);
        colorBtnGroup->addButton(button, colorStringList.indexOf(color));

        QLabel * selectedColorLabel = new QLabel(ui->controlWidget);
        QSizePolicy scSizePolicy = selectedColorLabel->sizePolicy();
        scSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
        scSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        selectedColorLabel->setSizePolicy(scSizePolicy);
        selectedColorLabel->setScaledContents(true);
        selectedColorLabel->setPixmap(QPixmap("://img/plugins/theme/selected.png"));
        //初始化选中图标状态
        selectedColorLabel->setVisible(button->isChecked());
        connect(colorBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this,[=]{
            selectedColorLabel->setVisible(button->isChecked());
        });



        colorVerLayout->addWidget(button, 0, Qt::AlignHCenter);
        colorVerLayout->addWidget(selectedColorLabel, 0, Qt::AlignHCenter);
        colorVerLayout->addStretch();

        ui->controlHorLayout->addLayout(colorVerLayout);
    }
}

void Theme::initCursorTheme(){

    QStringList cursorThemes = _getSystemCursorThemes();
//    qDebug() << cursorThemes;

    //获取当前指针主题
    QString currentCursorTheme = QString("Breeze_Snow");

    ui->cursorListWidget->setFixedHeight(cursorThemes.count() * ICONWIDGETHEIGH);

    for (QString cursor : cursorThemes){
        ThemeWidget * widget  = new ThemeWidget(QSize(24, 24), cursor, QStringList(""));
        widget->setAttribute(Qt::WA_DeleteOnClose);

        QListWidgetItem * item = new QListWidgetItem(ui->cursorListWidget);
        item->setData(Qt::UserRole, QVariant(cursor));
        item->setSizeHint(QSize(554, ICONWIDGETHEIGH));

        //初始化指针主题界面
        if (currentCursorTheme == cursor){
            ui->cursorListWidget->setCurrentItem(item);
            widget->setSelectedStatus(true);
        } else {
            widget->setSelectedStatus(false);
        }

        ui->cursorListWidget->setItemWidget(item, widget);

    }

    connect(ui->cursorListWidget, &QListWidget::currentItemChanged, this, [=](QListWidgetItem * current, QListWidgetItem * previous){
        //更新当前指针主题

        //刷新界面
        ThemeWidget * currentWidget = dynamic_cast<ThemeWidget *>(ui->cursorListWidget->itemWidget(current));
        currentWidget->setSelectedStatus(true);
        ThemeWidget * previousWidget = dynamic_cast<ThemeWidget *>(ui->cursorListWidget->itemWidget(previous));
        previousWidget->setSelectedStatus(false);
    });

}

void Theme::initEffectSettings(){
    ui->effectLabel->hide();
    ui->effectWidget->hide();
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
//        ui->themeHLayout->addWidget(button);
    }
//    ui->themeHLayout->addStretch();
    connect(setSignalMapper, SIGNAL(mapped(QString)), this, SLOT(set_theme_slots(QString)));

    //
//    QSize iconsize(48, 48);
//    ui->wallpaperBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
//    ui->wallpaperBtn->setIcon(QIcon(":/theme/background.png"));
//    ui->wallpaperBtn->setIconSize(iconsize);

//    ui->audioToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
//    ui->audioToolBtn->setIcon(QIcon(":/theme/audio.png"));
//    ui->audioToolBtn->setIconSize(iconsize);

//    ui->mouseToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
//    ui->mouseToolBtn->setIcon(QIcon(":/theme/cursor.png"));
//    ui->mouseToolBtn->setIconSize(iconsize);

//    ui->iconToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
//    ui->iconToolBtn->setIcon(QIcon(":/theme/icon.png"));
//    ui->iconToolBtn->setIconSize(iconsize);


//    ui->computerIconLabel->setPixmap(QPixmap("://theme/computer.png"));
//    ui->homefolderIconLabel->setPixmap(QPixmap("://theme/homefolder.png"));
//    ui->trashIconLabel->setPixmap(QPixmap("://theme/trash.png"));
//    ui->networkIconLabel->setPixmap(QPixmap("://theme//default.png"));
//    ui->volumesIconLabel->setPixmap(QPixmap("://theme/default.png"));

    QMap<QString, QString> desktopiconMap;
    desktopiconMap.insert("computerHLayout", COMPUTER_ICON);
    desktopiconMap.insert("homefolderHLayout", HOME_ICON);
    desktopiconMap.insert("networkHLayout", NETWORK_ICON);
    desktopiconMap.insert("trashHLayout", TRASH_ICON);
    desktopiconMap.insert("volumesHLayout", VOLUMES_ICON);

    QSignalMapper * desktopSignalMapper = new QSignalMapper(this);
//    QList<QHBoxLayout *>allHLayout = ui->page->findChildren<QHBoxLayout *>();
//    for (int i = 0; i < allHLayout.count(); i++){
//        QString key = desktopiconMap.find(allHLayout.at(i)->objectName()).value();

//        SwitchButton * button = new SwitchButton();
//        button->setAttribute(Qt::WA_DeleteOnClose);
//        button->setChecked(desktopsettings->get(key).toBool());
//        connect(button, SIGNAL(checkedChanged(bool)), desktopSignalMapper, SLOT(map()));
//        desktopSignalMapper->setMapping(button, key);
//        delsbMap.insert(key, button);
//        allHLayout.at(i)->addWidget(button);
//        allHLayout.at(i)->addStretch();
//    }
    connect(desktopSignalMapper, SIGNAL(mapped(QString)), this, SLOT(desktop_icon_settings_slots(QString)));

}

void Theme::status_init(){
    refresh_btn_select_status();

    //设置当前主题预览图
//    QSize size(300, 170);
//    ui->previewLabel->setPixmap(QPixmap("://theme/preview.png").scaled(size));

//    PublicData * publicdata = new PublicData();

    //
//    QList<QStringList> tmpList = publicdata->subfuncList;
//    connect(ui->wallpaperBtn, &QToolButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList[PERSONALIZED].at(0), PERSONALIZED, 0);});
//    connect(ui->audioToolBtn, &QToolButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList[DEVICES].at(3), DEVICES, 0);});
//    connect(ui->mouseToolBtn, &QToolButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList[DEVICES].at(1), DEVICES, 2);});
//    connect(ui->iconToolBtn, &QToolButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList[PERSONALIZED].at(1), PERSONALIZED, 1);});

//    connect(ui->desktopiconBtn, &QPushButton::clicked, this, [=]{ui->StackedWidget->setCurrentIndex(2);});

//    delete publicdata;
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

QStringList Theme::_getSystemCursorThemes(){
    QStringList themes;
    QDir themesDir(CURSORS_THEMES_PATH);

    if (themesDir.exists()){
        foreach (QString dirname, themesDir.entryList(QDir::Dirs)){
            if (dirname == "." || dirname == "..")
                continue;
//            QString fullpath(CURSORS_THEMES_PATH + dirname);
            QDir themeDir(CURSORS_THEMES_PATH + dirname + "/cursors/");
            if (themeDir.exists())
                themes.append(dirname);
        }
    }
    return themes;
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

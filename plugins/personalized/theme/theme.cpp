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

#include <QGSettings/QGSettings>

#include "SwitchButton/switchbutton.h"

#include "themewidget.h"
#include "widgetgroup.h"
#include "cursor/xcursortheme.h"

#include <QDebug>

/**
 * GTK主题
 */
#define THEME_GTK_SCHEMA "org.mate.interface"
#define MODE_GTK_KEY "gtk-theme"
/* GTK图标主题 */
#define ICON_GTK_KEY "icon-theme"

/**
 * QT主题
 */
#define THEME_QT_SCHEMA "org.ukui.style"
#define MODE_QT_KEY "style-name"
/* QT图标主题 */
#define ICON_QT_KEY "icon-theme-name"


/**
 * 窗口管理器Marco主题
 */
#define MARCO_SCHEMA "org.gnome.desktop.wm.preferences"
#define MARCO_THEME_KEY "theme"

#define ICONTHEMEPATH "/usr/share/icons/"
#define SYSTHEMEPATH "/usr/share/themes/"
#define CURSORS_THEMES_PATH "/usr/share/icons/"

#define CURSOR_THEME_SCHEMA "org.ukui.peripherals-mouse"
#define CURSOR_THEME_KEY "cursor-theme"

#define ICONWIDGETHEIGH 74


namespace {

    // Preview cursors
    const char * const cursor_names[] =
    {
        "left_ptr",
        "left_ptr_watch",
        "wait",
        "pointing_hand",
        "whats_this",
        "ibeam",
        "size_all",
        "size_fdiag",
        "cross",
        "split_h",
        "size_ver",
        "size_hor",
        "size_bdiag",
        "split_v",
    };

    const int numCursors      = 9;     // The number of cursors from the above list to be previewed
}

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

    ui->controlWidget->setStyleSheet("QWidget#controlWidget{background: #F4F4F4; border-radius: 6px;}");

    ui->effectWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->line->setStyleSheet("QFrame{border-top: 1px solid #CCCCCC; border-left: none; border-right: none; border-bottom: none;}");

    ui->resetBtn->setStyleSheet("QPushButton#resetBtn{border: none;}"
                                "QPushButton:hover:!pressed#resetBtn{border: none; background: #3D6BE5; border-radius: 2px;}"
                                "QPushButton:hover:pressed#resetBtn{border: none; background: #2C5AD6; border-radius: 2px;}");

    ui->transparencySlider->setStyleSheet("QSlider{height: 20px;}"
                                          "QSlider::groove:horizontal{border: none;}"
                                          "QSlider::add-page:horizontal{background: #808080; border-radius: 2px; margin-top: 8px; margin-bottom: 9px;}"
                                          "QSlider::sub-page:horizontal{background: #3D6BE5; border-radius: 2px; margin-top: 8px; margin-bottom: 9px;}"
                                          "QSlider::handle:horizontal{width: 20px; height: 20px; border-image: url(:/img/plugins/fonts/bigRoller.png);}"
                                          "");

    //初始化gsettings
    const QByteArray id(THEME_GTK_SCHEMA);
    gtkSettings = new QGSettings(id);
    const QByteArray idd(THEME_QT_SCHEMA);
    qtSettings = new QGSettings(idd);
    const QByteArray iid(CURSOR_THEME_SCHEMA);
    curSettings = new QGSettings(iid);

    setupComponent();
    initThemeMode();
    initIconTheme();
    initControlTheme();
    initCursorTheme();
    initEffectSettings();
}

Theme::~Theme()
{
    delete ui;
    delete gtkSettings;
    delete qtSettings;
    delete curSettings;

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

void Theme::setupComponent(){
    ui->defaultBtn->setProperty("value", "ukui-default");
    ui->lightBtn->setProperty("value", "ukui-white");
    ui->darkBtn->setProperty("value", "ukui-black");

    ui->effectLabel->hide();
    ui->effectWidget->hide();

    //构建并填充特效开关按钮
    effectSwitchBtn = new SwitchButton(pluginWidget);
    ui->effectHorLayout->addWidget(effectSwitchBtn);

}

void Theme::initThemeMode(){
    //获取当前主题
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    //设置界面
    for (QAbstractButton * button : ui->themeModeBtnGroup->buttons()){
        QVariant valueVariant = button->property("value");
        if (valueVariant.isValid() && valueVariant.toString() == currentThemeMode)
            button->setChecked(true);
    }

    //设置选中图标的显示状态
    ui->defaultSelectedLabel->setVisible(ui->defaultBtn->isChecked());
    ui->lightSelectedLabel->setVisible(ui->lightBtn->isChecked());
    ui->darkSelectedLabel->setVisible(ui->darkBtn->isChecked());
    //反向设置占位Labe的显示状态，放置选中图标隐藏后文字Label位置变化
    ui->defaultPlaceHolderLabel->setHidden(ui->defaultBtn->isChecked());
    ui->lightPlaceHolderLabel->setHidden(ui->lightBtn->isChecked());
    ui->darkPlaceHolderLabel->setHidden(ui->darkBtn->isChecked());

    connect(ui->themeModeBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, [=](QAbstractButton * button){
        ui->defaultSelectedLabel->setVisible(ui->defaultBtn->isChecked());
        ui->lightSelectedLabel->setVisible(ui->lightBtn->isChecked());
        ui->darkSelectedLabel->setVisible(ui->darkBtn->isChecked());
        ui->defaultPlaceHolderLabel->setHidden(ui->defaultBtn->isChecked());
        ui->lightPlaceHolderLabel->setHidden(ui->lightBtn->isChecked());
        ui->darkPlaceHolderLabel->setHidden(ui->darkBtn->isChecked());

        //设置主题
        QString themeMode = button->property("value").toString();
        qtSettings->set(MODE_QT_KEY, themeMode);
        gtkSettings->set(MODE_GTK_KEY, themeMode);
    });
}

void Theme::initIconTheme(){
    //获取当前图标主题(以QT为准，后续可以对比GTK两个值)
    QString currentIconTheme = qtSettings->get(ICON_QT_KEY).toString();

    //构建图标主题Widget Group，方便更新选中/非选中状态
    WidgetGroup * iconThemeWidgetGroup = new WidgetGroup;
    connect(iconThemeWidgetGroup, &WidgetGroup::widgetChanged, [=](ThemeWidget * preWidget, ThemeWidget * curWidget){
        if (preWidget)
            preWidget->setSelectedStatus(false);
        curWidget->setSelectedStatus(true);

        QString value = curWidget->getValue();
        //设置图标主题
        qtSettings->set(ICON_QT_KEY, value);
        gtkSettings->set(ICON_GTK_KEY, value);
    });

    //构建图标主题QDir
    QDir themesDir = QDir(ICONTHEMEPATH);

    foreach (QString themedir, themesDir.entryList(QDir::Dirs)) {
        if (themedir.startsWith("ukui-icon-theme-")){
            QDir appsDir = QDir(ICONTHEMEPATH + themedir + "/48x48/apps/");
            appsDir.setFilter(QDir::Files | QDir::NoSymLinks);
            QStringList appIconsList = appsDir.entryList();

            QStringList showIconsList;
            for (int i = 0; i < appIconsList.size(); i++){
                if (i%64 == 0 && i < 6 * 64){
                    showIconsList.append(appsDir.path() + "/" + appIconsList.at(i));
                }
            }

            ThemeWidget * widget = new ThemeWidget(QSize(48, 48), dullTranslation(themedir.section("-", -1, -1, QString::SectionSkipEmpty)), showIconsList);
            widget->setValue(themedir);
            //加入Layout
            ui->iconThemeVerLayout->addWidget(widget);

            //加入WidgetGround实现获取点击前Widget
            iconThemeWidgetGroup->addWidget(widget);

            if (themedir == currentIconTheme){
                iconThemeWidgetGroup->setCurrentWidget(widget);
                widget->setSelectedStatus(true);
            } else {
                widget->setSelectedStatus(false);
            }
        }
    }
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

        QPushButton * button = new QPushButton(ui->controlWidget);
        button->setFixedSize(QSize(48, 48));
        button->setCheckable(true);
        QString btnStyle = QString("QPushButton{background: %1; border-radius: 4px;}").arg(color);
        button->setStyleSheet(btnStyle);
        colorBtnGroup->addButton(button, colorStringList.indexOf(color));



        QVBoxLayout * colorVerLayout = new QVBoxLayout();
        colorVerLayout->setSpacing(0);
        colorVerLayout->setMargin(0);

        QHBoxLayout * colorHorLayout = new QHBoxLayout();
        colorHorLayout->setSpacing(0);
        colorHorLayout->setMargin(0);

        QLabel * selectedColorLabel = new QLabel(button);
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
            //设置控件主题
        });

        colorHorLayout->addStretch();
        colorHorLayout->addWidget(selectedColorLabel);
        colorVerLayout->addLayout(colorHorLayout);
        colorVerLayout->addStretch();

        button->setLayout(colorVerLayout);

        ui->controlHorLayout->addWidget(button);
    }
}

void Theme::initCursorTheme(){

    QStringList cursorThemes = _getSystemCursorThemes();
//    qDebug() << cursorThemes;

    //获取当前指针主题
    QString currentCursorTheme;
    currentCursorTheme = curSettings->get(CURSOR_THEME_KEY).toString();

    WidgetGroup * cursorThemeWidgetGroup = new WidgetGroup;
    connect(cursorThemeWidgetGroup, &WidgetGroup::widgetChanged, [=](ThemeWidget * preWidget, ThemeWidget * curWidget){
        if (preWidget)
            preWidget->setSelectedStatus(false);
        curWidget->setSelectedStatus(true);

        QString value = curWidget->getValue();
        //设置光标主题
        curSettings->set(CURSOR_THEME_KEY, value);
    });

    for (QString cursor : cursorThemes){

//        qDebug()<<"cursor is----------->"<<cursor<<endl;

        QList<QPixmap> cursorVec;
        QString path = CURSORS_THEMES_PATH + cursor;
        XCursorTheme *cursorTheme = new XCursorTheme(path);

        for(int i = 0; i < numCursors; i++){
            QImage image = cursorTheme->loadImage(cursor_names[i],20);
            cursorVec.append(QPixmap::fromImage(image));
        }

        ThemeWidget * widget  = new ThemeWidget(QSize(24, 24), cursor, cursorVec);
        widget->setValue(cursor);

        //加入Layout
        ui->cursorVerLayout->addWidget(widget);

        //加入WidgetGround实现获取点击前Widget
        cursorThemeWidgetGroup->addWidget(widget);

        //初始化指针主题选中界面
        if (currentCursorTheme == cursor){
            cursorThemeWidgetGroup->setCurrentWidget(widget);
            widget->setSelectedStatus(true);
        } else {
            widget->setSelectedStatus(false);
        }

    }
}

void Theme::initEffectSettings(){
//    ui->effectLabel->hide();
//    ui->effectWidget->hide();
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

QString Theme::dullTranslation(QString str){
    if (!QString::compare(str, "basic")){
        return QObject::tr("basic");
    } else if (!QString::compare(str, "classical")){
        return QObject::tr("classical");
    } else if (!QString::compare(str, "default")){
        return QObject::tr("default");
    } else
        return QObject::tr("Unknown");
}

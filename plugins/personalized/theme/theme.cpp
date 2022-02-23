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

#include <QGSettings>

#include "SwitchButton/switchbutton.h"

#include "themewidget.h"
#include "widgetgroup.h"
#include "cursor/xcursortheme.h"

#include "../../../shell/customstyle.h"

#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtConcurrent>
/**
 * GTK主题
 */
#define THEME_GTK_SCHEMA "org.mate.interface"
#define MODE_GTK_KEY "gtk-theme"
#define MODE_GTK_BLACK "ukui-black"
#define MODE_GTK_WHITE"ukui-white"
/* GTK图标主题 */
#define ICON_GTK_KEY "icon-theme"

/**
 * QT主题
 */
#define THEME_QT_SCHEMA "org.ukui.style"
#define MODE_QT_KEY "style-name"
#define THEME_TRAN_KEY "menu-transparency"
#define PEONY_TRAN_KEY "peony-side-bar-transparency"

/*自动主题*/
#define AUTO_THEME_SCHEMA "org.ukui.SettingsDaemon.plugins.color"
#define AUTO_THEME "theme-schedule-automatic"

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

/**
 * 透明度设置
 */
#define PERSONALSIE_SCHEMA "org.ukui.control-center.personalise"
#define PERSONALSIE_TRAN_KEY "transparency"
#define PERSONALSIE_BLURRY_KEY "blurry"
#define PERSONALSIE_EFFECT_KEY "effect"

const QString defCursor = "DMZ-White";
const int transparency = 95;

const QStringList effectList {"blur", "kwin4_effect_translucency", "kwin4_effect_maximize", "zoom"};

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

const int numCursors = 9;     // The number of cursors from the above list to be previewed
}

Theme::Theme()
{
    ui = new Ui::Theme;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("Theme");
    pluginType = PERSONALIZED;
    ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    ui->iconLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    ui->cursorLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    ui->effectLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    settingsCreate = false;

    const QByteArray id(THEME_GTK_SCHEMA);
    const QByteArray idd(THEME_QT_SCHEMA);
    const QByteArray iid(CURSOR_THEME_SCHEMA);
    const QByteArray iiid(PERSONALSIE_SCHEMA);
    const QByteArray iiiid(AUTO_THEME_SCHEMA);
    if (QGSettings::isSchemaInstalled(iiid)) {
        personliseGsettings = new QGSettings(iiid);
    }
    if (QGSettings::isSchemaInstalled(iiiid)) {
        autoThemeGsettings = new QGSettings(iiiid);
        if (autoThemeGsettings->keys().contains("themeScheduleAutomatic") || autoThemeGsettings->keys().contains(AUTO_THEME)) {
            //监听自动主题开关
            connect(autoThemeGsettings, &QGSettings::changed, this, [ = ]() {
                if (autoThemeGsettings->get(AUTO_THEME).toBool()) {
                    ui->defaultRadioBtn->setChecked(true);
                } else {
                    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
                    if ("ukui-light" == currentThemeMode || MODE_GTK_WHITE == currentThemeMode || "" == currentThemeMode) {
                        ui->lightRadioBtn->setChecked(true);
                    } else if ("ukui-dark" == currentThemeMode || MODE_GTK_BLACK == currentThemeMode) {
                        ui->darkRadioBtn->setChecked(true);
                    }
                }
            });
        }
    }

    connect(ui->defaultRadioBtn, &QRadioButton::clicked, this, [=]() {
        if (ui->defaultRadioBtn->isChecked()) {
            ui->systemDefaultLabel->setPixmap(QPixmap(QString("://img/plugins/theme/auto-selected.png")));
            ui->darkModeLabel->setPixmap(QPixmap(QString("://img/plugins/theme/dark mode.png")));
            ui->lightLabel->setPixmap(QPixmap(QString("://img/plugins/theme/light mode.png")));
        }
    });
    connect(ui->darkRadioBtn, &QRadioButton::clicked, this, [=]() {
        if (ui->darkRadioBtn->isChecked()) {
            ui->systemDefaultLabel->setPixmap(QPixmap(QString("://img/plugins/theme/auto.png")));
            ui->darkModeLabel->setPixmap(QPixmap(QString("://img/plugins/theme/dark mode-selected.png")));
            ui->lightLabel->setPixmap(QPixmap(QString("://img/plugins/theme/light mode.png")));
        }
    });
    connect(ui->lightRadioBtn, &QRadioButton::clicked, this, [=]() {
        if (ui->lightRadioBtn->isChecked()) {
            ui->systemDefaultLabel->setPixmap(QPixmap(QString("://img/plugins/theme/auto.png")));
            ui->darkModeLabel->setPixmap(QPixmap(QString("://img/plugins/theme/dark mode.png")));
            ui->lightLabel->setPixmap(QPixmap(QString("://img/plugins/theme/light mode-selected.png")));
        }
    });

    initSearchText();
    //设置组件
    setupComponent();

    if (QGSettings::isSchemaInstalled(id) && QGSettings::isSchemaInstalled(idd)
            && QGSettings::isSchemaInstalled(iid)){
        gtkSettings = new QGSettings(id);
        qtSettings = new QGSettings(idd);
        curSettings = new QGSettings(iid);

        //监听主题改变
        connect(qtSettings, &QGSettings::changed, this, [ = ]() {
            if (autoThemeGsettings->keys().contains("themeScheduleAutomatic") || autoThemeGsettings->keys().contains(AUTO_THEME)) {
                if (autoThemeGsettings->get(AUTO_THEME).toBool()) {
                    return;
                }
            }
            //当前不是自动主题时，随主题改变按钮状态
            QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            if ("ukui-light" == currentThemeMode || MODE_GTK_WHITE == currentThemeMode || "" == currentThemeMode) {
                ui->lightRadioBtn->setChecked(true);
            } else if ("ukui-dark" == currentThemeMode || MODE_GTK_BLACK == currentThemeMode) {
                ui->darkRadioBtn->setChecked(true);
            }
        });

        settingsCreate = true;
        buildThemeModeLabel();
        monitorThemeChanges();
        initIconTheme();
        initCursorTheme();
        initConnection();
    } else {
        qCritical() << THEME_GTK_SCHEMA << "or" << THEME_QT_SCHEMA << "or" << CURSOR_THEME_SCHEMA << "not installed\n";
    }
    // init kwin settings
    setupSettings();
}

Theme::~Theme()
{
    delete ui;
    if (settingsCreate){
        delete gtkSettings;
        delete qtSettings;
        delete curSettings;
    }
    if (kwinSettings ){
        delete kwinSettings;
    }
    if (kwinGsettings) {
        delete kwinGsettings;
    }
    if (personliseGsettings) {
        delete personliseGsettings;
    }
    if (autoThemeGsettings) {
        delete autoThemeGsettings;
        autoThemeGsettings = NULL;
    }
    if(radioBtn)
        delete radioBtn;
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

const QString Theme::name() const {

    return QStringLiteral("theme");
}

void Theme::initSearchText() {
    //~ contents_path /theme/Theme Mode
    ui->titleLabel->setText(tr("Theme Mode"));
    //~ contents_path /theme/Theme Mode
    ui->defaultRadioBtn->setText(tr("Automatic"));
    //~ contents_path /theme/Theme Mode
    ui->darkRadioBtn->setText(tr("Dark"));
    //~ contents_path /theme/Theme Mode
    ui->lightRadioBtn->setText(tr("Light color"));
    //~ contents_path /theme/Icon theme
    ui->iconLabel->setText(tr("Icon theme"));

    //屏蔽光标主题
    ////~ contents_path /theme/Cursor theme
    ui->cursorLabel->setText(tr("Cursor theme"));
    ui->cursorLabel->hide();
    ui->cursorWidget->hide();

    //~ contents_path /theme/Effects mode
    ui->perforLabel->setText(tr("Effects mode"));
    //~ contents_path /theme/Effect setting
    ui->effectLabel->setText(tr("Effect setting"));
}

void Theme::setupSettings() {
    QString filename = QDir::homePath() + "/.config/ukui-kwinrc";
    kwinSettings = new QSettings(filename, QSettings::IniFormat, this);

//    QStringList keys = kwinSettings->allKeys();

    kwinSettings->beginGroup("Plugins");

    bool kwin = kwinSettings->value("blurEnabled", kwin).toBool();
    if (!kwinSettings->contains("blurEnabled")) {
        kwin = true;
    }
    kwinSettings->endGroup();
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    writeKwinSettings(kwin, currentThemeMode, true);

    effectSwitchBtn->setChecked(kwin);
    QFileInfo dir(filename);
    if (!dir.isFile()) {
        effectSwitchBtn->setChecked(true);
    }
}

void Theme::setupComponent(){

    //    ui->lightButton->hide();
    //隐藏现阶段不支持功能
    ui->controlLabel->hide();
    ui->controlWidget->hide();
    ui->resetBtn->hide();

    //构建并填充特效开关按钮
    effectSwitchBtn = new SwitchButton(pluginWidget);
    ui->effectHorLayout->addWidget(effectSwitchBtn);

    ui->kwinFrame->setVisible(false);
//    ui->transFrame->setVisible(true);
}
void Theme::monitorThemeChanges(){
    connect(qtSettings, &QGSettings::changed, this, [=]{
        if (autoThemeGsettings->keys().contains("themeScheduleAutomatic") || autoThemeGsettings->keys().contains(AUTO_THEME)) {
            //若由自动主题切换，不响应
            bool is_auto_theme = autoThemeGsettings->get(AUTO_THEME).toBool();
            if (is_auto_theme) return;
        }
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        if (MODE_GTK_WHITE == currentThemeMode) {
            ui->lightRadioBtn->setChecked(true);
        } else if (MODE_GTK_BLACK == currentThemeMode ) {
            ui->darkRadioBtn->setChecked(true);
        }
    });
}
void Theme::buildThemeModeLabel(){
    ui->systemDefaultLabel->setScaledContents(true);
    QString fullicon_1 = QString("://img/plugins/theme/auto.png");
    ui->systemDefaultLabel->setFixedSize(144,88);
    ui->systemDefaultLabel->setPixmap(QPixmap(fullicon_1));
    ui->darkModeLabel->setScaledContents(true);
    QString fullicon_2 = QString("://img/plugins/theme/dark mode.png");
    ui->darkModeLabel->setFixedSize(144,88);
    ui->darkModeLabel->setPixmap(QPixmap(fullicon_2));

    ui->lightLabel->setScaledContents(true);
    QString fullicon_3 = QString("://img/plugins/theme/light mode.png");
    ui->lightLabel->setFixedSize(144,88);
    ui->lightLabel->setPixmap(QPixmap(fullicon_3));
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if (autoThemeGsettings && (autoThemeGsettings->keys().contains("themeScheduleAutomatic") || autoThemeGsettings->keys().contains(AUTO_THEME))) {
        bool is_auto_theme = autoThemeGsettings->get(AUTO_THEME).toBool();
        if (is_auto_theme) {
            ui->defaultRadioBtn->setChecked(true);
            ui->systemDefaultLabel->setPixmap(QPixmap(QString("://img/plugins/theme/auto-selected.png")));
        } else {
            if ("ukui-light" == currentThemeMode) {
                ui->lightRadioBtn->setChecked(true);
                ui->lightLabel->setPixmap(QPixmap(QString("://img/plugins/theme/light mode-selected.png")));
            } else if (MODE_GTK_WHITE == currentThemeMode) {
                ui->lightRadioBtn->setChecked(true);
                ui->lightLabel->setPixmap(QPixmap(QString("://img/plugins/theme/light mode-selected.png")));
            } else if ("ukui-dark" == currentThemeMode) {
                ui->darkRadioBtn->setChecked(true);
                ui->darkModeLabel->setPixmap(QPixmap(QString("://img/plugins/theme/dark mode-selected.png")));
            } else if (MODE_GTK_BLACK == currentThemeMode) {
                ui->darkRadioBtn->setChecked(true);
                ui->darkModeLabel->setPixmap(QPixmap(QString("://img/plugins/theme/dark mode-selected.png")));
            } else if ("" == currentThemeMode) {
                ui->lightRadioBtn->setChecked(true);
                ui->lightLabel->setPixmap(QPixmap(QString("://img/plugins/theme/light mode-selected.png")));
            }
        }
    }

    connect(ui->defaultRadioBtn,&QRadioButton::clicked,[=]{
        ui->defaultRadioBtn->clearFocus();
        //开启自动主题
        if (autoThemeGsettings->keys().contains("themeScheduleAutomatic") || autoThemeGsettings->keys().contains(AUTO_THEME)) {
            autoThemeGsettings->set(AUTO_THEME, true);
        }
    });
    qApp->setStyle(new InternalStyle("ukui"));
    connect(qtSettings,&QGSettings::changed,this,[=](const QString &key){
        if (key == "styleName" && ui->defaultRadioBtn->isChecked() ) {
            autoThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            qApp->setStyle(new InternalStyle("ukui"));
        }
    });
    connect(ui->lightRadioBtn,&QRadioButton::clicked,[=]{
        ui->lightRadioBtn->clearFocus();
        //intel定制版修改ukui-default为ukui-light，以防主题设置对部分组件不生效
        QString themeMode = "ukui-light";
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        qApp->setStyle(new InternalStyle("ukui"));
        if (QString::compare(currentThemeMode, themeMode)) {
            QString tmpMode;
            if ("ukui-dark" == themeMode) {
                tmpMode = MODE_GTK_BLACK;
            } else {
                tmpMode = MODE_GTK_WHITE;
            }
            gtkSettings->set(MODE_GTK_KEY, tmpMode);

            QtConcurrent::run([=](){
                qtSettings->set(MODE_QT_KEY, themeMode);
            });
            if(effectSwitchBtn->isChecked()){
                writeKwinSettings(true, currentThemeMode, true);
            } else {
                writeKwinSettings(false, currentThemeMode, true);
            }
        }
        //关闭自动主题
        if (autoThemeGsettings->keys().contains("themeScheduleAutomatic") || autoThemeGsettings->keys().contains(AUTO_THEME)) {
            autoThemeGsettings->set(AUTO_THEME, false);
        }
    });
    connect(ui->darkRadioBtn,&QRadioButton::clicked,[=]{
        ui->darkRadioBtn->clearFocus();
        QString themeMode = "ukui-dark";
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        qApp->setStyle(new InternalStyle("ukui"));
        if (QString::compare(currentThemeMode, themeMode)) {
            QString tmpMode;
            if ("ukui-dark" == themeMode) {
                tmpMode = MODE_GTK_BLACK;
            } else {
                tmpMode = MODE_GTK_WHITE;
            }
            gtkSettings->set(MODE_GTK_KEY, tmpMode);

            QtConcurrent::run([=](){
                qtSettings->set(MODE_QT_KEY, themeMode);
            });
            if(effectSwitchBtn->isChecked()){
                writeKwinSettings(true, currentThemeMode, true);
            } else {
                writeKwinSettings(false, currentThemeMode, true);
            }
        }
        //关闭自动主题
        if (autoThemeGsettings->keys().contains("themeScheduleAutomatic") || autoThemeGsettings->keys().contains(AUTO_THEME)) {
            autoThemeGsettings->set(AUTO_THEME, false);
        }
    });
}
void Theme::initIconTheme(){
    //获取当前图标主题(以QT为准，后续可以对比GTK两个值)
    QStringList iconThemes;
    iconThemes << "ukui" << "ukui-hp" << "ukui-classical";
    QString currentIconTheme = qtSettings->get(ICON_QT_KEY).toString();
    if (currentIconTheme == "ukui-icon-theme-default")
        currentIconTheme = "ukui-hp";
    if (!iconThemes.contains(currentIconTheme)) {
        //避免装机后第一次启动控制面板，图标主题默认值有误
        currentIconTheme = "ukui-hp";
    }

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
        //Intel三套图标主题分别位于/usr/share/icons/ukui、ukui-hp、ukui-classical,需要暂时隐藏ukui-light这套主题
        if (themedir == "ukui" || themedir == "ukui-hp" || themedir == "ukui-classical") {
            QDir appsDir = QDir(ICONTHEMEPATH + themedir + "/48x48/apps/");
            appsDir.setFilter(QDir::Files | QDir::NoSymLinks);
            QStringList appIconsList;
            appIconsList << "devices/computer.png" << "apps/ukui-control-center.png" << "apps/system-file-manager.png"
                         << "status/user-trash-full.png" << "apps/indicator-china-weather.png" << "apps/kylin-video.png" << "apps/kylin-software-center.png";
            QStringList showIconsList;
            for (int i = 0; i < appIconsList.size(); i++){
                showIconsList.append(QDir(ICONTHEMEPATH + themedir + "/48x48/").path() + "/" + appIconsList.at(i));
            }
            radioBtn = new QRadioButton;
            ThemeWidget * widget = new ThemeWidget(QSize(48, 48), dullTranslation(themedir), showIconsList,radioBtn);
            //            widget->setFrameShape(QFrame::Shape::Box);
            widget->setValue(themedir);
            //加入Layout
            if (themedir == "ukui-hp") {
                ui->iconThemeVerLayout->insertWidget(0, widget);
                widget->setStyleSheet("ThemeWidget{background: palette(base); border-top-left-radius: 12px;border-top-right-radius: 12px}");
            } else if (themedir == "ukui"){
                ui->iconThemeVerLayout->insertWidget(1, widget);
                widget->setStyleSheet("ThemeWidget{background: palette(base); border-radius: 0px}");
            } else {
                ui->iconThemeVerLayout->addWidget(widget);
                widget->setStyleSheet("ThemeWidget{background: palette(base); border-bottom-left-radius: 12px;border-bottom-right-radius: 12px}");
            }
            //加入WidgetGround实现获取点击前Widget
            iconThemeWidgetGroup->addWidget(widget);

            if (themedir == currentIconTheme) {
                iconThemeWidgetGroup->setCurrentWidget(widget);
                widget->setSelectedStatus(true);
            } else {
                widget->setSelectedStatus(false);
            }
        }
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

#if QT_VERSION <= QT_VERSION_CHECK(5,12,0)

#else
        QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
        QList<QVariant> args;
        args.append(5);
        args.append(0);
        message.setArguments(args);
        QDBusConnection::sessionBus().send(message);
#endif

    });

    for (QString cursor : cursorThemes){
        if (cursor == QString("dark-sense").toLocal8Bit() || cursor == QString("blue-crystal").toLocal8Bit() || cursor == QString("red-glass").toLocal8Bit()) {
            QList<QPixmap> cursorVec;
            QString path = CURSORS_THEMES_PATH + cursor;
            XCursorTheme *cursorTheme = new XCursorTheme(path);

            for(int i = 0; i < numCursors; i++){
                int size = qApp->devicePixelRatio() * 8;
                QImage image = cursorTheme->loadImage(cursor_names[i],size);
                cursorVec.append(QPixmap::fromImage(image));
            }
//            ThemeWidget * widget = new ThemeWidget(QSize(48, 48), dullTranslation(themedir.section("-", -1, -1, QString::SectionSkipEmpty)), showIconsList,radioBtn);

            ThemeWidget * widget  = new ThemeWidget(QSize(24, 24), cursorTranslation(cursor), cursorVec);
            //        widget->setFrameShape(QFrame::Shape::Box);
            widget->setValue(cursor);
            //加入Layout
            if (cursor == QString("dark-sense").toLocal8Bit()) {
                ui->cursorVerLayout->insertWidget(0, widget);
            } else {
                ui->cursorVerLayout->addWidget(widget);
            }

            //加入WidgetGround实现获取点击前Widget
            cursorThemeWidgetGroup->addWidget(widget);

            //初始化指针主题选中界面
            if (currentCursorTheme == cursor || (currentCursorTheme.isEmpty() && cursor == defCursor)) {
                cursorThemeWidgetGroup->setCurrentWidget(widget);
                widget->setSelectedStatus(true);
            } else {
                widget->setSelectedStatus(false);
            }

        }
    }
}
QString Theme::cursorTranslation(QString str) {
    if (!QString::compare(str, "blue-crystal")){
        ////~ contents_path /theme/blue-crystal
        return QObject::tr("blue-crystal");
    } else if (!QString::compare(str, "dark-sense")){
        ////~ contents_path /theme/dark-sense
        return QObject::tr("dark-sense");
    } else if (!QString::compare(str, "red-glass")){
        ////~ contents_path /theme/red-glass
        return QObject::tr("red-glass");
    }
}
void Theme::initConnection() {
    connect(ui->resetBtn, &QPushButton::clicked, this, &Theme::resetBtnClickSlot);
    connect(effectSwitchBtn, &SwitchButton::checkedChanged, [this](bool checked) {
        if (checked) {
            personliseGsettings->set(PERSONALSIE_TRAN_KEY,0.75);
        } else {
            personliseGsettings->set(PERSONALSIE_TRAN_KEY,1);
        }
        personliseGsettings->set(PERSONALSIE_EFFECT_KEY, checked);
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
//        ui->transFrame->setVisible(checked);
        writeKwinSettings(checked, currentThemeMode, true);
    });

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
    ui->transFrame->setVisible(false);
    ui->effectFrame->setVisible(false);
    ui->effectLabel->setVisible(false);
#else

#endif
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
    if (!QString::compare(str, "ukui-classical")){
        //~ contents_path /theme/classical
        return QObject::tr("classical");
    } else if (!QString::compare(str, "ukui-hp")){
        //~ contents_path /theme/ukui-hp
        return QObject::tr("ukui-hp");
    } else if (!QString::compare(str, "ukui-light")){
        return QObject::tr("light");
    } else if (!QString::compare(str, "ukui")){
        //~ contents_path /theme/basics
        return QObject::tr("basics");
    } else
        return QObject::tr("Unknown");
}

// reset all of themes, include cursor, icon,and etc...
void Theme::resetBtnClickSlot() {
    // reset cursor default theme
    QString cursorTheme = "DMZ-White";
    curSettings->set(CURSOR_THEME_KEY,cursorTheme);

    //reset icon default theme
    qtSettings->reset(ICON_QT_KEY);
    qtSettings->reset(THEME_TRAN_KEY);
    qtSettings->reset(PEONY_TRAN_KEY);
    gtkSettings->reset(ICON_GTK_KEY);

    clearLayout(ui->iconThemeVerLayout->layout(), true);
    clearLayout(ui->cursorVerLayout->layout(), true);

    //    initThemeMode();
    initIconTheme();
    initCursorTheme();
}

void Theme::writeKwinSettings(bool change, QString theme, bool effect) {

    if (!change) {
        kwinSettings->beginGroup("Plugins");
        kwinSettings->setValue("blurEnabled", false);
        kwinSettings->setValue("kwin4_effect_maximizeEnabled", false);
        kwinSettings->setValue("kwin4_effect_translucencyEnabled", false);
        kwinSettings->setValue("zoomEnabled", false);
        kwinSettings->endGroup();
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)

#else
        for (int i = 0; i < effectList.length(); i++) {
            QDBusMessage message = QDBusMessage::createMethodCall("org.ukui.KWin",
                                                                  "/Effects",
                                                                  "org.ukui.kwin.Effects",
                                                                  "unloadEffect");
            message << effectList.at(i);
            QDBusConnection::sessionBus().send(message);

        }
#endif
    } else {
        kwinSettings->beginGroup("Plugins");
        kwinSettings->setValue("blurEnabled", true);
        kwinSettings->setValue("kwin4_effect_maximizeEnabled", true);
        kwinSettings->setValue("kwin4_effect_translucencyEnabled", true);
        kwinSettings->setValue("zoomEnabled", true);
        kwinSettings->endGroup();
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)

#else
        // 开启模糊特效：
        for (int i = 0; i < effectList.length(); i++) {

            QDBusMessage message = QDBusMessage::createMethodCall("org.ukui.KWin",
                                                                  "/Effects",
                                                                  "org.ukui.kwin.Effects",
                                                                  "loadEffect");
            message << effectList.at(i);
            QDBusConnection::sessionBus().send(message);
        }
#endif

    }

    kwinSettings->sync();
}

void Theme::themeButtonClicked(QAbstractButton *button) {
    // 设置主题
    QString themeMode = button->property("value").toString();
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();

    qApp->setStyle(new InternalStyle(themeMode));
    if (QString::compare(currentThemeMode, themeMode)){
        QString tmpMode;
        if ("ukui-dark" == themeMode) {
            tmpMode = "ukui-black";
        } else {
            tmpMode = "ukui-white";
        }
        gtkSettings->set(MODE_GTK_KEY, tmpMode);

        QtConcurrent::run([=](){
            qtSettings->set(MODE_QT_KEY, themeMode);
        });
        writeKwinSettings(true, themeMode);
    }
}

void Theme::clearLayout(QLayout* mlayout, bool deleteWidgets)
{
    if ( mlayout->layout() != NULL )
    {
        QLayoutItem* item;
        while ( ( item = mlayout->layout()->takeAt( 0 ) ) != NULL )
        {
            delete item->widget();
            delete item;
        }
    }
}

double Theme::convertToTran(const int value)
{
    switch (value) {
    case 1:
        return 0.2;
        break;
    case 2:
        return 0.4;
        break;
    case 3:
        return 0.6;
        break;
    case 4:
        return 0.8;
        break;
    case 5:
        return 1.0;
        break;
    default:
        return 1.0;
        break;
    }
}

int Theme::tranConvertToSlider(const double value)
{
    if (0.2 ==  value) {
        return 1;
    } else if (0.4 ==  value){
        return 2;
    } else if (0.6 ==  value){
        return 3;
    } else if (0.8 ==  value){
        return 4;
    } else if (1.0 ==  value){
        return 5;
    } else {
        return 5;
    }
}

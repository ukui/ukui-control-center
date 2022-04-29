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
#ifndef THEME_H
#define THEME_H

#include <QObject>
#include <QtPlugin>
#include <QLayout>
#include <QMap>
#include <QDir>
#include <QDir>
#include <QSettings>
#include <QtDBus/QDBusMessage>
#include <QGSettings>
#include <QtDBus/QtDBus>
#include <QAbstractButton>

#include "shell/interface.h"
#include <ukcc/widgets/uslider.h>
#include "themewidget.h"
#include "widgetgroup.h"

class QPushButton;
class SwitchButton;
class QGSettings;
class WidgetGroup;

namespace Ui {
class Theme;
}

class Theme : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    enum ThemeType { ICON, CURSOR};

private:
    Ui::Theme *ui;

    QString pluginName;
    int pluginType;
    QWidget *pluginWidget;

    QGSettings *gtkSettings;
    QGSettings *qtSettings;
    QGSettings *curSettings;
    QSettings  *kwinSettings;
    QSettings  *themeSettings;
    QGSettings *kwinGsettings =  nullptr;
    QGSettings *personliseGsettings = nullptr;

    SwitchButton *effectSwitchBtn;

    bool settingsCreate;

    WidgetGroup *cursorThemeWidgetGroup;
    WidgetGroup *iconThemeWidgetGroup;

public:
    Theme();
    ~Theme();
    double i=0;
    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

    void initSearchText();
    void setupSettings();
    void setupComponent();
    void initThemeMode();
    void initIconTheme();
    void setupControlTheme();
    void initCursorTheme();
    void initConnection();
    void initIconThemeWidget(QString themedir , int count);
    void initCursorThemeWidget(QString themedir , int count);

    void buildThemeModeBtn(QPushButton * button, QString name, QString icon);

    QString dullTranslation(QString str);

    QStringList _getSystemCursorThemes();

    QFrame *setLine(QFrame *frame);
private:
    void setCheckStatus(QLayout* mlayout, QString checkName, ThemeType type);
    double convertToTran(const int value);
    int tranConvertToSlider(const double value);
    QStringList readFile(QString filepath);
    void setupGSettings();
    void kwinCursorSlot(QString value);

    QString dullCursorTranslation(QString str);
    QString getCursorName();

    void hideIntelComponent();  // 隐藏非intel功能

    bool isBlurEffect();

private slots:
    void resetBtnClickSlot();
    void writeKwinSettings(bool change, QString theme, bool effect = false);
    void themeBtnClickSlot(QAbstractButton *button);
};

#endif // THEME_H


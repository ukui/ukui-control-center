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
#ifndef AREA_H
#define AREA_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>

#include "shell/interface.h"
#include <ukcc/widgets/hoverwidget.h>
#include <ukcc/widgets/imageutil.h>
#include "shell/utils/mthread.h"

#include <QProcess>
#include <QDBusInterface>
#include <QDBusReply>
#include <QGSettings>
#include <QListView>
#include <ukcc/widgets/addbtn.h>


/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

namespace Ui {
class Area;
}

class Area : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Area();
    ~Area();

    QString plugini18nName()   Q_DECL_OVERRIDE;
    int pluginTypes()       Q_DECL_OVERRIDE;
    QWidget * pluginUi()   Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;
    void settingForIntel() Q_DECL_OVERRIDE;

    QStringList readFile(const QString& filePath);

private:
    void initUI();
    void initComponent();
    QStringList getUserDefaultLanguage();
    void connectToServer();
    void initFormFrame();
    void initFormComponent(int8_t value);
    void initConnect();
    void writeGsettings(const QString &key, const QString &value);
    void timeFormatClicked(bool flag);
    void initLanguage();
    void showMessageBox(const int flag);

private:
    Ui::Area *ui;

    int pluginType;

    QString objpath;
    QString pluginName;
    QString hourformat;
    QString mDateFormat;

    QWidget * pluginWidget;

    QDBusInterface *m_areaInterface;
    QGSettings     *m_gsettings = nullptr;
    HoverWidget    *addWgt;
    QDBusInterface *cloudInterface;

    bool mFirstLoad;
private slots:
    void run_external_app_slot();
    void change_area_slot(int);
    void add_lan_btn_slot();
    void cloudChangedSlot(const QString &key);
};

#endif // AREA_H

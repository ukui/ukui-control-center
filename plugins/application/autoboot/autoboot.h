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
#ifndef AUTOBOOT_H
#define AUTOBOOT_H

#include <QObject>
#include <QtPlugin>

#include "shell/interface.h"
#include "datadefined.h"
#include <ukcc/widgets/titlelabel.h>
#include <ukcc/widgets/hoverwidget.h>
#include <ukcc/widgets/addbtn.h>

#include <QtDBus>
#include <QGSettings>
#include <QVBoxLayout>
/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

namespace Ui {
class AutoBoot;
}

class AutoBoot : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    AutoBoot();
    ~AutoBoot();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    const QString name() const Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    void initAddBtn();
    void initStyle();
    void initUI(QWidget *widget);

    void initAutoUI();
    void initStatus();
    void initConnection();
    void connectToServer();
    bool initConfig();
    void setupGSettings();

    AutoApp setInformation(QString filepath);
    bool copyFileToLocal(QString bname);
    bool deleteLocalAutoapp(QString bname);
    bool setAutoAppStatus(QString bname, bool status);
    void clearAutoItem();
    gboolean _key_file_to_file(GKeyFile *keyfile, const gchar *path);

private:
    Ui::AutoBoot *ui;

    QString pluginName;
    int pluginType;
    QWidget *pluginWidget;

//    AddAutoBoot *dialog;
    QDBusInterface *m_cloudInterface;

    QMap<QString, AutoApp> appMaps;
    QMap<QString, AutoApp> localappMaps;
    QMap<QString, AutoApp> statusMaps;
    QMultiMap<QString, QWidget *> appgroupMultiMaps;

    AddBtn *addWgt;

    TitleLabel *mTitleLabel;
    QFrame *mAutoBootFrame;
    QVBoxLayout *mAutoBootLayout;

    bool mFirstLoad;

    QGSettings *mQtSettings;

    QStringList whitelist;

public slots:
    void checkboxChangedSlot(QString bname);
    void keyChangedSlot(const QString &key);
    void addAutobootRealizeSlot(QString path, QString name, QString exec, QString comment,
                                   QString icon);
    void openDesktopDirSlot();

Q_SIGNALS:
    void autobootAddingSignal(QString path, QString name, QString exec, QString comment, QString icon);
};

#endif // AUTOBOOT_H

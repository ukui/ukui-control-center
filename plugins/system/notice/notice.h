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
#ifndef NOTICE_H
#define NOTICE_H

#include <QObject>
#include <QtPlugin>
#include <QPushButton>
#include <QDebug>
#include <QVector>
#include <QGSettings>
#include <QVBoxLayout>
#include <QDir>

#include <shell/interface.h>
#include <ukcc/widgets/switchbutton.h>
#include <ukcc/widgets/titlelabel.h>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

QT_BEGIN_NAMESPACE
namespace Ui { class Notice; }
QT_END_NAMESPACE

class Notice : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Notice();
    ~Notice();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

    void initUi(QWidget *widget);
    void initSearchText();
    void setupComponent();
    void setupGSettings();
    void initNoticeStatus();
    void initOriNoticeStatus();
    void initListUI(QDir dir,QString mpath,QStringList *stringlist);


private:
    void setHiddenNoticeApp(bool status);

private:
    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

    TitleLabel *mNoticeLabel;
    QLabel *mGetNoticeLabel;

    QFrame *mNoticeAppFrame;
    QFrame *mGetNoticeFrame;

    SwitchButton * newfeatureSwitchBtn;
    SwitchButton * enableSwitchBtn;
    SwitchButton * lockscreenSwitchBtn;       

    QMap<QString, bool> appMap;

    QGSettings * nSetting;
    QGSettings * mThemeSetting;
    QGSettings * oriSettings;
    QStringList whitelist;
    QVector<QGSettings*> vecGsettins;

    QVBoxLayout *applistverticalLayout;

    QStringList *mstringlist;

    QList<char *> listChar;

    QStringList mblacklist;

    bool mFirstLoad;
    bool isCN_env;
    bool mEnv;

    QString mlocale;

    int count = 0;
    int mcount = 0;

public slots:
    void loadlist();

};
#endif // NOTICE_H

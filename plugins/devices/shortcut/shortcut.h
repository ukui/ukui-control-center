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
#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QObject>
#include <QtPlugin>
#include <QProcess>
#include <QThread>
#include <QListWidget>
#include <QDBusInterface>
#include <QPushButton>

#include "shell/interface.h"
#include "keymap.h"
#include "addshortcutdialog.h"
#include "getshortcutworker.h"
#include <ukcc/widgets/hoverwidget.h>
#include <ukcc/widgets/imageutil.h>
#include <ukcc/widgets/addbtn.h>
#include "shell/utils/mthread.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Shortcut;
}
QT_END_NAMESPACE

class Shortcut : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Shortcut();
    ~Shortcut();

public:
    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    const QString name() const Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

public:
    void setupComponent();
    void setupConnect();
    void initFunctionStatus();

    void appendGeneralItems(QMap<QString, QMap<QString, QString> > shortcutsMap);
    void appendCustomItems();
    void buildCustomItem(KeyEntry *nkeyEntry);
    QWidget *buildGeneralWidget(QString schema, QMap<QString, QString> subShortcutsMap);

    void createNewShortcut(QString path, QString name, QString exec, QString key, bool buildFlag = true, bool convertFlag = true);
    void deleteCustomShortcut(QString path);

    bool keyIsForbidden(QString key);
    void connectToServer();
    QMap<QString, QString> MergerOfTheSamekind(QMap<QString, QString> desktopMap);

    QString keyToUI(QString key);
    QString keyToLib(QString key);
    QString getShowShortcutString(QString src);

private:
    Ui::Shortcut *ui;

    QString pluginName;
    int pluginType;
    QWidget *pluginWidget;

private:
    QThread *pThread;
    GetShortcutWorker *pWorker;

    KeyMap *pKeyMap;

    QDBusInterface *cloudInterface;
    bool isCloudService;

    bool mFirstLoad;

private slots:
    void shortcutChangedSlot();

Q_SIGNALS:
    void hideDelBtn();
    void updateCustomShortcut();
};
#endif // SHORTCUT_H

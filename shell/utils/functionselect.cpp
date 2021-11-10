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
#include "functionselect.h"
#include "../interface.h"

#include <QDebug>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif
extern "C" {
#include <glib.h>
#include <gio/gio.h>
#include <dconf/dconf.h>
}

QList<QList<FuncInfo>> FunctionSelect::funcinfoList;
QList<QList<FuncInfo>> FunctionSelect::funcinfoListHomePage;
QStack<RecordFunc> FunctionSelect::recordFuncStack;

//FuncInfo FunctionSelect::displayStruct;


FunctionSelect::FunctionSelect()
{
}

FunctionSelect::~FunctionSelect()
{

}

void FunctionSelect::loadHomeModule()
{
    QList<FuncInfo> systemList, deviceList, networkList, personaliseList, accountList,
                    datetimeList, updateList, securityList, applicationList, searchList;
    bool installed = (QCoreApplication::applicationDirPath() == QDir(("/usr/bin")).canonicalPath());
    QDir pluginsDir;
    if (installed)
        pluginsDir = QDir(PLUGIN_INSTALL_DIRS);
    else {
        pluginsDir = QDir(qApp->applicationDirPath() + "/plugins");
    }

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        if (fileName.endsWith(".so")) {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            QObject * plugin = loader.instance();
            if (plugin) {
                CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);

                switch (pluginInstance->pluginTypes()) {
                case FunType::SYSTEM:
                    loadModule(systemList, pluginInstance->name(), pluginInstance->plugini18nName(), pluginInstance->pluginTypes(), pluginInstance->isShowOnHomePage());
                    break;
                case FunType::DEVICES:
                    loadModule(deviceList, pluginInstance->name(), pluginInstance->plugini18nName(), pluginInstance->pluginTypes(), pluginInstance->isShowOnHomePage());
                    break;
                case FunType::NETWORK:
                    loadModule(networkList, pluginInstance->name(), pluginInstance->plugini18nName(), pluginInstance->pluginTypes(), pluginInstance->isShowOnHomePage());
                    break;
                case FunType::PERSONALIZED:
                    loadModule(personaliseList, pluginInstance->name(), pluginInstance->plugini18nName(), pluginInstance->pluginTypes(), pluginInstance->isShowOnHomePage());
                    break;
                case FunType::ACCOUNT:
                    loadModule(accountList, pluginInstance->name(), pluginInstance->plugini18nName(), pluginInstance->pluginTypes(), pluginInstance->isShowOnHomePage());
                    break;
                case FunType::DATETIME:
                    loadModule(datetimeList, pluginInstance->name(), pluginInstance->plugini18nName(), pluginInstance->pluginTypes(), pluginInstance->isShowOnHomePage());
                    break;
                case FunType::UPDATE:
                    loadModule(updateList, pluginInstance->name(), pluginInstance->plugini18nName(), pluginInstance->pluginTypes(), pluginInstance->isShowOnHomePage());
                    break;
                case FunType::SECURITY:
                    loadModule(securityList, pluginInstance->name(), pluginInstance->plugini18nName(), pluginInstance->pluginTypes(), pluginInstance->isShowOnHomePage());
                    break;
                case FunType::APPLICATION:
                    loadModule(applicationList, pluginInstance->name(), pluginInstance->plugini18nName(), pluginInstance->pluginTypes(), pluginInstance->isShowOnHomePage());
                    break;
                case FunType::SEARCH_F:
                    loadModule(searchList, pluginInstance->name(), pluginInstance->plugini18nName(), pluginInstance->pluginTypes(), pluginInstance->isShowOnHomePage());
                    break;
                default:
                    break;
                }
            }
        }
    }
    funcinfoListHomePage.append(systemList);
    funcinfoListHomePage.append(deviceList);
    funcinfoListHomePage.append(networkList);
    funcinfoListHomePage.append(personaliseList);
    funcinfoListHomePage.append(accountList);
    funcinfoListHomePage.append(datetimeList);
    funcinfoListHomePage.append(updateList);
    funcinfoListHomePage.append(securityList);
    funcinfoListHomePage.append(applicationList);
    funcinfoListHomePage.append(searchList);


    funcinfoList = funcinfoListHomePage;
}

void FunctionSelect::loadModule(QList<FuncInfo> &systemList, QString name, QString i18nName, int type, bool isShow)
{
    FuncInfo module;
    module.nameString = name;
    module.namei18nString = i18nName;
    module.type = type;
    module.mainShow = isShow;
    systemList.append(module);
}

void FunctionSelect::initValue(){
    loadHomeModule();
}

void FunctionSelect::pushRecordValue(int type, QString name){
    RecordFunc reFuncStruct;
    reFuncStruct.type = type;
    reFuncStruct.namei18nString = name;
    if (recordFuncStack.length() < 1)
        recordFuncStack.push(reFuncStruct);
    else if (QString::compare(recordFuncStack.last().namei18nString, name) != 0){
//        qDebug() << recordFuncStack.last().namei18nString << name;
        recordFuncStack.push(reFuncStruct);
    }
}

void FunctionSelect::popRecordValue() {
    if (!recordFuncStack.isEmpty()) {
        recordFuncStack.pop();
    }
}

QList<char *> FunctionSelect::listExistsCustomNoticePath(const char *dir)
{
    char ** childs;
    int len;

    DConfClient * client = dconf_client_new();
    childs = dconf_client_list (client, dir, &len);
    g_object_unref (client);

    QList<char *> vals;

    for (int i = 0; childs[i] != NULL; i++){
        if (dconf_is_rel_dir (childs[i], NULL)){
            char * val = g_strdup (childs[i]);
            vals.append(val);
        }
    }
    g_strfreev (childs);
    return vals;
}


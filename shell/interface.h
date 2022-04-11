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
#ifndef INTERFACE_H
#define INTERFACE_H

#include <QPushButton>
#include <QFile>
#include <QIcon>

class QString;
class QWidget;
class QStringLiteral;

enum FunType{
    SYSTEM,
    DEVICES,
    NETWORK,
    PERSONALIZED,
    ACCOUNT,
    DATETIME,
    UPDATE,
    SECURITY,
    APPLICATION,
    SEARCH_F,
    //NOTICEANDTASKS,
    TOTALMODULES,
};

class CommonInterface{
public:
    virtual ~CommonInterface(){}
    virtual QString plugini18nName() = 0;         // 插件名字
    virtual int pluginTypes() = 0;                // 插件类型
    virtual QWidget * pluginUi() = 0;             // 插件主界面
    virtual bool isShowOnHomePage() const = 0;    // 是否显示在首页
    virtual QIcon icon() const = 0;               // 图标
    virtual bool isEnable() const = 0;            // 插件是否启用
    virtual const QString name() const = 0;       // 模块名称
    virtual QString translationPath() const {     // 获取多语言文件路径，用于搜索
        return QStringLiteral(":/i18n/%1.ts");
    }
    QPushButton *pluginBtn = nullptr;

    bool isIntel() const
    {
        QString sysVersion = "/etc/apt/ota_version";
        QFile file(sysVersion);
        return file.exists();
    }
    virtual void plugin_leave() {
        return ;
    }
    virtual void settingForIntel() {
        return;
    }
};

#define CommonInterface_iid "org.ukcc.CommonInterface"

Q_DECLARE_INTERFACE(CommonInterface, CommonInterface_iid)

#endif // INTERFACE_H

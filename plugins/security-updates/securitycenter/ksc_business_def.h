/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef KSC_BUSINESS_DEF_H
#define KSC_BUSINESS_DEF_H

#include <QDBusArgument>
#include <QDBusInterface>
#include <QDBusReply>
#include <QtDBus/qdbusmetatype.h>
#include <QtDBus>

enum DEFENDER_MODULE_TYPE{
    DEFENDER_SCAN = 0,
    DEFENDER_ACCOUNT,
    DEFENDER_NETWORK,
    DEFENDER_VIRUS,
    DEFENDER_CONTROL,
    DEFENDER_MODULE_TYPE_MAX,  //这个类型的数量
    DEFEDNER_MODULE_TYPE_BEGIN = DEFENDER_SCAN
};

typedef struct ksc_defender_module {
    QString ksc_name;           // 安全中心名称
    QString ksc_desc;           // 安全中心描述

    int module_type;
    QString module_name;            // 模块名称
    QString module_desc;            // 模块描述
    QString module_normal_icon;     // 正常模式下图标
    QString module_hover_icon;      // 悬停图标
    QString module_exec;            // 命令参数
    int module_status;              // 模块状态：正常 = 0/警告 = 1
    QStringList status_list;        // 模块状态描述列表

    ksc_defender_module()
    {
    }

    ksc_defender_module(const QString &name, const QString &des, const int type,
                        const QString &str1, const QString &str2, const QString &str3,
                        const QString &str4, const QString &str5,
                        const int status /*, QStringList strlist*/) :
        ksc_name(name),
        ksc_desc(des),
        module_type(type),
        module_name(str1),
        module_desc(str2),
        module_normal_icon(str3),
        module_hover_icon(str4),
        module_exec(str5),
        module_status(status)
    {
    }

    friend QDBusArgument &operator<<(QDBusArgument &argument, const ksc_defender_module &cfg)
    {
        argument.beginStructure();

        argument << cfg.ksc_name;
        argument << cfg.ksc_desc;

        argument << cfg.module_type;
        argument << cfg.module_name;
        argument << cfg.module_desc;
        argument << cfg.module_normal_icon;
        argument << cfg.module_hover_icon;
        argument << cfg.module_exec;
        argument << cfg.module_status;
        argument << cfg.status_list;

        argument.endStructure();

        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, ksc_defender_module &cfg)
    {
        argument.beginStructure();

        argument >> cfg.ksc_name;
        argument >> cfg.ksc_desc;

        argument >> cfg.module_type;
        argument >> cfg.module_name;
        argument >> cfg.module_desc;
        argument >> cfg.module_normal_icon;
        argument >> cfg.module_hover_icon;
        argument >> cfg.module_exec;
        argument >> cfg.module_status;
        argument >> cfg.status_list;

        argument.endStructure();

        return argument;
    }
}ksc_defender_module;
Q_DECLARE_METATYPE(ksc_defender_module)

typedef QList<ksc_defender_module> ksc_defender_module_list;

Q_DECLARE_METATYPE(ksc_defender_module_list)

#endif // KSC_BUSINESS_DEF_H

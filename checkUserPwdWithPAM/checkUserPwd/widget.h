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

#ifndef WIDGET_H
#define WIDGET_H


#include "auth-pam.h"


class Widget : public QObject
{
    Q_OBJECT

public:
    Widget();
    ~Widget();

public:
    void pwdCheck(QString userName, QString userPwd);

private:
    Auth * auth;

    bool accountlock;

private Q_SLOTS:
    void onShowMessage(const QString &message, Auth::MessageType type);
    void onShowPrompt(const QString &prompt, Auth::PromptType type);
    void onAuthComplete();
};
#endif // WIDGET_H

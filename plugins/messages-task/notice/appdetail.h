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

#ifndef APPDETAIL_H
#define APPDETAIL_H

#include <QDialog>
#include <QGSettings>
#include <QPainter>

#include "SwitchButton/switchbutton.h"

#define MESSAGES_KEY            "messages"
#define VOICE_KEY               "voice"
#define MAXIMINE_KEY            "maximize"
#define NAME_KEY_US             "name-us"
#define NAME_KEY_CN             "name-cn"

namespace Ui {
class AppDetail;
}

class AppDetail : public QDialog
{
    Q_OBJECT

public:
    explicit AppDetail(QString Name, QString key, QGSettings *gsettings, QWidget *parent = nullptr);
    ~AppDetail();

private:
    Ui::AppDetail *ui;
    QString appName;
    QString appKey;
    SwitchButton * enablebtn;
    QGSettings * m_gsettings;

private:
    void initUiStatus();
    void initComponent();
    void initConnect();

protected:
    void paintEvent(QPaintEvent *);

private slots:
    void confirmbtnSlot();
};

#endif // APPDETAIL_H

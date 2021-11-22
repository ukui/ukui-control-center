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

#ifndef CHANGEUSERNAME_H
#define CHANGEUSERNAME_H

#include <QDialog>
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>

namespace Ui {
class ChangeUserName;
}

class ChangeUserName : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeUserName(QStringList usernames, QStringList realnames, QWidget *parent = nullptr);
    ~ChangeUserName();

private:
    QStringList _usernames;
    QStringList _realnames;

private:
    Ui::ChangeUserName *ui;

protected:
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);

Q_SIGNALS:
    void sendNewName(QString name);
};

#endif // CHANGEUSERNAME_H

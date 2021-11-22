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

#ifndef COMBOXFRAME_H
#define COMBOXFRAME_H

#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>

class ComboxFrame : public QFrame
{
    Q_OBJECT
public:
    ComboxFrame(QString labelStr, QWidget *parent = nullptr);
    ComboxFrame(bool isNum, QString labelStr, QWidget *parent = nullptr);
    ~ComboxFrame();

public:
    QComboBox   *mCombox;
    QComboBox   *mNumCombox;
    QLabel      *mTitleLabel;
    QHBoxLayout *mHLayout;

private:
    QString     mTitleName;

signals:
};

#endif // COMBOXFRAME_H

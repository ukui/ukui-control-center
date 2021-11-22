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

#ifndef LANGUAGEFRAME_H
#define LANGUAGEFRAME_H

//添加语言功能未完善，所以暂时只考虑中英文的情况切换。

#include <QFrame>
#include <QLabel>

class LanguageFrame : public QFrame
{
    Q_OBJECT
public:
    LanguageFrame(QString name, QWidget *parent = nullptr);
    ~LanguageFrame();
    void showSelectedIcon(bool flag);
private:
    void mousePressEvent(QMouseEvent *e);

private:
    QLabel *nameLabel         = nullptr;
    QLabel *selectedIconLabel = nullptr;

Q_SIGNALS:
    void clicked();
};


#endif // LANGUAGEFRAME_H

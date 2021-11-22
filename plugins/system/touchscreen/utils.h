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

#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QSize>

#include <KF5/KScreen/kscreen/types.h>
#include <KF5/KScreen/kscreen/output.h>

//获取显示器名字和ID类
namespace Utils
{
    QString outputName(const KScreen::Output *output);
    QString outputName(const KScreen::OutputPtr &output);

    QString sizeToString(const QSize &size);
}


#endif // UTILS_H

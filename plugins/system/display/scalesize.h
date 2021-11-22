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

#ifndef SCALESIZE_H
#define SCALESIZE_H

#include <QSize>

const QSize KRsolution(1920, 1080);

const QVector<QSize> k150Scale{QSize(1280, 1024), QSize(1440, 900),  QSize(1600, 900),
                               QSize(1680, 1050), QSize(1920, 1080), QSize(1920, 1200),
                               QSize(1680, 1280), QSize(2048, 1080), QSize(2048, 1280),
                               QSize(2160, 1440), QSize(2560, 1440), QSize(3840, 2160)};

const QVector<QSize> k175Scale{QSize(2048, 1080), QSize(2048, 1280), QSize(2160, 1440),
                               QSize(2560, 1440), QSize(3840, 2160)};

const QVector<QSize> k200Scale{QSize(2048, 1080),QSize(2048, 1280), QSize(2160, 1440),
                               QSize(2560, 1440), QSize(3840, 2160)};

const QVector<QSize> k250Scale{QSize(2560, 1440), QSize(3840, 2160)};

const QVector<QSize> k275Scale{QSize(3840, 2160)};

#define SCALE_SCHEMAS "org.ukui.SettingsDaemon.plugins.xsettings"
#define SCALE_KEY     "scaling-factor"

extern QSize mScaleSize;

extern double mScaleres;

#endif // SCALESIZE_H

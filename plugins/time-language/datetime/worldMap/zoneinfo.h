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

#ifndef ZONEINFO_H
#define ZONEINFO_H

#include <QFile>
#include <QString>
#include <QList>

struct ZoneInfo_ {
    QString country;
    QString timezone;

    double latitude;
    double longtitude;
    double distance;
};
typedef QList<ZoneInfo_> ZoneinfoList;

class ZoneInfo {
public:
    QString readRile(const QString& filepath);
    ZoneinfoList getzoneInforList();
    QString getCurrentTimzone();

    QString getLocalTimezoneName(QString timzone, QString locale);
    int getZoneInfoByZone(ZoneinfoList list, QString timezone);



    double convertoPos(const QString& pos, int num);


    //gnome-control-center
    double radians(double degrees);
    double converLatitudeToY(double latitude, double map_height);
    double converLongtitudeToX(double longitude, double map_width);

    // 在宽度：mapWidth,高度mapHeight地图上获得鼠标周围小于阈值的所有时区
    ZoneinfoList getNearestZones(ZoneinfoList allZones,double threshold,
                                 int x, int y, int mapWidth, int mapHeight);



};
#endif // ZONEINFO_H

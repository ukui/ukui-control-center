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

#ifndef TIMEZONEMAP_H
#define TIMEZONEMAP_H

#include <QFrame>
#include <QLabel>
#include <QResizeEvent>
#include <QStringListModel>


#include "zoneinfo.h"
#include "poplist.h"
#include "toolpop.h"

QDebug& operator<<(QDebug& debug, const ZoneInfo& info);

class TimezoneMap : public QFrame
{
    Q_OBJECT
public:
    explicit TimezoneMap(QWidget* parent = nullptr);
    ~TimezoneMap();

    QString getTimezone() ;//获得选择的时区

Q_SIGNALS:
    void timezoneSelected(QString timezone);
public slots:
    void setTimezone(QString timezone);

protected:
    void mousePressEvent(QMouseEvent* event);

    void resizeEvent(QResizeEvent* event);
private:
    void initUI();
    void mark(); //标记
    void popupZoneList(QPoint pos);

    QPoint zoneInfoToPosition(ZoneInfo_ zone, int mapWidth, int mapHeight);

private:
    ZoneInfo*    m_zoninfo;
    ZoneInfo_    m_currentZone;
    ZoneinfoList m_totalZones;
    ZoneinfoList m_nearestZones;

    // 圆点
    QLabel* m_dot = nullptr;

    PopList* m_popList;
    ToolPop* m_singleList;
    PopList* m_popLists;
    QPixmap loadPixmap(const QString &path);

    QLabel* backgroundLabel;

private slots:
    void popListActiveSlot(int index);
};

#endif // TIMEZONEMAP_H

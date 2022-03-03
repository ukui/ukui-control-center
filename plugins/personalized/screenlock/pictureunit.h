/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef PICTUREUNIT_H
#define PICTUREUNIT_H

#include <QLabel>
#include <QMouseEvent>
#include <QGSettings>
class PictureUnit : public QLabel
{
    Q_OBJECT

public:
    explicit PictureUnit();
    ~PictureUnit();

public:
    void setFilenameText(QString fn);
    QString filenameText();
    bool getClickedFlag();
    void changeClickedFlag(bool clicked);

public:
    QString clickedStyleSheet;

protected:
    void mouseReleaseEvent(QMouseEvent * e);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

private:
    QString _filename;
    bool settingsCreate;
    QGSettings *dgsettings;

    QString hoverStyleSheet;

    bool clickedFlag;
Q_SIGNALS:
    void clicked(QString filename);

};

#endif // PICTUREUNIT_H

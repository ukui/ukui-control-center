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

#ifndef QL_ANIMATION_LABEL_H
#define QL_ANIMATION_LABEL_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include "svghandler.h"

class Blueeffect : public QWidget
{
    Q_OBJECT
public:
    explicit Blueeffect(QWidget *parent = nullptr);
    ~ Blueeffect();
    QLabel  *m_textLabel;
    QLabel  *m_iconLabel;
    void settext(const QString &t);
    void startmoive();
    void stop();
    QTimer *m_cTimer;
    QHBoxLayout *m_workLayout;
    SVGHandler *m_svgHandler;
    int m_cCnt = 1;
signals:

};

#endif // QL_ANIMATION_LABEL_H

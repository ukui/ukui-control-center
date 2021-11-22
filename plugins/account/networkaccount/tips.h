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

#ifndef QL_LABEL_INFO_H
#define QL_LABEL_INFO_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QSvgWidget>
#include <QGraphicsSvgItem>

class Tips : public QWidget
{
    Q_OBJECT
public:
    void set_text(const QString &m_textLabel);
    explicit Tips(QWidget *parent = nullptr);
    ~Tips();
private:
    QSvgWidget *m_iconWidget;
    QLabel *m_textLabel;
    QString m_szContext;
    QHBoxLayout *m_workLayout;
signals:

};

#endif // QL_LABEL_INFO_H

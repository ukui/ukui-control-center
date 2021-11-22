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

#ifndef PRESCENE_H
#define PRESCENE_H

#include <QSize>
#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>


class PreScene : public QWidget
{
public:
    PreScene(QLabel *label, QSize size, QWidget * parent = nullptr);
    ~PreScene();

private:
    QSize m_size;
    QHBoxLayout * m_hlayout;

    QVBoxLayout * m_vlayout;
    QWidget * titlebar;

    QLabel  * mTitleIcon;
    QLabel  * titleLabel;
    QLabel  * logoLabel;
    QHBoxLayout * m_logoLayout = nullptr;

protected:
    void paintEvent(QPaintEvent *event);
private:
    const QPixmap loadSvg(const QString &fileName);
};

#endif // PRESCENE_H

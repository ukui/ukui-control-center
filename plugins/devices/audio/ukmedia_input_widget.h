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
#ifndef UKMEDIAINPUTWIDGET_H
#define UKMEDIAINPUTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include "ukmedia_output_widget.h"
#include <QStyledItemDelegate>
#include <QFrame>
#include <QProgressBar>
#include <QListWidget>
#include "ukui_custom_style.h"

class UkmediaInputWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UkmediaInputWidget(QWidget *parent = nullptr);
    ~UkmediaInputWidget();
    friend class UkmediaMainWidget;
Q_SIGNALS:

private:
    QWidget *m_pInputWidget;
    QFrame *m_pInputDeviceWidget;
    QFrame *m_pVolumeWidget;
    QFrame *m_pInputLevelWidget;

    QListWidget *m_pInputListWidget;

    TitleLabel *m_pInputLabel;
    QLabel *m_pInputDeviceLabel;
    QLabel *m_pIpVolumeLabel;
    QLabel *m_pInputLevelLabel;
    QLabel *m_pIpVolumePercentLabel;

    UkuiButtonDrawSvg *m_pInputIconBtn;
    AudioSlider *m_pIpVolumeSlider;
    QProgressBar *m_pInputLevelProgressBar;

    QString sliderQss;
    QVBoxLayout *m_pVlayout;

};

#endif // UKMEDIAINPUTWIDGET_H

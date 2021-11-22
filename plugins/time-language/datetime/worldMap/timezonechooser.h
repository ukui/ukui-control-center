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

#ifndef TIMEZONECHOOSER_H
#define TIMEZONECHOOSER_H

#include "timezonemap.h"
#include <QFrame>
#include <QMap>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QAbstractItemView>
#include <QPropertyAnimation>
#include <QDialog>

class TimeZoneChooser : public QDialog
{
    Q_OBJECT
public:
    explicit TimeZoneChooser(QWidget *parent);
    void setTitle(QString title);

public slots:
    void setMarkedTimeZoneSlot(QString timezone);

Q_SIGNALS:
    void confirmed(QString zone);
    void cancelled();

protected:
    void keyPressEvent(QKeyEvent* event);
    void keyRealeaseEvent(QKeyEvent* event);
    bool eventFilter(QObject* obj, QEvent* event);

private:
    QSize getFitSize();
    void initSize();
    void paintEvent(QPaintEvent *event);

private:
    ZoneInfo* m_zoneinfo;

    QMap<QString, QString> m_zoneCompletion;
    QAbstractItemView      *m_popup;

    TimezoneMap* m_map;
    QLineEdit* m_searchInput;
    QLabel      *m_title;
    QPushButton *m_cancelBtn;
    QPushButton *m_confirmBtn;
    QPushButton *m_closeBtn;

    QLabel            *m_queryIcon;
    QLabel            *m_queryText  = nullptr;
    QPropertyAnimation *m_animation = nullptr;
    QWidget           *m_queryWid   = nullptr;
    bool              m_isSearching = false;
    QString           m_searchKeyWords;

public slots:
    void animationFinishedSlot();
};

#endif // TIMEZONECHOOSER_H

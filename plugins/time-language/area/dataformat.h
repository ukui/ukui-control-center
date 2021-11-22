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

#ifndef DATAFORMAT_H
#define DATAFORMAT_H

#include <QTimer>
#include <QDialog>
#include <QGSettings>
#include <QListView>
#include <QFile>
#include <QPainter>
#include <QPainterPath>

namespace Ui {
class DataFormat;
}

class DataFormat : public QDialog
{
    Q_OBJECT

public:
    explicit DataFormat(QWidget *parent = nullptr);
    ~DataFormat();

protected:
    void paintEvent(QPaintEvent *);

private:
    QTimer *m_itimer;
    QGSettings *m_gsettings = nullptr;
    QString qss;
    QString locale;

private:
    Ui::DataFormat *ui;

    void initUi();
    void initConnect();
    void initComponent();
    void writeGsettings(const QString &key, const QString &value);

Q_SIGNALS:
    void dataChangedSignal();

private slots:
    void datetime_update_slot();
    void confirm_btn_slot();
};

#endif // DATAFORMAT_H

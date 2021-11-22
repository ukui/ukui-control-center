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

#ifndef BLUETOOTHNAMELABEL_H
#define BLUETOOTHNAMELABEL_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>
#include <QObject>
#include <QGSettings/QGSettings>
#include <QIcon>
#include <QHBoxLayout>
#include <QMessageBox>

#define DEVNAMELENGTH 20

class BluetoothNameLabel : public QWidget
{
    Q_OBJECT
public:
    BluetoothNameLabel(QWidget *parent = nullptr, int x = 200,int y = 40);
    ~BluetoothNameLabel();
    void set_dev_name(const QString &dev_name);
    void dev_name_limit_fun();
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    void enterEvent(QEvent *event);

signals:
    void send_adapter_name(const QString &value);

public slots:
    void LineEdit_Input_Complete();
    void set_label_text(const QString &value);
    void settings_changed(const QString &key);

private:
    QGSettings *settings;
    bool style_flag = false;

    QLabel *m_label = nullptr;
    QLabel *icon_pencil=nullptr;

    QLineEdit *m_lineedit = nullptr;
    QString device_name;
    int font_width;

    QMessageBox *messagebox = nullptr;
    QHBoxLayout *hLayout = nullptr;
};

#endif // BLUETOOTHNAMELABEL_H

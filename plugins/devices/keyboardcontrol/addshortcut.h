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
#ifndef ADDSHORTCUT_H
#define ADDSHORTCUT_H

#include <QDialog>
#include <QFileDialog>

namespace Ui {
class AddShortcut;
}

class AddShortcut : public QDialog
{
    Q_OBJECT

public:
    explicit AddShortcut(QWidget *parent = 0);
    ~AddShortcut();

    void update_dialog_set(QString gsettings_path, QString name, QString exec);
    void update_dialog_unset();

    void refresh_certainbtn_status();

private:
    Ui::AddShortcut *ui;

    QString gspath;

private slots:
    void open_filedialog_slot();

Q_SIGNALS:
    void program_info_signal(QString path, QString name, QString exec);
};

#endif // ADDSHORTCUT_H

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
#include "addautoboot.h"
#include "ui_addautoboot.h"

#include <QDebug>

//#define DESKTOPPATH "/etc/xdg/autostart/"
#define DESKTOPPATH "/usr/share/applications/"

AddAutoBoot::AddAutoBoot(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddAutoBoot)
{
    ui->setupUi(this);
    selectFile = "";

    connect(ui->openBtn, SIGNAL(clicked(bool)), this, SLOT(open_desktop_dir_slots()));
    connect(ui->cancelBtn, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->certainBtn, &QPushButton::clicked, this, [=]{emit autoboot_adding_signals(selectFile, ui->nameLineEdit->text(), ui->execLineEdit->text(), ui->commentLineEdit->text());close();});
}

AddAutoBoot::~AddAutoBoot()
{
    delete ui;
}

void AddAutoBoot::open_desktop_dir_slots(){
    QString filters = "Desktop files(*.desktop)";
    QFileDialog fd;
    fd.setDirectory(DESKTOPPATH);
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilter(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("selsect autoboot desktop"));
    fd.setLabelText(QFileDialog::Accept, "Select");

    if (fd.exec() != QDialog::Accepted)
        return;

    QString selectedfile;
    selectedfile = fd.selectedFiles().first();
    selectFile = selectedfile;

    QByteArray ba;
    ba = selectedfile.toUtf8();

    //解析desktop文件
    GKeyFile * keyfile;
    char *name, * comment, * exec;

    keyfile = g_key_file_new();
    if (!g_key_file_load_from_file(keyfile, ba.data(), G_KEY_FILE_NONE, NULL)){
        g_key_file_free (keyfile);
        return;
    }

    name = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
    comment = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL, NULL);
    exec = g_key_file_get_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, NULL);

//    if (ui->nameLineEdit->text().isEmpty())
        ui->nameLineEdit->setText(QString(name));
//    if (ui->execLineEdit->text().isEmpty())
        ui->execLineEdit->setText(QString(exec));
//    if (ui->commentLineEdit->text().isEmpty())
        ui->commentLineEdit->setText(QString(comment));

    g_key_file_free(keyfile);
}

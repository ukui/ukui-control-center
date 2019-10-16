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
#include "changefacedialog.h"
#include "ui_changefacedialog.h"

#include <QDebug>

#define FACEPATH "/usr/share/pixmaps/faces/"

ChangeFaceDialog::ChangeFaceDialog(QWidget *parent) :
    ui(new Ui::ChangeFaceDialog)
{
    this->setParent(parent);
    ui->setupUi(this);

    show_faces();
    connect(ui->customfaceBtn, SIGNAL(clicked(bool)), this, SLOT(custom_face_choosed_slot()));
}

ChangeFaceDialog::~ChangeFaceDialog()
{
    delete ui;
}

void ChangeFaceDialog::show_faces(){

    //遍历头像目录
    QStringList facesList;
    QDir facesDir = QDir(FACEPATH);
    foreach (QString filename, facesDir.entryList(QDir::Files)){
        facesList.append(FACEPATH + filename);
    }

    //设置listwidget
    QSize IMAGE_SIZE(64, 64);
    QSize ITEM_SIZE(70, 70);
    ui->listWidget->setIconSize(IMAGE_SIZE);
    ui->listWidget->setResizeMode(QListView::Adjust);
    ui->listWidget->setViewMode(QListView::IconMode);
    ui->listWidget->setMovement(QListView::Static);
    ui->listWidget->setSpacing(5);

    for (int row = 0; row < facesList.count(); row++){
        QPixmap pixmap(facesList[row]);
        QListWidgetItem * item = new QListWidgetItem(QIcon(pixmap.scaled(IMAGE_SIZE)), "");
        item->setSizeHint(ITEM_SIZE);
        item->setData(Qt::UserRole, facesList[row]);
        delitemMap.insert(facesList[row], item);
        ui->listWidget->insertItem(row, item);
    }
}

void ChangeFaceDialog::set_face_label(QString iconfile){
    ui->faceLabel->setPixmap(QPixmap(iconfile).scaled(QSize(80, 80)));
}

void ChangeFaceDialog::set_username_label(QString username){
    ui->usernameLabel->setText(username);
}

void ChangeFaceDialog::set_account_type_label(QString atype){
    ui->typeLabel->setText(atype);
}

void ChangeFaceDialog::set_face_list_status(QString facefile){
    QMap<QString, QListWidgetItem *>::iterator it = delitemMap.find(facefile);

    if (it != delitemMap.end()){
        QListWidgetItem * item = (QListWidgetItem *) it.value();
        ui->listWidget->setCurrentItem(item);
    }
    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(item_changed_slot(QListWidgetItem *, QListWidgetItem * )));
}

void ChangeFaceDialog::item_changed_slot(QListWidgetItem * current, QListWidgetItem *previous){
    QString facefile = current->data(Qt::UserRole).toString();

    //show dialog更新头像
    set_face_label(facefile);

    emit face_file_send(facefile, ui->usernameLabel->text());
}

void ChangeFaceDialog::custom_face_choosed_slot(){
    QString filters = "Face files(*.png *.jpg *.svg)";
    QFileDialog fd;
    fd.setDirectory(QString(const_cast<char *>(g_get_user_special_dir(G_USER_DIRECTORY_PICTURES))));
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilter(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("selsect custom face file"));
    fd.setLabelText(QFileDialog::Accept, tr("Select"));
    fd.setLabelText(QFileDialog::LookIn, tr("Position: "));
    fd.setLabelText(QFileDialog::FileName, tr("FileName: "));
    fd.setLabelText(QFileDialog::FileType, tr("FileType: "));
    fd.setLabelText(QFileDialog::Reject, tr("Cancel"));

    if (fd.exec() != QDialog::Accepted)
        return;

    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    set_face_label(selectedfile);
    emit face_file_send(selectedfile, ui->usernameLabel->text());

}

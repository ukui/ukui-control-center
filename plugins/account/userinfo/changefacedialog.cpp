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

#include "FlowLayout/flowlayout.h"

#include <QDebug>

#define FACEPATH "/usr/share/ukui/faces/"

ChangeFaceDialog::ChangeFaceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeFaceDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->frame->setStyleSheet("QFrame{background: #ffffff; border: none; border-radius: 6px;}");
    ui->closeBtn->setStyleSheet("QPushButton{background: #ffffff; border: none;}");

    ui->closeBtn->setIcon(QIcon("://img/titlebar/close.png"));

    loadSystemFaces();


    connect(ui->closeBtn, &QPushButton::clicked, [=]{
        close();
    });
    connect(ui->customfaceBtn, &QPushButton::clicked, [=]{
        showLocalFaceDialog();
    });
}

ChangeFaceDialog::~ChangeFaceDialog()
{
    delete ui;
}

void ChangeFaceDialog::loadSystemFaces(){

    FlowLayout * facesFlowLayout = new FlowLayout(ui->facesWidget);
    ui->facesWidget->setLayout(facesFlowLayout);

    //遍历头像目录
    QStringList facesList;
    QDir facesDir = QDir(FACEPATH);
    foreach (QString filename, facesDir.entryList(QDir::Files)){
//        facesList.append(FACEPATH + filename);
        QString fullface = QString("%1%2").arg(FACEPATH).arg(filename);

        QPushButton * button = new QPushButton;
        button->setAttribute(Qt::WA_DeleteOnClose);
        button->setFixedSize(QSize(48, 48));

        QHBoxLayout * mainHorLayout = new QHBoxLayout(button);
        mainHorLayout->setSpacing(0);
        mainHorLayout->setMargin(0);
        QLabel * iconLabel = new QLabel(button);
        iconLabel->setScaledContents(true);
        iconLabel->setPixmap(QPixmap(fullface));

        mainHorLayout->addWidget(iconLabel);

        button->setLayout(mainHorLayout);

        connect(button, &QPushButton::clicked, [=]{
            //show dialog更新头像
            setFace(fullface);

            emit face_file_send(fullface, ui->usernameLabel->text());
        });

        facesFlowLayout->addWidget(button);
    }
}

void ChangeFaceDialog::setFace(QString iconfile){
    ui->faceLabel->setPixmap(QPixmap(iconfile));
}

void ChangeFaceDialog::setUsername(QString username){
    ui->usernameLabel->setText(username);
}

void ChangeFaceDialog::setAccountType(QString atype){
    ui->typeLabel->setText(atype);
}

void ChangeFaceDialog::showLocalFaceDialog(){
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

    setFace(selectedfile);
    emit face_file_send(selectedfile, ui->usernameLabel->text());

}

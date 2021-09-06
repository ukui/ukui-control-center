/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2020 KYLINOS Information Technology Co., Ltd.
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

#include "creategroupinteldialog.h"
#include "ui_creategroupinteldialog.h"
#include "userinfo_intel.h"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

CreateGroupIntelDialog::CreateGroupIntelDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateGroupIntelDialog)
{
    ui->setupUi(this);
    setupInit();
    getUsersList();
    signalsBind();
}

CreateGroupIntelDialog::~CreateGroupIntelDialog()
{
    delete ui;
}

void CreateGroupIntelDialog::limitInput()
{
    QIntValidator *intValidator = new QIntValidator;
    //QRegExp rx("^[a-zA-z]+$");// 首字符为字母
    QRegExp rx("[a-zA-z]{40}");
    QRegExpValidator *regValidator = new QRegExpValidator(rx);
    //intValidator->setRange(0, 65535);
    intValidator->setBottom(0);
    // 整形输入限制
    ui->lineEdit_id->setValidator(intValidator);
    // 字母输入限制
    ui->lineEdit_name->setValidator(regValidator);
    // 字符长度限制
    //ui->lineEdit_name->setMaxLength(4);
}

void CreateGroupIntelDialog::refreshCertainBtnStatus(){
    if (ui->lineEdit_name->text().isEmpty() ||
            ui->lineEdit_id->text().isEmpty())
        ui->certainBtn->setEnabled(false);
    else
        ui->certainBtn->setEnabled(true);
}

void CreateGroupIntelDialog::getUsersList()
{
    UserInfoIntel * userinfo = new UserInfoIntel;
    QStringList usersList = userinfo->getUsersList();
    qDebug() << "CreateGroupIntelDialog::getUsersList" << usersList.at(0) << usersList.at(1);
    for(int i = 0; i < usersList.size(); i++){
        QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
        item->setSizeHint(QSize(ui->listWidget->width(), 36));
        item->setData(Qt::UserRole, "");
        QCheckBox * box = new QCheckBox(usersList.at(i));
        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, box);
    }
}

void CreateGroupIntelDialog::signalsBind()
{
    connect(ui->closeBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        close();
    });
    connect(ui->cancelBtn,&QPushButton::clicked,[=](){
        close();
    });
    connect(ui->lineEdit_name,&QLineEdit::textChanged,[=](QString txt){
        refreshCertainBtnStatus();
    });
    connect(ui->lineEdit_id,&QLineEdit::textChanged,[=](QString txt){
        refreshCertainBtnStatus();
    });
}

void CreateGroupIntelDialog::setupInit()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->titleLabel->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");
    ui->closeBtn->setProperty("useIconHighlightEffect", true);
    ui->closeBtn->setProperty("iconHighlightEffectMode", 1);
    ui->closeBtn->setFlat(true);

    ui->closeBtn->setStyleSheet("QPushButton:hover:!pressed#closeBtn{background: #FA6056; border-radius: 4px;}"
                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border-radius: 4px;}");
    ui->closeBtn->setIcon(QIcon("://img/titlebar/close.svg"));

    ui->listWidget->setFocusPolicy(Qt::NoFocus);
    ui->listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    ui->listWidget->setSpacing(5);
    ui->lineEdit_name->setStyleSheet("QLineEdit{background:#EEEEEE;}");
    ui->lineEdit_id->setStyleSheet("QLineEdit{background:#EEEEEE;}");
    ui->listWidget->setStyleSheet("QListWidget{background:#EEEEEE; border-radius: 4px;}"
                                  "QListWidget::Item:hover{background:#FFE1E1E1}"
                                  "QListWidget::Item{padding-left:20px;}");

    // 设置确认按钮
    refreshCertainBtnStatus();
    // 限制组名输入规则
    limitInput();
    //
}

void CreateGroupIntelDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 6, 6);
    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    // 绘制一个背景
    p.save();
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.restore();

}

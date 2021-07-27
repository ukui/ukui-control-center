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
#include "certificationdialog.h"
#include "ui_certificationdialog.h"

#include <QGSettings>
#include "SwitchButton/switchbutton.h"

#include <QDebug>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

CertificationDialog::CertificationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CertificationDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Certification"));
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    const QByteArray id(HTTP_PROXY_SCHEMA);
    cersettings = new QGSettings(id);

    component_init();
    status_init();
}

CertificationDialog::~CertificationDialog()
{
    delete ui;
    delete cersettings;
}

void CertificationDialog::component_init(){

    activeSwitchBtn = new SwitchButton;
    activeSwitchBtn->setAttribute(Qt::WA_DeleteOnClose);
    titleFrame = new QFrame;
    titleLabel = new QLabel;
    titleLabel->setText(tr("UserCertification"));
    titleLayout = new QHBoxLayout(titleFrame);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(activeSwitchBtn);
    titleFrame->setLayout(titleLayout);
    titleFrame->setFixedHeight(48);
    titleLayout->setContentsMargins(0,0,0,0);
    ui->mainLayout->addWidget(titleFrame);
    iconFrame = new QFrame;
    iconLayout = new QHBoxLayout(iconFrame);
    iconLabel = new QLabel;
    iconLabel->setPixmap(QPixmap("://img/plugins/proxy/icon.png"));
    iconLabel->setFixedSize(64,64);
    iconLayout->addStretch();
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch();
    userLineEdit = new QLineEdit;
    pwdLineEdit = new QLineEdit;
    userLineEdit->setStyleSheet("QLineEdit{background: palette(button); border: none; border-radius: 8px; font-size: 14px; color: palette(text);}");
    pwdLineEdit->setStyleSheet("QLineEdit{background: palette(button); border: none; border-radius: 8px; font-size: 14px; color: palette(text);}");
    userLineEdit->setPlaceholderText(tr("Username"));
    pwdLineEdit->setPlaceholderText(tr("Password"));
    userLineEdit->setFixedHeight(48);
    pwdLineEdit->setFixedHeight(48);
    pwdLineEdit->setEchoMode(QLineEdit::Password);
    btnFrame = new QFrame;
    btnLayout = new QHBoxLayout(btnFrame);
    closePushBtn = new QPushButton;
    closePushBtn->setFixedSize(144, 48);
    closePushBtn->setText(tr("Cancel"));

    confirmPushBtn = new QPushButton;
    confirmPushBtn->setFixedSize(144,48);
    confirmPushBtn->setText(tr("Confirm"));

    btnLayout->setContentsMargins(0,0,0,0);
    btnLayout->addStretch();
    btnLayout->addWidget(closePushBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(confirmPushBtn);
    btnLayout->addStretch();

    ui->mainLayout->addWidget(iconFrame);
    ui->mainLayout->addWidget(userLineEdit);
    ui->mainLayout->addWidget(pwdLineEdit);
    ui->mainLayout->addWidget(btnFrame);
    ui->mainLayout->setContentsMargins(24,24,24,40);
    ui->mainLayout->setSpacing(16);
}

void CertificationDialog::status_init(){
    //获取认证状态
    bool status = cersettings->get(HTTP_AUTH_KEY).toBool();
    activeSwitchBtn->setChecked(status);
    userLineEdit->setEnabled(status);
    pwdLineEdit->setEnabled(status);

    //获取用户名密码
    QString user = cersettings->get(HTTP_AUTH_USER_KEY).toString();
    userLineEdit->setText(user);
    QString pwd = cersettings->get(HTTP_AUTH_PASSWD_KEY).toString();
    pwdLineEdit->setText(pwd);

    connect(activeSwitchBtn, SIGNAL(checkedChanged(bool)), this, SLOT(active_status_changed_slot(bool)));
    connect(closePushBtn, SIGNAL(released()), this, SLOT(close()));
    connect(confirmPushBtn, SIGNAL(released()), this, SLOT(close()));

    connect(userLineEdit, SIGNAL(textChanged(QString)), this, SLOT(user_edit_changed_slot(QString)));
    connect(pwdLineEdit, SIGNAL(textChanged(QString)), this, SLOT(pwd_edit_changed_slot(QString)));
}

void CertificationDialog::active_status_changed_slot(bool status){
    userLineEdit->setEnabled(status);
    pwdLineEdit->setEnabled(status);
    cersettings->set(HTTP_AUTH_KEY, QVariant(status));
}

void CertificationDialog::user_edit_changed_slot(QString edit){
    cersettings->set(HTTP_AUTH_USER_KEY, QVariant(edit));
}

void CertificationDialog::pwd_edit_changed_slot(QString edit){
    cersettings->set(HTTP_AUTH_PASSWD_KEY, QVariant(edit));
}

void CertificationDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 16, 16);

    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.setOpacity(0.65);
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


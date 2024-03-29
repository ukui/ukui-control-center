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

#include "changeprojectionname.h"
#include "ui_changeprojectionname.h"

#include <QDebug>


extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

ChangeProjectionName::ChangeProjectionName(QWidget *parent) 
:
    QDialog(parent),
    ui(new Ui::ChangeProjectionName)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    // setWindowFlags(Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->saveBtn->setEnabled(false);

    connect(ui->lineEdit, &QLineEdit::textChanged, this, [=](QString txt){
        if (txt.toLocal8Bit().length() > 31){
            ui->tipLabel->setText(tr("Name is too long, change another one."));
            ui->saveBtn->setEnabled(false);
        }else {
            ui->tipLabel->setText(tr(""));
        }

        if (!txt.isEmpty() && ui->tipLabel->text().isEmpty()){
            ui->saveBtn->setEnabled(true);
        } else {
            ui->saveBtn->setEnabled(false);
        }
    });

    connect(ui->cancelBtn, &QPushButton::clicked, [=]{
        close();
    });

    connect(ui->saveBtn, &QPushButton::clicked, [=]{
        emit sendNewProjectionName(ui->lineEdit->text());
        close();
    });
}

ChangeProjectionName::~ChangeProjectionName()
{
    delete ui;
}

void ChangeProjectionName::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

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

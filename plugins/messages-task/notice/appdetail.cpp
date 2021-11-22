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

#include "appdetail.h"
#include "CloseButton/closebutton.h"
#include <QDebug>

#include "ui_appdetail.h"


#define NOTICE_ORIGIN_SCHEMA "org.ukui.control-center.noticeorigin"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

AppDetail::AppDetail(QString Name,QString key, QGSettings *gsettings, QWidget *parent) :
    QDialog(parent), ui(new Ui::AppDetail), appName(Name), appKey(key),
    m_gsettings(gsettings)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowTitle(appName);

    ui->titleLabel->setStyleSheet("QLabel{color: palette(windowText);}");

    initUiStatus();
    initComponent();
    initConnect();
}

AppDetail::~AppDetail()
{
    delete ui;
    ui = nullptr;
}

void AppDetail::initUiStatus() {

    enablebtn = new SwitchButton;
    ui->enableLayout->addWidget(enablebtn);
}

void AppDetail::initComponent() {
    ui->titleLabel->setText(appName);

    for(int i = 1; i < 5; i++) {
        ui->numberComboBox->addItem(QString::number(i));
    }

    if (m_gsettings) {
        bool judge = m_gsettings->get(MESSAGES_KEY).toBool();
        QString numvalue = m_gsettings->get(MAXIMINE_KEY).toString();
        enablebtn->setChecked(judge);
        ui->numberComboBox->setCurrentText(numvalue);
    }
}

void AppDetail::initConnect() {

    connect(ui->cancelBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        close();
    });

    connect(ui->confirmBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        confirmbtnSlot();
    });
}

void AppDetail::paintEvent(QPaintEvent *event) {
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

void AppDetail::confirmbtnSlot() {
    //TODO: get gsetting may invalid, so program will throw crash error
    if (m_gsettings) {
        bool judge = enablebtn->isChecked();
        int num = ui->numberComboBox->currentIndex() + 1;

        m_gsettings->set(MESSAGES_KEY, judge);
        m_gsettings->set(MAXIMINE_KEY, num);
    }
    close();
}



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

#include "blueeffect.h"

Blueeffect::Blueeffect(QWidget *parent) : QWidget(parent)
{
    m_textLabel = new QLabel(this);
    m_iconLabel = new QLabel(this);
    m_cTimer = new QTimer(this);
    m_svgHandler = new SVGHandler(this);
    m_workLayout = new QHBoxLayout;

    this->setStyleSheet("Blueeffect{background-color:#3790FA;border-radius:4px;}");
    m_textLabel->setStyleSheet("color:#ffffff;background:transparent;");
    m_iconLabel->setStyleSheet("background:transparent;");
    m_iconLabel->setFixedSize(24,24);
    m_workLayout->setSpacing(8);
    m_workLayout->setMargin(0);
    m_workLayout->setAlignment(Qt::AlignCenter);
    m_workLayout->addWidget(m_textLabel);
    m_workLayout->addWidget(m_iconLabel);
    setLayout(m_workLayout);
    m_cTimer->stop();
    connect(m_cTimer,&QTimer::timeout, this,[=] () {
        QPixmap pixmap = m_svgHandler->loadSvgColor(QString(":/new/image/loading1%1.svg").arg(m_cCnt),"white",16);
        m_iconLabel->setPixmap(pixmap);
        m_cCnt = (m_cCnt + 9) % 8;
    });
    hide();
}

Blueeffect::~Blueeffect(){
    m_cTimer->stop();
}

void Blueeffect::settext(const QString &t) {
    m_textLabel->setText(t);
}

void Blueeffect::startmoive() {
    m_cTimer->start(140);
    show();
    adjustSize();
}

void Blueeffect::stop() {
    m_cCnt = 1;
    m_iconLabel->setPixmap(m_svgHandler->loadSvgColor(":/new/image/loading11.svg","white",16));
    if (m_cTimer->isActive())
        m_cTimer->stop();
    hide();
}

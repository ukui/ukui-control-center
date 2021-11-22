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

#include "tips.h"

Tips::Tips(QWidget *parent) : QWidget(parent)
{
    m_textLabel = new QLabel(this);
    m_iconWidget = new QSvgWidget(":/new/image/_.svg");
    m_workLayout = new QHBoxLayout;
    m_iconWidget->setFixedSize(16,16);
    m_workLayout->addWidget(m_iconWidget);
    m_workLayout->addWidget(m_textLabel);
    m_workLayout->setContentsMargins(0,0,0,0);
    m_workLayout->setSpacing(8);
    m_textLabel->setStyleSheet("QLabel{color:#F53547}");
    setLayout(m_workLayout);
    m_szContext = "";
    hide();
}

void Tips::set_text(const QString &text) {
    m_szContext = text;
    this->m_textLabel->setText(m_szContext);
}

Tips::~Tips() {
    delete m_iconWidget;
    m_iconWidget = nullptr;
}

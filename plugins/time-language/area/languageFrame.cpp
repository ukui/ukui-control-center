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

#include "languageFrame.h"
#include <QHBoxLayout>

LanguageFrame::LanguageFrame(QString name, QWidget *parent)
    :QFrame(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    this->setFixedHeight(50);
    this->setFrameShape(QFrame::Box);
    this->setMinimumWidth(550);
    this->setMaximumWidth(960);
    layout->setContentsMargins(16,0,16,0);

    nameLabel = new QLabel();
    nameLabel->setFixedWidth(200);
    nameLabel->setText(name);

    selectedIconLabel = new QLabel();
    selectedIconLabel->setFixedSize(16,16);
    selectedIconLabel->setScaledContents(true);
    selectedIconLabel->setPixmap(QPixmap("://img/plugins/theme/selected.svg"));
    selectedIconLabel->setVisible(false);

    layout->addWidget(nameLabel);
    layout->addStretch();
    layout->addWidget(selectedIconLabel);

}

LanguageFrame::~LanguageFrame()
{

}

void LanguageFrame::mousePressEvent(QMouseEvent *e)
{
    this->showSelectedIcon(true);
    Q_EMIT clicked();
}

void LanguageFrame::showSelectedIcon(bool flag)
{
    selectedIconLabel->setVisible(flag);
}

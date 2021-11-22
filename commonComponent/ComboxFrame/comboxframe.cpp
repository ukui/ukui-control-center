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

#include "comboxframe.h"

ComboxFrame:: ComboxFrame(QString labelStr, QWidget *parent) : QFrame(parent), mTitleName(labelStr) {

    this->setMinimumSize(550, 50);
    this->setMaximumSize(16777215, 50);
    this->setFrameShape(QFrame::Shape::Box);

    mTitleLabel = new QLabel(mTitleName, this);
    mCombox = new QComboBox(this);

    mHLayout = new QHBoxLayout(this);
    mHLayout->addWidget(mTitleLabel);
    mHLayout->addWidget(mCombox);

    this->setLayout(mHLayout);
}

ComboxFrame::ComboxFrame(bool isNum, QString labelStr, QWidget *parent) : QFrame(parent), mTitleName(labelStr)
{
    Q_UNUSED(isNum)
    this->setMinimumSize(550, 50);
    this->setMaximumSize(16777215, 50);
    this->setFrameShape(QFrame::Shape::Box);

    mTitleLabel = new QLabel(mTitleName, this);
    mNumCombox  = new QComboBox(this);
    mCombox     = new QComboBox(this);

    mHLayout = new QHBoxLayout(this);
    mHLayout->addWidget(mTitleLabel);
    mHLayout->addWidget(mNumCombox);
    mHLayout->addWidget(mCombox);

    this->setLayout(mHLayout);
}

ComboxFrame::~ComboxFrame() {

}

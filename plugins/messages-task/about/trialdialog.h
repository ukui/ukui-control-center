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

#ifndef TRIALDIALOG_H
#define TRIALDIALOG_H

#include <QDialog>
#include <QTextBrowser>
#include "Label/titlelabel.h"

class TrialDialog : public QDialog
{
    Q_OBJECT
public:
    TrialDialog(QWidget *parent);
    ~TrialDialog();

private:
    void initUi(QDialog *mTrialDialog);

private:
    TitleLabel *mTitleLabel;

    QLabel *mContentLabel_1;
    QTextBrowser *mContentLabel_2;
    QLabel *mContentLabel_3;
    QLabel *mContentLabel_4;
    QLabel *mContentLabel_5;


};

#endif // TRIALDIALOG_H

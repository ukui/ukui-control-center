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

#ifndef INPUTPWDDIALOG_H
#define INPUTPWDDIALOG_H
#include <QDialog>
#include <QVBoxLayout>
#include <QGSettings>
#include <QLabel>
#include <QLineEdit>


namespace Ui {
class InputPwdDialog;
}
class InputPwdDialog : public QDialog
{
    Q_OBJECT

public:
    InputPwdDialog(QGSettings *Keygsettiings,QWidget *parent = nullptr);
    ~InputPwdDialog();


private:
    QGSettings *mgsettings;

    QPushButton *mCancelBtn;
    QPushButton *mConfirmBtn;

    QLabel *mHintLabel;

    QLineEdit *mpwd;

    QByteArray secPwd;

    bool mfirstload;

    bool mstatus;

private:
    void setupInit();

    void initConnect();

    bool eventFilter(QObject *wcg, QEvent *event);

private slots:
    void mpwdInputSlot(const QString &pwd);


};


#endif // INPUTPWDDIALOG_H

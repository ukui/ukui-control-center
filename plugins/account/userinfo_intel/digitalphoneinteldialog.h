/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: ZHAI Kang-ning <zhaikangning@kylinos.cn>
**/
#ifndef DIGITALPHONEINTELDIALOG_H
#define DIGITALPHONEINTELDIALOG_H

#include "passwordbar.h"
#include "numbersbuttonintel.h"
#include "qtdbus/systemdbusdispatcher.h"
#include "qtdbus/userdispatcher.h"
#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include <QRect>
#include <QDBusInterface>
#include <QDBusReply>

class Auth;
class QEvent;
class QVBoxLayout;
class QHBoxLayout;

class DigitalPhoneIntelDialog : public QWidget
{
    Q_OBJECT
public:
    explicit DigitalPhoneIntelDialog(QString username,QWidget *parent = nullptr);
    ~DigitalPhoneIntelDialog();

    void initUI();
    void initConnect();
    void setQSS();
    void hidePromptMsg();
    void showPromptMsg();
protected:
    void showEvent(QShowEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void drawBg(QPainter *painter);

protected:
    void paintEvent(QPaintEvent *event);
    virtual void keyPressEvent(QKeyEvent *ev);
Q_SIGNALS:
    void numberBtnPress(int btn_id);
    void forgetpassword();
    void phone();
public slots:
    void onReset();
private slots:
    void onNumerPress(int btn_id);
private:
    QLabel * label2;
    QColor m_bgColor;                   //背景色

    QVBoxLayout *mainLayout;
    enum InputMode{
       InputOldPwd,
       InputPwd,
       ReInputPwd,
    };
    QLabel *m_pTitle;               // 标题
    NumbersButtonIntel *m_pNumbersBtn;
    PasswordBar *m_pPasswordBar;
    QLabel *m_pPromptMessage;
    QLabel *m_pPromptMessageClone; // 控制格式

    InputMode m_curInputMode;
    QString m_oldPwd;   // 记录旧密码
    QString m_curPwd;   // 记录当前密码
    QString m_reInputPwd; // 记录再次输入的密码
    QString m_username;//用户名

    QDBusInterface *m_interface1;
};

#endif // DIGITALPHONEINTELDIALOG_H

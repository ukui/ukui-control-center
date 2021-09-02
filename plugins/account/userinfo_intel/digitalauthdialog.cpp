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
#include "digitalauthdialog.h"
#include <QDebug>
#include <cmath>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QMessageBox>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

DigitalAuthDialog::DigitalAuthDialog(QString username ,QWidget *parent) : QWidget(parent),
    m_bgColor("#FFFFFF")
{
    this->resize(360, 529);
    m_username = username;

    initUI();
    setQSS();
    initConnect();
}

DigitalAuthDialog::~DigitalAuthDialog()
{

}

void DigitalAuthDialog::initUI(){

    mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    m_pTitle = new QLabel(tr("Enter Old Password"));
    m_pTitle->setProperty("class", "titleLB");
    mainLayout->addWidget(m_pTitle, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);

    m_pPasswordBar = new PasswordBar(this);
    m_pPasswordBar->setBallRadius(8);
    mainLayout->addWidget(m_pPasswordBar, 0, Qt::AlignHCenter);

    m_pPromptMessage = new QLabel(this);
    m_pPromptMessageClone = new QLabel(this);
    m_pPromptMessage->setProperty("class", "promptMessageLabel");
    hidePromptMsg();
    mainLayout->addWidget(m_pPromptMessage, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_pPromptMessageClone, 0, Qt::AlignHCenter);

    m_pNumbersBtn = new NumbersButton(this);

    //mainLayout->addSpacing(20);
    mainLayout->addWidget(m_pNumbersBtn, 0, Qt::AlignHCenter);

    m_curInputMode = InputMode::InputOldPwd;

    label2 = new QLabel(this);
    label2 -> setText(tr("Forget Password?"));
//    label2->move(140, 528);
    label2->show();
    label2->installEventFilter(this);//安装事件过滤
    mainLayout->addSpacing(20);
    mainLayout->addStretch();
    mainLayout->addWidget(label2, 0, Qt::AlignHCenter);
    mainLayout->addStretch();
    m_interface1 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                      "/cn/kylinos/SSOBackend",
                                                      "cn.kylinos.SSOBackend.accounts",
                                                      QDBusConnection::systemBus());
    if (m_interface1->isValid()) {
        QDBusMessage result = m_interface1->call("GetAccountPincode",m_username);
        QList<QVariant> outArgs = result.arguments();
        delete m_interface1;
        pinstatus = outArgs.at(1).value<int>();
        if (pinstatus == 0) {
            QString oldpwd = outArgs.at(0).value<QString>();
            if (oldpwd == "") {
                m_curInputMode = InputMode::InputPwd;
                m_pTitle->setText(tr("Input New Password"));
                hidePromptMsg();
                m_pPasswordBar->setFillBall(0);
                m_oldPwd = "";
                m_curPwd = "";
                label2->hide();
                pinstatus = 1;
            }
        }
    } else {
        qCritical() << "Create DBus Interface Failed: " << QDBusConnection::systemBus().lastError();
    }
}

void DigitalAuthDialog::initConnect()
{
    connect(m_pNumbersBtn, &NumbersButton::numbersButtonPress, this, &DigitalAuthDialog::onNumerPress);
}

void DigitalAuthDialog::setQSS()
{
    setStyleSheet(".promptMessageLabel{"
                  "border: 2px solid #FD625E;"
                  "border-radius: 8px;"
                  "font-size:14px;"
                  "font-family: NotoSansCJKsc-Regular, NotoSansCJKsc;"
                  "background: #FD625E"
                  "}");
}

void DigitalAuthDialog::showPromptMsg()
{
    m_pPromptMessage->show();
    m_pPromptMessageClone->hide();
}

void DigitalAuthDialog::hidePromptMsg()
{
    m_pPromptMessage->hide();
    m_pPromptMessageClone->show();
    m_pPromptMessageClone->setFixedHeight(m_pPromptMessage->height());
}

void DigitalAuthDialog::onReset()
{
    m_pTitle->setText(tr("Input Password"));
    m_pPasswordBar->setFillBall(0);
    m_curPwd = "";
    m_reInputPwd = "";
    m_curInputMode = InputMode::InputPwd;
}

void DigitalAuthDialog::gotonext()
{
    m_curInputMode = InputMode::InputPwd;
    m_pTitle->setText(tr("Input New Password"));
    hidePromptMsg();
    label2->hide();
    m_pPasswordBar->setFillBall(0);
    m_curPwd = "";
    m_interface1 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                      "/cn/kylinos/SSOBackend",
                                                      "cn.kylinos.SSOBackend.accounts",
                                                      QDBusConnection::systemBus());
    if (!m_interface1->isValid()) {
        qCritical() << "Create DBus Interface Failed: " << QDBusConnection::systemBus().lastError();
        return;
    }
    QDBusMessage result = m_interface1->call("GetAccountPincode",m_username);

    QList<QVariant> outArgs = result.arguments();
    delete m_interface1;
    m_oldPwd = outArgs.at(0).value<QString>();

}

void DigitalAuthDialog::onNumerPress(int btn_id)
{
    if (btn_id == 10)
    {
        if(InputMode::InputOldPwd == m_curInputMode && m_oldPwd.size())
            m_oldPwd = m_oldPwd.left(m_oldPwd.size() - 1);
        else if(InputMode::InputPwd == m_curInputMode && m_curPwd.size())
            m_curPwd = m_curPwd.left(m_curPwd.size() - 1);
        else if(InputMode::ReInputPwd == m_curInputMode && m_reInputPwd.size())
            m_reInputPwd = m_reInputPwd.left(m_reInputPwd.size() - 1);
        m_pPasswordBar->delFillBall();
    } else if (btn_id == 11){
        if(InputMode::InputOldPwd == m_curInputMode && m_oldPwd.size())
            m_oldPwd = "";
        else if(InputMode::InputPwd == m_curInputMode && m_curPwd.size())
            m_curPwd = "";
        else if(InputMode::ReInputPwd == m_curInputMode && m_reInputPwd.size())
            m_reInputPwd = "";
        m_pPasswordBar->setFillBall(0);
    } else {
        if(InputMode::InputOldPwd == m_curInputMode && m_oldPwd.size() < 6)
            m_oldPwd += QChar(btn_id + '0');
        else if(InputMode::InputPwd == m_curInputMode && m_curPwd.size() < 6)
            m_curPwd += QChar(btn_id + '0');
        else if(InputMode::ReInputPwd == m_curInputMode && m_reInputPwd.size() < 6)
            m_reInputPwd += QChar(btn_id + '0');

        m_pPasswordBar->addFillBall();
        hidePromptMsg();

        if (m_oldPwd.size() == 6 && InputMode::InputOldPwd == m_curInputMode) {
            m_interface1 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                              "/cn/kylinos/SSOBackend",
                                                              "cn.kylinos.SSOBackend.accounts",
                                                              QDBusConnection::systemBus());
            if (!m_interface1->isValid()) {
                qCritical() << "Create DBus Interface Failed: " << QDBusConnection::systemBus().lastError();
                return;
            }
            QDBusMessage result = m_interface1->call("GetAccountPincode",m_username);

            QList<QVariant> outArgs = result.arguments();
            delete m_interface1;
            int status = outArgs.at(1).value<int>();

            if (status == 0) {
                QString oldpwd = outArgs.at(0).value<QString>();
                if (oldpwd == m_oldPwd) {
                    qDebug() << "旧密码为：" << m_oldPwd;
                    m_curInputMode = InputMode::InputPwd;
                    m_pTitle->setText(tr("Input New Password"));
                    hidePromptMsg();
                    label2->hide();
                    m_pPasswordBar->setFillBall(0);
                    m_curPwd = "";

                } else {
                    m_pPromptMessage->setText(tr("The password input is error"));
                    showPromptMsg();
                    m_pPasswordBar->setFillBall(0);
                    m_curPwd = "";
                    m_reInputPwd = "";
                    m_oldPwd ="";

                }

            }

        } else if (m_curPwd.size() == 6 && InputMode::InputPwd == m_curInputMode) {
            qDebug() << "设置新密码为：" << m_curPwd;
            m_curInputMode = InputMode::ReInputPwd;
            m_pTitle->setText(tr("Confirm New Password"));
            hidePromptMsg();
            m_pPasswordBar->setFillBall(0);
            m_reInputPwd = "";
        } else if(m_reInputPwd.size() == 6 && InputMode::ReInputPwd == m_curInputMode) {
            if (m_curPwd != m_reInputPwd) {
                m_pPromptMessage->setText(tr("The password input is inconsistent"));
                showPromptMsg();
                m_curInputMode = InputMode::InputPwd;
                m_pTitle->setText(tr("Input New Password"));
                m_pPasswordBar->setFillBall(0);
                m_curPwd = "";
                m_reInputPwd = "";
            } else {
                // TODO 更改密码
                if (m_oldPwd == m_curPwd) {
                    m_pPromptMessage->setText(tr("New password can not be consistent of old password"));
                    showPromptMsg();
                    m_curInputMode = InputMode::InputPwd;
                    m_pTitle->setText(tr("Input New Password"));
                    m_pPasswordBar->setFillBall(0);
                    m_curPwd = "";
                    m_reInputPwd = "";
                } else {
                    if(pinstatus == 1) {
                        m_interface2 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                                          "/cn/kylinos/SSOBackend",
                                                                          "cn.kylinos.SSOBackend.accounts",
                                                                          QDBusConnection::systemBus());
                        QDBusMessage result = m_interface2->call("SetAccountPincode",m_username,m_curPwd);

                        QList<QVariant> outArgs = result.arguments();
                        delete m_interface2;
                        qDebug()<<"outArgs=="<<outArgs;
                        int status = outArgs.at(0).toInt();
                        qDebug()<<"status=="<<status;
                        if (status == 0) {
                            hidePromptMsg();
                            emit ended();
                        } else {
                            m_pPromptMessage->setText(tr("Password Change Failed"));
                            showPromptMsg();
                            m_curInputMode = InputMode::InputOldPwd;
                            m_pTitle->setText(tr("Enter Old Password"));
                            m_pPasswordBar->setFillBall(0);
                            m_curPwd ="";
                            m_reInputPwd = "";
                            m_oldPwd ="";
                        }
                    } else {
                        m_interface2 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                                          "/cn/kylinos/SSOBackend",
                                                                          "cn.kylinos.SSOBackend.accounts",
                                                                          QDBusConnection::systemBus());
                        QDBusMessage result = m_interface2->call("ChangeAccountPincode",m_username,m_oldPwd,m_curPwd);

                        QList<QVariant> outArgs = result.arguments();
                        delete m_interface2;
                        qDebug()<<"outArgs=="<<outArgs;
                        int status = outArgs.at(0).toInt();
                        qDebug()<<"status=="<<status;
                        if (status == 0) {
                            hidePromptMsg();
                            emit ended();
                        } else {
                            m_pPromptMessage->setText(tr("Password Change Failed"));
                            showPromptMsg();
                            m_curInputMode = InputMode::InputOldPwd;
                            m_pTitle->setText(tr("Enter Old Password"));
                            m_pPasswordBar->setFillBall(0);
                            m_curPwd ="";
                            m_reInputPwd = "";
                            m_oldPwd ="";
                        }
                    }

                }

            }

        }
    }
}


bool DigitalAuthDialog::eventFilter(QObject *obj, QEvent *event){
    if(obj == label2)//需要操作label
    {
        if(event->type() == QEvent::MouseButtonPress)//判断事件类型
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if(mouseEvent->button() == Qt::LeftButton)
            {
                emit forgetpassword();
//                QMessageBox::information(NULL,QString::fromLocal8Bit("点击"),QString::fromLocal8Bit("牛逼吗？"),QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
                return true;
            }
            return false;
        }
        return false;
    }
    return DigitalAuthDialog::eventFilter(obj, event);
}

void DigitalAuthDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    setFocus();
}

void DigitalAuthDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
}

void DigitalAuthDialog::drawBg(QPainter *painter)
{
    painter->save();

    int width = this->width();
    int height = this->height();

    painter->setPen(Qt::NoPen);
    m_bgColor.setAlpha(255);
    painter->setBrush(m_bgColor);
    painter->drawRoundRect(0, 0, width, height, 5*height/width, 5);

    painter->restore();
}
void DigitalAuthDialog::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_0) {
        onNumerPress(0);
        return;
    }
    if (ev->key() == Qt::Key_1) {
        onNumerPress(1);
        return;
    }
    if (ev->key() == Qt::Key_2) {
        onNumerPress(2);
        return;
    }
    if (ev->key() == Qt::Key_3) {
        onNumerPress(3);
        return;
    }
    if (ev->key() == Qt::Key_4) {
        onNumerPress(4);
        return;
    }
    if (ev->key() == Qt::Key_5) {
        onNumerPress(5);
        return;
    }
    if (ev->key() == Qt::Key_6) {
        onNumerPress(6);
        return;
    }
    if (ev->key() == Qt::Key_7) {
        onNumerPress(7);
        return;
    }
    if (ev->key() == Qt::Key_8) {
        onNumerPress(8);
        return;
    }
    if (ev->key() == Qt::Key_9) {
        onNumerPress(9);
        return;
    }
    if (ev->key()== Qt::Key_Backspace){
        onNumerPress(10);
        return;
    }

    QWidget::keyPressEvent(ev);
}

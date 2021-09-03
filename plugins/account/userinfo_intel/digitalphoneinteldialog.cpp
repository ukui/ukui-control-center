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
#include "digitalphoneinteldialog.h"
#include <QDebug>
#include <cmath>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QMessageBox>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

DigitalPhoneIntelDialog::DigitalPhoneIntelDialog(QString username ,QWidget *parent) : QWidget(parent),
    m_bgColor("#FFFFFF")
{
    this->resize(360, 529);
    m_username = username;

    initUI();
    setQSS();
    initConnect();
}

DigitalPhoneIntelDialog::~DigitalPhoneIntelDialog()
{

}

void DigitalPhoneIntelDialog::initUI(){

    mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    m_pTitle = new QLabel(tr("Please Enter Edu OS Password"));
    m_pTitle->setProperty("class", "titleLB");
    mainLayout->addWidget(m_pTitle, 0, Qt::AlignHCenter);

    m_pPasswordBar = new PasswordBar(this);
    m_pPasswordBar->setBallRadius(8);
    mainLayout->addWidget(m_pPasswordBar, 0, Qt::AlignHCenter);

    m_pPromptMessage = new QLabel(this);
    m_pPromptMessageClone = new QLabel(this);
    m_pPromptMessage->setProperty("class", "promptMessageLabel");
    hidePromptMsg();
    mainLayout->addWidget(m_pPromptMessage, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_pPromptMessageClone, 0, Qt::AlignHCenter);

    m_pNumbersBtn = new NumbersButtonIntel(this);

    //mainLayout->addSpacing(20);
    mainLayout->addWidget(m_pNumbersBtn, 0, Qt::AlignHCenter);

    m_curInputMode = InputMode::InputOldPwd;


    label2 = new QLabel(this);
//    label2 -> setText(tr("<u>Forget Password?</u>"));
//    label2->move(140, 528);
    label2->show();
    label2->installEventFilter(this);//安装事件过滤
    mainLayout->addWidget(label2, 0, Qt::AlignHCenter);
    //label2->hide();
}

void DigitalPhoneIntelDialog::initConnect()
{
    connect(m_pNumbersBtn, &NumbersButtonIntel::numbersButtonPress, this, &DigitalPhoneIntelDialog::onNumerPress);
}

void DigitalPhoneIntelDialog::setQSS()
{
    setStyleSheet(".promptMessageLabel{"
                  "border: 2px solid #FD625E;"
                  "border-radius: 8px;"
                  "font-size:14px;"
                  "font-family: NotoSansCJKsc-Regular, NotoSansCJKsc;"
                  "background: #FD625E"
                  "}");
}

void DigitalPhoneIntelDialog::showPromptMsg()
{
    m_pPromptMessage->show();
    m_pPromptMessageClone->hide();
}

void DigitalPhoneIntelDialog::hidePromptMsg()
{
    m_pPromptMessage->hide();
    m_pPromptMessageClone->show();
    m_pPromptMessageClone->setFixedHeight(m_pPromptMessage->height());
}

void DigitalPhoneIntelDialog::onReset()
{
    m_pTitle->setText("请输入密码");
    m_pPasswordBar->setFillBall(0);
    m_curPwd = "";
    m_reInputPwd = "";
    m_curInputMode = InputMode::InputPwd;
}

void DigitalPhoneIntelDialog::onNumerPress(int btn_id)
{
    if(btn_id == 10)
    {
        if(InputMode::InputOldPwd == m_curInputMode && m_oldPwd.size())
            m_oldPwd = m_oldPwd.left(m_oldPwd.size() - 1);
        else if(InputMode::InputPwd == m_curInputMode && m_curPwd.size())
            m_curPwd = m_curPwd.left(m_curPwd.size() - 1);
        else if(InputMode::ReInputPwd == m_curInputMode && m_reInputPwd.size())
            m_reInputPwd = m_reInputPwd.left(m_reInputPwd.size() - 1);

        m_pPasswordBar->delFillBall();
    }
    else
    {
        if(InputMode::InputOldPwd == m_curInputMode && m_oldPwd.size() < 6)
            m_oldPwd += QChar(btn_id + '0');

        m_pPasswordBar->addFillBall();

        if (m_oldPwd.size() == 6 && InputMode::InputOldPwd == m_curInputMode) {
            m_interface1 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                              "/cn/kylinos/SSOBackend",
                                                              "cn.kylinos.SSOBackend.accounts",
                                                              QDBusConnection::systemBus());
            QDBusMessage result = m_interface1->call("GetAccountPincode",m_username);
            delete m_interface1;

            QList<QVariant> outArgs = result.arguments();

            int status = outArgs.at(1).value<int>();

            if (status == 0) {
                QString oldpwd = outArgs.at(0).value<QString>();
                if (oldpwd == m_oldPwd) {
                    qDebug() << "旧密码为：" << m_oldPwd;
                    hidePromptMsg();
                    m_pPasswordBar->setFillBall(0);
                    m_oldPwd = "";
                    emit phone();
                } else {
                    m_pPromptMessage->setText(tr("The password input is error"));
                    showPromptMsg();
                    m_pPasswordBar->setFillBall(0);
                    m_oldPwd = "";
                }

            }

        }
    }
}


bool DigitalPhoneIntelDialog::eventFilter(QObject *obj, QEvent *event){
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
    return DigitalPhoneIntelDialog::eventFilter(obj, event);
}

void DigitalPhoneIntelDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    setFocus();
}

void DigitalPhoneIntelDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
}

void DigitalPhoneIntelDialog::drawBg(QPainter *painter)
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
void DigitalPhoneIntelDialog::keyPressEvent(QKeyEvent *ev)
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

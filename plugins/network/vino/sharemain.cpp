/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "sharemain.h"

#include <QProcess>
#include <QHBoxLayout>
#include <QAbstractButton>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QInputDialog>
#include <QMessageBox>

ShareMain::ShareMain(QWidget *parent) :
    QWidget(parent),
    mSettingsIni(Q_NULLPTR),
    mIsOpen(0),
    mNeedPwd(1),
    mProtocol("vnc")
{
    mVlayout = new QVBoxLayout(this);
    mVlayout->setContentsMargins(0, 0, 32, 0);
    initUI();
    krd = new ComKylinRemoteDesktopInterface("com.kylin.RemoteDesktop",
                                             "/com/kylin/RemoteDesktop",
                                             QDBusConnection::sessionBus(),
                                             this);
    if (!krd->isValid())
        qDebug() << "start com.kylin.RemoteDesktop service";
    initData();
    initConnection();

    update_outputs();
    update_inputs();
    update_auth();
    update_clients();
    initComponentStatus();
}

ShareMain::~ShareMain()
{
    if(mSettingsIni)
    {
        delete mSettingsIni;
        mSettingsIni = nullptr;
    }
}

void ShareMain::initData()
{
    QString confFile = QDir::homePath()+"/.config/kylin-remote-desktop/krd.ini";
    mSettingsIni = new QSettings(confFile, QSettings::IniFormat);
    if(!QFile::exists(confFile)) {
        mSettingsIni->setValue("mIsOpen", "0");
        mSettingsIni->setValue("password", "");
        mSettingsIni->setValue("mNeedPwd", "1");
        mSettingsIni->setValue("protocol", mProtocol);
        mIsOpen = mSettingsIni->value("mIsOpen").toInt();
        mNeedPwd = mSettingsIni->value("mNeedPwd").toInt();
    } else {
        mIsOpen = mSettingsIni->value("mIsOpen").toInt();
        mNeedPwd = mSettingsIni->value("mNeedPwd").toInt();
        if(mIsOpen == 1) {
            qDebug() << "ZDEBUG " << "enable setchecked true " << __LINE__ ;

            mEnableBtn->setChecked(true);
            update_outputs();
            enableSlot(true);
            qDebug() << "ZDEBUG " << "enable setchecked true " << __LINE__ ;

            mSecurityPwdFrame->setVisible(true);
            mSecurityTitleLabel->setVisible(true);
        } else if(mIsOpen == 0) {
            qDebug() << "ZDEBUG " << "enable setchecked true " << __LINE__ ;
            mEnableBtn->setChecked(false);
        }
        QString protocol = mSettingsIni->value("protocol").toString();
        mProtocol = protocol.isEmpty() ? mProtocol : protocol;
    }
}

void ShareMain::initUI()
{
    initTitleLabel();
    initEnableUI();
    initPwdUI();
    initOutputUI();
    initProtocolUI();
    initInputUI();
    initClientUI();
    setFrame();
}

void ShareMain::initConnection()
{
    connect(mEnableBtn, &SwitchButton::checkedChanged, this, &ShareMain::enableSlot);
    connect(mPwdBtn, &SwitchButton::checkedChanged, this, &ShareMain::pwdEnableSlot);
    connect(mPwdLineEdit, &QLineEdit::textChanged, this, &ShareMain::pwdInputSlot);
    connect(mMaxClientSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxClientValueChangedSlot(int)));
    connect(mPointBtn, &SwitchButton::checkedChanged, this, &ShareMain::onPointerClickedSlot);
    connect(mKeyboardBtn, &SwitchButton::checkedChanged, this, &ShareMain::onKeyboardClickedSlot);
    connect(mClipboardBtn, &SwitchButton::checkedChanged, this, &ShareMain::onClipboardClickedSlot);
    connect(mBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &ShareMain::onProtocolSelectSlot);
}

void ShareMain::onChanged(int type)
{
    if (type == 1)
        update_outputs();
    else if (type == 2)
        update_clients();
}

void ShareMain::update_outputs()
{
    QStringList outputs = krd->supportedOutputDevice();

    /* first delete output that is removed */
    foreach (QRadioButton *button, output_list) {
        if (!outputs.contains(button->text())) {
            mOutputHLayout->removeWidget(button);
            output_list.removeOne(button);
            qDebug() << "remove output" << button->text();
            delete button;
        }
    }

    /* add output that is new */
    foreach (const QString &output, outputs) {
        bool found = false;
        foreach (QRadioButton *button, output_list) {
            if (button->text() == output)
                found = true;
        }
        if (found)
            continue;
        qDebug() << "add output" << output;
        QRadioButton *button = new QRadioButton(output, this);
        output_list.append(button);
        mOutputHLayout->addWidget(button);
    }

    QString output = krd->currrentOutput();

    if (!output.isEmpty()) {
        foreach (QRadioButton *button, output_list) {
            if (button->text() == output)
            {
                button->setChecked(true);
            }
        }
    } else {
        output_list.first()->setChecked(true);
    }
}
void ShareMain::update_inputs()
{
    uint input = krd->supportedInputDevice();
    mPointBtn->setChecked(input & 0x1);
    mKeyboardBtn->setChecked(input & 0x2);
    mClipboardBtn->setChecked(krd->clipBoard());
    mMaxClientSpinBox->setValue(krd->allowedMaxClient());
}

void ShareMain::update_auth()
{
    checkPwdEnableState();

    QString pwd = mSettingsIni->value("password").toString();
    mPwdLineEdit->setText(pwd);
}

void ShareMain::update_clients()
{
    ClientInfo client_info = krd->clientsInfo();
    mTbClients->setRowCount(0);
    foreach (QVariantMap client, client_info) {
        int row = mTbClients->rowCount();
        mTbClients->insertRow(row);
        mTbClients->setItem(row, 0, new QTableWidgetItem(QString::number(client.value("id").toInt())));
        mTbClients->setItem(row, 1, new QTableWidgetItem(QString(client.value("ip").toString())));
        mTbClients->setItem(row, 2, new QTableWidgetItem(QString::number(client.value("viewOnly").toBool())));
    }
}

void ShareMain::initComponentStatus()
{
    if (!mProtocol.compare("vnc")) {
        mVncRadioBtn->setChecked(true);
    } else {
        mRdpRadioBtn->setChecked(true);
    }
}

void ShareMain::onPointerClickedSlot(bool checked)
{
    uint input = mKeyboardBtn->isChecked() << 1 | checked;
    krd->setSupportedInputDevice(input);
}

void ShareMain::onKeyboardClickedSlot(bool checked)
{
    uint input = checked << 1 | mPointBtn->isChecked();
    krd->setSupportedInputDevice(input);
}

void ShareMain::onClipboardClickedSlot(bool checked)
{
    krd->setClipBoard(checked);
}

void ShareMain::on_wl_speed_valueChanged(int arg1)
{
    krd->setWheelSpeed(arg1);
}

void ShareMain::on_pb_start_clicked()
{
    qDebug() << "ZDEBUG " << "start " << __LINE__ ;
    QString output;
    foreach (QRadioButton *button, output_list) {
        if (button->isChecked())
            output = button->text();
    }
    if (output.isEmpty()) {
        QMessageBox::warning(NULL, tr("Warning"), tr("please select an output"));
        mEnableBtn->setChecked(false);
        return;
    }
    mSettingsIni->setValue("mIsOpen","1");
    mIsOpen = mSettingsIni->value("mIsOpen").toInt();

    if (!mProtocol.compare("vnc")) {
        krd->Start(output);
    } else {
        krd->StartRDP(output);
    }

}

void ShareMain::maxClientValueChangedSlot(int cNum)
{
    qDebug() << "client active number == " << mTbClients->rowCount();

    if(mTbClients->rowCount() > cNum && mTbClients->rowCount() > 0)
    {
      mMaxClientSpinBox->setMinimum(mTbClients->rowCount());
      mSettingsIni->setValue("clientMaxNum",mTbClients->rowCount());
    }
    else if(mTbClients->rowCount() == 0)
    {
      mMaxClientSpinBox->setMinimum(1);
      mSettingsIni->setValue("clientMaxNum",1);
    }
    krd->setAllowedMaxClient(cNum);
}

void ShareMain::on_pb_viewonly_clicked()
{
    int row = mTbClients->currentRow();
    if (row < 0)
        return;
    int id = mTbClients->item(row, 0)->text().toInt();
    bool view = mTbClients->item(row, 2)->text().toUInt();
    krd->SetViewOnly(id, !view);
}

void ShareMain::on_pb_close_clicked()
{
    int row = mTbClients->currentRow();
    if (row < 0)
        return;
    int id = mTbClients->item(row, 0)->text().toInt();

    krd->CloseClient(id);
}

void ShareMain::exitAllClient()
{
    mSettingsIni->setValue("mIsOpen","0");
    mIsOpen = mSettingsIni->value("mIsOpen").toInt();
    mMaxClientSpinBox->setMinimum(1);
    mSettingsIni->setValue("clientMaxNum",1);

    krd->Exit();
}

void ShareMain::on_pb_passwd_clicked()
{
    bool ok;
    QString pwd = QInputDialog::getText(NULL, tr("Input Password"),
                                        tr("Password"), QLineEdit::Password,
                                        NULL, &ok);
    if (ok && !pwd.isEmpty())
        krd->SetPassword(pwd);
}

void ShareMain::onProtocolSelectSlot(int protocol)
{
    QString pro = protocol ? "rdp" : "vnc";
    mProtocol = pro;
    mSettingsIni->setValue("protocol", pro);
    on_pb_start_clicked();
    pwdInputSlot(mPwdLineEdit->text());
}

void ShareMain::enableSlot(bool status)
{
    qDebug() << "ZDEBUG " << "enableSlot " << __LINE__ ;

    savePwdEnableState();

    setFrameVisible(status);

    if(status) {
        on_pb_start_clicked();
    } else {
        exitAllClient();
    }

    qDebug() << "ZDEBUG " << "enableSlot " << __LINE__ ;

    //viewBoxSlot(!mViewBtn->isChecked());
}

void ShareMain::savePwdEnableState()
{
    if(krd->authMethod()) {
        qDebug()<<"ZDEBUG auth 1" << __LINE__ ;

        mNeedPwd = 1;
        mSettingsIni->setValue("mNeedPwd",mNeedPwd);
    }
    else {
        qDebug()<<"ZDEBUG auth 0" << __LINE__ ;
        mNeedPwd = 0;
        mSettingsIni->setValue("mNeedPwd",mNeedPwd);
    }
}

void ShareMain::checkPwdEnableState()
{
    if(mNeedPwd == 1) {
        mPwdBtn->setChecked(true);
        pwdEnableSlot(true);
    } else {
        mPwdBtn->setChecked(false);
        pwdEnableSlot(false);
    }
}

void ShareMain::setFrameVisible(bool visible)
{
    //mControlFrame->setVisible(visible);
    mSecurityPwdFrame->setVisible(visible);
    mSecurityTitleLabel->setVisible(visible);
    mProtocolTitleLabel->setVisible(visible);
    mVncProtocolFrame->setVisible(visible);
    mRdpProtocalFrame->setVisible(visible);
}

void ShareMain::pwdEnableSlot(bool status)
{
    if(krd->authMethod())
    qDebug() << "ZDEBUG " << "krd->authMethod() = true " << __LINE__ ;
    else
    qDebug() << "ZDEBUG " << "krd->authMethod() = false " << __LINE__ ;


    krd->setAuthMethod(status);
    savePwdEnableState();

    if (status) {
        mPwdLineEdit->setVisible(true);
        mHintLabel->setVisible(true);
    } else {
        mPwdLineEdit->setVisible(false);
        mHintLabel->setVisible(false);
    }
}

void ShareMain::pwdInputSlot(const QString &pwd)
{
    if (pwd.isEmpty() && mPwdLineEdit->text().isEmpty()) {
        mHintLabel->setText(tr("Password can not be blank"));
        mHintLabel->setStyleSheet("color:red;");
        mHintLabel->setVisible(true);
        krd->SetPassword(pwd);
        mSettingsIni->setValue("password", pwd);
    } else if (pwd.length() <= 8 && !pwd.isEmpty()) {
        mHintLabel->setText(tr(""));
        mHintLabel->setVisible(false);
        krd->SetPassword(pwd);
        //if(mSettingsIni->value("password", "NULL") == "NULL")
        mSettingsIni->setValue("password", pwd);
    } else if (pwd.length() > 8) {
        mHintLabel->setText(tr("Password length must be less than or equal to 8"));
        mHintLabel->setStyleSheet("color:red;");
        mHintLabel->setVisible(true);
    }
}

void ShareMain::initTitleLabel()
{
    mShareTitleLabel = new QLabel(tr("Share"), this);

    QFont font;
    font.setPixelSize(18);
    mShareTitleLabel->setFont(font);
}

void ShareMain::initEnableUI()
{
    mEnableFrame = new QFrame(this);
    mEnableFrame->setFrameShape(QFrame::Shape::Box);
    mEnableFrame->setMinimumSize(550, 50);
    mEnableFrame->setMaximumSize(960, 50);

    QHBoxLayout *enableHLayout = new QHBoxLayout();

    mEnableBtn = new SwitchButton(this);
    mEnableLabel = new QLabel(tr("Allow others to view your desktop"), this);
    enableHLayout->addWidget(mEnableLabel);
    enableHLayout->addStretch();
    enableHLayout->addWidget(mEnableBtn);

    mEnableFrame->setLayout(enableHLayout);

}

void ShareMain::initPwdUI()
{
    mSecurityTitleLabel = new QLabel(tr("Security"), this);

    mSecurityPwdFrame = new QFrame(this);
    mSecurityPwdFrame->setFrameShape(QFrame::Shape::Box);
    mSecurityPwdFrame->setMinimumSize(550, 50);
    mSecurityPwdFrame->setMaximumSize(960, 50);

    QHBoxLayout *pwdHLayout = new QHBoxLayout();

    mPwdBtn = new SwitchButton(this);
    mPwdsLabel = new QLabel(tr("Require user to enter this password: "), this);

    mHintLabel = new QLabel(tr("Password can not be blank"), this);
    mHintLabel->setStyleSheet("color:red;");

    mPwdLineEdit = new QLineEdit(this);
    pwdHLayout->addWidget(mPwdsLabel);
    pwdHLayout->addStretch();
    pwdHLayout->addWidget(mPwdLineEdit);
    pwdHLayout->addStretch();
    pwdHLayout->addWidget(mHintLabel);
    pwdHLayout->addWidget(mPwdBtn);

    mSecurityPwdFrame->setLayout(pwdHLayout);
}

void ShareMain::initProtocolUI()
{
    mProtocolTitleLabel = new QLabel(tr("Protocol"), this);

    mVncProtocolFrame = new QFrame(this);
    mVncProtocolFrame->setFrameShape(QFrame::Shape::Box);
    mVncProtocolFrame->setMinimumSize(550, 50);
    mVncProtocolFrame->setMaximumSize(960, 50);

    mVncRadioBtn = new QRadioButton("VNC", mVncProtocolFrame);
    QHBoxLayout *vncHLayout = new QHBoxLayout(this);
    vncHLayout->addWidget(mVncRadioBtn);
    vncHLayout->addStretch();

    mVncProtocolFrame->setLayout(vncHLayout);

    mRdpProtocalFrame = new QFrame(this);
    mRdpProtocalFrame->setFrameShape(QFrame::Shape::Box);
    mRdpProtocalFrame->setMinimumSize(550, 50);
    mRdpProtocalFrame->setMaximumSize(960, 50);

    mRdpRadioBtn = new QRadioButton("RDP", mRdpProtocalFrame);
    QHBoxLayout *rdpHLayout = new QHBoxLayout(this);
    rdpHLayout->addWidget(mRdpRadioBtn);
    rdpHLayout->addStretch();

    mRdpProtocalFrame->setLayout(rdpHLayout);

    mBtnGroup = new QButtonGroup(this);
    mBtnGroup->addButton(mVncRadioBtn);
    mBtnGroup->addButton(mRdpRadioBtn);

    mBtnGroup->setId(mVncRadioBtn, Protocol::VNC);
    mBtnGroup->setId(mRdpRadioBtn, Protocol::RDP);
}

void ShareMain::initOutputUI()
{
    mOutputTitleLabel = new QLabel(tr("Output"), this);

    mOutputFrame = new QFrame(this);
    mOutputFrame->setFrameShape(QFrame::Shape::Box);
    mOutputFrame->setMinimumSize(550, 50);
    mOutputFrame->setMaximumSize(960, 50);

    mOutputHLayout = new QHBoxLayout(this);
    mOutputFrame->setLayout(mOutputHLayout);
}

void ShareMain::initInputUI()
{
    mInputTitleLabel = new QLabel(tr("Input"), this);

    mInputFrame = new QFrame(this);
    mInputFrame->setFrameShape(QFrame::Shape::Box);
    mInputFrame->setMinimumSize(550, 50);
    mInputFrame->setMaximumSize(960, 50);

    QHBoxLayout *inputHLayout = new QHBoxLayout();
    //QHBoxLayout *wheelHLayout = new QHBoxLayout();

    mPointBtn = new SwitchButton(this);
    mPointLabel = new QLabel(tr("Point"), this);
    mKeyboardBtn = new SwitchButton(this);
    mKeyboardLabel = new QLabel(tr("Keyboard"), this);
    mClipboardBtn = new SwitchButton(this);
    mClipboardLabel = new QLabel(tr("Clipboard"), this);

    inputHLayout->addWidget(mPointLabel);
    inputHLayout->addWidget(mPointBtn);
    inputHLayout->addStretch();

    inputHLayout->addWidget(mKeyboardLabel);
    inputHLayout->addWidget(mKeyboardBtn);
    inputHLayout->addStretch();

    inputHLayout->addWidget(mClipboardLabel);
    inputHLayout->addWidget(mClipboardBtn);

    mInputFrame->setLayout(inputHLayout);
    mInputFrame->setVisible(false);
}

void ShareMain::initClientUI()
{
    mControlFrame = new QFrame(this);
    mControlFrame->setFrameShape(QFrame::Shape::Box);
    mControlFrame->setMinimumSize(550, 50);
    mControlFrame->setMaximumSize(960, 50);

    QHBoxLayout *controlHLayout = new QHBoxLayout();

    mCloseBtn = new QPushButton(this);
    mCloseBtn->setText(tr("Close"));
    mViewOnlyNBtn = new QPushButton(this);
    mViewOnlyNBtn->setText(tr("ViewOnly"));
    controlHLayout->addStretch();
    controlHLayout->addWidget(mCloseBtn);
    controlHLayout->addWidget(mViewOnlyNBtn);

    mControlFrame->setLayout(controlHLayout);

    //暂时隐藏，不需要显示，只需要后台提供数据
    mClientTitleLabel = new QLabel(tr("Client Setting"), this);

    mClientNumFrame = new QFrame(this);
    mClientNumFrame->setFrameShape(QFrame::Shape::Box);
    mClientNumFrame->setMinimumSize(550, 50);
    mClientNumFrame->setMaximumSize(960, 50);

    QHBoxLayout *clientHLayout = new QHBoxLayout();

    mMaxClientSpinBox = new QSpinBox(this);
    mMaxClientSpinBox->setMinimum(1);
    mMaxClientSpinBox->setMaximum(10);
    mMaxClientLabel = new QLabel(tr("Client Number"), this);

    clientHLayout->addWidget(mMaxClientLabel);
    clientHLayout->addStretch();
    clientHLayout->addWidget(mMaxClientSpinBox);

    mClientNumFrame->setLayout(clientHLayout);

    mClientFrame = new QFrame(this);
    mClientFrame->setFrameShape(QFrame::Shape::Box);
    mClientFrame->setMinimumSize(550, 200);
    mClientFrame->setMaximumSize(960, 500);

    QVBoxLayout *clientVLayout = new QVBoxLayout();

    mClientLabel = new QLabel(tr("Client IP："), this);
    mTbClients = new QTableWidget();

    mTbClients->horizontalHeader()->setDefaultSectionSize(160);
    mTbClients->horizontalHeader()->setSectionsClickable(false); //设置表头不可点击（默认点击后进行排序）

    if (mTbClients->columnCount() < 3)
        mTbClients->setColumnCount(3);
    QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
    mTbClients->setHorizontalHeaderItem(0, __qtablewidgetitem);
    QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
    mTbClients->setHorizontalHeaderItem(1, __qtablewidgetitem1);
    __qtablewidgetitem1->setSizeHint(QSize(10,10));
    QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
    mTbClients->setHorizontalHeaderItem(2, __qtablewidgetitem2);
    mTbClients->setObjectName(QString::fromUtf8("mTbClients"));
    mTbClients->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mTbClients->setSelectionMode(QAbstractItemView::SingleSelection);
    mTbClients->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTbClients->horizontalHeader()->setVisible(true);
    mTbClients->verticalHeader()->setVisible(false);

    QTableWidgetItem *tbClientsItemId = mTbClients->horizontalHeaderItem(0);
    tbClientsItemId->setText("Id");

    QTableWidgetItem *tbClientsItemIP = mTbClients->horizontalHeaderItem(1);
    tbClientsItemIP->setText("Ip Address");
    QTableWidgetItem *tbClientsItemViewOnly = mTbClients->horizontalHeaderItem(2);
    tbClientsItemViewOnly->setText("ViewOnly");

    clientVLayout->addWidget(mClientLabel);
    clientVLayout->addWidget(mTbClients);
    clientVLayout->addStretch();

    mClientFrame->setLayout(clientVLayout);
}

void ShareMain::setFrame()
{
    mVlayout->addWidget(mShareTitleLabel);
    mVlayout->addWidget(mEnableFrame);

    mVlayout->addWidget(mSecurityTitleLabel);
    mVlayout->addWidget(mSecurityPwdFrame);

    mVlayout->addWidget(mProtocolTitleLabel);
    mVlayout->addWidget(mVncProtocolFrame);
    mVlayout->addWidget(mRdpProtocalFrame);

    mVlayout->addWidget(mOutputTitleLabel);
    mVlayout->addWidget(mOutputFrame);

    mVlayout->addWidget(mClientTitleLabel);
    mVlayout->addWidget(mClientFrame);
    mVlayout->addWidget(mControlFrame);
    mClientTitleLabel->setVisible(false);
    mClientFrame->setVisible(false);
    mControlFrame->setVisible(false);

    mVlayout->addWidget(mInputTitleLabel);
    mVlayout->addWidget(mClientNumFrame);
    mVlayout->addWidget(mInputFrame);

    mVlayout->addStretch();

    setFrameVisible(mEnableBtn->isChecked());
}

#if 0
void ShareMain::viewBoxSlot()
{
    int row = mTbClients->currentRow();
    if (row < 0)
        return;
    int id = mTbClients->item(row, 0)->text().toInt();
    bool view = mTbClients->item(row, 2)->text().toUInt();
    krd->SetViewOnly(id, !view);
}

void ShareMain::closeAllClient()
{
    ClientInfo client_info = krd->clientsInfo();
    mTbClients->setRowCount(0);

    foreach (QVariantMap client, client_info) {
        int id = client.value("id").toInt();
        krd->CloseClient(id);
    }
}

//托盘功能，暂时屏蔽，后续开发
//1.头文件声明：

//2.窗口初始化时创建托盘对象：

void ShareMain::initSysTrayIcon()
{
    mSysTrayIcon = new QSystemTrayIcon(this);
    //新建托盘要显示的icon
    QIcon icon = QIcon(":/new/images/im_icon.png");
    //将icon设到QSystemTrayIcon对象中
    mSysTrayIcon->setIcon(icon);
    //当鼠标移动到托盘上的图标时，会显示此处设置的内容
    mSysTrayIcon->setToolTip(QObject::trUtf8("xxx客户端"));
    //给QSystemTrayIcon添加槽函数
    connect(mSysTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason)));
    //在系统托盘显示此对象
    mSysTrayIcon->show();
}

//3.槽函数实现点击事件。
//系统托盘功能
void ShareMain::on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        //单击托盘图标，显示窗体

        this->show();
        break;
    case QSystemTrayIcon::DoubleClick:
        //双击托盘图标
        //双击后显示主程序窗口
        QMessageBox::information(this, "xxxx" , "xxxx");
        break;
//    case QSystemTrayIcon::
    default:
        break;
    }
}

//4.体现图标作用：决定是否退出程序或缩小图标
// closeEvents
void ShareMain::closeEvent(QCloseEvent *event)
{
    QMessageBox button(QMessageBox::Warning, QString(tr("退出程序")),QString(tr("确认退出程序！")));
    button.setStandardButtons (QMessageBox::Yes|QMessageBox::No);
    button.setSizeIncrement(250,120);
    button.setButtonText (QMessageBox::Yes,QString("确定退出"));
    button.setButtonText (QMessageBox::No,QString("缩小托盘"));

    if(button.exec() != QMessageBox::Yes)
    {
        this->hide();
        event->ignore();
    }else{
        event->accept();
    }
}
#endif

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
#include "createuserinteldialog.h"
#include "ui_createuserinteldialog.h"

#include "passwdcheckutil.h"

#include <QDebug>
#include <QDir>

#define MOBILE 0
#define PC 1
#define PWD_LOW_LENGTH 6
#define PWD_HIGH_LENGTH 20
#define PIN_LENGTH 6
#define USER_LENGTH 32

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

CreateUserIntelDialog::CreateUserIntelDialog(QStringList userlist, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateUserIntelDialog),
    usersStringList(userlist)
{
//    installEventFilter(this);
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->titleLabel->setStyleSheet("QLabel{font-size: 18px; color: palette(windowText);}");
    ui->closeBtn->setProperty("useIconHighlightEffect", true);
    ui->closeBtn->setProperty("iconHighlightEffectMode", 1);
    ui->closeBtn->setFlat(true);

    ui->closeBtn->setStyleSheet("QPushButton:hover:!pressed#closeBtn{background: #FA6056; border-radius: 4px;}"
                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border-radius: 4px;}");
//    ui->tipLabel->setAlignment(Qt::AlignCenter);

    ui->label_8->adjustSize();
    ui->label_8->setWordWrap(true);

    ui->label_10->adjustSize();
    ui->label_10->setWordWrap(true);

    ui->closeBtn->setIcon(QIcon("://img/titlebar/close.svg"));

    initPwdChecked();
    setupComonpent();
    setupConnect();
}

CreateUserIntelDialog::~CreateUserIntelDialog()
{
    delete ui;
//    delete process;
}

void CreateUserIntelDialog::initPwdChecked(){

#ifdef ENABLEPQ
    int ret;
    void *auxerror;
    char buf[255];

    settings = pwquality_default_settings();
    if (settings == NULL) {
        enablePwdQuality = false;
        qDebug() << "init pwquality settings failed";
    } else {
        enablePwdQuality = true;
    }

    ret = pwquality_read_config(settings, PWCONF, &auxerror);
    if (ret != 0){
        enablePwdQuality = false;
        qDebug() << "Reading pwquality configuration file failed: " << pwquality_strerror(buf, sizeof(buf), ret, auxerror);
    } else {
        enablePwdQuality = true;
    }

    if (PasswdCheckUtil::getCurrentPamState()){
        enablePwdQuality = true;
    } else {
        enablePwdQuality = false;
    }

#else
    enablePwdQuality = false;
#endif

}

void CreateUserIntelDialog::setupComonpent(){
    nameTip = "";
    pwdTip = "";
    pwdSureTip = "";

    //给选项卡加背景色，此部分样式应由主题控制，此处只为预览样式改进效果
    ui->widget->setStyleSheet(".QWidget{background: rgba(245,245,245,1); border-radius: 8px;}");
    ui->widget_2->setStyleSheet(".QWidget{background: rgba(245,245,245,1); border-radius: 8px;}");

    ui->pwdLineEdit->setEchoMode(QLineEdit::Password);
    ui->pwdsureLineEdit->setEchoMode(QLineEdit::Password);
//    ui->pinLineEdit->setEchoMode(QLineEdit::Password);
//    ui->pinsureLineEdit->setEchoMode(QLineEdit::Password);

//    QString required = tr("(Required)");
//    QString optional = tr("(Optional)");
//    if (ostype == PC){
//        ui->pwdLabel->setText(required);
//        ui->pwdsurelabel->setText(required);
//        ui->pinLabel->setText(optional);
//        ui->pinsurelabel->setText(optional);
//    }
//    else{
//        ui->pwdLabel->setText(optional);
//        ui->pwdsurelabel->setText(optional);
//        ui->pinLabel->setText(required);
//        ui->pinsurelabel->setText(required);
//    }

    ui->usernameLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                        "background: palett(base);}");
    ui->pwdLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                      "background: palett(base);}");
    ui->pwdsureLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                      "background: palett(base);}");
    ui->usernameLineEdit->installEventFilter(this);
    ui->pwdLineEdit->installEventFilter(this);
    ui->pwdsureLineEdit->installEventFilter(this);

    ui->usernameLineEdit->setPlaceholderText(tr("UserName"));
    ui->pwdLineEdit->setPlaceholderText(tr("Password"));
    ui->pwdsureLineEdit->setPlaceholderText(tr("Password Identify"));

    ui->usernameLineEdit->setTextMargins(16,0,32,0);
    ui->pwdLineEdit->setTextMargins(16,0,32,0);
    ui->pwdsureLineEdit->setTextMargins(16,0,32,0);

    pwdLineEditHLayout = new QHBoxLayout();
    pwdsureLineEditHLayout = new QHBoxLayout();

    showPwdBtn = new QPushButton();
    showPwdBtn->setFixedSize(QSize(16, 16));

    pwdLineEditHLayout->addStretch();
    pwdLineEditHLayout->addWidget(showPwdBtn);
    pwdLineEditHLayout->setContentsMargins(0,0,16,0);
    ui->pwdLineEdit->setLayout(pwdLineEditHLayout);

    showSurePwdBtn = new QPushButton();
    showSurePwdBtn->setFixedSize(QSize(16, 16));

    pwdsureLineEditHLayout->addStretch();
    pwdsureLineEditHLayout->addWidget(showSurePwdBtn);
    pwdsureLineEditHLayout->setContentsMargins(0,0,16,0);
    ui->pwdsureLineEdit->setLayout(pwdsureLineEditHLayout);

    showPwdBtn->setCursor(QCursor(Qt::ArrowCursor));
    showSurePwdBtn->setCursor(QCursor(Qt::ArrowCursor));

    showPwdBtn->setStyleSheet("QPushButton{border-image: url(:/img/plugins/userinfo_intel/pwdEncrypted.png);}");
    showSurePwdBtn->setStyleSheet("QPushButton{border-image: url(:/img/plugins/userinfo_intel/pwdEncrypted.png);}");

//    connect(showPwdBtn, &QPushButton::pressed, this, [=]{
//        ui->pwdLineEdit->setEchoMode(QLineEdit::Normal);
//    });
//    connect(showPwdBtn, &QPushButton::released, this, [=]{
//        ui->pwdLineEdit->setEchoMode(QLineEdit::Password);
//        ui->pwdLineEdit->setFocus();
//    });
//    connect(showSurePwdBtn, &QPushButton::pressed, this, [=]{
//        ui->pwdsureLineEdit->setEchoMode(QLineEdit::Normal);
//    });
//    connect(showSurePwdBtn, &QPushButton::released, this, [=]{
//        ui->pwdsureLineEdit->setEchoMode(QLineEdit::Password);
//        ui->pwdsureLineEdit->setFocus();
//    });
    connect(showPwdBtn, &QPushButton::clicked, this, [=]{
        if(isPwdVisibel){
            ui->pwdLineEdit->setEchoMode(QLineEdit::Password);
            isPwdVisibel = false;
            showPwdBtn->setStyleSheet("QPushButton{border-image: url(:/img/plugins/userinfo_intel/pwdEncrypted.png);}");

        } else {
            ui->pwdLineEdit->setEchoMode(QLineEdit::Normal);
            isPwdVisibel = true;
            showPwdBtn->setStyleSheet("QPushButton{border-image: url(:/img/plugins/userinfo_intel/pwdUnscramble.png);}");
        }
        ui->usernameLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                          "background: palett(base);}");
        ui->pwdLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                          "background: palett(base);"
                                          "border: 2px solid #2FB3E8;}");
        ui->pwdsureLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                          "background: palett(base);}");
        ui->pwdLineEdit->setFocus();
    });
    connect(showSurePwdBtn, &QPushButton::clicked, this, [=]{
        if(isSurePwdVisibel){
            ui->pwdsureLineEdit->setEchoMode(QLineEdit::Password);
            isSurePwdVisibel = false;
            showSurePwdBtn->setStyleSheet("QPushButton{border-image: url(:/img/plugins/userinfo_intel/pwdEncrypted.png);}");

        } else {
            ui->pwdsureLineEdit->setEchoMode(QLineEdit::Normal);
            isSurePwdVisibel = true;
            showSurePwdBtn->setStyleSheet("QPushButton{border-image: url(:/img/plugins/userinfo_intel/pwdUnscramble.png);}");
        }
        ui->usernameLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                          "background: palett(base);}");
        ui->pwdLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                          "background: palett(base);}");
        ui->pwdsureLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                          "background: palett(base);"
                                          "border: 2px solid #2FB3E8;}");
        ui->pwdsureLineEdit->setFocus();
    });
//    ui->pinLineEdit->setPlaceholderText(tr("PIN Code"));
//    ui->pinsureLineEdit->setPlaceholderText(tr("PIN Code Identify"));

//    ui->pwdTypeComBox->addItem(tr("General Password"));

//    //给radiobtn设置id，id即accoutnType，方便直接返回id值
    ui->buttonGroup->setId(ui->standardRadioBtn, 0);
    ui->buttonGroup->setId(ui->adminRadioBtn, 1);

//    //默认标准用户
    ui->standardRadioBtn->setChecked(true);

    //设置确定按钮
    refreshConfirmBtnStatus();

//    confirm_btn_status_refresh();
}

bool CreateUserIntelDialog::eventFilter(QObject *w, QEvent *e) {
    if(w == ui->usernameLineEdit) {
        if(e->type() == QEvent::MouseButtonPress) {
            ui->usernameLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                              "background: palett(base);"
                                              "border: 2px solid #2FB3E8;}");
            ui->pwdLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                              "background: palett(base);}");
            ui->pwdsureLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                              "background: palett(base);}");
        }
    } else if(w == ui->pwdLineEdit) {
        if(e->type() == QEvent::MouseButtonPress) {
            ui->usernameLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                              "background: palett(base);}");
            ui->pwdLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                              "background: palett(base);"
                                              "border: 2px solid #2FB3E8;}");
            ui->pwdsureLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                              "background: palett(base);}");
        }
    } else if(w == ui->pwdsureLineEdit) {
        if(e->type() == QEvent::MouseButtonPress) {
            ui->usernameLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                              "background: palett(base);}");
            ui->pwdLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                              "background: palett(base);}");
            ui->pwdsureLineEdit->setStyleSheet("QLineEdit{border-radius: 8px;"
                                              "background: palett(base);"
                                              "border: 2px solid #2FB3E8;}");
        }
    }
    return QWidget::eventFilter(w,e);
}

void CreateUserIntelDialog::setupConnect(){

    connect(ui->closeBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        close();
    });

    connect(ui->usernameLineEdit, &QLineEdit::textChanged, [=](QString text){
        nameLegalityCheck(text);
    });

    connect(ui->pwdLineEdit, &QLineEdit::textChanged, [=](QString text){
        pwdLegalityCheck(text);
    });

    connect(ui->pwdsureLineEdit, &QLineEdit::textChanged, [=](QString text){
        if (!text.isEmpty() && text != ui->pwdLineEdit->text()){
            pwdSureTip = tr("Inconsistency with pwd");
        } else {
            pwdSureTip = "";
        }

        ui->tipLabel->setText(pwdSureTip);
//        if (pwdSureTip.isEmpty()){
//            pwdTip.isEmpty() ? ui->tipLabel->setText(nameTip) : ui->tipLabel->setText(pwdTip);
//        }

        refreshConfirmBtnStatus();
    });

    //    connect(ui->confirmPushBtn, SIGNAL(clicked(bool)), this, SLOT(confirm_btn_clicked_slot()));
    //    connect(ui->usernameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(name_legality_check_slot(QString)));
    //    connect(ui->pwdLineEdit, SIGNAL(textChanged(QString)), this, SLOT(pwd_legality_check_slot(QString)));
    //    connect(ui->pwdsureLineEdit, SIGNAL(textChanged(QString)), this, SLOT(pwdsure_legality_check_slot(QString)));

    connect(ui->confirmBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        this->accept();
        QString uName, pwd, pin;

        uName = ui->usernameLineEdit->text();
//        if (ui->pwdTypeComBox->currentIndex() == 0){
//            pwd = ui->pwdLineEdit->text();
//            pin = "";
//        } else {
//            pwd = "";
//            pin = ui->pwdLineEdit->text();
//        }
        pwd = ui->pwdLineEdit->text();
        pin = "";

        emit newUserWillCreate(uName, pwd, pin, ui->buttonGroup->checkedId());

    });

    connect(ui->cancelBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        reject();
    });
}

//void CreateUserIntelDialog::set_face_label(QString iconfile){
//    ui->faceLabel->setPixmap(QPixmap(iconfile).scaled(QSize(80, 80)));
//}

//void CreateUserIntelDialog::tiplabel_status_refresh(){
//    if (ui->usernametipLabel->text().isEmpty())
//        ui->usernametipLabel->hide();
//    else
//        ui->usernametipLabel->show();

//    if (ui->pwdtipLabel->text().isEmpty())
//        ui->pwdtipLabel->hide();
//    else
//        ui->pwdtipLabel->show();

//    if (ui->pwdsuretipLabel->text().isEmpty())
//        ui->pwdsuretipLabel->hide();
//    else
//        ui->pwdsuretipLabel->show();

//    if (ui->pintipLabel->text().isEmpty())
//        ui->pintipLabel->hide();
//    else
//        ui->pintipLabel->show();

//    if (ui->pinsuretipLabel->text().isEmpty())
//        ui->pinsuretipLabel->hide();
//    else
//        ui->pinsuretipLabel->show();
//}

void CreateUserIntelDialog::setRequireLabel(QString msg){
//    ui->requireLabel->setText(msg);
}

void CreateUserIntelDialog::refreshConfirmBtnStatus(){
    if (ui->usernameLineEdit->text().isEmpty() ||
            ui->pwdLineEdit->text().isEmpty() ||
            ui->pwdsureLineEdit->text().isEmpty() ||
            !nameTip.isEmpty() || !pwdTip.isEmpty() || !pwdSureTip.isEmpty())
        ui->confirmBtn->setEnabled(false);
    else
        ui->confirmBtn->setEnabled(true);
}


void CreateUserIntelDialog::pwdLegalityCheck(QString pwd){
    if (enablePwdQuality){
#ifdef ENABLEPQ
        void * auxerror;
        int ret;
        const char * msg;
        char buf[256];

        QByteArray ba = pwd.toLatin1();

        ret = pwquality_check(settings, ba.data(), NULL, NULL, &auxerror);
        if (ret < 0 && pwd.length() > 0){
            msg = pwquality_strerror(buf, sizeof(buf), ret, auxerror);
            pwdTip = QString(msg);
        } else {
            pwdTip = "";
        }
#endif
    } else {
        if (pwd.length() < PWD_LOW_LENGTH) {
            pwdTip = tr("Password length needs to more than %1 character!").arg(PWD_LOW_LENGTH - 1);
        } else if (pwd.length() > PWD_HIGH_LENGTH) {
            pwdTip = tr("Password length needs to less than %1 character!").arg(PWD_HIGH_LENGTH + 1);
        } else {
            pwdTip = "";
        }
    }

    //防止先输入确认密码，再输入密码后pwdsuretipLabel无法刷新
    if (!ui->pwdsureLineEdit->text().isEmpty()){
        if (ui->pwdLineEdit->text() == ui->pwdsureLineEdit->text()) {
            pwdSureTip = "";
        } else {
            pwdSureTip = tr("Inconsistency with pwd");
        }
    }

    ui->tipLabel_2->setText(pwdTip);
//    if (pwdTip.isEmpty()){
//        pwdSureTip.isEmpty() ? ui->tipLabel->setText(nameTip) : ui->tipLabel->setText(pwdSureTip);
//    }

    refreshConfirmBtnStatus();
}


void CreateUserIntelDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    QRect rect = this->rect();
    rect.setWidth(rect.width()-0);
    rect.setHeight(rect.height()-0);
    rectPath.addRoundedRect(rect,12,12);
    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.setOpacity(0.65);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    // 绘制一个背景
    p.save();
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.restore();

}

void CreateUserIntelDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ) {
        if( ui->confirmBtn->isEnabled()) {
            ui->confirmBtn->click();
        }
        return;
    }
    QDialog::keyPressEvent(event);
}

QStringList CreateUserIntelDialog::getHomeUser()
{
    QStringList homeList;
    QDir dir("/home");
    if (dir.exists())
    {
        homeList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    }

    return homeList;
}

bool CreateUserIntelDialog::nameTraverse(QString username){
    QString::const_iterator cit = NULL;
    for (cit = username.cbegin(); cit < username.cend(); cit++){
        QString str = *cit;
        if (str.contains(QRegExp("[a-z]"))){
        } else if (str.contains(QRegExp("[0-9]"))){
        } else if (str.contains("_")){
        } else{
            return false;
        }
    }
    return true;
}

void CreateUserIntelDialog::nameLegalityCheck(QString username){

    if (username.isEmpty())
        nameTip = tr("The user name cannot be empty");
    else if (username.startsWith("_") || username.left(1).contains((QRegExp("[0-9]")))){
        nameTip = tr("The first character must be lowercase letters!");
    }
    else if (username.contains(QRegExp("[A-Z]"))){
        nameTip = tr("User name can not contain capital letters!");
    }
    else if (nameTraverse(username))
        if (username.length() > 0 && username.length() < USER_LENGTH){
            /*
             * 此处代码需要优化
             */
//            back = false;
//            QString cmd = QString("getent group %1").arg(username);
//            process = new QProcess(this);
//            connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(name_conflict_group_slot()));
//            process->start(cmd);

            if (usersStringList.contains(username)){
                nameTip = tr("The user name is already in use, please use a different one.");
            } else {
                nameTip = "";
            }
        } else {
            nameTip = tr("User name length need to less than %1 letters!").arg(USER_LENGTH);
    } else {
        nameTip = tr("The user name can only be composed of letters, numbers and underline!");
    }

     QStringList homeDir = getHomeUser();
     if (homeDir.contains(username) && nameTip.isEmpty()) {
         nameTip = tr("The username is configured, please change the username");
     }

    ui->tipLabel_3->setText(nameTip);

//    if (nameTip.isEmpty()){
//        pwdTip.isEmpty() ? ui->tipLabel->setText(pwdSureTip) : ui->tipLabel->setText(pwdTip);
//    }

    refreshConfirmBtnStatus();
}
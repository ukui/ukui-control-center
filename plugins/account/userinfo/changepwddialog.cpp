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
#include "changepwddialog.h"
#include "ui_changepwddialog.h"

#include "elipsemaskwidget.h"
#include "passwdcheckutil.h"

#include <QStyledItemDelegate>

#include <QDebug>

#include "CloseButton/closebutton.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>

}


#define PWD_LOW_LENGTH 6
#define PWD_HIGH_LENGTH 20


extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);


ChangePwdDialog::ChangePwdDialog(bool _isCurrentUser, QString _username, QWidget *parent) :
    QDialog(parent),
    isCurrentUser(_isCurrentUser),
    ui(new Ui::ChangePwdDialog),
    currentUserName(_username)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Change pwd"));

    curPwdTip = "";
    pwdChecking = false;

    ui->titleLabel->setStyleSheet("QLabel{color: palette(windowText);}");
    ui->pwdFrame->setFrameShape(QFrame::Shape::Box);
    ui->tipLabel->setAlignment(Qt::AlignCenter);
    ui->tipLabel->setStyleSheet("color:red;");
    ui->pwdtypeComboBox->setText(tr("general password"));

//    ui->closeBtn->setIcon(QIcon("://img/titlebar/close.svg"));

//    isCurrentUser = true;

    pcThread = new PwdCheckThread();

    remoteUser = isRemoteUser();


    initPwdChecked();
    setupComponent();
    setupConnect();
}

ChangePwdDialog::~ChangePwdDialog()
{
    delete ui;
    ui = nullptr;

//    pcThread->terminate();
//    delete pcThread;
}

bool ChangePwdDialog::isRemoteUser(){

    FILE * stream;
    char output[256];

    char * command = "cat /etc/passwd | awk -F : '{print$1}'";
    bool result;

    QStringList userslist;

    if ((stream = popen(command, "r")) == NULL){
        return false;
    }

    while(fgets(output, 256, stream) != NULL){
        userslist.append(QString(output).simplified());
    }

    if (userslist.contains(currentUserName)){
        result = false;
    } else {
        result = true;
    }

    pclose(stream);
    return result;

}

void ChangePwdDialog::initPwdChecked(){

    if (remoteUser){
        enablePwdQuality = false;
        return;
    }

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

    if (PasswdCheckUtil::getCurrentPamState())
        enablePwdQuality = true;
    else
        enablePwdQuality = false;

#else
    enablePwdQuality = false;
#endif
}

void ChangePwdDialog::setupComponent(){

    ElipseMaskWidget * cpMaskWidget = new ElipseMaskWidget(ui->faceLabel);
    cpMaskWidget->setGeometry(0, 0, ui->faceLabel->width(), ui->faceLabel->height());

    ui->curPwdLineEdit->setEchoMode(QLineEdit::Password);
    ui->curPwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->pwdLineEdit->setEchoMode(QLineEdit::Password);
    ui->pwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->pwdsureLineEdit->setEchoMode(QLineEdit::Password);
    ui->pwdsureLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->curPwdLineEdit->setPlaceholderText(tr("Current Password"));
    ui->pwdLineEdit->setPlaceholderText(tr("New Password"));
    ui->pwdsureLineEdit->setPlaceholderText(tr("New Password Identify"));

    refreshConfirmBtnStatus();
}

bool ChangePwdDialog::isDaShangSuo()
{
    // 大连商品交易所
    QProcess *process = new QProcess;
    process->start("grep -r 大连商品交易所 /etc/.kyinfo");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();
    delete process;
    QString mOutput = QString(ba.data());
    return mOutput.contains("大连商品交易所");
}

bool ChangePwdDialog::isSudoGroupNumber(QString uname)
{
    QString cmd = QString("cat /etc/group | grep sudo | awk -F: '{ print $NF}'");
    QString output;

    FILE   *stream;
    char buf[256];

    if ((stream = popen(cmd.toLatin1().data(), "r" )) == NULL){
        return false;
    }

    while(fgets(buf, 256, stream) != NULL){
        output = QString(buf).simplified();
    }

    pclose(stream);

    QStringList users = output.split(",");

    if (users.contains(uname)){
        return true;
    } else {
        return false;
    }
}

void ChangePwdDialog::setupConnect(){

    connect(pcThread, &PwdCheckThread::complete, this, [=](QString re){
        curPwdTip = re;

        if (re.isEmpty()){ //密码校验成功
            // 大连商品交易所，当前登录用户为普通用户，同步修改密码
            if (isDaShangSuo() && !isSudoGroupNumber(currentUserName)) {
                QNetworkRequest request;
                QNetworkAccessManager* naManager = new QNetworkAccessManager(this);
                QMetaObject::Connection connRet = QObject::connect(naManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));
                Q_ASSERT(connRet);

                request.setUrl(QUrl("https//iam.dce.com.cn"));

                // 设置请求头
                QString appId = "875500bf-2035-4942-8c97-2ef31168734a";
                QString secret = "8d4097a9-29d7-4e2a-86be-d80a148b667f";
                QString timeStamp = QString::number(QDateTime::currentMSecsSinceEpoch() / 1000);
                QString appToken = appId + secret + timeStamp;
                QCryptographicHash SHA256(QCryptographicHash::Sha256);
                SHA256.addData(appToken.toLatin1().data());

                QJsonObject headerJson;
                headerJson.insert("X-App-Token", SHA256.result().data());
                headerJson.insert("X-App-Id", appId);
                headerJson.insert("X-Timestamp", timeStamp);
                headerJson.insert("Content-Type", "application/json");

                request.setHeader(QNetworkRequest::ContentTypeHeader, headerJson);

                // 发送post请求
                QJsonObject bodyJson;
                bodyJson.insert("userName", currentUserName);
                bodyJson.insert("oldPassword", QString::fromStdString(encryptByPublicKey(ui->curPwdLineEdit->text().toStdString())));
                bodyJson.insert("newPassword", QString::fromStdString(encryptByPublicKey(ui->pwdLineEdit->text().toStdString())));

                QJsonDocument m_httpDocum;
                m_httpDocum.setObject(bodyJson);
                QByteArray  m_httpData = m_httpDocum.toJson(QJsonDocument::Compact);
                QNetworkReply* reply = naManager->post(request, m_httpData);

            } else {
                this->accept();

                emit passwd_send(ui->curPwdLineEdit->text(), ui->pwdLineEdit->text());
            }
        } else {

            if (curPwdTip.contains("Failed")){
                curPwdTip = tr("Authentication failed, input authtok again!");
            }

            if (QLabelSetText(ui->tipLabel, curPwdTip)){
                ui->tipLabel->setToolTip(curPwdTip);
            }

            ui->curPwdLineEdit->setText("");

            refreshConfirmBtnStatus();
        }

        pwdChecking = false;

        refreshCancelBtnStatus();

    });

    if (isCurrentUser){

        if (remoteUser) {
            connect(ui->confirmPushBtn, &QPushButton::clicked, [=]{

                this->accept();

                emit passwd_send3(ui->curPwdLineEdit->text(), ui->pwdLineEdit->text());
            });
        } else {
            connect(ui->curPwdLineEdit, &QLineEdit::textChanged, [=](QString txt){

                if (!txt.isEmpty()){
                    curPwdTip = "";
                    if (QLabelSetText(ui->tipLabel, curPwdTip)){
                        ui->tipLabel->setToolTip(curPwdTip);
                    }
                    pwdLegalityCheck();
                }

                refreshConfirmBtnStatus();
            });

            connect(ui->confirmPushBtn, &QPushButton::clicked, [=]{
                if (pwdChecking)
                    return;

                pcThread->setArgs(currentUserName, ui->curPwdLineEdit->text());

                pcThread->start();

                pwdChecking = true;

                refreshCancelBtnStatus();

            });

        }
    } else {
        connect(ui->confirmPushBtn, &QPushButton::clicked, [=]{
            this->accept();

            emit passwd_send2(ui->pwdLineEdit->text());
        });
    }

    connect(ui->pwdLineEdit, &QLineEdit::textChanged, [=]{
        pwdLegalityCheck();

        refreshConfirmBtnStatus();
    });
    connect(ui->pwdsureLineEdit, &QLineEdit::textChanged, [=](QString text){
        if (!text.isEmpty() && text != ui->pwdLineEdit->text()){
            pwdSureTip = tr("Inconsistency with pwd");
        } else {
            pwdSureTip = "";
        }

        ui->tipLabel->setText(pwdSureTip);
        if (pwdSureTip.isEmpty()){
            if (!pwdTip.isEmpty()){
                if (QLabelSetText(ui->tipLabel, pwdTip)){
                    ui->tipLabel->setToolTip(pwdTip);
                }
            } else if (!curPwdTip.isEmpty()){
                if (QLabelSetText(ui->tipLabel, curPwdTip)){
                    ui->tipLabel->setToolTip(curPwdTip);
                }
            }
        }

        refreshConfirmBtnStatus();
    });

    connect(ui->cancelPushBtn, &QPushButton::clicked, [=]{
//        reject();
        close();
    });
}

void LoadKey( const string& filename, RSA::PublicKey& PublicKey )
{
    // DER Encode Key - X.509 key format
    PublicKey.Load(
        FileSource( filename.c_str(), true, NULL, true /*binary*/ ).Ref()
    );
}

std::string ChangePwdDialog::encryptByPublicKey(std::string data) {
    std::string cipher;
    std::string result;
    try
    {
        ////////////////////////////////////////////////
        // Generate keys
        AutoSeededRandomPool rng;
        InvertibleRSAFunction parameters;
        parameters.GenerateRandomWithKeySize( rng, 1024 );
        RSA::PrivateKey privateKey( parameters );
        RSA::PublicKey publicKey( parameters );
        LoadKey("/var/idlink_publicKey",publicKey);
        CryptoPP::RSAES_PKCS1v15_Encryptor e(publicKey);

        StringSource( data, true,
                new PK_EncryptorFilter( rng, e,
                new CryptoPP::Base64Encoder(new CryptoPP::StringSink(cipher))) // PK_EncryptorFilter
              ); // StringSource

        std::regex pattern(" |\n|\r|\t");
        std::string fmt = "";
        result = std::regex_replace(cipher, pattern, fmt);
    }
    catch( CryptoPP::Exception& e )
    {
        cout << "Caught Exception..." << endl;
        cout << e.what() << endl;
    }

    return result;
}


void ChangePwdDialog::requestFinished(QNetworkReply* reply) {
    qDebug() << "reply " << reply << "*********";
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::information(this, tr("Tips"), tr("timed out, Modified failed!"));
    }
    QByteArray responseData = reply->readAll();
    QJsonParseError json_error;
    QJsonDocument doucment = QJsonDocument::fromJson(responseData, &json_error);
    if (json_error.error == QJsonParseError::NoError) {
        if (doucment.isObject()) {
            const QJsonObject obj = doucment.object();
            qDebug() << obj;
            if (obj.contains("result")) {
                bool result = obj.value("result").toBool();
                if (result) {
                    // tips success
                    QMessageBox::information(this, tr("Tips"), tr("Remote modified successfully!"));
                    this->accept();

                    emit passwd_send(ui->curPwdLineEdit->text(), ui->pwdLineEdit->text());
                }
            }
            if (obj.contains("errorCode")) {
                QString errorCode = obj.value("errorCode").toString();
                if (errorCode != nullptr && obj.contains("errorMsg")) {
                    QString errorMsg = obj.value("errorMsg").toString();
                    QMessageBox::information(this, tr("Tips"), errorMsg);
                }
            }
        }
    }
}


void ChangePwdDialog::setFace(QString iconfile){
    ui->faceLabel->setPixmap(QPixmap(iconfile).scaled(QSize(80, 80)));

}

void ChangePwdDialog::setUsername(QString realname){
    if (QLabelSetText(ui->usernameLabel, realname)){
        ui->usernameLabel->setToolTip(realname);
    }
}

void ChangePwdDialog::setAccountType(QString aType){
    ui->aTypeLabel->setText(aType);
}

void ChangePwdDialog::haveCurrentPwdEdit(bool have){
    ui->curPwdLineEdit->setVisible(have);
    ui->label->setVisible(have);
}

void ChangePwdDialog::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 6, 6);

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

void ChangePwdDialog::pwdLegalityCheck(){
    //
    if (!checkCharLegitimacy(ui->pwdLineEdit->text())){
        pwdTip = tr("Contains illegal characters!");
    } else if (QString::compare(ui->pwdLineEdit->text(), ui->curPwdLineEdit->text()) == 0 && !ui->pwdLineEdit->text().isEmpty()){
        pwdTip = tr("Same with old pwd");
    } else {
        if (enablePwdQuality){
#ifdef ENABLEPQ
            void * auxerror;
            int ret;
            const char * msg;
            char buf[256];

            QByteArray ba = ui->pwdLineEdit->text().toLatin1();
            QByteArray ba1 = ui->curPwdLineEdit->text().toLatin1();

            if (isCurrentUser){
               ret = pwquality_check(settings, ba.data(), ba1.data(), currentUserName.toLatin1().data(), &auxerror);
            } else {
               ret = pwquality_check(settings, ba.data(), NULL, currentUserName.toLatin1().data(), &auxerror);
            }

            if (ret < 0 && ui->pwdLineEdit->text().length() > 0){
                msg = pwquality_strerror(buf, sizeof(buf), ret, auxerror);
                pwdTip = QString(msg);
            } else {
                pwdTip = "";
            }
#endif

        } else { //系统未开启pwdquality模块
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

    if (QLabelSetText(ui->tipLabel, pwdTip)){
        ui->tipLabel->setToolTip(pwdTip);
    }
    if (pwdTip.isEmpty()){
        if (!pwdSureTip.isEmpty()){
            if (QLabelSetText(ui->tipLabel, pwdSureTip)){
                ui->tipLabel->setToolTip(pwdSureTip);
            }

        } else if (!curPwdTip.isEmpty()){
            if (QLabelSetText(ui->tipLabel, curPwdTip)){
                ui->tipLabel->setToolTip(curPwdTip);
            }
        }
    }
}

bool ChangePwdDialog::checkCharLegitimacy(QString password){
    //需要用'在shell解释器中做强引用
    if (password.contains("'"))
        return false;

    foreach (QChar ch, password){
        if (int(ch.toLatin1() <= 0 || int(ch.toLatin1()) > 127)){
            return false;
        }
    }
    return true;
}

void ChangePwdDialog::refreshCancelBtnStatus(){
    if (pwdChecking){
        ui->cancelPushBtn->setEnabled(false);
    } else {
        ui->cancelPushBtn->setEnabled(true);
    }
}


void ChangePwdDialog::refreshConfirmBtnStatus(){

    if (getuid() && isCurrentUser){
        if (!ui->tipLabel->text().isEmpty() || \
                ui->curPwdLineEdit->text().isEmpty() || ui->curPwdLineEdit->text() == tr("Current Password") || \
                ui->pwdLineEdit->text().isEmpty() || ui->pwdLineEdit->text() == tr("New Password") || \
                ui->pwdsureLineEdit->text().isEmpty() || ui->pwdsureLineEdit->text() == tr("New Password Identify") ||
                !curPwdTip.isEmpty() || !pwdTip.isEmpty() || !pwdSureTip.isEmpty())
            ui->confirmPushBtn->setEnabled(false);
        else
            ui->confirmPushBtn->setEnabled(true);
    } else {
        if (!ui->tipLabel->text().isEmpty() || \
                ui->pwdLineEdit->text().isEmpty() || ui->pwdLineEdit->text() == tr("New Password") || \
                ui->pwdsureLineEdit->text().isEmpty() || ui->pwdsureLineEdit->text() == tr("New Password Identify") ||
                !pwdTip.isEmpty() || !pwdSureTip.isEmpty())
            ui->confirmPushBtn->setEnabled(false);
        else
            ui->confirmPushBtn->setEnabled(true);
    }
}

bool ChangePwdDialog::QLabelSetText(QLabel *label, QString string)
{
    bool is_over_length = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);
    QString str = string;
    if (fontSize > (label->width()-5)) {
        str = fontMetrics.elidedText(string, Qt::ElideRight, label->width()-10);
        is_over_length = true;
    }
    label->setText(str);
    return is_over_length;
}


void ChangePwdDialog::keyPressEvent(QKeyEvent * event){
    switch (event->key())
    {
    case Qt::Key_Escape:
        break;
    case Qt::Key_Enter:
        break;
    case Qt::Key_Return:
        if (ui->confirmPushBtn->isEnabled())
            ui->confirmPushBtn->clicked();
        break;
    default:
        QDialog::keyPressEvent(event);
    }
}

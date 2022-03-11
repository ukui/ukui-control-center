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
#ifndef CHANGEPWDDIALOG_H
#define CHANGEPWDDIALOG_H

#include <QDialog>
#include <QPainter>
#include <QPainterPath>
#include <QThread>
#include <QLabel>
#include <QTimer>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonParseError>
#include <QProcess>

#include <QMessageBox>

#include "pwdcheckthread.h"

#include "cryptopp/rsa.h"
using CryptoPP::RSA;
using CryptoPP::InvertibleRSAFunction;
using CryptoPP::RSAES_OAEP_SHA_Encryptor;
using CryptoPP::RSAES_OAEP_SHA_Decryptor;

#include "cryptopp/sha.h"
using CryptoPP::SHA1;

#include "cryptopp/filters.h"
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::PK_EncryptorFilter;
using CryptoPP::PK_DecryptorFilter;

#include "cryptopp/files.h"
using CryptoPP::FileSink;
using CryptoPP::FileSource;

#include "cryptopp/osrng.h"
using CryptoPP::AutoSeededRandomPool;

#include "cryptopp/secblock.h"
using CryptoPP::SecByteBlock;

#include "cryptopp/cryptlib.h"
using CryptoPP::Exception;
using CryptoPP::DecodingResult;

#include <string>
using std::string;

#include <exception>
using std::exception;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <assert.h>
#include <regex>
#include "cryptopp/base64.h"

#include <cryptopp/filters.h>
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::PK_EncryptorFilter;
using CryptoPP::PK_DecryptorFilter;

#include <iostream> //std:cerr
#include <sstream>  //std::stringstream
#include <string>
#include <random>
using std::string;
#include <exception>
using std::exception;
using std::cout;
using std::cerr;
using std::endl;
#include <assert.h>
#include <regex>
//#include "encrypt.h"
#include "cryptopp/base64.h"
#include "cryptopp/aes.h"
#include "cryptopp/files.h"   // FileSource, FileSink
#include "cryptopp/osrng.h"    // AutoSeededRandomPool
#include "cryptopp/modes.h"    // CFB_Mode
#include "cryptopp/hex.h"      // HexEncoder
#include "cryptopp/base64.h"   // Base64Encoder
#include "cryptopp/gcm.h"      // GCM模式支持
#include "cryptopp/sha.h"
#include "cryptopp/rsa.h"      // RSAES_OAEP_SHA_Decryptor
using namespace CryptoPP;
using namespace std;
using std::string;
using std::random_device;
using std::default_random_engine;

#ifdef ENABLEPQ
extern "C" {

#include <pwquality.h>

}

#define PWCONF "/etc/security/pwquality.conf"
#define RFLAG 0x1
#define CFLAG 0x2

#endif

namespace Ui {
class ChangePwdDialog;
}

class ChangePwdDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePwdDialog(bool _isCurrentUser, QString _username, QWidget *parent = 0);
    ~ChangePwdDialog();

public:
    void initPwdChecked();
    void setupComponent();
    void setupConnect();
    bool isDaShangSuo();

    void refreshConfirmBtnStatus();
    void refreshCancelBtnStatus();

    void setFace(QString iconfile);
    void setUsername(QString realname);
    void setPwdType(QString type);
    void setAccountType(QString text);
    void haveCurrentPwdEdit(bool have);

    bool isRemoteUser();

    bool isCurrentUser;

protected:
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);

private:
    Ui::ChangePwdDialog *ui;

    bool checkCharLegitimacy(QString password);
    bool checkOtherPasswd(QString name, QString pwd);

    bool QLabelSetText(QLabel *label, QString string);
    string encryptByPublicKey(string data);

    QString currentUserName;
    QString pwdTip;
    QString pwdSureTip;
    QString curPwdTip;
    bool remoteUser;

    bool enablePwdQuality;
    bool pwdChecking;

#ifdef ENABLEPQ
    pwquality_settings_t *settings;
#endif

private:
    PwdCheckThread * pcThread;

    QTimer * timerForCheckPwd;


private Q_SLOTS:
    void pwdLegalityCheck();
    void requestFinished(QNetworkReply* reply);

Q_SIGNALS:
    void passwd_send(QString oldpwd, QString pwd);
    void passwd_send2(QString pwd);
    void passwd_send3(QString currentpwd, QString pwd);
    void pwdCheckOver();
};

#endif // CHANGEPWDDIALOG_H

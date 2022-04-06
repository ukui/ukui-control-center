#include "phoneauthdialog.h"
#include <QPushButton>
#include <QLineEdit>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFile>
#include <QLatin1String>
#include <QString>
#include <QEventLoop>
#include <QAction>
#include <QStackedWidget>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#define KYLIN_WIFI_GSETTING_VALUE "org.kylinnm.settings"

PhoneAuthDialog::PhoneAuthDialog(QString username,QWidget *parent) : QWidget(parent)
{
    this->resize(360, 529);
    m_username = username;
    initUI();
    setQSS();
}

PhoneAuthDialog::~PhoneAuthDialog()
{
    if(m_interface3){
        delete m_interface3;
        m_interface3 = NULL;
    }
    if(m_interface4){
        delete m_interface4;
        m_interface4 = NULL;
    }
}

void PhoneAuthDialog::initUI(){
    wechatAuthBtn = new QPushButton(tr("Wechat Auth"));
    wechatAuthBtn->setFixedSize(156, 48);
    phoneAuthBtn = new QPushButton(tr("Phone Auth"));
    phoneAuthBtn->setFixedSize(156, 48);
    is_phoneVerifyChecked = true;
    stackWidget = new QStackedWidget(this);

    const QByteArray id(KYLIN_WIFI_GSETTING_VALUE);
    if (QGSettings::isSchemaInstalled(id)){
        m_wifi = new QGSettings(id);
    }

    wechatAuthBtn->setObjectName("wechatAuthBtn");
    phoneAuthBtn->setObjectName("phoneAuthBtn");
    QHBoxLayout * p_hLayout1 = new QHBoxLayout();
    p_hLayout1->setSpacing(0);
    p_hLayout1->addStretch();
    p_hLayout1->addWidget(wechatAuthBtn);
    p_hLayout1->addWidget(phoneAuthBtn);
    p_hLayout1->addStretch();
    is_phoneVerifyChecked = true;
    phoneAuthBtn->setStyleSheet("QPushButton, QPushButton:hover:!pressed, QPushButton:pressed{background: #2FB3E8; "
                                "border-top-right-radius: 8px; "
                                "border-bottom-right-radius: 8px; "
                                "font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc; font-weight: bold;color: white;}");

    m_qr = new QLabel();
    phoneNumLine = new QLineEdit(this);
    phoneNumLine->setFixedSize(312, 64);
    phoneNumLine->setPlaceholderText(tr("Phone number"));
    phoneNumLine->setProperty("class", "PhoneLineEdit");
    verifyCodeLine = new QLineEdit(this);
    verifyCodeLine->setFixedSize(184, 64);
    verifyCodeLine->setPlaceholderText(tr("SMS verification code"));
    verifyCodeLine->setProperty("class", "PhoneLineEdit");
    QRegExp regx_DeviceID1("^[0-9]{1,11}$");
    QRegExp regx_DeviceID2("^[0-9]{1,6}$");
    QValidator *validator_DeviceID1 = new QRegExpValidator(regx_DeviceID1,phoneNumLine);
    QValidator *validator_DeviceID2 = new QRegExpValidator(regx_DeviceID2,verifyCodeLine);
    phoneNumLine->setValidator(validator_DeviceID1);
    verifyCodeLine->setValidator(validator_DeviceID2);

    phonePicture = new QAction(this);
    smsPicture = new QAction(this);
    phonePicture->setIcon(QIcon(":/img/plugins/userinfo/phonenum.svg"));
    smsPicture->setIcon(QIcon(":/img/plugins/userinfo/smscode.svg"));
    phoneNumLine->addAction(phonePicture, QLineEdit::LeadingPosition);
    verifyCodeLine->addAction(smsPicture, QLineEdit::LeadingPosition);
    phoneNumLine->setTextMargins(10, 0, 0, 0);
    verifyCodeLine->setTextMargins(10, 0, 0, 0);


    QWidget *phoneWidget = new QWidget(stackWidget);
    phoneWidget->setFixedSize(312, 64);
    QHBoxLayout * p_hPhoneLayout = new QHBoxLayout(phoneWidget);
    p_hPhoneLayout->setContentsMargins(0, 0, 0, 0);
    p_hPhoneLayout->setSpacing(0);
    p_hPhoneLayout->addWidget(phoneNumLine);
    phoneWidget->setObjectName("phoneWidget");


    QWidget *codeWidget = new QWidget(stackWidget);
    codeWidget->setFixedSize(312, 64);
    getVerifyCodeBtn = new QPushButton(tr("GetCode"));
    getVerifyCodeBtn->setObjectName("getVerifyCodeBtn");
    getVerifyCodeBtn->setFixedSize(128, 64);
    QHBoxLayout * p_hCodeLayout = new QHBoxLayout(codeWidget);
    p_hCodeLayout->setContentsMargins(0, 0, 0, 0);
    p_hCodeLayout->setSpacing(0);
    p_hCodeLayout->addWidget(verifyCodeLine);
    p_hCodeLayout->addWidget(getVerifyCodeBtn);
    codeWidget->setObjectName("codeWidget");

    returnButton = new QPushButton(tr("Return"));
    confirmButton = new QPushButton(tr("Commit"));
    returnButton->setProperty("class", "phoneBT");
    confirmButton->setProperty("class", "phoneBT");
    confirmButton->setDefault(false);
    confirmButton->setFlat(true);
    returnButton->setFlat(true);
    QHBoxLayout * p_hLayout = new QHBoxLayout();
    p_hLayout->setSpacing(20);
    p_hLayout->addStretch();
    p_hLayout->addWidget(returnButton);
    p_hLayout->addWidget(confirmButton);
    p_hLayout->addStretch();

    QVBoxLayout *p_mainLayout = new QVBoxLayout();

    p_mainLayout->setContentsMargins(5, 15, 5, 24);
    p_mainLayout->addLayout(p_hLayout1);
    p_mainLayout->setSpacing(0);

    /*
     * 微信stackwidget
    */
    QWidget *phonestackWidget = new QWidget();
    QWidget *wechatstackWidget = new QWidget();
    stackWidget->addWidget(phonestackWidget);
    stackWidget->addWidget(wechatstackWidget);
    m_pQRPromptMessage = new QLabel();
    m_pQRPromptMessage->setProperty("class", "promptMessageLabel");
    QVBoxLayout * v_wechatLayout = new QVBoxLayout(wechatstackWidget);
    v_wechatLayout->addStretch();
    v_wechatLayout->addWidget(m_pQRPromptMessage, 0, Qt::AlignHCenter);
    v_wechatLayout->addWidget(m_qr, 0, Qt::AlignHCenter);
    v_wechatLayout->addStretch();

    /*
     * 手机stackwidget
    */
    m_pPromptMessage = new QLabel();
    m_pPromptMessage->setProperty("class", "promptMessageLabel"); 
    QWidget * widget_1 = new QWidget(phonestackWidget);
    widget_1->setFixedSize(312, 40);
    QVBoxLayout *v_boxLayout1 = new QVBoxLayout(widget_1);
    v_boxLayout1->addWidget(m_pPromptMessage, 0, Qt::AlignHCenter);
    v_boxLayout1->addStretch();
    QVBoxLayout * v_phoneLayout = new QVBoxLayout(phonestackWidget);
    v_phoneLayout->setSpacing(0);
    v_phoneLayout->addWidget(widget_1, 0, Qt::AlignHCenter);
    v_phoneLayout->addWidget(phoneWidget, 0, Qt::AlignHCenter);
    v_phoneLayout->addWidget(codeWidget, 0, Qt::AlignHCenter);
    v_phoneLayout->addStretch();
    hidePromptMsg();

    p_mainLayout->addWidget(stackWidget);
    p_mainLayout->addLayout(p_hLayout);
    m_interface3 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                      "/cn/kylinos/SSOBackend",
                                                      "cn.kylinos.SSOBackend.eduplatform",
                                                      QDBusConnection::systemBus());
    m_interface4 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                      "/cn/kylinos/SSOBackend",
                                                      "cn.kylinos.SSOBackend.accounts",
                                                      QDBusConnection::systemBus());
    if(!m_interface3->isValid())
        qDebug() << "connect to cn.kylinos.SSOBackend 3failed";
    if(!m_interface4->isValid())
        qDebug() << "connect to cn.kylinos.SSOBackend 4failed";
    connect(m_interface3, SIGNAL(QRStatusChange(QString, QString,int)), this, SLOT(QRStatusChangedSlots(QString, QString,int)));
    m_qrTimeout = new QTimer();
    m_qrTimeout->start(1000);
    connect(m_qrTimeout,&QTimer::timeout,this,[=](){
        if (qrstatus) {
            m_qrTimeout->stop();
            qDebug()<<"停止获取";
        } else {
            QPixmap qrcode;
            getQRCodeFromURL(qrcode);
            qrcode = beautifyQRCode(qrcode);
            m_qr->setPixmap(qrcode);
            m_qr->show();
        }

    });

    this->setLayout(p_mainLayout);
    connect(wechatAuthBtn, &QPushButton::clicked, this, [=](){
        confirmButton->setText(tr("confirm"));
        stackWidget->setCurrentIndex(1);
        is_phoneVerifyChecked = false;
        hideQRPromptMsg();
        wechatAuthBtn->setStyleSheet("QPushButton, QPushButton:hover:!pressed, QPushButton:pressed{background: #2FB3E8; "
                                    "border-top-left-radius: 8px; "
                                    "border-bottom-left-radius: 8px; "
                                    "font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc; font-weight: bold;color: white;}");
        if(!is_nightTheme){
            phoneAuthBtn->setStyleSheet("QPushButton{background: #F6F6F6; border-top-right-radius: 8px; border-bottom-right-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Regular, NotoSansCJKsc;}"
                                         "QPushButton:hover:!pressed{background: #F6F6F6; border-top-right-radius: 8px; border-bottom-right-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc;font-weight: bold;color: #2FB3E8;}"
                                         "QPushButton:pressed{background: #2FB3E8; border-top-right-radius: 8px; border-bottom-right-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc; font-weight: bold;color: white;}");
        }
        else{
            phoneAuthBtn->setStyleSheet("QPushButton{background: #31373F; border-top-right-radius: 8px; border-bottom-right-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Regular, NotoSansCJKsc;}"
                                         "QPushButton:hover:!pressed{background: #31373F; border-top-right-radius: 8px; border-bottom-right-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc;font-weight: bold;color: #2FB3E8;}"
                                         "QPushButton:pressed{background: #2FB3E8; border-top-right-radius: 8px; border-bottom-right-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc; font-weight: bold;color: white;}");
        }
    });
    connect(phoneAuthBtn, &QPushButton::clicked, this, [=](){
        confirmButton->setText(tr("commit"));
        stackWidget->setCurrentIndex(0);
        is_phoneVerifyChecked = true;
        hidePromptMsg();
        phoneAuthBtn->setStyleSheet("QPushButton, QPushButton:hover:!pressed, QPushButton:pressed{background: #2FB3E8; "
                                    "border-top-right-radius: 8px; "
                                    "border-bottom-right-radius: 8px; "
                                    "font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc; font-weight: bold;color: white;}");
        if(!is_nightTheme){
            wechatAuthBtn->setStyleSheet("QPushButton{background: #F6F6F6; border-top-left-radius: 8px; border-bottom-left-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Regular, NotoSansCJKsc;}"
                                         "QPushButton:hover:!pressed{background: #F6F6F6; border-top-left-radius: 8px; border-bottom-left-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc;font-weight: bold;color: #2FB3E8;}"
                                         "QPushButton:pressed{background: #2FB3E8; border-top-left-radius: 8px; border-bottom-left-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc; font-weight: bold;color: white;}");
        }
        else{
            wechatAuthBtn->setStyleSheet("QPushButton{background: #31373F; border-top-left-radius: 8px; border-bottom-left-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Regular, NotoSansCJKsc;}"
                                         "QPushButton:hover:!pressed{background: #31373F; border-top-left-radius: 8px; border-bottom-left-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc;font-weight: bold;color: #2FB3E8;}"
                                         "QPushButton:pressed{background: #2FB3E8; border-top-left-radius: 8px; border-bottom-left-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc; font-weight: bold;color: white;}");
        }
    });
    connect(this, &PhoneAuthDialog::getCodeChange, this, &PhoneAuthDialog::getCodeChanged);
    QString m_clonephone;
    QDBusMessage result = m_interface4->call("GetAccountBasicInfo", m_username);
    if (QDBusMessage::ErrorMessage == result.type()) {
        qDebug() << "result.type() = " << result.type();
        qDebug() << "QDBusMessage::ErrorMessage = " << QDBusMessage::ErrorMessage;
        qDebug()<<"error";
        m_clonephone = "error";
    } else {
        QList<QVariant> outArgs2 = result.arguments();
        m_phone = outArgs2.at(5).value<QString>();
        m_clonephone = m_phone.mid(0,3)+"****"+m_phone.mid(7,4);
    }
    if (m_clonephone == "error") {
        QPalette pa;
        pa.setColor(QPalette::WindowText,Qt::red);
        m_pPromptMessage->setPalette(pa);
        m_pPromptMessage->setText(tr("Mobile number acquisition failed"));
        showPromptMsg();
        phonestatus = false;
        getVerifyCodeBtn->setEnabled(false);
        emit getCodeChange();
    } else {
        phoneNumLine->setText(m_clonephone);
        phoneNumLine->setReadOnly(true);
        phonestatus = true;
    }
    connect(phoneNumLine, &QLineEdit::textChanged, this, [=](){
        hidePromptMsg();
        if (phoneNumLine->text().count() == 11) {
            getVerifyCodeBtn->setEnabled(true);
            emit getCodeChange();
            phonestatus = true;
        } else {
            getVerifyCodeBtn->setEnabled(false);
            emit getCodeChange();
            phonestatus = false;
        }
        if (phonestatus && codestatus) {
            confirmButton->setEnabled(true);
        } else {
            confirmButton->setEnabled(false);
        }
    });

    connect(getVerifyCodeBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked);
        QTimer *start_timer = new QTimer();
        daojishi = 60;
        QString s = tr("Recapture");
        QString s1 = QString::number(daojishi);
        QString s2 = "("+s1+")";
        QString s3 = s+s2;
        m_interface1 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                          "/cn/kylinos/SSOBackend",
                                                          "cn.kylinos.SSOBackend.eduplatform",
                                                          QDBusConnection::systemBus());
        QDBusMessage result;
        if (m_clonephone == "error") {
            result = m_interface1->call("GetVerifyCode",phoneNumLine->text());
        } else {
            result = m_interface1->call("GetVerifyCode",m_phone);
        }
        hidePromptMsg();

        QList<QVariant> outArgs = result.arguments();
        delete m_interface1;
        int codestatus = outArgs.at(0).value<int>();
        if (codestatus == 6 || codestatus == 28) {
            QPalette pa;
            pa.setColor(QPalette::WindowText,Qt::red);
            m_pPromptMessage->setPalette(pa);
            m_pPromptMessage->setText(tr("Network connection failure, please check"));
            showPromptMsg();
        }
        getVerifyCodeBtn->setEnabled(false);
        emit getCodeChange();
        getVerifyCodeBtn->setText(s3);
        start_timer->start(1000);
        connect(start_timer,&QTimer::timeout,this,[=](){
           QString s = tr("Recapture");
           if (daojishi > 0){
               daojishi--;
               QString s1 = QString::number(daojishi);
               QString s2 = "("+s1+")";
               QString s3 = s+s2;
               getVerifyCodeBtn->setText(s3);
           } else {
               getVerifyCodeBtn->setText(tr("GetCode"));
               getVerifyCodeBtn->setEnabled(true);
               emit getCodeChange();
               start_timer->stop();
           }

        });


    });

    confirmButton->setEnabled(false);
    codestatus =false;
    connect(verifyCodeLine, &QLineEdit::textChanged, this, [=](){
        hidePromptMsg();
        if (verifyCodeLine->text().count() == 6) {
            codestatus =true;
        } else {
            confirmButton->setEnabled(false);
            codestatus =false;
        }
        if (phonestatus && codestatus) {
            confirmButton->setEnabled(true);
        } else {
            confirmButton->setEnabled(false);
        }
    });

    connect(confirmButton, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        m_interface2 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                          "/cn/kylinos/SSOBackend",
                                                          "cn.kylinos.SSOBackend.eduplatform",
                                                          QDBusConnection::systemBus());
        QDBusMessage result;
        if (m_clonephone == "error") {
            result = m_interface2->call("CheckUserByPhone",m_username,phoneNumLine->text(),verifyCodeLine->text());
        } else {
            result = m_interface2->call("CheckUserByPhone",m_username,m_phone,verifyCodeLine->text());
        }
        QList<QVariant> outArgs = result.arguments();
        delete m_interface2;
        int status = outArgs.at(0).value<int>();
        if (status == 0) {
            hidePromptMsg();
            emit confirmSignal();
        } else if (status == 9014) {
            QPalette pa;
            pa.setColor(QPalette::WindowText,Qt::red);
            m_pPromptMessage->setPalette(pa);
            m_pPromptMessage->setText(tr("Phone is lock,try again in an hour"));
            showPromptMsg();
        }  else if (status == 9000) {
            QPalette pa;
            pa.setColor(QPalette::WindowText,Qt::red);
            m_pPromptMessage->setPalette(pa);
            m_pPromptMessage->setText(tr("Phone code is wrong"));
            verifyCodeLine->clear();
            showPromptMsg();
        } else if (status == 6 || status == 28) {
            QPalette pa;
            pa.setColor(QPalette::WindowText,Qt::red);
            m_pPromptMessage->setPalette(pa);
            m_pPromptMessage->setText(tr("Network connection failure, please check"));
            showPromptMsg();
        } else if (status == 2300) {
            QPalette pa;
            pa.setColor(QPalette::WindowText,Qt::red);
            m_pPromptMessage->setPalette(pa);
            m_pPromptMessage->setText(tr("Current login expired,using wechat code!"));
            showPromptMsg();
        } else {
            QPalette pa;
            pa.setColor(QPalette::WindowText,Qt::red);
            m_pPromptMessage->setPalette(pa);
            m_pPromptMessage->setText(tr("Unknown error, please try again later"));
            verifyCodeLine->clear();
            showPromptMsg();
        }

    });
    connect(returnButton, &QPushButton::clicked, [=](){
        hidePromptMsg();
        emit returnSignal();
    });

    wechatAuthBtn->setAutoDefault(false);
}

void PhoneAuthDialog::getCodeChanged()
{
    if (is_nightTheme) {
        if (getVerifyCodeBtn->isEnabled()) {
            qDebug()<<__FUNCTION__<<__LINE__;
            getVerifyCodeBtn->setStyleSheet("QPushButton{background-color:#030303;"
                                             "selection-background-color:#030303;"
                                             "border-bottom-right-radius:8px;"
                                             "color:#2FB3E8"
                                              "}");
        } else {
            getVerifyCodeBtn->setStyleSheet("QPushButton{background-color:#030303;"
                                             "selection-background-color:#030303;"
                                             "border-bottom-right-radius:8px;"
                                             "color:#DDDDDD"
                                              "}");
        }
    } else {
        if (getVerifyCodeBtn->isEnabled()) {
            getVerifyCodeBtn->setStyleSheet("QPushButton{background-color:#F6F6F6;"
                                             "selection-background-color:#F6F6F6;"
                                             "border-bottom-right-radius:8px;"
                                             "color:#2FB3E8"
                                              "}");
        } else {
            getVerifyCodeBtn->setStyleSheet("QPushButton{background-color:#F6F6F6;"
                                             "selection-background-color:#F6F6F6;"
                                             "border-bottom-right-radius:8px;"
                                             "color:#DDDDDD"
                                              "}");
        }
    }
}

void PhoneAuthDialog::themeChanged(const quint32 currentTheme){
    if(currentTheme == 0){
        is_nightTheme = false;
        confirmButton->setStyleSheet("QPushButton{background-color:#F6F6F6;}"
                                     "QPushButton:hover:!pressed{background: #F6F6F6;}"
                                     "QPushButton:pressed{background: #2FB3E8;}");
        returnButton->setStyleSheet("QPushButton{background-color:#F6F6F6;}"
                                    "QPushButton:hover:!pressed{background: #F6F6F6;}"
                                    "QPushButton:pressed{background: #2FB3E8;}");
        phoneNumLine->setStyleSheet("QLineEdit{background-color: #F6F6F6; border: 0px;"
                                        "border-top-left-radius:8px;"
                                        "border-top-right-radius:8px;"
                                        "}");
        verifyCodeLine->setStyleSheet("QLineEdit{background-color: #F6F6F6; border: 0px;"
                                          "border-bottom-left-radius:8px"
                                          "}");
        if (getVerifyCodeBtn->isEnabled()) {
            getVerifyCodeBtn->setStyleSheet("QPushButton{background-color:#F6F6F6;"
                                             "selection-background-color:#F6F6F6;"
                                             "border-bottom-right-radius:8px;"
                                             "color:#2FB3E8"
                                              "}");
        } else {
            getVerifyCodeBtn->setStyleSheet("QPushButton{background-color:#F6F6F6;"
                                             "selection-background-color:#F6F6F6;"
                                             "border-bottom-right-radius:8px;"
                                             "color:#DDDDDD"
                                              "}");
        }
        if(is_phoneVerifyChecked){
            wechatAuthBtn->setStyleSheet("QPushButton{background: #F6F6F6; border-top-left-radius: 8px; border-bottom-left-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Regular, NotoSansCJKsc;}"
                                         "QPushButton:hover:!pressed{background: #F6F6F6; border-top-left-radius: 8px; border-bottom-left-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc;font-weight: bold;color: #2FB3E8;}"
                                         "QPushButton:pressed{background: #2FB3E8; border-top-left-radius: 8px; border-bottom-left-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc; font-weight: bold;color: white;}");
        }
        else{
            phoneAuthBtn->setStyleSheet("QPushButton{background: #F6F6F6; border-top-right-radius: 8px; border-bottom-right-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Regular, NotoSansCJKsc;}"
                                         "QPushButton:hover:!pressed{background: #F6F6F6; border-top-right-radius: 8px; border-bottom-right-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc;font-weight: bold;color: #2FB3E8;}"
                                         "QPushButton:pressed{background: #2FB3E8; border-top-right-radius: 8px; border-bottom-right-radius: 8px;font-weight: bold;color: white;}");
        }
    }
    else{
        is_nightTheme = true;
        confirmButton->setStyleSheet("QPushButton{background-color:#31373F;}"
                                     "QPushButton:hover:!pressed{background: #31373F;}"
                                     "QPushButton:pressed{background: #2FB3E8;}"
                                     );
        returnButton->setStyleSheet("QPushButton{background-color:#31373F;}"
                                    "QPushButton:hover:!pressed{background: #31373F;}"
                                    "QPushButton:pressed{background: #2FB3E8;}"
                                    );
        phoneNumLine->setStyleSheet("QLineEdit{background-color: #030303; border: 0px;"
                                        "border-top-left-radius:8px;"
                                        "border-top-right-radius:8px;"
                                        "}");
        verifyCodeLine->setStyleSheet("QLineEdit{background-color: #030303; border: 0px;"
                                          "border-bottom-left-radius:8px"
                                          "}");
        if (getVerifyCodeBtn->isEnabled()) {
            getVerifyCodeBtn->setStyleSheet("QPushButton{background-color:#030303;"
                                             "selection-background-color:#030303;"
                                             "border-bottom-right-radius:8px;"
                                             "color:#2FB3E8"
                                              "}");
        } else {
            getVerifyCodeBtn->setStyleSheet("QPushButton{background-color:#030303;"
                                             "selection-background-color:#030303;"
                                             "border-bottom-right-radius:8px;"
                                             "color:#DDDDDD"
                                              "}");
        }
        if(is_phoneVerifyChecked){
            wechatAuthBtn->setStyleSheet("QPushButton, {background: #31373F; border-top-left-radius: 8px; border-bottom-left-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Regular, NotoSansCJKsc;}"
                                         "QPushButton:hover:!pressed{background: #31373F; border-top-left-radius: 8px; border-bottom-left-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc;font-weight: bold;color: #2FB3E8;}"
                                         "QPushButton:pressed{background: #2FB3E8; border-top-left-radius: 8px; border-bottom-left-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc; font-weight: bold;color: white;}");
        }
        else{
            phoneAuthBtn->setStyleSheet("QPushButton{background: #31373F; border-top-right-radius: 8px; border-bottom-right-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Regular, NotoSansCJKsc;}"
                                         "QPushButton:hover:!pressed{background: #31373F; border-top-right-radius: 8px; border-bottom-right-radius: 8px; font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc;font-weight: bold;color: #2FB3E8;}"
                                         "QPushButton:pressed{background: #2FB3E8; border-top-right-radius: 8px; border-bottom-right-radius: 8px;font-weight: bold;color: white;}");
        }
    }
}

void PhoneAuthDialog::showPromptMsg()
{
    m_pPromptMessage->show();
}

void PhoneAuthDialog::hidePromptMsg()
{
    m_pPromptMessage->hide();
}
void PhoneAuthDialog::showQRPromptMsg()
{
    m_pQRPromptMessage->setVisible(true);
}

void PhoneAuthDialog::hideQRPromptMsg()
{
    m_pQRPromptMessage->setVisible(false);
}
void PhoneAuthDialog::getQRCodeFromURL(QPixmap &qrcode)
{
    // 从网络上下载二维码   并加载到qrcode
    QDBusMessage result = m_interface3->call("CheckUserByQR");
    QList<QVariant> outArgs = result.arguments();
    int status = outArgs.at(1).value<int>();
    if (status == 6 || status == 28) {
        QPalette pa;
        pa.setColor(QPalette::WindowText,Qt::red);
        m_pQRPromptMessage->setPalette(pa);
        m_pQRPromptMessage->setText(tr("Network connection failure, please check"));
        showQRPromptMsg();
        qrstatus = false;
        qrcode = QPixmap(":/img/plugins/userinfo/qring.png");
        if (!m_qrTimeout->isActive()) {
            m_qrTimeout->start(5000);
        }
        return;
    }
    if (QDBusMessage::ErrorMessage == result.type()) {
        qDebug()<<"error pix";
        qrstatus = false;

        qrcode = QPixmap(":/img/plugins/userinfo/qring.png");
        if (!m_qrTimeout->isActive()) {
            m_qrTimeout->start(5000);
        }
        return;
    } else {
        qrstatus = true;
    }


    const QString url = outArgs.at(0).value<QString>();
    QNetworkRequest request;
    QNetworkAccessManager networkManager;
    request.setUrl(url);
    QNetworkReply *reply = networkManager.get(request);

    // 超时 \ 同步处理
    QEventLoop loop;
    QTimer timer;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &timer, &QTimer::stop);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(3000);
    loop.exec();

    if(!timer.isActive())
    {
        reply->abort();
        return;
    }
    timer.stop();

    // 生成二维码
    if(reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "info: [EduPlatformInterface][GetLoginQR]: Network error!";
        qrstatus = false;
        qrcode = QPixmap(":/img/plugins/userinfo/qring.png");
        if (!m_qrTimeout->isActive()) {
            m_qrTimeout->start(5000);
        }
        return;
    } else {
        qrstatus = true;
        QByteArray bytes = reply->readAll();
        hideQRPromptMsg();
        qrcode.loadFromData(bytes);
        reply->deleteLater();
    }
}
QPixmap PhoneAuthDialog::beautifyQRCode(QPixmap &pixImg)
{
    // 去除白边
    QImage img = pixImg.toImage();
    int rect_x, rect_y, rect_width, rect_height;
    for(int i = 0;i < img.size().width();i++)
    {
        bool isEnd = false;
        for(int j = 0;j < img.size().height();j++)
        {
            if(img.pixel(i, j) != qRgb(255, 255, 255))
            {
                rect_x = i - 1;
                rect_y = j - 1;
                rect_width = img.size().width() - 2 * rect_x;
                rect_height = img.size().height() - 2 * rect_y;
                isEnd = true;
                break;
            }
        }
        if(isEnd) break;
    }
    QPixmap dealImg = QPixmap::fromImage(img.copy(rect_x, rect_y, rect_width, rect_height)).scaledToWidth(150);
    img = dealImg.toImage();

    dealImg = QPixmap::fromImage(img);
    return dealImg;
}
void PhoneAuthDialog::QRStatusChangedSlots(QString name, QString passwd, int status){
    if (status == 4) {
       if (name == m_username) {
           emit confirmSignal();
       } else {
           QPalette pa;
           pa.setColor(QPalette::WindowText,Qt::red);
           m_pQRPromptMessage->setPalette(pa);
           m_pQRPromptMessage->setText(tr("Please use the correct wechat scan code"));
           showQRPromptMsg();
           QPixmap qrcode;
           getQRCodeFromURL(qrcode);
           qrcode = beautifyQRCode(qrcode);
           m_qr->setPixmap(qrcode);
       }
    }
    if (status == 5) {

        QPixmap qrcode;
        getQRCodeFromURL(qrcode);
        qrcode = beautifyQRCode(qrcode);
        m_qr->setPixmap(qrcode);
    }
    if (status == 6) {
        QPalette pa;
        pa.setColor(QPalette::WindowText,Qt::red);
        m_pQRPromptMessage->setPalette(pa);
        m_pQRPromptMessage->setText(tr("Network connection failure, please check"));
        showQRPromptMsg();
        QPixmap qrcode;
        getQRCodeFromURL(qrcode);
        qrcode = beautifyQRCode(qrcode);
        m_qr->setPixmap(qrcode);
    }
}
void PhoneAuthDialog::setQSS(){
    QFile qss(":/phoneauthdialog.qss");
    if(qss.open(QFile::ReadOnly))
    {
        qDebug("open success");
        QString style = QLatin1String(qss.readAll());
        this->setStyleSheet(style);
        qss.close();
    }
    else
       qDebug("Open failed");
}

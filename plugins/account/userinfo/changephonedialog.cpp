#include "changephonedialog.h"
#include "ui_changephonedialog.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QDebug>

#define KYLIN_WIFI_GSETTING_VALUE "org.kylinnm.settings"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

ChangePhoneDialog::ChangePhoneDialog(QString username, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangePhoneDialog),
    m_interface1(NULL),
    m_interface2(NULL),
    m_isNightMode(NULL)
{
    ui->setupUi(this);
    qDebug() << "test3" << username;
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    initDbus();
    initUI(username);
    initConnect();
}

void ChangePhoneDialog::initDbus(){
    m_interface1 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                      "/cn/kylinos/SSOBackend",
                                                      "cn.kylinos.SSOBackend.eduplatform",
                                                      QDBusConnection::systemBus());
    m_interface2 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                      "/cn/kylinos/SSOBackend",
                                                      "cn.kylinos.SSOBackend.accounts",
                                                      QDBusConnection::systemBus());
    const QByteArray id(KYLIN_WIFI_GSETTING_VALUE);
    if (QGSettings::isSchemaInstalled(id)){
        m_wifi = new QGSettings(id);
    }
}

void ChangePhoneDialog::initUI(QString username){
    m_username = username;
    QFont ft, ft1;
    ft.setPointSize(16);
    ft1.setPointSize(10);
    ui->title_label->setText(tr("Change Phone"));
    ui->title_label->setObjectName("titleLabel");
    ui->title_label->setFont(ft);
    QPalette pa;
    pa.setColor(QPalette::WindowText,Qt::red);
    ui->label_3->setPalette(pa);
    ui->label_3->setFont(ft1);
    ui->label_3->hide();
    closeBtn = new QPushButton(this);
    closeBtn->setProperty("useIconHighlightEffect", true);
    closeBtn->setProperty("iconHighlightEffectMode", 1);
    closeBtn->setFlat(true);
    closeBtn->setIcon(QIcon(":/img/plugins/userinfo/close.svg"));
    closeBtn->setAutoDefault(false);
    ui->horizontalLayout->addWidget(closeBtn);
    ui->phoneNumLine->setStyleSheet("QLineEdit{background-color: #F6F6F6; border: 0px;"
                                    "border-radius:8px;"
                                    "}");
    ui->verifyCodeLine->setStyleSheet("QLineEdit{background-color: #F6F6F6; border: 0px}");
    connect(closeBtn, &QPushButton::clicked, [=]{
        close();
    });
    const QByteArray id_1(UKUI_QT_STYLE);
    if (QGSettings::isSchemaInstalled(id_1)) {
        m_style = new QGSettings(id_1);
        QString themeName = m_style->get(UKUI_STYLE_KEY).toString();
        if( themeName == "ukui-light" || themeName == "ukui-default" | themeName == "ukui" )
            m_isNightMode = false;
        else
            m_isNightMode = true;
        qDebug() << "m_isNightMode = " << m_isNightMode;
        connect(m_style, &QGSettings::changed, this, &ChangePhoneDialog::setphonestyle);
    }

    phonePicture = new QAction(this);
    smsPicture = new QAction(this);
    phonePicture->setIcon(QIcon(":/img/plugins/userinfo/phonenum.svg"));
    smsPicture->setIcon(QIcon(":/img/plugins/userinfo/smscode.svg"));
    ui->phoneNumLine->setPlaceholderText(tr("Phone number"));
    ui->verifyCodeLine->setPlaceholderText(tr("SMS verification code"));
    ui->phoneNumLine->addAction(phonePicture, QLineEdit::LeadingPosition);
    ui->verifyCodeLine->addAction(smsPicture, QLineEdit::LeadingPosition);

    /*
     * 输入手机号界面
    */
    QRegExp regx_DeviceID1("^[0-9]{1,11}$");
    QRegExp regx_DeviceID2("^[0-9]{1,6}$");
    QValidator *validator_DeviceID1 = new QRegExpValidator(regx_DeviceID1, ui->phoneNumLine);
    QValidator *validator_DeviceID2 = new QRegExpValidator(regx_DeviceID2, ui->verifyCodeLine);
    ui->phoneNumLine->setTextMargins(10, 0, 0, 0);
    ui->verifyCodeLine->setTextMargins(10, 0, 0, 0);
    ui->phoneNumLine->setValidator(validator_DeviceID1);
    ui->verifyCodeLine->setValidator(validator_DeviceID2);


    QDBusMessage result1 = m_interface1->call("CheckPhoneNumBind", m_username);
    QList<QVariant> outArgs1 = result1.arguments();
    int status1 = outArgs1.at(0).value<int>();
    if (status1 == 1) {
        oldphonestatus = true;
        QDBusMessage result2 = m_interface2->call("GetAccountBasicInfo", m_username);
        if (QDBusMessage::ErrorMessage == result2.type()) {
            qDebug() << "result2.type() = " << result2.type();
            qDebug() << "QDBusMessage::ErrorMessage = " << QDBusMessage::ErrorMessage;
            qDebug()<<"error";
            oldphone = "error";
        } else {
            QList<QVariant> outArgs2 = result2.arguments();
            oldphone = outArgs2.at(5).value<QString>();
        }
        ui->m_pTitle->setText(tr("Please input old phone number"));
        ui->submitButton->setText(tr("Next"));
    } else {
        oldphonestatus =false;      // 该微信号没有绑定手机号
        ui->m_pTitle->setText(tr("Please enter new mobile number"));
        ui->submitButton->setText(tr("Submit"));
    }
    phonechangestatus = false;
    phonestatus = false;
    codestatus = false;
    phoneNumChangeSuccess = false;
    ui->submitButton->setEnabled(false);
    //ui->getVerifyCode->setEnabled(false);

    /*
     * 修改成功界面
    */
    QFont ft2;
    ft2.setPointSize(18);
    ui->label->setFont(ft);
    ui->label->setProperty("class", "titleLB");
    ui->label->setText(tr("changed success"));
    ui->label_2->setText(tr("You have successfully modified your phone"));
    ui->successicon->setPixmap(QPixmap(":/img/plugins/userinfo/successed.png"));
}

void ChangePhoneDialog::initConnect(){
    connect(ui->getVerifyCode, SIGNAL(clicked(bool)), this, SLOT(slotGetVerifyCode(bool)));
    if (oldphonestatus) {
        QString m_clonephone = oldphone.mid(0,3)+"****"+oldphone.mid(7,4);
        ui->phoneNumLine->setText(m_clonephone);
        ui->phoneNumLine->setReadOnly(true);
        phonestatus = true;
    } else {
        phonestatus = false;
    }
    connect(ui->phoneNumLine, &QLineEdit::textChanged, this, [=](){
        ui->label_3->hide();
        if (ui->phoneNumLine->text().count() == 11) {
            ui->getVerifyCode->setEnabled(true);
            phonestatus = true;
        } else {
            ui->getVerifyCode->setEnabled(false);
            phonestatus = false;
        }
        if (phonestatus && codestatus) {
            ui->submitButton->setEnabled(true);
        } else {
            ui->submitButton->setEnabled(false);
        }
    });

    connect(ui->verifyCodeLine, &QLineEdit::textChanged, this, [=](){
        if (ui->verifyCodeLine->text().count() == 6) {
            codestatus =true;
        } else {
            ui->submitButton->setEnabled(false);
            codestatus =false;
        }
        if (phonestatus && codestatus) {
            ui->submitButton->setEnabled(true);
        } else {
            ui->submitButton->setEnabled(false);
        }
    });

    connect(ui->submitButton, &QPushButton::clicked, this, &ChangePhoneDialog::slotSubmitClicked);
}

void ChangePhoneDialog::slotGetVerifyCode(bool clicked){
    Q_UNUSED(clicked);
    start_timer = new QTimer();
    countdown = 60;
    QString s = tr("Recapture");
    QString s1 = QString::number(countdown);
    QString s2 = "("+s1+")";
    QString s3 = s+s2;
    QDBusMessage result;
    if (phonechangestatus || !oldphonestatus) {
        result = m_interface1->call("GetVerifyCode",ui->phoneNumLine->text());
    } else {
        result = m_interface1->call("GetVerifyCode",oldphone);
    }
    ui->label_3->hide();
    QList<QVariant> outArgs = result.arguments();
    int codestatus = outArgs.at(0).value<int>();
    if (codestatus == 6 || codestatus == 28) {
        ui->label_3->setText(tr("Network connection failure, please check"));
        if(!(ui->label_3->isVisible())){
           ui->label_3->show();
        }
        return;
    }
    ui->getVerifyCode->setEnabled(false);
    ui->getVerifyCode->setText(s3);
    start_timer->start(1000);
    connect(start_timer,&QTimer::timeout,this,[=](){
       QString s = tr("Recapture");
       if (countdown > 0){
           countdown--;
           qDebug() << countdown;
           QString s1 = QString::number(countdown);
           QString s2 = "("+s1+")";
           QString s3 = s+s2;
           qDebug() << s3;
           ui->getVerifyCode->setText(s3);
       } else {
           ui->getVerifyCode->setText(tr("GetCode"));
           ui->getVerifyCode->setEnabled(true);
           start_timer->stop();
       }
    });
}

void ChangePhoneDialog::slotSubmitClicked(bool clicked){
    Q_UNUSED(clicked)
    if ( phoneNumChangeSuccess == true ){
        close();
    }
    if (oldphonestatus == true) {           // 微信已经绑定过手机号
        if (phonechangestatus == false) {
            QDBusMessage result3 = m_interface1->call("ApplyChangeInfoToken",m_username, oldphone, ui->verifyCodeLine->text());
            QList<QVariant> outArgs3 = result3.arguments();
            temptoken = outArgs3.at(0).value<QString>();
            int status = outArgs3.at(1).value<int>();
            if (status == 0) {
                ui->label_3->hide();
                ui->m_pTitle->setText(tr("Please enter new mobile number"));
                ui->submitButton->setText(tr("Submit"));
                if (start_timer->isActive()){
                    start_timer->stop();
                }
                ui->getVerifyCode->setText(tr("GetCode"));
                oldcode = ui->verifyCodeLine->text();
                qDebug() << oldcode;
                ui->phoneNumLine->setReadOnly(false);
                ui->phoneNumLine->clear();
                ui->verifyCodeLine->clear();
                phonechangestatus = true;
            } else if (status == 9014) {
                ui->label_3->setText(tr("Phone is lock,try again in an hour"));
                ui->label_3->show();
            }  else if (status == 9000) {
                ui->label_3->setText(tr("Phone code is wrong"));
                ui->label_3->show();
                ui->verifyCodeLine->clear();
            } else if (status == 6 || status == 28) {
                ui->label_3->setText(tr("Network connection failure, please check"));
                ui->label_3->show();
            } else {
                ui->label_3->setText(tr("Unknown error, please try again later"));
                ui->verifyCodeLine->clear();
                ui->label_3->show();
            }

        } else {
            if (oldphone == ui->phoneNumLine->text()) {
                ui->label_3->setText(tr("Phone can not same"));
                ui->label_3->show();
            } else {
                QDBusMessage result3 = m_interface1->call("ReBindPhone", m_username, ui->phoneNumLine->text(),ui->verifyCodeLine->text(),temptoken);
                QList<QVariant> outArgs3 = result3.arguments();
                int status = outArgs3.at(0).value<int>();
                if (status == 0) {
                    phoneNumChangeSuccess = true;
                    ui->label_3->hide();
                    ui->submitButton->setText(tr("finished"));
                    ui->m_pStackedWidget->setCurrentIndex(1);
                } else if (status == 9014) {
                    ui->label_3->setText(tr("Phone is lock,try again in an hour"));
                    ui->label_3->show();
                }  else if (status == 9000) {
                    ui->label_3->setText(tr("Phone code is wrong"));
                    ui->label_3->show();
                    ui->verifyCodeLine->clear();
                } else if (status == 9009) {
                    ui->label_3->setText(tr("Phone number already in used!"));
                    ui->label_3->show();
                } else if (status == 6 || status == 28) {
                    ui->label_3->setText(tr("Network connection failure, please check"));
                    ui->label_3->show();
                } else {
                    ui->label_3->setText(tr("Unknown error, please try again later"));
                    ui->verifyCodeLine->clear();
                    ui->label_3->show();
                }
            }
        }
    } else {                // 微信没有绑定手机号，直接绑定即可
        QDBusMessage result4 = m_interface1->call("BindPhoneNum", m_username, ui->phoneNumLine->text(), ui->verifyCodeLine->text());
        QList<QVariant> outArgs4 = result4.arguments();
        int status = outArgs4.at(0).value<int>();
        if (status == 0) {
            phoneNumChangeSuccess = true;
            ui->submitButton->setText(tr("finished"));
            ui->label_3->setText(tr("You have successfully modified your phone"));
            ui->m_pStackedWidget->setCurrentIndex(1);
        } else if (status == 9014) {
            ui->label_3->setText(tr("Phone is lock,try again in an hour"));
            ui->label_3->show();
        } else if (status == 9000) {
            ui->label_3->setText(tr("Phone code is wrong"));
            ui->label_3->show();
            ui->verifyCodeLine->clear();
        } else if (status == 9009) {
            ui->label_3->setText(tr("Phone number already in used!"));
            ui->label_3->show();
        } else {
            ui->label_3->setText(tr("Unknown error, please try again later"));
            ui->label_3->show();
            ui->verifyCodeLine->clear();
        }
    }
}

void ChangePhoneDialog::setphonestyle(QString key)
{
    if(key == "styleName") {
        QString themeName = m_style->get(UKUI_STYLE_KEY).toString();
        if( themeName == "ukui-light" || themeName == "ukui-default" | themeName == "ukui" )
            m_isNightMode = false;
        else
            m_isNightMode = true;
        qDebug() << "m_isNightMode = " << m_isNightMode;
        update();
    }
}
void ChangePhoneDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    /*
     * 由于设置了窗口透明，所以对QLineEdit的背景设置会因为border没有设置而不生效
     * 圆角设置只有在paintEvent里才可以生效
    */
    if(!m_isNightMode){
        ui->phoneNumLine->setStyleSheet("QLineEdit{background-color: #F6F6F6; border: 0px;"
                                        "border-top-left-radius:8px;"
                                        "border-top-right-radius:8px;"
                                        "}");
        ui->verifyCodeLine->setStyleSheet("QLineEdit{background-color: #F6F6F6; border: 0px;"
                                          "border-bottom-left-radius:8px"
                                          "}");
        if (ui->getVerifyCode->isEnabled()) {
            ui->getVerifyCode->setStyleSheet("QPushButton{background-color:#F6F6F6;"
                                             "selection-background-color:#F6F6F6;"
                                             "border-bottom-right-radius:8px;"
                                             "color:#2FB3EB"
                                              "}");
        } else {
            ui->getVerifyCode->setStyleSheet("QPushButton{background-color:#F6F6F6;"
                                             "selection-background-color:#F6F6F6;"
                                             "border-bottom-right-radius:8px;"
                                             "color:#DDDDDD"
                                              "}");
        }
        ui->submitButton->setStyleSheet("QPushButton{border-radius:8px;background-color:#DDDDDD;}"
                                        "QPushButton:hover{border-radius:8px;background-color:#2FB3EB;}"
                                        "QPushButton:clicked{border-radius:8px;background-color:#2FB3EB;}");
    }
    else{
        ui->phoneNumLine->setStyleSheet("QLineEdit{background-color: #030303; border: 0px;"
                                        "border-top-left-radius:8px;"
                                        "border-top-right-radius:8px;"
                                        "}");
        ui->verifyCodeLine->setStyleSheet("QLineEdit{background-color: #030303; border: 0px;"
                                          "border-bottom-left-radius:8px"
                                          "}");
        if (ui->getVerifyCode->isEnabled()) {
            ui->getVerifyCode->setStyleSheet("QPushButton{background-color:#030303;"
                                             "selection-background-color:#030303;"
                                             "border-bottom-right-radius:8px;"
                                             "color:#2FB3EB"
                                              "}");
        } else {
            ui->getVerifyCode->setStyleSheet("QPushButton{background-color:#030303;"
                                             "selection-background-color:#030303;"
                                             "border-bottom-right-radius:8px;"
                                             "color:#DDDDDD"
                                              "}");
        }
        ui->submitButton->setStyleSheet("QPushButton{border-radius:8px;background-color:#31373F;}"
                                        "QPushButton:hover{border-radius:8px;background-color:#2FB3EB;}"
                                        "QPushButton:clicked{border-radius:8px;background-color:#2FB3EB;}");
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 16, 16);
//    rectPath.addRoundedRect(this->rect(), 16, 16);

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
    qt_blurImage(img, 16, false, false);

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
    /*

    int width = this->width();
    int height = this->height();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    QString s = m_style->get(UKUI_STYLE_KEY).toString();
    if (s=="ukui-dark" || s == "ukui-black") {
        QColor m_bgColor("grey");
        m_bgColor.setAlpha(255);
        painter.setBrush(m_bgColor);
        painter.drawRoundRect(0, 0, width, height, 5*height/width, 5);
    } else {
        QColor m_bgColor("#FFFFFF");
        m_bgColor.setAlpha(255);
        painter.setBrush(m_bgColor);
        painter.drawRoundRect(0, 0, width, height, 5*height/width, 5);
    }
    */
}

ChangePhoneDialog::~ChangePhoneDialog(){
    qDebug() << "调用析构函数";
    delete m_style;
    if(m_interface1){
        delete m_interface1;
        m_interface1 = NULL;
    }
    if(m_interface2){
        delete m_interface2;
        m_interface2 = NULL;
    }
}

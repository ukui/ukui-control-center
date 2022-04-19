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
#include "proxy.h"
#include "aptproxydialog.h"
#include <QDebug>
#include "aptinfo.h"
#include "../../../shell/utils/utils.h"

#define PROXY_SCHEMA              "org.gnome.system.proxy"
#define PROXY_MODE_KEY            "mode"
#define PROXY_AUTOCONFIG_URL_KEY  "autoconfig-url"
#define IGNORE_HOSTS_KEY          "ignore-hosts"

#define HTTP_PROXY_SCHEMA         "org.gnome.system.proxy.http"
#define HTTP_USE_AUTH_KEY         "use-authentication"
#define HTTP_AUTH_USER_KEY        "authentication-user"
#define HTTP_AUTH_PASSWD_KEY      "authentication-password"

#define HTTPS_PROXY_SCHEMA        "org.gnome.system.proxy.https"

#define FTP_PROXY_SCHEMA          "org.gnome.system.proxy.ftp"

#define SOCKS_PROXY_SCHEMA        "org.gnome.system.proxy.socks"

#define PROXY_HOST_KEY       "host"
#define PROXY_PORT_KEY       "port"

Proxy::Proxy() : mFirstLoad(true)
{
    pluginName = tr("Proxy");
    pluginType = NETWORK;
}

Proxy::~Proxy()
{

}

QString Proxy::plugini18nName() {
    return pluginName;
}

int Proxy::pluginTypes() {
    return pluginType;
}

QWidget *Proxy::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        initUi(pluginWidget);
        retranslateUi();

        mfileWatch_1 = new QFileSystemWatcher(this);
        mfileWatch_2 = new QFileSystemWatcher(this);

        QString dir_1("/etc/apt/apt.conf.d");
        QString dir_2("/etc/profile.d");
        mfileWatch_1->addPath(dir_1);
        mfileWatch_2->addPath(dir_2);

        const QByteArray id(PROXY_SCHEMA);
        const QByteArray idd(HTTP_PROXY_SCHEMA);
        const QByteArray iddd(HTTPS_PROXY_SCHEMA);
        const QByteArray iid(FTP_PROXY_SCHEMA);
        const QByteArray iiid(SOCKS_PROXY_SCHEMA);
         const QByteArray iVd(APT_PROXY_SCHEMA);

        initSearchText();
        setupComponent();

        if (QGSettings::isSchemaInstalled(id) && QGSettings::isSchemaInstalled(idd) &&
                QGSettings::isSchemaInstalled(iddd) && QGSettings::isSchemaInstalled(iid) &&
                QGSettings::isSchemaInstalled(iiid) && QGSettings::isSchemaInstalled(iVd)){

            proxysettings = new QGSettings(id,QByteArray(),this);
            httpsettings = new QGSettings(idd,QByteArray(),this);
            securesettings = new QGSettings(iddd,QByteArray(),this);
            ftpsettings = new QGSettings(iid,QByteArray(),this);
            sockssettings = new QGSettings(iiid,QByteArray(),this);

            setupConnect();
            initProxyModeStatus();
            initAutoProxyStatus();
            initManualProxyStatus();
            initIgnoreHostStatus();
        } else {
            qCritical() << "Xml needed by Proxy is not installed";
        }
    }
    return pluginWidget;
}

const QString Proxy::name() const {

    return QStringLiteral("Proxy");
}

bool Proxy::isShowOnHomePage() const
{
    return true;
}

QIcon Proxy::icon() const
{
    return QIcon();
}

bool Proxy::isEnable() const
{
    return true;
}

void Proxy::initUi(QWidget *widget)
{
    QVBoxLayout *mverticalLayout = new QVBoxLayout(widget);
    mverticalLayout->setSpacing(8);
    mverticalLayout->setContentsMargins(0, 0, 0, 0);

    mProxyBtnGroup = new QButtonGroup(this);

    mTitleLabel = new TitleLabel(widget);

    // 自动代理模块
    mAutoFrame = new QFrame(widget);
    mAutoFrame->setMinimumSize(QSize(550, 0));
    mAutoFrame->setMaximumSize(QSize(16777215, 16777215));
    mAutoFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *AutobootLayout = new QVBoxLayout(mAutoFrame);
    AutobootLayout->setContentsMargins(0, 0, 0, 0);
    AutobootLayout->setSpacing(0);

    mAutoProxyWidget = new HoverWidget("",widget);
    mAutoProxyWidget->setObjectName("mAutoProxyWidget");
    mAutoProxyWidget->setMinimumSize(QSize(550, 60));
    mAutoProxyWidget->setMaximumSize(QSize(16777215, 60));
    mAutoProxyWidget->setStyleSheet(QString("HoverWidget#mAutoProxyWidget{background: palette(base);\
                                   border-radius: 4px;}"));

    QHBoxLayout *mAutoProxyLayout = new QHBoxLayout(mAutoProxyWidget);
    mAutoProxyLayout->setContentsMargins(16, 0, 24, 0);
    mAutoProxyLayout->setSpacing(0);

    mAutoProxyLabel = new QLabel(mAutoProxyWidget);
    mAutoProxyLabel->setFixedWidth(400);

    mAutoBtn = new QRadioButton(mAutoProxyWidget);
    mProxyBtnGroup->addButton(mAutoBtn);

    mAutoProxyLayout->addWidget(mAutoProxyLabel);
    mAutoProxyLayout->addStretch();
    mAutoProxyLayout->addWidget(mAutoBtn);

    line_1 = setLine(mAutoFrame);

    mUrlFrame = new QFrame(mAutoFrame);
    setFrame_Noframe(mUrlFrame);

    QHBoxLayout *mUrlLayout = new QHBoxLayout(mUrlFrame);
    mUrlLayout->setContentsMargins(16, 0, 16, 0);
    mUrlLayout->setSpacing(8);

    mUrlLabel = new QLabel(mUrlFrame);
    mUrlLabel->setFixedWidth(136);

    mUrlLineEdit = new QLineEdit(mUrlFrame);
    mUrlLineEdit->setFixedHeight(36);

    mUrlLayout->addWidget(mUrlLabel);
    mUrlLayout->addWidget(mUrlLineEdit);

    AutobootLayout->addWidget(mAutoProxyWidget);
    AutobootLayout->addWidget(line_1);
    AutobootLayout->addWidget(mUrlFrame);

    // 手动代理模块
    mManualFrame = new QFrame(widget);
    mManualFrame->setMinimumSize(QSize(550, 0));
    mManualFrame->setMaximumSize(QSize(16777215, 16777215));
    mManualFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *mManualLayout = new QVBoxLayout(mManualFrame);
    mManualLayout->setContentsMargins(0, 0, 0, 0);
    mManualLayout->setSpacing(0);

    mManualProxyWidget = new HoverWidget("",widget);
    mManualProxyWidget->setObjectName("mManualProxyWidget");
    mManualProxyWidget->setMinimumSize(QSize(550, 60));
    mManualProxyWidget->setMaximumSize(QSize(16777215, 60));
    mManualProxyWidget->setStyleSheet(QString("HoverWidget#mManualProxyWidget{background: palette(base);\
                                   border-radius: 4px;}"));

    QHBoxLayout *mManualProxyLayout = new QHBoxLayout(mManualProxyWidget);
    mManualProxyLayout->setContentsMargins(16, 0, 24, 0);
    mManualProxyLayout->setSpacing(0);

    mManualProxyLabel = new QLabel(mManualProxyWidget);
    mManualProxyLabel->setFixedWidth(400);

    mManualBtn = new QRadioButton(mManualProxyWidget);
    mProxyBtnGroup->addButton(mManualBtn);

    mManualProxyLayout->addWidget(mManualProxyLabel);
    mManualProxyLayout->addStretch();
    mManualProxyLayout->addWidget(mManualBtn);

    line_2 = setLine(mManualFrame);


    mHTTPFrame = new QFrame(mManualFrame);
    setFrame_Noframe(mHTTPFrame);


//    QVBoxLayout *mHTTPLayout = new QVBoxLayout(mHTTPFrame);
//    mHTTPLayout->setSpacing(0);
//    mHTTPLayout->setContentsMargins(16, 0, 16, 0);

//    QFrame *mHTTPFrame_1 = new QFrame(mHTTPFrame);
//    mHTTPFrame_1->setMinimumSize(QSize(550, 60));
//    mHTTPFrame_1->setMaximumSize(QSize(16777215, 60));
//    mHTTPFrame_1->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mHTTPLayout_1 = new QHBoxLayout(mHTTPFrame);
    mHTTPLayout_1->setSpacing(8);
    mHTTPLayout_1->setContentsMargins(16, 0, 16, 0);
    mHTTPLabel = new QLabel(mHTTPFrame);
    mHTTPLabel->setFixedWidth(136);
    mHTTPPortLabel = new QLabel(mHTTPFrame);
    mHTTPPortLabel->setFixedWidth(100);
    mHTTPPortLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mHTTPLineEdit_1 = new QLineEdit(mHTTPFrame);
    mHTTPLineEdit_1->resize(300, 36);
    mHTTPLineEdit_2 = new QLineEdit(mHTTPFrame);
    mHTTPLineEdit_2->setFixedHeight(36);
    mHTTPLayout_1->addWidget(mHTTPLabel);
    mHTTPLayout_1->addWidget(mHTTPLineEdit_1);
    mHTTPLayout_1->addWidget(mHTTPPortLabel);
    mHTTPLayout_1->addWidget(mHTTPLineEdit_2);

    line_3 = setLine(mManualFrame);

    mHTTPSFrame = new QFrame(mManualFrame);
   setFrame_Noframe(mHTTPSFrame);

    QHBoxLayout *mHTTPSLayout = new QHBoxLayout(mHTTPSFrame);
    mHTTPSLayout->setSpacing(8);
    mHTTPSLayout->setContentsMargins(16, 0, 16, 0);
    mHTTPSLabel = new QLabel(mHTTPSFrame);
    mHTTPSLabel->setFixedWidth(136);
    mHTTPSPortLabel = new QLabel(mHTTPSFrame);
    mHTTPSPortLabel->setFixedWidth(100);
    mHTTPSPortLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mHTTPSLineEdit_1 = new QLineEdit(mHTTPSFrame);
    mHTTPSLineEdit_1->resize(300, 36);
    mHTTPSLineEdit_2 = new QLineEdit(mHTTPSFrame);
    mHTTPSLineEdit_2->setFixedHeight(36);
    mHTTPSLayout->addWidget(mHTTPSLabel);
    mHTTPSLayout->addWidget(mHTTPSLineEdit_1);
    mHTTPSLayout->addWidget(mHTTPSPortLabel);
    mHTTPSLayout->addWidget(mHTTPSLineEdit_2);

    line_4 = setLine(mManualFrame);

    mFTPFrame = new QFrame(mManualFrame);
    setFrame_Noframe(mFTPFrame);

    QHBoxLayout *mFTPLayout = new QHBoxLayout(mFTPFrame);
    mFTPLayout->setSpacing(8);
    mFTPLayout->setContentsMargins(16, 0, 16, 0);
    mFTPLabel = new QLabel(mFTPFrame);
    mFTPLabel->setFixedWidth(136);
    mFTPPortLabel = new QLabel(mFTPFrame);
    mFTPPortLabel->setFixedWidth(100);
    mFTPPortLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mFTPLineEdit_1 = new QLineEdit(mFTPFrame);
    mFTPLineEdit_1->resize(300, 36);
    mFTPLineEdit_2 = new QLineEdit(mFTPFrame);
    mFTPLineEdit_2->setFixedHeight(36);
    mFTPLayout->addWidget(mFTPLabel);
    mFTPLayout->addWidget(mFTPLineEdit_1);
    mFTPLayout->addWidget(mFTPPortLabel);
    mFTPLayout->addWidget(mFTPLineEdit_2);

    line_5 = setLine(mManualFrame);

    mSOCKSFrame = new QFrame(mManualFrame);
    setFrame_Noframe(mSOCKSFrame);

    QHBoxLayout *mSOCKSLayout = new QHBoxLayout(mSOCKSFrame);
    mSOCKSLayout->setSpacing(8);
    mSOCKSLayout->setContentsMargins(16, 0, 16, 0);
    mSOCKSLabel = new QLabel(mSOCKSFrame);
    mSOCKSLabel->setFixedWidth(136);
    mSOCKSPortLabel = new QLabel(mSOCKSFrame);
    mSOCKSPortLabel->setFixedWidth(100);
    mSOCKSPortLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mSOCKSLineEdit_1 = new QLineEdit(mSOCKSFrame);
    mSOCKSLineEdit_1->resize(300, 36);
    mSOCKSLineEdit_2 = new QLineEdit(mSOCKSFrame);
    mSOCKSLineEdit_2->setFixedHeight(36);
    mSOCKSLayout->addWidget(mSOCKSLabel);
    mSOCKSLayout->addWidget(mSOCKSLineEdit_1);
    mSOCKSLayout->addWidget(mSOCKSPortLabel);
    mSOCKSLayout->addWidget(mSOCKSLineEdit_2);

    line_6 = setLine(mManualFrame);

    mIgnoreFrame = new QFrame(mManualFrame);
    mIgnoreFrame->setMinimumSize(QSize(550, 0));
    mIgnoreFrame->setMaximumSize(QSize(16777215, 16777215));
    mIgnoreFrame->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *mIgnoreLayout = new QVBoxLayout(mIgnoreFrame);
    mIgnoreLayout->setSpacing(10);
    mIgnoreLayout->setContentsMargins(16, 0, 16, 24);
    mIgnoreLabel = new QLabel(mIgnoreFrame);
    mIgnoreLabel->setFixedHeight(36);
    mIgnoreLineEdit = new QTextEdit(mIgnoreFrame);
    mIgnoreLineEdit->setFixedHeight(120);
    mIgnoreLineEdit->setStyleSheet("border-radius:6px;background-color: palette(button)");
    mIgnoreLayout->addWidget(mIgnoreLabel);
    mIgnoreLayout->addWidget(mIgnoreLineEdit);

    mManualLayout->addWidget(mManualProxyWidget);
    mManualLayout->addWidget(line_2);
    mManualLayout->addWidget(mHTTPFrame);
    mManualLayout->addWidget(line_3);
    mManualLayout->addWidget(mHTTPSFrame);
    mManualLayout->addWidget(line_4);
    mManualLayout->addWidget(mFTPFrame);
    mManualLayout->addWidget(line_5);
    mManualLayout->addWidget(mSOCKSFrame);
    mManualLayout->addWidget(line_6);
    mManualLayout->addWidget(mIgnoreFrame);

    //APT代理模块
    mAptProxyLabel = new TitleLabel(widget);
    mAPTFrame = new QFrame(widget);
    mAPTFrame->setMinimumSize(QSize(550, 0));
    mAPTFrame->setMaximumSize(QSize(16777215, 16777215));
    mAPTFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *AptLayout = new QVBoxLayout(mAPTFrame);
    AptLayout->setContentsMargins(0, 0, 0, 0);
    AptLayout->setSpacing(0);

    mAPTFrame_1 = new QFrame(mAPTFrame);
    setFrame_Noframe(mAPTFrame_1);

    QHBoxLayout *mAptLayout_1 = new QHBoxLayout(mAPTFrame_1);
    mAptLayout_1->setContentsMargins(16, 0, 16, 0);
    mAptLayout_1->setSpacing(8);

    mAptLabel = new QLabel(mAPTFrame_1);
    mAptLabel->setFixedWidth(200);
    mAptBtn = new SwitchButton(mAPTFrame_1);
    mAptLayout_1->addWidget(mAptLabel);
    mAptLayout_1->addStretch();
    mAptLayout_1->addWidget(mAptBtn);

    mAPTFrame_2 = new QFrame(mAPTFrame);
    setFrame_Noframe(mAPTFrame_2);

    QHBoxLayout *mAptLayout_2 = new QHBoxLayout(mAPTFrame_2);
    mAptLayout_2->setContentsMargins(16, 0, 16, 0);
    mAptLayout_2->setSpacing(8);

    mAPTHostLabel_1 = new QLabel(mAPTFrame_2);
    mAPTHostLabel_2 = new QLabel(mAPTFrame_2);
    mAPTPortLabel_1 = new QLabel(mAPTFrame_2);
    mAPTPortLabel_2 = new QLabel(mAPTFrame_2);
    mEditBtn = new QPushButton(mAPTFrame_2);
    mEditBtn->setFixedWidth(80);
    mAptLayout_2->addWidget(mAPTHostLabel_1);
    mAptLayout_2->addWidget(mAPTHostLabel_2);
    mAptLayout_2->addSpacing(100);
    mAptLayout_2->addWidget(mAPTPortLabel_1);
    mAptLayout_2->addWidget(mAPTPortLabel_2);
    mAptLayout_2->addStretch();
    mAptLayout_2->addWidget(mEditBtn,Qt::AlignRight);

    line_7 = setLine(mAPTFrame);

    AptLayout->addWidget(mAPTFrame_1);
    AptLayout->addWidget(line_7);
    AptLayout->addWidget(mAPTFrame_2);

    mverticalLayout->addWidget(mTitleLabel);
    mverticalLayout->addWidget(mAutoFrame);
    mverticalLayout->addWidget(mManualFrame);
    mverticalLayout->addSpacing(24);
    mverticalLayout->addWidget(mAptProxyLabel);
    mverticalLayout->addWidget(mAPTFrame);
    mverticalLayout->addStretch();

}

void Proxy::initSearchText() {

}

void Proxy::retranslateUi()
{
    mTitleLabel->setText(tr("System Proxy"));
    //~ contents_path /Proxy/Auto Proxy
    mAutoProxyLabel->setText(tr("Auto Proxy"));
    //~ contents_path /Proxy/Auto url
    mUrlLabel->setText(tr("Auto url"));
    //~ contents_path /Proxy/Manual Proxy
    mManualProxyLabel->setText(tr("Manual Proxy"));
    //~ contents_path /Proxy/Http Proxy
    mHTTPLabel->setText(tr("Http Proxy"));
    //~ contents_path /Proxy/Https Proxy
    mHTTPSLabel->setText(tr("Https Proxy"));
    //~ contents_path /Proxy/Ftp Proxy
    mFTPLabel->setText(tr("Ftp Proxy"));
    //~ contents_path /Proxy/Socks Proxy
    mSOCKSLabel->setText(tr("Socks Proxy"));
    mHTTPPortLabel->setText(tr("Port"));
    mHTTPSPortLabel->setText(tr("Port"));
    mFTPPortLabel->setText(tr("Port"));
    mSOCKSPortLabel->setText(tr("Port"));
    mIgnoreLabel->setText(tr("List of ignored hosts. more than one entry, please separate with english semicolon(;)"));

    //~ contents_path /Proxy/Apt Proxy
    mAptProxyLabel->setText(tr("Apt Proxy"));
    mAptLabel->setText(tr("Open"));
    mAPTHostLabel_1->setText(tr("Server Address : "));
    mAPTPortLabel_1->setText(tr("Port : "));
    mEditBtn->setText(tr("Edit"));
}

void Proxy::setupComponent(){
    //QLineEdit 设置数据
    GSData httpHostData;
    httpHostData.schema = HTTP_PROXY_SCHEMA;
    httpHostData.key = PROXY_HOST_KEY;
    mHTTPLineEdit_1->setProperty("gData", QVariant::fromValue(httpHostData));

    GSData httpsHostData;
    httpsHostData.schema = HTTPS_PROXY_SCHEMA;
    httpsHostData.key = PROXY_HOST_KEY;
    mHTTPSLineEdit_1->setProperty("gData", QVariant::fromValue(httpsHostData));

    GSData ftpHostData;
    ftpHostData.schema = FTP_PROXY_SCHEMA;
    ftpHostData.key = PROXY_HOST_KEY;
    mFTPLineEdit_1->setProperty("gData", QVariant::fromValue(ftpHostData));

    GSData socksHostData;
    socksHostData.schema = SOCKS_PROXY_SCHEMA;
    socksHostData.key = PROXY_HOST_KEY;
    mSOCKSLineEdit_1->setProperty("gData", QVariant::fromValue(socksHostData));

    GSData httpPortData;
    httpPortData.schema = HTTP_PROXY_SCHEMA;
    httpPortData.key = PROXY_PORT_KEY;
    mHTTPLineEdit_2->setProperty("gData", QVariant::fromValue(httpPortData));

    GSData httpsPortData;
    httpsPortData.schema = HTTPS_PROXY_SCHEMA;
    httpsPortData.key = PROXY_PORT_KEY;
    mHTTPSLineEdit_2->setProperty("gData", QVariant::fromValue(httpsPortData));

    GSData ftpPortData;
    ftpPortData.schema = FTP_PROXY_SCHEMA;
    ftpPortData.key = PROXY_PORT_KEY;
    mFTPLineEdit_2->setProperty("gData", QVariant::fromValue(ftpPortData));

    GSData socksPortData;
    socksPortData.schema = SOCKS_PROXY_SCHEMA;
    socksPortData.key = PROXY_PORT_KEY;
    mSOCKSLineEdit_2->setProperty("gData", QVariant::fromValue(socksPortData));
}

void Proxy::setupConnect(){
    connect(mAutoProxyWidget,&HoverWidget::widgetClicked,[=](){
        emit mAutoBtn->click();
    });

    connect(mManualProxyWidget,&HoverWidget::widgetClicked,[=](){
        emit mManualBtn->click();
    });

    connect(mEditBtn ,&QPushButton::clicked, this, &Proxy::setAptProxySlot);

    connect(mProxyBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton * eBtn){
        if (eBtn == mAutoBtn) {
            mAutoBtn->setChecked(true);
            mManualBtn->setChecked(false);
            proxysettings->set(PROXY_MODE_KEY,"auto");
        }
        else if (eBtn == mManualBtn){
            mAutoBtn->setChecked(false);
            mManualBtn->setChecked(true);
            proxysettings->set(PROXY_MODE_KEY,"manual");
        } else {
            mAutoBtn->setChecked(false);
            mManualBtn->setChecked(false);
            proxysettings->set(PROXY_MODE_KEY,"none");
        }
        _setSensitivity();
    });

    connect(mUrlLineEdit, &QLineEdit::textChanged, this, [=](const QString &txt){proxysettings->set(PROXY_AUTOCONFIG_URL_KEY, QVariant(txt));});

    connect(mHTTPLineEdit_1, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(mHTTPSLineEdit_1, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(mFTPLineEdit_1, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(mSOCKSLineEdit_1, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(mHTTPLineEdit_2, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(mHTTPSLineEdit_2, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(mFTPLineEdit_2, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});
    connect(mSOCKSLineEdit_2, &QLineEdit::textChanged, this, [=](const QString &txt){manualProxyTextChanged(txt);});

    connect(mIgnoreLineEdit, &QTextEdit::textChanged, this, [=](){
        QString text = mIgnoreLineEdit->toPlainText();
        QStringList hostStringList = text.split(";");
        proxysettings->set(IGNORE_HOSTS_KEY, QVariant(hostStringList));
    });

    connect(mAptBtn , &SwitchButton::checkedChanged, this ,[=](bool checked){
        if (checked) {
            emit mEditBtn->click();
        } else {   // 关闭APT代理，删除对应的配置文件
            if (QString(qgetenv("http_proxy").data()).isEmpty()) {
                line_7->hide();
                mAPTFrame_2->hide();
                setAptProxy("" ,0 ,false);
            } else {
                QMessageBox *mReboot = new QMessageBox(pluginWidget->topLevelWidget());
                mReboot->setIcon(QMessageBox::Warning);
                mReboot->setText(tr("The apt proxy  has been turned off and needs to be restarted to take effect"));
                QPushButton *laterbtn =  mReboot->addButton(tr("Reboot Later"), QMessageBox::RejectRole);
                QPushButton *nowbtn =   mReboot->addButton(tr("Reboot Now"), QMessageBox::AcceptRole);
                mReboot->exec();
                if (mReboot->clickedButton() == nowbtn) {  //选择了立即重启，一秒后系统会重启
                    line_7->hide();
                    mAPTFrame_2->hide();
                    setAptProxy("" ,0 ,false);
                    sleep(1);
                    reboot();
                } else {  //选择了稍后重启,删掉对应文件，但删不了已生效的环境变量
                    line_7->hide();
                    mAPTFrame_2->hide();
                    setAptProxy("" ,0 ,false);
                }
            }
        }
    });
}

void Proxy::initProxyModeStatus(){
    int mode = _getCurrentProxyMode();
    QHash<QString, QVariant> mAptinfo = getAptProxy();

    mAutoBtn->blockSignals(true);
    mManualBtn->blockSignals(true);
    mAptBtn->blockSignals(true);

    if (mode == AUTO){
        mAutoBtn->setChecked(true);
    } else if (mode == MANUAL){
        mManualBtn->setChecked(true);
    } else{
        mAutoBtn->setChecked(false);
        mManualBtn->setChecked(false);
    }

    if (Utils::isTablet()) {
        mAPTFrame->hide();
        mAptProxyLabel->hide();
    } else {
        if (mAptinfo["open"].toBool()) {
            mAptBtn->setChecked(true);
            mAPTHostLabel_2->setText(mAptinfo["ip"].toString());
            mAPTPortLabel_2->setText(mAptinfo["port"].toString());
        } else {
            mAptBtn->setChecked(false);
            line_7->setVisible(false);
            mAPTFrame_2->setVisible(false);
        }
    }


    mAutoBtn->blockSignals(false);
    mManualBtn->blockSignals(false);
    mAptBtn->blockSignals(false);

    _setSensitivity();
}

void Proxy::initAutoProxyStatus(){

    mUrlLineEdit->blockSignals(true);
    //设置当前url
    QString urlString = proxysettings->get(PROXY_AUTOCONFIG_URL_KEY).toString();
    mUrlLineEdit->setText(urlString);

    mUrlLineEdit->blockSignals(false);
}

void Proxy::initManualProxyStatus(){
    //信号阻塞
    mHTTPLineEdit_1->blockSignals(true);
    mHTTPSLineEdit_1->blockSignals(true);
    mFTPLineEdit_1->blockSignals(true);
    mSOCKSLineEdit_1->blockSignals(true);

    mHTTPLineEdit_2->blockSignals(true);
    mHTTPSLineEdit_2->blockSignals(true);
    mFTPLineEdit_2->blockSignals(true);
    mSOCKSLineEdit_2->blockSignals(true);

    //HTTP
    QString httphost = httpsettings->get(PROXY_HOST_KEY).toString();
    mHTTPLineEdit_1->setText(httphost);
    int httpport = httpsettings->get(PROXY_PORT_KEY).toInt();
    mHTTPLineEdit_2->setText(QString::number(httpport));

    //HTTPS
    QString httpshost = securesettings->get(PROXY_HOST_KEY).toString();
    mHTTPSLineEdit_1->setText(httpshost);
    int httpsport = securesettings->get(PROXY_PORT_KEY).toInt();
    mHTTPSLineEdit_2->setText(QString::number(httpsport));

    //FTP
    QString ftphost = ftpsettings->get(PROXY_HOST_KEY).toString();
    mFTPLineEdit_1->setText(ftphost);
    int ftppost = ftpsettings->get(PROXY_PORT_KEY).toInt();
    mFTPLineEdit_2->setText(QString::number(ftppost));

    //SOCKS
    QString sockshost = sockssettings->get(PROXY_HOST_KEY).toString();
    mSOCKSLineEdit_1->setText(sockshost);
    int socksport = sockssettings->get(PROXY_PORT_KEY).toInt();
    mSOCKSLineEdit_2->setText(QString::number(socksport));

    //解除信号阻塞
    mHTTPLineEdit_1->blockSignals(false);
    mHTTPSLineEdit_1->blockSignals(false);
    mFTPLineEdit_1->blockSignals(false);
    mSOCKSLineEdit_1->blockSignals(false);

    mHTTPLineEdit_2->blockSignals(false);
    mHTTPSLineEdit_2->blockSignals(false);
    mFTPLineEdit_2->blockSignals(false);
    mSOCKSLineEdit_2->blockSignals(false);
}

void Proxy::initIgnoreHostStatus(){
    mIgnoreLineEdit->blockSignals(true);

    //设置当前ignore host
    QStringList ignorehost = proxysettings->get(IGNORE_HOSTS_KEY).toStringList();
    mIgnoreLineEdit->setPlainText(ignorehost.join(";"));

    mIgnoreLineEdit->blockSignals(false);
}

int Proxy::_getCurrentProxyMode(){
    GSettings * proxygsettings;
    proxygsettings = g_settings_new(PROXY_SCHEMA);
    int mode = g_settings_get_enum(proxygsettings, PROXY_MODE_KEY);
    g_object_unref(proxygsettings);

    return mode;
}

void Proxy::_setSensitivity(){
    //自动配置代理界面敏感性
    bool autoChecked = mAutoBtn->isChecked();
    mUrlFrame->setEnabled(autoChecked);


    //手动配置代理界面敏感性
    bool manualChecked = mManualBtn->isChecked();
    mHTTPFrame->setEnabled(manualChecked);
    mHTTPSFrame->setEnabled(manualChecked);
    mFTPFrame->setEnabled(manualChecked);
    mSOCKSFrame->setEnabled(manualChecked);
    mIgnoreFrame->setEnabled(manualChecked);

}

void Proxy::setAptProxy(QString host, QString port, bool status)
{
    QDBusInterface *mAptproxyDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                                             "/",
                                                             "com.control.center.interface",
                                                             QDBusConnection::systemBus());
    if (mAptproxyDbus->isValid())
        QDBusReply<bool> reply = mAptproxyDbus->call("setaptproxy", host, port , status);
}

QHash<QString, QVariant> Proxy::getAptProxy()
{
     QHash<QString, QVariant> mAptInfo;
     QDBusInterface *mAptproxyDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                                              "/",
                                                              "com.control.center.interface",
                                                              QDBusConnection::systemBus());
    if (mAptproxyDbus->isValid()) {
        QDBusMessage result = mAptproxyDbus->call("getaptproxy");

        QList<QVariant> outArgs = result.arguments();
        QVariant first = outArgs.at(0);
        QDBusArgument dbvFirst = first.value<QDBusArgument>();
        QVariant vFirst = dbvFirst.asVariant();
        const QDBusArgument &dbusArgs = vFirst.value<QDBusArgument>();

        QVector<AptInfo> aptinfo;

        dbusArgs.beginArray();
        while (!dbusArgs.atEnd()) {
            AptInfo info;
            dbusArgs >> info;
            aptinfo.push_back(info);
        }
        dbusArgs.endArray();

        for (AptInfo it : aptinfo) {
            mAptInfo.insert(it.arg, it.out.variant());
        }
    }
    return mAptInfo;
}

void Proxy::setAptInfo()
{
    QMessageBox *mReboot = new QMessageBox(pluginWidget->topLevelWidget());
    mReboot->setIcon(QMessageBox::Warning);
    mReboot->setText(tr("The system needs to be restarted to set the Apt proxy, whether to reboot"));
    QPushButton *laterbtn =  mReboot->addButton(tr("Reboot Later"), QMessageBox::RejectRole);
    QPushButton *nowbtn =   mReboot->addButton(tr("Reboot Now"), QMessageBox::AcceptRole);
    mReboot->exec();
    if (mReboot->clickedButton() == nowbtn) {  //选择了立即重启，一秒后系统会重启
        sleep(1);
        reboot();
    } else  {  //选择了稍后重启或点击了关闭按钮，配置文件已写入，但是/etc/profile.d目录下新增的脚本文件未执行
        line_7->show();
        mAPTFrame_2->show();
        mAPTHostLabel_2->setText(getAptProxy()["ip"].toString());
        mAPTPortLabel_2->setText(getAptProxy()["port"].toString());
    }
}

void Proxy::reboot()
{
    QDBusInterface *rebootDbus = new QDBusInterface("org.gnome.SessionManager",
                                                             "/org/gnome/SessionManager",
                                                             "org.gnome.SessionManager",
                                                             QDBusConnection::sessionBus());

    rebootDbus->call("reboot");
    delete rebootDbus;
    rebootDbus = nullptr;
}

void Proxy::setFrame_Noframe(QFrame *frame)
{
    frame->setMinimumSize(QSize(550, 60));
    frame->setMaximumSize(QSize(16777215, 60));
    frame->setFrameShape(QFrame::NoFrame);
}

QFrame *Proxy::setLine(QFrame *frame)
{
    QFrame *line = new QFrame(frame);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

void Proxy::setAptProxySlot()
{
    mAptBtn->blockSignals(true);
    QHash<QString, QVariant> preaptinfo = getAptProxy();
    bool prestatus = preaptinfo["open"].toBool();
    AptProxyDialog *mwindow = new AptProxyDialog(pluginWidget);
    mwindow->exec();
    if (getAptProxy()["open"].toBool() && !prestatus) { // open值为true，用户点击了确定按钮，进行配置文件的写入，提示用户重启系统
        setAptInfo();
    }
    if (getAptProxy()["open"].toBool() && prestatus) {
        if (getAptProxy()["ip"].toString() == preaptinfo["ip"].toString() && getAptProxy()["port"].toString() == preaptinfo["port"].toString() && prestatus){  //点击了编辑按钮，且在设置IP和端口号的弹窗中，点击了取消或者关闭按钮
            line_7->show();
            mAPTFrame_2->show();
        } else {
             setAptInfo();
        }
    }
    if(!getAptProxy()["open"].toBool() && !prestatus){ // 点击了APT开关按钮，但是在设置IP和端口号的弹窗中，点击了取消或者关闭按钮
       mAptBtn->setChecked(false);
    }
    mAptBtn->blockSignals(false);
}

void Proxy::manualProxyTextChanged(QString txt){
    //获取被修改控件
    QObject * pobject = this->sender();
    QLineEdit * who = dynamic_cast<QLineEdit *>(pobject);

    //获取控件保存的用户数据
    GSData currentData = who->property("gData").value<GSData>();
    QString schema = currentData.schema;
    qDebug()<<schema;
    QString key = currentData.key;

    //构建临时QGSettings
    const QByteArray id = schema.toUtf8();
    const QByteArray iidd(id.data());
    QGSettings * setting = new QGSettings(iidd);

    setting->set(key, QVariant(txt));

    delete setting;
    setting = nullptr;
}

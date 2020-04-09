#include "appdetail.h"

#include <QDebug>

#include "ui_appdetail.h"


#define NOTICE_ORIGIN_SCHEMA "org.ukui.control-center.noticeorigin"

AppDetail::AppDetail(QString Name,QString key, QGSettings *gsettings, QWidget *parent) :
    QDialog(parent),appKey(key),m_gsettings(gsettings), appName(Name),
    ui(new Ui::AppDetail)
{
//    qDebug()<<"name is------>"<<keys<<endl;
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    initUiStatus();
//    initGSettings();
    initComponent();
    initConnect();
}

AppDetail::~AppDetail()
{
    delete ui;
}

void AppDetail::initUiStatus(){

    ui->frame->setStyleSheet("QFrame#frame{background: #ffffff; border: none; border-radius: 6px;}");

    ui->enableWidget->setStyleSheet("QWidget#enableWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->numberWidget->setStyleSheet("QWidget#numberWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->numberWidget->setStyleSheet("QWidget#numberWidget{background: #F4F4F4; border-radius: 6px;}");

    ui->closeBtn->setIcon(QIcon("://img/titlebar/close.png"));
    ui->closeBtn->setStyleSheet("QPushButton#closeBtn{background: #ffffff; border: none; border-radius: 6px;}"
                                "QPushButton:hover:!pressed#closeBtn{background: #FA6056; border: none; border-top-left-radius: 2px; border-top-right-radius: 6px; border-bottom-left-radius: 2px; border-bottom-right-radius: 2px;}"
                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border: none; border-top-left-radius: 2px; border-top-right-radius: 6px; border-bottom-left-radius: 2px; border-bottom-right-radius: 2px;}");

    enablebtn = new SwitchButton;
    ui->enableLayout->addWidget(enablebtn);
}

void AppDetail::initComponent() {
    ui->titleLabel->setText(appName);

    for(int i = 1; i < 5; i++) {
        ui->numberComboBox->addItem(QString::number(i));
    }

    if (m_gsettings) {
        bool judge = m_gsettings->get(MESSAGES_KEY).toBool();
        QString numvalue = m_gsettings->get(MAXIMINE_KEY).toString();

//        qDebug()<<"numvalue is------->"<<numvalue<<endl;

        enablebtn->setChecked(judge);
        ui->numberComboBox->setCurrentText(numvalue);
    }
}

void AppDetail::initConnect() {
    connect(ui->closeBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        close();
    });

    connect(ui->cancelBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        close();
    });

    connect(ui->confirmBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        confirmbtnSlot();
    });

}

//void AppDetail::initGSettings() {
//    if(QGSettings::isSchemaInstalled(NOTICE_ORIGIN_SCHEMA)) {

//        QByteArray orid(NOTICE_ORIGIN_SCHEMA);
//        m_gsettings = new QGSettings(orid);
//    }
//}

void AppDetail::confirmbtnSlot() {
    //TODO: get gsetting may invalid, so program will throw crash error
    if (m_gsettings) {
        bool judge = enablebtn->isChecked();
        int num = ui->numberComboBox->currentIndex() + 1;

        m_gsettings->set(MESSAGES_KEY, judge);
        m_gsettings->set(MAXIMINE_KEY, num);
    }
    close();
}



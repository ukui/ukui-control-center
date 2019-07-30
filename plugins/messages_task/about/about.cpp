#include <QWidget>

#include "about.h"
#include "ui_about.h"

#include <QDebug>

About::About()
{
    ui = new Ui::About;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("about");
    pluginType = MESSAGES_TASK;

//    qDebug() << "--------start------->";
//    qDebug() << QSysInfo::kernelType();
//    qDebug() << QSysInfo::kernelVersion();
//    qDebug() << "---------1---------->";
//    qDebug() << QSysInfo::machineHostName();
//    qDebug() << QSysInfo::currentCpuArchitecture();
//    qDebug() << "----------2----------->";
//    qDebug() << QSysInfo::prettyProductName();
//    qDebug() << QSysInfo::productType();
//    qDebug() << QSysInfo::productVersion();
//    qDebug() << "-------end---------->";
    initUI();
}

About::~About()
{
    delete ui;
}

QString About::get_plugin_name(){
    return pluginName;
}

int About::get_plugin_type(){
    return pluginType;
}

QWidget * About::get_plugin_ui(){
    return pluginWidget;
}

void About::initUI(){

    QLabel * manufacturers = new QLabel("manufacturers");
    QLabel * model = new QLabel("model");

    ui->sysinfoFormLayout->addRow(tr("manufacturers:"), manufacturers);
    ui->sysinfoFormLayout->addRow(tr("model:"), model);
//    ui->sysinfoFormLayout->addRow(tr("version:"), "version");
//    ui->sysinfoFormLayout->addRow(tr("Serial number:"), "Serial number");
//    ui->sysinfoFormLayout->addRow(tr("hostname:"), "hostname");
//    ui->sysinfoFormLayout->addRow(tr("running time:"), "running time");
//    ui->sysinfoFormLayout->addRow(tr("os type:"), "os type");
//    ui->sysinfoFormLayout->addRow(tr("os version"), "os version");
//    ui->sysinfoFormLayout->addRow(tr("system bit"), "system bit");
//    ui->sysinfoFormLayout->addRow(tr("kernel version"), "kernel version");
//    ui->sysinfoFormLayout->addRow(tr("architecture"), "architecture");

    logoLabel = new QLabel(pluginWidget);
    logoLabel->setAutoFillBackground(true);
    logoLabel->setScaledContents(true); //自动缩放，显示图像大小自动调整为QLabel大小
    QPixmap logopixmap("://manufacturers/NOKIA.jpg");
    if (logopixmap.isNull()){
        logopixmap = QPixmap("://manufacturers/UBUNTUKYLIN.jpg");
    }
    logoLabel->setPixmap(logopixmap);
    logoLabel->setFixedSize(logopixmap.size());
    logoLabel->setGeometry(QRect(pluginWidget->width() - 70 - logoLabel->width(), ui->infoLabel->geometry().bottom() + 25, logoLabel->width(), logoLabel->height()));
//    logoLabel->setMask(logopixmap.mask());

}

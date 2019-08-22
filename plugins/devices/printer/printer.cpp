#include "printer.h"
#include "ui_printer.h"

#include <QDebug>

Printer::Printer(){
    ui = new Ui::Printer;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("printer");
    pluginType = DEVICES;


    ui->addBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->addBtn->setIcon(QIcon(":/printer/add.png"));
    ui->addBtn->setIconSize(QSize(48,48));

    ui->listWidget->setStyleSheet("border: none; background-color: #f5f6f7");

    QStringList printer = QPrinterInfo::availablePrinterNames();

    for (int num = 0; num < printer.count(); num++){

        QWidget * printerdevWidget = new QWidget();
        printerdevWidget->setAttribute(Qt::WA_DeleteOnClose);
        QVBoxLayout * printerdevVerLayout = new QVBoxLayout(printerdevWidget);
        QToolButton * printerdevToolBtn = new QToolButton(printerdevWidget);
        printerdevToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        printerdevToolBtn->setIcon(QIcon("://printer/printerdev.png"));
        printerdevToolBtn->setIconSize(QSize(48,48));
        printerdevToolBtn->setText(printer.at(num));

        printerdevVerLayout->addWidget(printerdevToolBtn);
        printerdevWidget->setLayout(printerdevVerLayout);

        QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
        item->setSizeHint(QSize(180, 64));
        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, printerdevWidget);
    }

    connect(ui->addBtn, SIGNAL(clicked()), this, SLOT(run_external_app_slot()));
}

Printer::~Printer()
{
//    delete pluginWidget;
    delete ui;
}

QString Printer::get_plugin_name(){
    return pluginName;
}

int Printer::get_plugin_type(){
    return pluginType;
}

QWidget * Printer::get_plugin_ui(){
    return pluginWidget;
}

void Printer::run_external_app_slot(){
    QString cmd = "system-config-printer";

    QProcess process(this);
    process.startDetached(cmd);
}

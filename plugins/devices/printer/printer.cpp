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

    QStringList printer = QPrinterInfo::availablePrinterNames();

    for (int num = 0; num < printer.count(); num++){
        QListWidgetItem * item = new QListWidgetItem(printer.at(num), ui->listWidget);
        ui->listWidget->addItem(item);
    }

    connect(ui->addPushBtn, SIGNAL(clicked()), this, SLOT(run_external_app_slot()));
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

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

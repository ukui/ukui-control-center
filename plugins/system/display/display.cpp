#include <QtWidgets>

#include "display.h"
#include "ui_display.h"

Display::Display(){
    ui = new Ui::DisplayWindow();
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("display");

    pluginType = SYSTEM;

}

Display::~Display(){
//    delete pluginWidget;
    delete ui;
}

QWidget * Display::get_plugin_ui(){
    return pluginWidget;
}

QString Display::get_plugin_name(){
    return pluginName;
}

int Display::get_plugin_type(){
    return pluginType;
}

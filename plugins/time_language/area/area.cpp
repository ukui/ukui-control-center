#include "area.h"
#include "ui_area.h"

Area::Area()
{
    ui = new Ui::Area;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("area");
    pluginType = TIME_LANGUAGE;
}

Area::~Area()
{
    delete ui;
}

QString Area::get_plugin_name(){
    return pluginName;
}

int Area::get_plugin_type(){
    return pluginType;
}

QWidget * Area::get_plugin_ui(){
    return pluginWidget;
}

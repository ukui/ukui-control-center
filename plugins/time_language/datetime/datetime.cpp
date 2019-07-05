#include "datetime.h"
#include "ui_datetime.h"

DateTime::DateTime()
{
    ui = new Ui::DateTime;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("datetime");
    pluginType = TIME_LANGUAGE;
}

DateTime::~DateTime()
{
    delete ui;
}

QString DateTime::get_plugin_name(){
    return pluginName;
}

int DateTime::get_plugin_type(){
    return pluginType;
}

QWidget * DateTime::get_plugin_ui(){
    return pluginWidget;
}

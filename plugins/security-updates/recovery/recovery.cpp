#include "recovery.h"
#include "ui_recovery.h"

Recovery::Recovery()
{
    ui = new Ui::Recovery;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("recovery");
    pluginType = SECURITY_UPDATES;
}

Recovery::~Recovery()
{
    delete ui;
}

QString Recovery::get_plugin_name(){
    return pluginName;
}

int Recovery::get_plugin_type(){
    return pluginType;
}

CustomWidget *Recovery::get_plugin_ui(){
    return pluginWidget;
}

void Recovery::plugin_delay_control(){

}

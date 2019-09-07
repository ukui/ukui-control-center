#include "loginoptions.h"
#include "ui_loginoptions.h"

LoginOptions::LoginOptions()
{
    ui = new Ui::LoginOptions;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("loginoptions");
    pluginType = ACCOUNT;
}

LoginOptions::~LoginOptions()
{
    delete ui;
}

QString LoginOptions::get_plugin_name(){
    return pluginName;
}

int LoginOptions::get_plugin_type(){
    return pluginType;
}

CustomWidget * LoginOptions::get_plugin_ui(){
    return pluginWidget;
}

void LoginOptions::plugin_delay_control(){

}

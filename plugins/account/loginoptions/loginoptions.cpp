#include "loginoptions.h"
#include "ui_loginoptions.h"

LoginOptions::LoginOptions()
{
    ui = new Ui::LoginOptions;
    pluginWidget = new QWidget;
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

QWidget * LoginOptions::get_plugin_ui(){
    return pluginWidget;
}

#include "netconnect.h"
#include "ui_netconnect.h"

NetConnect::NetConnect()
{
    ui = new Ui::NetConnect;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("netconnect");
    pluginType = NETWORK;
}

NetConnect::~NetConnect()
{
    delete ui;
}

QString NetConnect::get_plugin_name(){
    return pluginName;
}

int NetConnect::get_plugin_type(){
    return pluginType;
}

QWidget * NetConnect::get_plugin_ui(){
    return pluginWidget;
}

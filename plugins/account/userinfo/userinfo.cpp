#include "userinfo.h"
#include "ui_userinfo.h"

UserInfo::UserInfo()
{
    ui = new Ui::UserInfo;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("userinfo");
    pluginType = ACCOUNT;
}

UserInfo::~UserInfo()
{
    delete ui;
}

QString UserInfo::get_plugin_name(){
    return pluginName;
}

int UserInfo::get_plugin_type(){
    return pluginType;
}

QWidget * UserInfo::get_plugin_ui(){
    return pluginWidget;
}

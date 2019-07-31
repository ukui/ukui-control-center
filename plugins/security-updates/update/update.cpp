#include "update.h"
#include "ui_update.h"

Update::Update()
{
    ui = new Ui::Update;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("update");
    pluginType = SECURITY_UPDATES;
}

Update::~Update()
{
    delete ui;
}

QString Update::get_plugin_name(){
    return pluginName;
}

int Update::get_plugin_type(){
    return pluginType;
}

QWidget * Update::get_plugin_ui(){
    return pluginWidget;
}

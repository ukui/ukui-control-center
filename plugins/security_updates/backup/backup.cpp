#include "backup.h"
#include "ui_backup.h"

Backup::Backup()
{
    ui = new Ui::Backup;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("backup");
    pluginType = SECURITY_UPDATES;
}

Backup::~Backup()
{
    delete ui;
}

QString Backup::get_plugin_name(){
    return pluginName;
}

int Backup::get_plugin_type(){
    return pluginType;
}

QWidget * Backup::get_plugin_ui(){
    return pluginWidget;
}

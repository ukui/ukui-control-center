#include "backup.h"
#include "ui_backup.h"

Backup::Backup()
{
    ui = new Ui::Backup;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("backup");
    pluginType = SECURITY_UPDATES;

    ui->restoringBtn->setEnabled(false);

    connect(ui->backupBtn, SIGNAL(clicked()), this, SLOT(backup_btn_clicked_slot()));

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

CustomWidget *Backup::get_plugin_ui(){
    return pluginWidget;
}

void Backup::plugin_delay_control(){

}

void Backup::backup_btn_clicked_slot(){
    QString cmd = "/usr/bin/deja-dup";

    QProcess process(this);
    process.startDetached(cmd);
}

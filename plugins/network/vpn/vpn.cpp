#include "vpn.h"
#include "ui_vpn.h"

Vpn::Vpn()
{
    ui = new Ui::Vpn;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("vpn");
    pluginType = NETWORK;

    connect(ui->addPushBtn, SIGNAL(clicked()), this, SLOT(run_external_app_slot()));
}

Vpn::~Vpn()
{
    delete ui;
}

QString Vpn::get_plugin_name(){
    return pluginName;
}

int Vpn::get_plugin_type(){
    return pluginType;
}

QWidget * Vpn::get_plugin_ui(){
    return pluginWidget;
}

void Vpn::run_external_app_slot(){
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}

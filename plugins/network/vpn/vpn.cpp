#include "vpn.h"
#include "ui_vpn.h"

Vpn::Vpn()
{
    ui = new Ui::Vpn;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("vpn");
    pluginType = NETWORK;

    component_init();

    connect(ui->addBtn, SIGNAL(clicked()), this, SLOT(run_external_app_slot()));
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

CustomWidget *Vpn::get_plugin_ui(){
    return pluginWidget;
}

void Vpn::plugin_delay_control(){

}

void Vpn::component_init(){
    ui->addBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->addBtn->setIcon(QIcon(":/vpn/add.png"));
    ui->addBtn->setIconSize(QSize(48,48));
    ui->addBtn->setText(tr("Add VPN Connect"));

    PublicData * publicdata = new PublicData();
    QStringList tmpList = publicdata->subfuncList[NETWORK];
    connect(ui->availablenetBtn, &QPushButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList.at(0), NETWORK, 0);});

    delete publicdata;
}

void Vpn::run_external_app_slot(){
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}

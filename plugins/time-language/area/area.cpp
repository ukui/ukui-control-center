#include "area.h"
#include "ui_area.h"

Area::Area()
{
    ui = new Ui::Area;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("area");
    pluginType = TIME_LANGUAGE;

    PublicData * publicdata = new PublicData();
    QStringList tmpList = publicdata->subfuncList[DEVICES];
    connect(ui->kbdBtn, &QPushButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList.at(2), DEVICES, 0);});
    connect(ui->setPushButton, SIGNAL(clicked(bool)), this, SLOT(run_external_app_slot()));

    delete publicdata;
}

Area::~Area()
{
    delete ui;
}

QString Area::get_plugin_name(){
    return pluginName;
}

int Area::get_plugin_type(){
    return pluginType;
}

CustomWidget *Area::get_plugin_ui(){
    return pluginWidget;
}

void Area::plugin_delay_control(){

}

void Area::run_external_app_slot(){
    QString cmd = "gnome-language-selector";

    QProcess process(this);
    process.startDetached(cmd);
}


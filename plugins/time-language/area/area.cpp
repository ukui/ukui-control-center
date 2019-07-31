#include "area.h"
#include "ui_area.h"

Area::Area()
{
    ui = new Ui::Area;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("area");
    pluginType = TIME_LANGUAGE;

    connect(ui->setPushButton, SIGNAL(clicked(bool)), this, SLOT(run_external_app_slot()));
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

QWidget * Area::get_plugin_ui(){
    return pluginWidget;
}

void Area::run_external_app_slot(){
    QString cmd = "gnome-language-selector";

    QProcess process(this);
    process.startDetached(cmd);
}


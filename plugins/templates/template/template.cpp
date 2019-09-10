#include "template.h"
#include "ui_template.h"

Template::Template()
{
    ui = new Ui::Template;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = "template";
    pluginType = SYSTEM;

    QLabel * label = new QLabel(pluginWidget);
    label->setPixmap(QPixmap("://about/logo.svg"));
//    label->setText("aaaaaaaaaaa");
}

Template::~Template()
{
    delete ui;
}

QString Template::get_plugin_name(){
    return pluginName;
}

int Template::get_plugin_type(){
    return pluginType;
}

CustomWidget *Template::get_plugin_ui(){
    return pluginWidget;
}

void Template::plugin_delay_control(){

}

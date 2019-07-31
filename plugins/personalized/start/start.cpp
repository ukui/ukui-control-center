#include "start.h"
#include "ui_start.h"

#include <QDebug>

Start::Start()
{
    ui = new Ui::Start;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("start");
    pluginType = PERSONALIZED;

    //SwitchButton * pSwitchBtn = new SwitchButton(this);
    //ui->horizontalLayout->addWidget(pSwitchBtn);
    //ui->horizontalLayout->addStretch();
}

Start::~Start()
{
    delete ui;
}

QString Start::get_plugin_name(){
    return pluginName;
}

int Start::get_plugin_type(){
    return pluginType;
}

QWidget * Start::get_plugin_ui(){
    return pluginWidget;
}

void Start::onToggled(bool bChecked){
    qDebug() << "Current: " << bChecked;
}

#include "noticeoperation.h"
#include "ui_noticeoperation.h"

NoticeOperation::NoticeOperation()
{
    ui = new Ui::NoticeOperation;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("notice_operation");
    pluginType = MESSAGES_TASK;
}

NoticeOperation::~NoticeOperation()
{
    delete ui;
}

QString NoticeOperation::get_plugin_name(){
    return pluginName;
}

int NoticeOperation::get_plugin_type(){
    return pluginType;
}

QWidget * NoticeOperation::get_plugin_ui(){
    return pluginWidget;
}

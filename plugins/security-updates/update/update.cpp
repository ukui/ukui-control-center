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

    ui_init();

    connect(ui->checkBtn, SIGNAL(clicked()), this, SLOT(update_btn_clicked()));
    connect(ui->changesettingsBtn, SIGNAL(clicked()), this, SLOT(update_settings_btn_clicked()));
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

void Update::ui_init(){
    QPixmap pixmap("://update/logo.svg");
    ui->logoLabel->setPixmap(pixmap.scaled(QSize(64,64)));

    QDateTime current =QDateTime::currentDateTime();
    QString current_date_time =current.toString(QString("yyyy-MM-dd hh:mm:ss"));
    ui->updatetimeLabel->setText(current_date_time);
}

void Update::update_btn_clicked(){
    QString cmd = "/usr/bin/update-manager";

    QProcess process(this);
    process.startDetached(cmd);
}

void Update::update_settings_btn_clicked(){
    QString cmd = "software-properties-gtk";

    QProcess process(this);
    process.startDetached(cmd);
}

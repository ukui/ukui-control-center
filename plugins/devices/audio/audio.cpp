#include "audio.h"
#include "ui_audio.h"

#include <QDebug>

Audio::Audio()
{
    ui = new Ui::Audio;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("audio");
    pluginType = DEVICES;

    connect(ui->soundthemeBtn, SIGNAL(clicked()), this, SLOT(change_soundtheme_page()));
    connect(ui->preferencesBtn, SIGNAL(clicked()), this, SLOT(change_preference_page()));

    connect(ui->soundsettingsBtn, SIGNAL(clicked()), this, SLOT(sound_settings_btn_clicked_slot()));
}

Audio::~Audio()
{
    delete ui;
}

QString Audio::get_plugin_name(){
    return pluginName;
}

int Audio::get_plugin_type(){
    return pluginType;
}

CustomWidget *Audio::get_plugin_ui(){
    return pluginWidget;
}

void Audio::plugin_delay_control(){

}

void Audio::change_soundtheme_page(){
    ui->StackedWidget->setCurrentIndex(1);
}

void Audio::change_preference_page(){
    ui->StackedWidget->setCurrentIndex(2);
}

void Audio::sound_settings_btn_clicked_slot(){
    QString cmd = "ukui-volume-control";

    QProcess process(this);
    process.startDetached(cmd);
}

#include "audio.h"
#include "ui_audio.h"

Audio::Audio()
{
    ui = new Ui::Audio;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("audio");
    pluginType = DEVICES;

    connect(ui->soundthemeBtn, SIGNAL(clicked()), this, SLOT(change_soundtheme_page()));
    connect(ui->preferencesBtn, SIGNAL(clicked()), this, SLOT(change_preference_page()));
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

QWidget * Audio::get_plugin_ui(){
    return pluginWidget;
}

void Audio::change_soundtheme_page(){
    ui->StackedWidget->setCurrentIndex(1);
}

void Audio::change_preference_page(){
    ui->StackedWidget->setCurrentIndex(2);
}

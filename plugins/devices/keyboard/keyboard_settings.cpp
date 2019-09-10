#include <QWidget>
#include <QSlider>
#include <QDebug>
#include <QTimer>
#include <string.h>
#include "keyboard_settings.h"
#include "kylin-keyboard-interface.h"
#include "kylin-interface-interface.h"
#include "keybinding_view.h"

#define XKB_KBD_SCHEMA "org.mate.peripherals-keyboard-xkb.kbd"

Keyboard::Keyboard(){
    
    ui = new Ui::KeyboardWidget;
    pluginWidget = new  QWidget;
    pluginName = tr("keyboard");
    pluginType = DEVICES;
    ui->setupUi(pluginWidget);
    InitDBusKeyboard();
    InitDBusInterface();
    init_setting();
    setup_dialog();
    KeybindingView *keyview =  new KeybindingView(ui->treeView);
    keyboardXkb = new KeyboardXkb();
}

Keyboard::~Keyboard(){
    DeInitDBusKeyboard();
    DeInitDBusInterface();
    delete ui;
}

QString Keyboard::get_plugin_name(){
    return pluginName;
}

int Keyboard::get_plugin_type(){
    return pluginType;
}

QWidget * Keyboard::get_plugin_ui(){
    return pluginWidget;
}

void Keyboard::set_kb_repeat_visible(){

}

void Keyboard::set_kb_cursorblink_visible(){

}

void Keyboard::init_setting(){
    ui->checkBox->setChecked(kylin_hardware_keyboard_get_repeat());
    ui->horizontalSlider->setValue(kylin_hardware_keyboard_get_delay());
    ui->horizontalSlider_2->setValue(kylin_hardware_keyboard_get_rate());
    ui->checkBox_2->setChecked(kylin_hardware_keyboard_get_cursorblink());
    ui->horizontalSlider_3->setValue(kylin_hardware_keyboard_get_cursorblinktime());
    set_kb_repeat_visible();
    set_kb_cursorblink_visible();
    layout_settings = g_settings_new(XKB_KBD_SCHEMA);
    QTimer::singleShot(1000, this, SLOT(getCountries()) );
}

void Keyboard::setup_dialog(){
    connect(ui->checkBox,&QCheckBox::stateChanged,this,&Keyboard::on_checkBox_stateChanged);
    connect(ui->horizontalSlider,&QSlider::valueChanged,this,&Keyboard::on_horizontalSlider_valueChanged);
    connect(ui->horizontalSlider_2,&QSlider::valueChanged,this,&Keyboard::on_horizontalSlider_2_valueChanged);
    connect(ui->checkBox_2,&QCheckBox::stateChanged,this,&Keyboard::on_checkBox_2_stateChanged);
    connect(ui->horizontalSlider_3,&QSlider::valueChanged,this,&Keyboard::on_horizontalSlider_3_valueChanged);
    connect(ui->pushButton_3,SIGNAL(clicked()),this,SLOT(on_pushButton_3_clicked()));
    connect(ui->pushButton_5,SIGNAL(clicked()),this,SLOT(on_pushButton_5_clicked()));
    connect(ui->treeView,SIGNAL(clicked(const QModelIndex)),this,SLOT(on_treeView_clicked(const QModelIndex)));
}

void Keyboard::getCountries()
{
    QList<Item> list = keyboardXkb->GetCountries();
    QStringList slist;
    if(list.count()<1)
        return ;
    char **layouts = g_settings_get_strv(layout_settings,"layouts");
    if(layouts){
        while(*layouts)
        {

            QString s = *layouts;
            s = s.toUpper();
            for(int i=0;i<list.count();i++)
            {
                if(list.at(i).name == s)
                   slist.append(list.at(i).desc);
            }
            layouts++;
        }
    }
    ui->comboBox->addItems(slist);
    ui->comboBox->setCurrentIndex(0);
    return ;
}

void Keyboard::on_checkBox_stateChanged(int arg1)
{
    kylin_hardware_keyboard_set_repeat(arg1);
}

void Keyboard::on_horizontalSlider_valueChanged(int value)
{
    kylin_hardware_keyboard_set_delay(value);
}

void Keyboard::on_horizontalSlider_2_valueChanged(int value)
{
    kylin_hardware_keyboard_set_rate(value);
}

void Keyboard::on_checkBox_2_stateChanged(int arg1)
{
    kylin_hardware_keyboard_set_cursorblink(arg1);
}

void Keyboard::on_horizontalSlider_3_valueChanged(int value)
{
    kylin_hardware_keyboard_set_cursorblinktime(value);
}

void Keyboard::on_pushButton_3_clicked()
{

}

void Keyboard::on_treeView_clicked(const QModelIndex &index)
{
}

void Keyboard::on_pushButton_5_clicked()
{
    ui->StackedWidget->setCurrentIndex(1);
}

#include "keyboardcontrol.h"
#include "ui_keyboardcontrol.h"

#include <QDebug>

#define KBD_SCHEMA "org.mate.peripherals-keyboard-xkb.kbd"
#define KBD_LAYOUTS_KEY "layouts"

KeyboardControl::KeyboardControl()
{
    ui = new Ui::KeyboardControl;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("keyboardcontrol");
    pluginType = DEVICES;

    const QByteArray id(KBD_SCHEMA);
    kbdsettings = new QGSettings(id);

//    kbl = new KeyboardLayout();
    layoutmanagerObj = new KbdLayoutManager(kbdsettings->get(KBD_LAYOUTS_KEY).toStringList());


    //设置按键
    InitDBusKeyboard();

    //设置光标闪烁
    InitDBusInterface();

    component_init();
    status_init();
}

KeyboardControl::~KeyboardControl()
{
    delete ui;
}

QString KeyboardControl::get_plugin_name(){
    return pluginName;
}

int KeyboardControl::get_plugin_type(){
    return pluginType;
}

QWidget * KeyboardControl::get_plugin_ui(){
    return pluginWidget;
}

void KeyboardControl::component_init(){
    // add
    keySwitchBtn = new SwitchButton(pluginWidget);
    ui->keyHLayout->addWidget(keySwitchBtn);
    ui->keyHLayout->addStretch();

    //add
    cursorSwitchBtn = new SwitchButton(pluginWidget);
    ui->cursorHLayout->addWidget(cursorSwitchBtn);
    ui->cursorHLayout->addStretch();

    //get layouts
    QStringList layoutsList = kbdsettings->get(KBD_LAYOUTS_KEY).toStringList();

    //add layouts
    for (QString layout : layoutsList){
        qDebug() << layout << layoutmanagerObj->kbd_get_description_by_id(const_cast<const char *>(layout.toLatin1().data()));
//        ui->layoutsComboBox->addwidgetItem(layoutmanagerObj->kbd_get_description_by_id(const_cast<const char *>(layout.toLatin1().data())));
        ui->layoutsComboBox->addItem(layoutmanagerObj->kbd_get_description_by_id(const_cast<const char *>(layout.toLatin1().data())), layout);
    }
}

void KeyboardControl::status_init(){
    //设置按键重复状态
    keySwitchBtn->setChecked(kylin_hardware_keyboard_get_repeat());

    //设置按键重复的延时
    ui->delayHSlider->setValue(kylin_hardware_keyboard_get_delay());

    //设置按键重复的速度
    ui->repeatspeedHSlider->setValue(kylin_hardware_keyboard_get_rate());

    //设置光标闪烁状态
    cursorSwitchBtn->setChecked(kylin_hardware_keyboard_get_cursorblink());

    //设置光标杉闪烁速度
    ui->blinkspeedHSlider->setValue(kylin_hardware_keyboard_get_cursorblinktime());

    connect(keySwitchBtn, &SwitchButton::checkedChanged, this, [=](bool status){kylin_hardware_keyboard_set_repeat(status);});

    connect(ui->delayHSlider, &QSlider::valueChanged, this, [=](int value){kylin_hardware_keyboard_set_delay(value);});
    connect(ui->repeatspeedHSlider, &QSlider::valueChanged, this, [=](int value){kylin_hardware_keyboard_set_rate(value);});

    connect(cursorSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool status){kylin_hardware_keyboard_set_cursorblink(status);});

    connect(ui->blinkspeedHSlider, &QSlider::valueChanged, this, [=](int value){kylin_hardware_keyboard_set_cursorblinktime(value);});

    connect(ui->shortcutsBtn, &QPushButton::clicked,  this, [=]{ui->StackedWidget->setCurrentIndex(1);});

    connect(ui->showlmBtn, &QPushButton::clicked, this, [=]{layoutmanagerObj->show();});

    connect(ui->layoutsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(layout_combobox_changed_slot(int)));

    connect(layoutmanagerObj, &KbdLayoutManager::add_new_variant_signals, this, [=](QString id){rebuild_layouts_component(id);});
}

void KeyboardControl::rebuild_layouts_component(QString id){
    ui->layoutsComboBox->addItem(layoutmanagerObj->kbd_get_description_by_id(const_cast<const char *>(id.toLatin1().data())), id);

    QStringList layoutsList = kbdsettings->get(KBD_LAYOUTS_KEY).toStringList();
    layoutsList.append(id);
    kbdsettings->set(KBD_LAYOUTS_KEY, layoutsList);
}

void KeyboardControl::layout_combobox_changed_slot(int index){
    QStringList layoutsList;
    layoutsList.append(ui->layoutsComboBox->currentData(Qt::UserRole).toString());
    for (int i = 0; i < ui->layoutsComboBox->count(); i++){
        QString id = ui->layoutsComboBox->itemData(i, Qt::UserRole).toString();
        if (i != index) //跳过当前item
            layoutsList.append(id);
    }
    kbdsettings->set(KBD_LAYOUTS_KEY, layoutsList);
}

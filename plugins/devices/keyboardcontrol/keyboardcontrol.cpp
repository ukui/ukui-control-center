#include "keyboardcontrol.h"
#include "ui_keyboardcontrol.h"

#include <QDebug>

#define KBD_LAYOUTS_SCHEMA "org.mate.peripherals-keyboard-xkb.kbd"
#define KBD_LAYOUTS_KEY "layouts"

#define KEYBINDINGS_DESKTOP_SCHEMA "org.ukui.SettingsDaemon.plugins.media-keys"
#define KEYBINDINGS_CUSTOM_SCHEMA "org.ukui.control-center.keybinding"
#define KEYBINDINGS_SYSTEM_SCHEMA "org.gnome.desktop.wm.keybindings"

typedef struct _KeyEntry KeyEntry;
typedef enum{
    SHIFT_MASK    = 1 << 0,
    CONTROL_MASK  = 1 << 2,
    ALT_MASK      = 1 << 3,
    SUPER_MASK = 1 << 26,
}ModifierType;

struct _KeyEntry{
    int keyval;
    ModifierType mask;
    QString name;
    QString description;
    QString shortcut;
};

KeyboardControl::KeyboardControl()
{
    ui = new Ui::KeyboardControl;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("keyboardcontrol");
    pluginType = DEVICES;

    const QByteArray id(KBD_LAYOUTS_SCHEMA);
    kbdsettings = new QGSettings(id);

    const QByteArray iid(KEYBINDINGS_DESKTOP_SCHEMA);
    keybindsettings = new QGSettings(iid);

    const QByteArray idd(KEYBINDINGS_SYSTEM_SCHEMA);
    syskeysettings = new QGSettings(idd);

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
    delete kbdsettings;
    delete keybindsettings;
    delete syskeysettings;
}

QString KeyboardControl::get_plugin_name(){
    return pluginName;
}

int KeyboardControl::get_plugin_type(){
    return pluginType;
}

CustomWidget * KeyboardControl::get_plugin_ui(){
    return pluginWidget;
}

void KeyboardControl::plugin_delay_control(){

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

    //理想情况应该以xml文件的形式，当前先写死在代码中

    desktopshortcut << "logout" << "terminal" << "screensaver" << "screenshot" << "window-screenshot" << "area-screenshot";

    //快捷键表格设置
    ui->tableWidget->setStyleSheet("background-color: #f5f6f7");
    //列头高度
    ui->tableWidget->horizontalHeader()->setMinimumHeight(30);
    //列宽
    ui->tableWidget->setColumnWidth(0, 345);
    ui->tableWidget->setColumnWidth(1, 413); // 760 - 345 - 2
    // 隐藏行头
    ui->tableWidget->verticalHeader()->hide();

    ui->tableWidget->setRowCount(desktopshortcut.count());

    append_keys_from_desktop();
    append_keys_from_system();

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

void KeyboardControl::append_keys_from_desktop(){
    for (int row = 0; row < desktopshortcut.count(); row++){
        QString key = desktopshortcut.at(row);
        QString value = keybindsettings->get(key).toString();
        QTableWidgetItem * item = new QTableWidgetItem();
        item->setText(key);
        ui->tableWidget->setItem(row, 0, item);

        CustomLineEdit * line = new CustomLineEdit(value);
        line->setAttribute(Qt::WA_DeleteOnClose);
        line->setStyleSheet("QLineEdit{border: 0px solid}");
        line->setText(value);
        line->setReadOnly(true);
        ui->tableWidget->setCellWidget(row, 1, line);

        ui->tableWidget->setRowHeight(row, 30);
    }
}

void KeyboardControl::append_keys_from_system(){
    //使用QGsettings获取keys，部分key与实际不同，缺少-、变大写等
//    for (QString key : syskeysettings->keys()){
//        QString value = syskeysettings->get(key).toString();
//        qDebug() << key << value;
//    }
    GSettings * systemgsettings = g_settings_new(KEYBINDINGS_SYSTEM_SCHEMA);
    char ** gs = g_settings_list_keys(systemgsettings); //该方法已经废弃
    for (int i=0; gs[i]!= NULL; i++){
        //切换为mutter后，原先为string的变为字符串数组，这块只取了字符串数组的第一个元素
        GVariant *variant = g_settings_get_value(systemgsettings, gs[i]);
        gsize size = g_variant_get_size(variant);
        char **tmp = const_cast<char **>(g_variant_get_strv(variant, &size));
        char *str = tmp[0];
//        g_warning("%s------->%s", gs[i], str);
    }
    g_strfreev(gs);
    g_object_unref(systemgsettings);
    qDebug() << "***" << binding_from_string("<contRol><Alt>Escape");
}

QList<int> KeyboardControl::binding_from_string(QString keyString){
    QList<int> keycodeList;
    keycodeList << 1 << 2;
    QString operationStr = keyString;
    while(operationStr.contains("<")){
        qDebug() << operationStr;
        int len = operationStr.length();
        if (len >= 9 && (QString::compare(operationStr.left(9), "<release>", Qt::CaseInsensitive) == 0)){
            qDebug() << "release press";
            operationStr.remove(0, 9);
        }
        else if (len >= 9 && (QString::compare(operationStr.left(9), "<control>", Qt::CaseInsensitive) == 0)){
            qDebug() << "control press";
            operationStr.remove(0, 9);
        }
        else if (len >= 9 && (QString::compare(operationStr.left(9), "<primary>", Qt::CaseInsensitive) == 0)){
            qDebug() << "primary press";
            operationStr.remove(0, 9);
        }
        else if (len >= 7 && (QString::compare(operationStr.left(7), "<shift>", Qt::CaseInsensitive) == 0)){
            qDebug() << "shift press";
            operationStr.remove(0, 7);
        }
        else if (len >= 6 && (QString::compare(operationStr.left(6), "<shft>", Qt::CaseInsensitive) == 0)){
            qDebug() << "shft press";
            operationStr.remove(0, 6);
        }
        else if (len >= 6 && (QString::compare(operationStr.left(6), "<ctrl>", Qt::CaseInsensitive) == 0)){
            qDebug() << "ctrl press";
            operationStr.remove(0, 6);
        }
        else if (len >= 5 && (QString::compare(operationStr.left(5), "<alt>", Qt::CaseInsensitive) == 0)){
            qDebug() << "ctrl press";
            operationStr.remove(0, 5);
        }
        else if (len >= 7 && (QString::compare(operationStr.left(7), "<super>", Qt::CaseInsensitive) == 0)){
            qDebug() << "ctrl press";
            operationStr.remove(0, 7);
        }
    }


    return keycodeList;
}

#include "keyboardcontrol.h"
#include "ui_keyboardcontrol.h"

#include <QDebug>

#define KBD_LAYOUTS_SCHEMA "org.mate.peripherals-keyboard-xkb.kbd"
#define KBD_LAYOUTS_KEY "layouts"

#define KEYBINDINGS_DESKTOP_SCHEMA "org.ukui.SettingsDaemon.plugins.media-keys"
#define KEYBINDINGS_CUSTOM_SCHEMA "org.ukui.control-center.keybinding"
#define KEYBINDINGS_SYSTEM_SCHEMA "org.gnome.desktop.wm.keybindings"

typedef struct _KeyEntry KeyEntry;

struct _KeyEntry : QObjectUserData{
    int keyval;
    QString settings;
    QString gsettings_key;
    QString description;
    QString gsettings_value;
};

Q_DECLARE_METATYPE(KeyEntry)

QList<KeyEntry *> systemEntries;
QList<KeyEntry *> desktopEntries;

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
    desktopsettings = new QGSettings(iid);

    const QByteArray idd(KEYBINDINGS_SYSTEM_SCHEMA);
    syskeysettings = new QGSettings(idd);

//    kbl = new KeyboardLayout();
    layoutmanagerObj = new KbdLayoutManager(kbdsettings->get(KBD_LAYOUTS_KEY).toStringList());

    keymapobj = new KeyMap();


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
    delete desktopsettings;
    delete syskeysettings;

    delete keymapobj;

    for (KeyEntry * keyentry: desktopEntries){
        delete keyentry;
    }

    for (KeyEntry * keyentry: systemEntries){
        delete keyentry;
    }
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

    rebuild_layouts_component();

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

    QStringList forbidden_keyList = {
        //Navigation keys
        "Home",
        "Left",
        "Up",
        "Right",
        "Down",
        "Page_Up",
        "Page_Down",
        "End",
        "Tab",

        // Return
        "Return",
        "Enter",

        "Space",
    };

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

    connect(ui->showlmBtn, &QPushButton::clicked, this, [=]{layoutmanagerObj->exec(); });

    connect(ui->layoutsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(layout_combobox_changed_slot(int)));

    connect(layoutmanagerObj, &KbdLayoutManager::add_new_variant_signals, this, [=]{rebuild_layouts_component();});
    connect(layoutmanagerObj, &KbdLayoutManager::del_variant_signals, this, [=]{rebuild_layouts_component();});

}

void KeyboardControl::rebuild_layouts_component(){
    QStringList layouts = kbdsettings->get(KBD_LAYOUTS_KEY).toStringList();
    ui->layoutsComboBox->blockSignals(true);
    ui->layoutsComboBox->clear();
    for (QString layout : layouts){
//        qDebug() << layout << layoutmanagerObj->kbd_get_description_by_id(const_cast<const char *>(layout.toLatin1().data()));
//        ui->layoutsComboBox->addwidgetItem(layoutmanagerObj->kbd_get_description_by_id(const_cast<const char *>(layout.toLatin1().data())));
        ui->layoutsComboBox->addItem(layoutmanagerObj->kbd_get_description_by_id(const_cast<const char *>(layout.toLatin1().data())), layout);
    }


//    ui->layoutsComboBox->addItem(layoutmanagerObj->kbd_get_description_by_id(const_cast<const char *>(id.toLatin1().data())), id);

//    QStringList layoutsList = kbdsettings->get(KBD_LAYOUTS_KEY).toStringList();
//    layoutsList.append(id);
//    kbdsettings->set(KBD_LAYOUTS_KEY, layoutsList);

    ui->layoutsComboBox->blockSignals(false);
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
    desktopEntries.clear();

    for (int row = 0; row < desktopshortcut.count(); row++){
        QString key = desktopshortcut.at(row);
        QString value = desktopsettings->get(key).toString();

        //保存桌面快捷键
        KeyEntry * tmpkeyentry = new KeyEntry();
        tmpkeyentry->settings = KEYBINDINGS_DESKTOP_SCHEMA;
        tmpkeyentry->gsettings_key = key;
        tmpkeyentry->gsettings_value = value;
        desktopEntries.append(tmpkeyentry);

        QTableWidgetItem * item = new QTableWidgetItem();
        item->setText(key);
        ui->tableWidget->setItem(row, 0, item);

        CustomLineEdit * line = new CustomLineEdit(value);
        line->setAttribute(Qt::WA_DeleteOnClose);
        line->setStyleSheet("QLineEdit{border: 0px solid}");
        line->setText(value);
        line->setReadOnly(true);
        line->setUserData(Qt::UserRole, tmpkeyentry);
        connect(line, SIGNAL(shortcut_code_signals(QList<int>)), this, SLOT(receive_shortcut_slot(QList<int>)));
        //
//        connect(desktopsettings, &QGSettings::changed, this, [=](QString value){line->setText(value);});
        ui->tableWidget->setCellWidget(row, 1, line);



        ui->tableWidget->setRowHeight(row, 30);
    }
}

void KeyboardControl::append_keys_from_system(){
    systemEntries.clear(); // 初始化

    //使用QGsettings获取keys，部分key与实际不同，缺少-、变大写等
//    for (QString key : syskeysettings->keys()){
//        QString value = syskeysettings->get(key).toString();
//        qDebug() << key << value;
//    }

    GSettings * systemgsettings = g_settings_new(KEYBINDINGS_SYSTEM_SCHEMA);
    char ** gs = g_settings_list_keys(systemgsettings); //该方法已经废弃,会报警告
    for (int i=0; gs[i]!= NULL; i++){
        //切换为mutter后，原先为string的变为字符串数组，这块只取了字符串数组的第一个元素
        GVariant *variant = g_settings_get_value(systemgsettings, gs[i]);
        gsize size = g_variant_get_size(variant);
        char **tmp = const_cast<char **>(g_variant_get_strv(variant, &size));
        char *str = tmp[0];

        //保存系统快捷键
        QString value = QString(str); QString key = QString(gs[i]);
        if (value != ""){
            KeyEntry * keyentry = new KeyEntry();
            keyentry->gsettings_key = key;
            keyentry->gsettings_value = value;
            systemEntries.append(keyentry);
        }
    }
    g_strfreev(gs);
    g_object_unref(systemgsettings);
}

QString KeyboardControl::binding_name(QList<int> shortcutList){ //整形键值转可读字符串，以gsettings的键值格式为基准
    QStringList tmpList;
    for (int keycode : shortcutList){
        if (keycode >= 16777216 && keycode <= 16777254){ //1677216=Escape; 16777254=ScrollLock
            tmpList.append(QString("<%1>").arg(keymapobj->keycodeTokeystring(keycode)));
        }
        else if (keycode >= 48 && keycode <= 57){ // 48 = 0; 57 = 9
            QString str = keymapobj->keycodeTokeystring(keycode);
            tmpList.append(str.split("K_").at(1));
        }
        else if (keycode >= 65 && keycode <= 90){
            QString str = keymapobj->keycodeTokeystring(keycode);
            tmpList.append(str.toLower());
        }
        else
            tmpList.append(keymapobj->keycodeTokeystring(keycode));
    }
    return tmpList.join("");
}

QString KeyboardControl::binding_from_string(QString keyString){
    QStringList keyStringList;
    QString operationStr = keyString;
    while(operationStr.length() > 0){
//        qDebug() << operationStr;
        if (operationStr.startsWith("<")){
            int len = operationStr.length();
            if (len >= 9 && (QString::compare(operationStr.left(9), "<release>", Qt::CaseInsensitive) == 0)){
                keyStringList.append("<Release>");
                operationStr.remove(0, 9);
            }
            else if (len >= 9 && (QString::compare(operationStr.left(9), "<control>", Qt::CaseInsensitive) == 0)){
                keyStringList.append("<Control>");
                operationStr.remove(0, 9);
            }
            else if (len >= 9 && (QString::compare(operationStr.left(9), "<primary>", Qt::CaseInsensitive) == 0)){
                keyStringList.append("<Control>");
                operationStr.remove(0, 9);
            }
            else if (len >= 7 && (QString::compare(operationStr.left(7), "<shift>", Qt::CaseInsensitive) == 0)){
                keyStringList.append("<Shift>");
                operationStr.remove(0, 7);
            }
            else if (len >= 6 && (QString::compare(operationStr.left(6), "<shft>", Qt::CaseInsensitive) == 0)){
                keyStringList.append("<Shift>");
                operationStr.remove(0, 6);
            }
            else if (len >= 6 && (QString::compare(operationStr.left(6), "<ctrl>", Qt::CaseInsensitive) == 0)){
                keyStringList.append("<Control>");
                operationStr.remove(0, 6);
            }
            else if (len >= 5 && (QString::compare(operationStr.left(5), "<alt>", Qt::CaseInsensitive) == 0)){
                keyStringList.append("<Alt>");
                operationStr.remove(0, 5);
            }
            else if (len >= 7 && (QString::compare(operationStr.left(7), "<super>", Qt::CaseInsensitive) == 0)){
                keyStringList.append("<Super>");
                operationStr.remove(0, 7);
            }
        }
        else{
            keyStringList.append(operationStr);
            operationStr.remove(0, operationStr.length());
        }
    }
    return keyStringList.join("");
}

bool KeyboardControl::key_is_forbidden(QString key){
    for (int i = 0; i < forbidden_keys.length(); i++){
        if (key == forbidden_keys[i])
            return true;
    }
    return false;

}

void KeyboardControl::receive_shortcut_slot(QList<int> shortcutList){
    QObject * object = QObject::sender();

    CustomLineEdit * current = qobject_cast<CustomLineEdit *>(object);
    KeyEntry * keyentry = dynamic_cast<KeyEntry *>(current->userData(Qt::UserRole));

    QString shortcutString = binding_name(shortcutList);
    qDebug() << "current shortcut string is: " << shortcutString;

    //check for unmodified keys
    if (shortcutList.count() == 1){
        if (shortcutString.contains(QRegExp("[A-Z]")) ||
                shortcutString.contains(QRegExp("[0-9]")) ||
                key_is_forbidden(shortcutString)){
            qDebug() << "Please try with a key such as Control, Alt or Shift at the same time.";
            return;
        }
    }

    /* flag to see if the new accelerator was in use by something */
    for (KeyEntry * cpkeyentry : desktopEntries){
        if (shortcutString == cpkeyentry->gsettings_value){
            qDebug() << QString("The shortcut \"%1\" is already used for\n\"%2\",please reset!!!").arg(shortcutString).arg(cpkeyentry->gsettings_key);
            return;
        }
    }

    for (KeyEntry * cpkeyentry : systemEntries){
        if (shortcutString == cpkeyentry->gsettings_value){
            qDebug() << QString("The shortcut \"%1\" is already used for\n\"%2\",please reset!!!").arg(shortcutString).arg(cpkeyentry->gsettings_key);
            return;
        }
    }

    current->setText(shortcutString);
    if (keyentry->settings == KEYBINDINGS_DESKTOP_SCHEMA){
        desktopsettings->set(keyentry->gsettings_key, shortcutString);
    }
    else if (keyentry->settings == KEYBINDINGS_SYSTEM_SCHEMA){
        syskeysettings->set(keyentry->gsettings_key, shortcutString);
    }
}

#include "mousecontrol.h"
#include "ui_mousecontrol.h"

#include <QDebug>

#define NONE_ID 0
#define CURSORSIZE_SMALLER 18
#define CURSORSIZE_MEDIUM 32
#define CURSORSIZE_LARGER 48

struct RollingType : QObjectUserData{
    QString type;
};

MouseControl::MouseControl()
{
    ui = new Ui::MouseControl;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = "mousecontrol";
    pluginType = DEVICES;

    const QByteArray id(TOUCHPAD_SCHEMA);
    tpsettings = new QGSettings(id);

    InitDBusMouse();

    component_init();
    status_init();
}

MouseControl::~MouseControl()
{
    delete ui;
}

QString MouseControl::get_plugin_name(){
    return pluginName;
}

int MouseControl::get_plugin_type(){
    return pluginType;
}

QWidget * MouseControl::get_plugin_ui(){
    return pluginWidget;
}

void MouseControl::component_init(){

    // Cursors themes
    QStringList themes = _get_cursors_themes();
    ui->CursorthemesComboBox->addItem(tr("Default"));
    ui->CursorthemesComboBox->addItems(themes);

    // add switchbtn for active touchpad
    activeBtn = new SwitchButton(pluginWidget);
    ui->activeHLayout->addWidget(activeBtn);
    ui->activeHLayout->addStretch();

    // hide helper radiobutton
    ui->noneRadioButton->hide();
    // set buttongroup id
    ui->rollingbuttonGroup->setId(ui->noneRadioButton, NONE_ID);

    // set user data rolling radiobutton
    RollingType * vedge = new RollingType(); vedge->type = V_EDGE_KEY;
    ui->vedgeRadioBtn->setUserData(Qt::UserRole, vedge);
    RollingType * hedge = new RollingType(); hedge->type = H_EDGE_KEY;
    ui->hedgeRadioBtn->setUserData(Qt::UserRole, hedge);
    RollingType * vfinger = new RollingType(); vfinger->type = V_FINGER_KEY;
    ui->vfingerRadioBtn->setUserData(Qt::UserRole, vfinger);
    RollingType * hfinger = new RollingType(); hfinger->type = H_FINGER_KEY;
    ui->hfingerRadioBtn->setUserData(Qt::UserRole, hfinger);


    // set buttongroup id
    ui->cursorsizebuttonGroup->setId(ui->smallerRadioBtn, CURSORSIZE_SMALLER);
    ui->cursorsizebuttonGroup->setId(ui->mediumRadioBtn, CURSORSIZE_MEDIUM);
    ui->cursorsizebuttonGroup->setId(ui->largerRadioBtn, CURSORSIZE_LARGER);

}

void MouseControl::status_init(){
    if (kylin_hardware_mouse_get_lefthanded())
        ui->leftRadioBtn->setChecked(true);
    else
        ui->rightRadioBtn->setChecked(true);

    //cursor theme
    QString curtheme  = kylin_hardware_mouse_get_cursortheme();
    if (curtheme == "")
        ui->CursorthemesComboBox->setCurrentIndex(0);
    else
        ui->CursorthemesComboBox->setCurrentText(curtheme);

    // speed sensitivity
    int accel_numerator, accel_denominator, threshold;  //当加速值和灵敏度为系统默认的-1时，从底层获取到默认的具体值

//    XGetPointerControl(QX11Info::display(), &accel_numerator, &accel_denominator, &threshold);
//    qDebug() << "--->" << accel_numerator << accel_denominator << threshold;

    double mouse_acceleration = kylin_hardware_mouse_get_motionacceleration();//当前系统指针加速值，-1为系统默认
    int mouse_threshold =  kylin_hardware_mouse_get_motionthreshold();//当前系统指针灵敏度，-1为系统默认

    //set speed
    qDebug() << kylin_hardware_mouse_get_motionacceleration() << kylin_hardware_mouse_get_motionthreshold() << "end";
    ui->speedSlider->setValue(kylin_hardware_mouse_get_motionacceleration()*100);

    //set sensitivity
    ui->sensitivitySlider->setValue(kylin_hardware_mouse_get_motionthreshold()*100);

    //set visibility position
    ui->posCheckBtn->setChecked(kylin_hardware_mouse_get_locatepointer());

    connect(ui->CursorthemesComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(cursor_themes_changed_slot(QString)));
    connect(ui->lefthandbuttonGroup, SIGNAL(buttonToggled(QAbstractButton*,bool)), this, SLOT(mouseprimarykey_changed_slot(QAbstractButton*, bool)));
    connect(ui->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(speed_value_changed_slot(int)));
    connect(ui->sensitivitySlider, SIGNAL(valueChanged(int)), this, SLOT(sensitivity_value_changed_slot(int)));
    connect(ui->posCheckBtn, SIGNAL(clicked(bool)), this, SLOT(show_pointer_position_slot(bool)));
    connect(ui->touchpadBtn, SIGNAL(clicked(bool)), this, SLOT(touchpad_settings_btn_clicked_slot()));
    connect(ui->cursorBtn, SIGNAL(clicked(bool)), this, SLOT(cursor_settings_btn_clicked_slot()));


    //touchpad settings
    activeBtn->setChecked(tpsettings->get(ACTIVE_TOUCHPAD_KEY).toBool());
    _refresh_touchpad_widget_status();

    // disable touchpad when typing
    ui->disablecheckBox->setChecked(tpsettings->get(DISABLE_WHILE_TYPING_KEY).toBool());

    // enable touchpad click
    ui->tpclickcheckBox->setChecked(tpsettings->get(TOUCHPAD_CLICK_KEY).toBool());

    // scrolling
    ui->vedgeRadioBtn->setChecked(tpsettings->get(V_EDGE_KEY).toBool());
    ui->hedgeRadioBtn->setChecked(tpsettings->get(H_EDGE_KEY).toBool());
    ui->vfingerRadioBtn->setChecked(tpsettings->get(V_FINGER_KEY).toBool());
    ui->hfingerRadioBtn->setChecked(tpsettings->get(H_FINGER_KEY).toBool());
    ui->noneRadioButton->setChecked(false);

    if (ui->rollingbuttonGroup->checkedButton() == 0)
        ui->rollingCheckBtn->setChecked(true);
    else
        ui->rollingCheckBtn->setChecked(false);
    _refresh_rolling_btn_status();


    connect(activeBtn, SIGNAL(checkedChanged(bool)), this, SLOT(active_touchpad_changed_slot(bool)));
    connect(ui->disablecheckBox, SIGNAL(clicked(bool)), this, SLOT(disable_while_typing_clicked_slot(bool)));
    connect(ui->tpclickcheckBox, SIGNAL(clicked(bool)), this, SLOT(touchpad_click_clicked_slot(bool)));
    connect(ui->rollingCheckBtn, SIGNAL(clicked(bool)), this, SLOT(rolling_enable_clicked_slot(bool)));
    connect(ui->rollingbuttonGroup, SIGNAL(buttonToggled(QAbstractButton*,bool)), this, SLOT(rolling_type_changed_slot(QAbstractButton*, bool)));


    //cursor settings

    int cursorsize = kylin_hardware_mouse_get_cursorsize();
    if (cursorsize <= CURSORSIZE_SMALLER)
        ui->smallerRadioBtn->setChecked(true);
    else if (cursorsize <= CURSORSIZE_MEDIUM)
        ui->mediumRadioBtn->setChecked(true);
    else
        ui->largerRadioBtn->setChecked(true);
    connect(ui->cursorsizebuttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(cursor_size_changed_slot()));

    //reset
    connect(ui->resetBtn, SIGNAL(clicked(bool)), this, SLOT(reset_btn_clicked_slot()));
}

QStringList MouseControl::_get_cursors_themes(){
    QStringList themes;
    QDir themesDir(CURSORS_THEMES_PATH);

    if (themesDir.exists()){
        foreach (QString dirname, themesDir.entryList(QDir::Dirs)){
            if (dirname == "." || dirname == "..")
                continue;
            QString fullpath(CURSORS_THEMES_PATH + dirname);
            QDir themeDir(CURSORS_THEMES_PATH + dirname + "/cursors/");
            if (themeDir.exists())
                themes.append(dirname);
        }
    }
    return themes;
}

void MouseControl::_refresh_touchpad_widget_status(){
    if (activeBtn->isChecked())
        ui->touchpadWidget->show();
    else
        ui->touchpadWidget->hide();
}

void MouseControl::_refresh_rolling_btn_status(){
    if (ui->rollingCheckBtn->isChecked())
        ui->rollingWidget->hide();
    else
        ui->rollingWidget->show();
}

void MouseControl::mouseprimarykey_changed_slot(QAbstractButton *button, bool status){
    QRadioButton * btn = dynamic_cast<QRadioButton *>(button);
    if (btn->text() == "left hand")
        kylin_hardware_mouse_set_lefthanded(status);
}

void MouseControl::cursor_themes_changed_slot(QString text){
    QString value = text;
    if (text == tr("Default"))
        value = "";

    QByteArray ba = value.toLatin1();
    kylin_hardware_mouse_set_cursortheme(ba.data());
}

void MouseControl::speed_value_changed_slot(int value){
    kylin_hardware_mouse_set_motionacceleration((double)value/ui->speedSlider->maximum()*10);
}

void MouseControl::sensitivity_value_changed_slot(int value){
    kylin_hardware_mouse_set_motionthreshold(10*value/ui->sensitivitySlider->maximum());
}

void MouseControl::show_pointer_position_slot(bool status){
    kylin_hardware_mouse_set_locatepointer(status);
}

void MouseControl::touchpad_settings_btn_clicked_slot(){
    ui->StackedWidget->setCurrentIndex(1);
}

void MouseControl::cursor_settings_btn_clicked_slot(){
    ui->StackedWidget->setCurrentIndex(2);
}

void MouseControl::active_touchpad_changed_slot(bool status){
    tpsettings->set(ACTIVE_TOUCHPAD_KEY, status);
    _refresh_touchpad_widget_status();
}

void MouseControl::disable_while_typing_clicked_slot(bool status){
    tpsettings->set(DISABLE_WHILE_TYPING_KEY, status);
}

void MouseControl::touchpad_click_clicked_slot(bool status){
    tpsettings->set(TOUCHPAD_CLICK_KEY, status);
}

void MouseControl::rolling_enable_clicked_slot(bool status){

    if (status)
        ui->noneRadioButton->setChecked(true);
    _refresh_rolling_btn_status();
}

void MouseControl::rolling_type_changed_slot(QAbstractButton *basebtn, bool status){
    if (ui->rollingbuttonGroup->checkedId() != NONE_ID)
        ui->rollingCheckBtn->setChecked(false);

    if (basebtn->text() == "None")
        return;

    QRadioButton * button = dynamic_cast<QRadioButton *>(basebtn);
    QString type = static_cast<RollingType *>(button->userData(Qt::UserRole))->type;
    tpsettings->set(type, status);
}

void MouseControl::cursor_size_changed_slot(){
    kylin_hardware_mouse_set_cursorsize(ui->cursorsizebuttonGroup->checkedId());
}

void MouseControl::reset_btn_clicked_slot(){
    ui->smallerRadioBtn->setChecked(true);
}

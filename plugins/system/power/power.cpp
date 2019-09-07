#include "power.h"
#include "ui_power.h"

#include <QDebug>

#define POWERMANAGER_SCHEMA "org.ukui.power-manager"
#define ICONPOLICY "icon-policy"
#define SLEEP_COMPUTER_AC_KEY "sleep-computer-ac"
#define SLEEP_COMPUTER_BATT_KEY "sleep-computer-battery"
#define SLEEP_DISPLAY_AC_KEY "sleep-display-ac"
#define SLEEP_DISPLAY_BATT_KEY "sleep-display-battery"
#define BUTTON_LID_AC_KEY "button-lid-ac"
#define BUTTON_LID_BATT_KET "button-lid-battery"
#define BUTTON_SUSPEND_KEY "button-suspend"
#define BUTTON_POWER_KEY "button-power"

#define PRESENT_VALUE "present"
#define ALWAYS_VALUE "always"

#define FIXES 60

typedef enum {
    BALANCE,
    SAVING,
    CUSTDOM
}MODE;

typedef enum {
    PRESENT,
    ALWAYS
}ICONDISPLAY;

Power::Power()
{
    ui = new Ui::Power;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("power");
    pluginType = SYSTEM;

    const QByteArray id(POWERMANAGER_SCHEMA);
    settings = new QGSettings(id);

    pluginWidget->setStyleSheet("QWidget#widget{background-color: #ffffff; border: 1px solid #cccccc}");

    component_init();
    status_init();

    ui->resetPushBtn->hide();
}

Power::~Power()
{
    delete ui;
    delete settings;
}

QString Power::get_plugin_name(){
    return pluginName;
}

int Power::get_plugin_type(){
    return pluginType;
}

CustomWidget *Power::get_plugin_ui(){
    return pluginWidget;
}

void Power::plugin_delay_control(){

}

void Power::component_init(){
    //
    ui->powerBtn->setIcon(QIcon("://power/power.png"));
    ui->batteryBtn->setIcon(QIcon("://power/battery.png"));

    //
    ui->powerBtnGroup->setId(ui->balanceRadioBtn, BALANCE);
    ui->powerBtnGroup->setId(ui->savingRadioBtn, SAVING);
    ui->powerBtnGroup->setId(ui->custdomRadioBtn, CUSTDOM);

    //电脑睡眠延迟
    sleepStringList  << tr("never") << tr("10 min") << tr("30 min") << tr("60 min") << tr("120 min");
    ui->acsleepComboBox->addItems(sleepStringList);
    ui->batsleepComboBox->addItems(sleepStringList);

    //显示器关闭延迟
    closeStringList  << tr("never") << tr("1 min") << tr("5 min") << tr("10 min") << tr("30 min") << tr("60 min");
    ui->accloseComboBox->addItems(closeStringList);
    ui->batcloseComboBox->addItems(closeStringList);

    //lid
    lidStringList << tr("nothing") << tr("blank") << tr("suspend") << tr("shutdown");
    ui->aclidComboBox->addItems(lidStringList);
    ui->batlidComboBox->addItems(lidStringList);

    //button
    buttonStringList << tr("interactive") << tr("suspend") << tr("shutdown");
    ui->powerbtnComboBox->addItems(buttonStringList);
    ui->suspendComboBox->addItems(buttonStringList);

    //
//    ui->icondisplayBtnGroup->setId(ui->presentRadioBtn, PRESENT);
//    ui->icondisplayBtnGroup->setId(ui->alwaysRadioBtn, ALWAYS);

    //平衡和节能模式暂未开放
    ui->custdomRadioBtn->setChecked(true);
    ui->balanceRadioBtn->setCheckable(false);
    ui->savingRadioBtn->setCheckable(false);
    refreshUI();
}

void Power::status_init(){

    //计算机睡眠延迟
    int acsleep = settings->get(SLEEP_COMPUTER_AC_KEY).toInt();
    QString acsleepString;
    if (acsleep != 0)
        acsleepString = QString("%1 min").arg(QString::number(acsleep/FIXES));
    else
        acsleepString = tr("never");
    ui->acsleepComboBox->setCurrentText(acsleepString);

    int batsleep = settings->get(SLEEP_COMPUTER_BATT_KEY).toInt();
    QString batsleepString;
    if (batsleep != 0)
        batsleepString = QString("%1 min").arg(QString::number(batsleep/FIXES));
    else
        batsleepString = tr("never");
    ui->batsleepComboBox->setCurrentText(batsleepString);

    //显示器关闭延迟
    int acclose = settings->get(SLEEP_DISPLAY_AC_KEY).toInt();
    QString accloseString;
    if (acclose != 0)
        accloseString = QString("%1 min").arg(QString::number(acclose/FIXES));
    else
        accloseString = tr("never");
    ui->accloseComboBox->setCurrentText(accloseString);

    int batclose = settings->get(SLEEP_DISPLAY_BATT_KEY).toInt();
    QString batcloseString;
    if (batclose != 0)
        batcloseString = QString("%1 min").arg(QString::number(batclose/FIXES));
    else
        batcloseString = tr("never");
    ui->batcloseComboBox->setCurrentText(batcloseString);

    //lid 枚举类型但是toint为零只能toString
    QString aclidString = settings->get(BUTTON_LID_AC_KEY).toString();
    ui->aclidComboBox->setCurrentText(aclidString);
    QString batlidString = settings->get(BUTTON_LID_BATT_KET).toString();
    ui->batlidComboBox->setCurrentText(batlidString);

    //power button
    QString powerbtn = settings->get(BUTTON_POWER_KEY).toString();
    ui->powerbtnComboBox->setCurrentText(powerbtn);
    QString suspentbtn = settings->get(BUTTON_SUSPEND_KEY).toString();
    ui->suspendComboBox->setCurrentText(suspentbtn);

    //电池图标  枚举类型但是toint为零只能toString
//    QString ipvalue = settings->get(ICONPOLICY).toString();
//    if (ipvalue == PRESENT_VALUE)
//        ui->presentRadioBtn->setChecked(true);
//    else if (ipvalue == ALWAYS_VALUE)
//        ui->alwaysRadioBtn->setChecked(true);

    connect(ui->powerBtnGroup, SIGNAL(buttonClicked(int)), this, SLOT(powerBtnGroup_changed_slot(int)));
//    connect(ui->icondisplayBtnGroup, SIGNAL(buttonClicked(int)), this, SLOT(icondisplayBtnGroup_changed_slot(int)));

    connect(ui->acsleepComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(ac_sleep_changed_slot(QString)));
    connect(ui->batsleepComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(bat_sleep_changed_slot(QString)));
    connect(ui->accloseComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(ac_close_changed_slot(QString)));
    connect(ui->batcloseComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(bat_close_changed_slot(QString)));
    connect(ui->aclidComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(ac_lid_changed_slot(QString)));
    connect(ui->batlidComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(bat_lid_changed_slot(QString)));

    connect(ui->powerbtnComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(power_btn_changed_slot(QString)));
    connect(ui->suspendComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(suspend_btn_changed_slot(QString)));
}

void Power::refreshUI(){
    if (ui->powerBtnGroup->checkedId() != CUSTDOM)
        ui->widget->setEnabled(false);
    else
        ui->widget->setEnabled(true);
}

void Power::power_btn_changed_slot(QString value){
    settings->set(BUTTON_POWER_KEY, QVariant(value));
}

void Power::suspend_btn_changed_slot(QString value){
    settings->set(BUTTON_SUSPEND_KEY, QVariant(value));
}

void Power::ac_lid_changed_slot(QString value){
    settings->set(BUTTON_LID_AC_KEY, QVariant(value));
}

void Power::bat_lid_changed_slot(QString value){
    settings->set(BUTTON_LID_BATT_KET, QVariant(value));
}

void Power::ac_sleep_changed_slot(QString value){
    int setvalue;
    if (value == "never")
        setvalue = 0;
    else{
        QString num = value.split(' ')[0];
        setvalue = (num.toInt()) * 60;
    }
    settings->set(SLEEP_COMPUTER_AC_KEY, QVariant(setvalue));
}

void Power::bat_sleep_changed_slot(QString value){
    int setvalue;
    if (value == "never")
        setvalue = 0;
    else{
        QString num = value.split(' ')[0];
        setvalue = (num.toInt()) * 60;
    }
    settings->set(SLEEP_COMPUTER_BATT_KEY, QVariant(setvalue));
}

void Power::ac_close_changed_slot(QString value){
    int setvalue;
    if (value == "never")
        setvalue = 0;
    else{
        QString num = value.split(' ')[0];
        setvalue = (num.toInt()) * 60;
    }
    settings->set(SLEEP_DISPLAY_AC_KEY, QVariant(setvalue));
}

void Power::bat_close_changed_slot(QString value){
    int setvalue;
    if (value == "never")
        setvalue = 0;
    else{
        QString num = value.split(' ')[0];
        setvalue = (num.toInt()) * 60;
    }
    settings->set(SLEEP_DISPLAY_BATT_KEY, QVariant(setvalue));
}

//void Power::icondisplayBtnGroup_changed_slot(int index){
//    if (index == PRESENT)
//        settings->set(ICONPOLICY, QVariant(PRESENT_VALUE));
//    else if (index == ALWAYS)
//        settings->set(ICONPOLICY, QVariant(ALWAYS_VALUE));
//}

void Power::powerBtnGroup_changed_slot(int index){
    if (index == BALANCE){

    }
    else if (index == SAVING)
    {

    }
    else{

    }
    refreshUI();
}

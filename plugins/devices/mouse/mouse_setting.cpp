#include <QWidget>
#include <QDebug>
#include <QDir>
#include <QTimer>
#include "mouse_setting.h"
#include "kylin-mouse-interface.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QX11Info>
#define COMMON_SIZE 18
#define MAXIMIZATION_SIZE 48
#define MOUSE_DBLCLCK_ICON_SIZE  67
#define TOUCHPAD_SCHEMA "org.ukui.peripherals-touchpad"
MouseControl::MouseControl(){
    
    ui = new Ui::MouseWidget;
    pluginWidget = new  QWidget;
    pluginName = tr("mouse");
    pluginType = DEVICES;
     timer = new QTimer(this);
    InitDBusMouse();
    ui->setupUi(pluginWidget);
    init_setting();
    setup_dialog();
}

MouseControl::~MouseControl(){
    DeInitDBusMouse();
    delete timer;
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

void MouseControl::draw_cursor()
{

}

void MouseControl::setup_dialog(){
    connect(ui->radioButton,SIGNAL(clicked()),this,SLOT(on_radioButton_clicked()));
    connect(ui->radioButton_6,SIGNAL(clicked()),this,SLOT(on_radioButton_6_clicked()));
    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(on_comboBox_currentIndexChanged(int)));
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(on_pushButton_clicked()));
    connect(ui->pushButton_2,SIGNAL(clicked()),this,SLOT(on_pushButton_2_clicked()));
    connect(ui->checkBox,&QCheckBox::stateChanged,this,&MouseControl::on_checkBox_stateChanged);
    connect(ui->horizontalSlider_1,&QSlider::valueChanged,this,&MouseControl::on_horizontalSlider_1_valueChanged);
    connect(ui->horizontalSlider_2,&QSlider::valueChanged,this,&MouseControl::on_horizontalSlider_2_valueChanged);
    connect(ui->checkBox_1,&QCheckBox::stateChanged,this,&MouseControl::on_checkBox_1_stateChanged);
    connect(ui->checkBox_2,&QCheckBox::stateChanged,this,&MouseControl::on_checkBox_2_stateChanged);
    connect(ui->checkBox_3,&QCheckBox::stateChanged,this,&MouseControl::on_checkBox_3_stateChanged);
     connect(ui->checkBox_6,&QCheckBox::stateChanged,this,&MouseControl::on_checkBox_6_stateChanged);
    connect(ui->radioButton_2,SIGNAL(clicked()),this,SLOT(on_radioButton_2_clicked()));
    connect(ui->radioButton_3,SIGNAL(clicked()),this,SLOT(on_radioButton_3_clicked()));
    connect(ui->radioButton_4,SIGNAL(clicked()),this,SLOT(on_radioButton_4_clicked()));
    connect(ui->radioButton_5,SIGNAL(clicked()),this,SLOT(on_radioButton_5_clicked()));
    connect(ui->radioButton_7,SIGNAL(clicked()),this,SLOT(on_radioButton_7_clicked()));
    connect(ui->radioButton_8,SIGNAL(clicked()),this,SLOT(on_radioButton_8_clicked()));
    connect(ui->radioButton_9,SIGNAL(clicked()),this,SLOT(on_radioButton_9_clicked()));

}
void MouseControl::init_setting(){

    bool  left_hand = kylin_hardware_mouse_get_lefthanded();
    if(left_hand)
        ui->radioButton->setChecked(true);
    else
        ui->radioButton_6->setChecked(true);

    bool local_pointer = kylin_hardware_mouse_get_locatepointer();
    if(local_pointer)
        ui->checkBox->setChecked(true);
    else
        ui->checkBox->setChecked(false);

    int accel_numerator, accel_denominator, threshold;  //目的是当底下两个值为-1时，从底层获取到默认的具体值
    double mouse_acceleration;                          //当前系统指针加速值，-1为系统默认
    int mouse_threshold;                                //当前系统指针灵敏度，-1为系统默认
    XGetPointerControl(QX11Info::display(), &accel_numerator, &accel_denominator, &threshold);

    mouse_acceleration = kylin_hardware_mouse_get_motionacceleration();
    mouse_threshold =  kylin_hardware_mouse_get_motionthreshold();

    if(mouse_acceleration == -1.0)
        kylin_hardware_mouse_set_motionacceleration((double)(accel_numerator/accel_denominator));
    if(mouse_threshold == -1)
        kylin_hardware_mouse_set_motionthreshold(mouse_threshold);

    ui->horizontalSlider_1->setValue(kylin_hardware_mouse_get_motionacceleration()*100);
    ui->horizontalSlider_2->setValue(kylin_hardware_mouse_get_motionthreshold()*100);


    QString  current;

    current  = kylin_hardware_mouse_get_cursortheme();

    QStringList cursorThemes;
    cursorThemes = get_cursor_themes();
    int index = -1,comboxid = -1;

    for(int i = 0; i< cursorThemes.size();++i)
    {
        ui->comboBox->addItem(cursorThemes.at(i));
        comboxid++;
        if(current.compare(cursorThemes.at(i))==0)
            index = i;
    }
    ui->comboBox->addItem("default");
    if(-1 == index)
        ui->comboBox->setCurrentIndex(comboxid+1);
    else
        ui->comboBox->setCurrentIndex(index);
    
    touchpad_settings = g_settings_new(TOUCHPAD_SCHEMA);

    bool enable_touchpad = g_settings_get_boolean(touchpad_settings,"touchpad-enabled");
    ui->checkBox_1->setChecked(enable_touchpad);

    bool disable_typing = g_settings_get_boolean(touchpad_settings,"disable-while-typing");
    ui->checkBox_2->setChecked(disable_typing);
    bool tap_click = g_settings_get_boolean(touchpad_settings,"tap-to-click");
    ui->checkBox_3->setChecked(tap_click);

    if(g_settings_get_boolean(touchpad_settings,"vertical-edge-scrolling") == true)
        ui->radioButton_2->setChecked(true);
    else if(g_settings_get_boolean(touchpad_settings,"horizontal-edge-scrolling") == true)
        ui->radioButton_3->setChecked(true);
    else if(g_settings_get_boolean(touchpad_settings,"vertical-two-finger-scrolling") == true)
        ui->radioButton_4->setChecked(true);
    else if(g_settings_get_boolean(touchpad_settings,"horizontal-two-finger-scrolling") == true)
        ui->radioButton_5->setChecked(true);
    else
        ui->checkBox_6->setChecked(true);

    int currentsize;

    currentsize = kylin_hardware_mouse_get_cursorsize();
    if(currentsize == 24)
        ui->radioButton_7->setChecked(true);
    else if (currentsize <=30) {
        ui->radioButton_8->setChecked(true);
    }
    else {
        ui->radioButton_9->setChecked(true);
    }


}

QStringList MouseControl::get_cursor_themes()
{
    QString  dirName = "/usr/share/icons/";
    QString  subdirPath;
    QDir dir(dirName);

    QStringList  themes;

    if (dir.exists()){

         Q_FOREACH(QFileInfo subdir ,dir.entryInfoList()){
            if(subdir.isDir()){
                QDir subdirDir(subdir.filePath());
                 Q_FOREACH (QFileInfo subfile, subdirDir.entryInfoList()) {
                    if(subfile.isDir()==true&&subfile.fileName()=="cursors")
                    {
                        themes.append(subdir.fileName());
                        break;
                    }
                }
            }
        }
    }

    return themes;
}

bool MouseControl::find_synaptics()
{
    XDeviceInfo *device_info;
    int n_devices;
    bool retval;

    if (supports_xinput_devices() == false)
    {
        return true;
    }
    retval = false;
    device_info = XListInputDevices (QX11Info::display(), &n_devices);
    if (device_info == NULL)
        return false;

    for (int i = 0; i < n_devices; i++) {
        XDevice *device;

        device = device_is_touchpad (&device_info[i]);
        if (device != NULL) {
            retval = true;
            break;
        }
    }
    if (device_info != NULL)
        XFreeDeviceList (device_info);

    return retval;
}

bool MouseControl::device_has_property (XDevice    *device,
                                 const char *property_name)
{
    Atom realtype, prop;
    int realformat;
    unsigned long nitems, bytes_after;
    unsigned char *data;

    prop = XInternAtom (QX11Info::display(), property_name, True);
    if (!prop)
        return false;
    if ((XGetDeviceProperty (QX11Info::display(), device, prop, 0, 1, False,
                             XA_INTEGER, &realtype, &realformat, &nitems,
                             &bytes_after, &data) == Success) && (realtype != None))
    {
        XFree (data);
        return true;
    }
    return false;
}

XDevice* MouseControl::device_is_touchpad (XDeviceInfo *deviceinfo)
{
    XDevice *device;
    if (deviceinfo->type != XInternAtom (QX11Info::display(), XI_TOUCHPAD, true))
        return NULL;
    device = XOpenDevice (QX11Info::display(), deviceinfo->id);
    if(device == NULL)
    {
        qDebug()<<"device== null";
        return NULL;
    }

    if (device_has_property (device, "libinput Tapping Enabled") ||
            device_has_property (device, "Synaptics Off")) {
        return device;
    }
    XCloseDevice (QX11Info::display(), device);
    return NULL;
}

bool MouseControl::supports_xinput_devices (void)
{
    int op_code, event, error;

    return XQueryExtension (QX11Info::display(),
                            "XInputExtension",
                            &op_code,
                            &event,
                            &error);

}


void MouseControl::on_radioButton_6_clicked()
{

    kylin_hardware_mouse_set_lefthanded(false);
}


void MouseControl::on_radioButton_clicked()
{

    kylin_hardware_mouse_set_lefthanded(true);
}

void MouseControl::on_comboBox_currentIndexChanged(int index)
{

    if(ui->comboBox->currentText().compare("default")==0)
    {
        kylin_hardware_mouse_set_cursortheme("");
        return ;
    }
    const char*  ch;
    QByteArray ba = ui->comboBox->currentText().toLatin1(); // must
    ch=ba.data();
    kylin_hardware_mouse_set_cursortheme(ch);
}


void MouseControl::on_checkBox_stateChanged(int arg1)
{
    kylin_hardware_mouse_set_locatepointer(arg1);
}

void MouseControl::on_horizontalSlider_1_valueChanged(int value)
{
    kylin_hardware_mouse_set_motionacceleration((double)value/ui->horizontalSlider_1->maximum()*10);
}

void MouseControl::on_horizontalSlider_2_valueChanged(int value)
{
    kylin_hardware_mouse_set_motionthreshold(10*value/ui->horizontalSlider_2->maximum());
}

void MouseControl::on_checkBox_1_stateChanged(int arg1)
{   
    g_settings_set_boolean(touchpad_settings,"touchpad-enabled",arg1);
}

void MouseControl::on_checkBox_2_stateChanged(int arg1)
{
    g_settings_set_boolean(touchpad_settings,"disable-while-typing",arg1);
}

void MouseControl::on_checkBox_3_stateChanged(int arg1)
{
    g_settings_set_boolean(touchpad_settings,"tap-to-click",arg1);
}

void MouseControl::on_checkBox_6_stateChanged(int arg1)
{
    g_settings_set_boolean(touchpad_settings,"vertical-edge-scrolling",arg1);
    g_settings_set_boolean(touchpad_settings,"horizontal-edge-scrolling",arg1);
    g_settings_set_boolean(touchpad_settings,"vertical-two-finger-scrolling",arg1);
    g_settings_set_boolean(touchpad_settings,"horizontal-two-finger-scrolling",arg1);
}

void MouseControl::on_radioButton_2_clicked()
{
    g_settings_set_boolean(touchpad_settings,"vertical-edge-scrolling",true);
}

void MouseControl::on_radioButton_3_clicked()
{
    g_settings_set_boolean(touchpad_settings,"horizontal-edge-scrolling",true);
}

void MouseControl::on_radioButton_4_clicked()
{
    g_settings_set_boolean(touchpad_settings,"vertical-two-finger-scrolling",true);
}

void MouseControl::on_radioButton_5_clicked()
{
    g_settings_set_boolean(touchpad_settings,"horizontal-two-finger-scrolling",true);
}

void MouseControl::on_pushButton_clicked()
{
    ui->StackedWidget->setCurrentIndex(1);

}

void MouseControl::on_pushButton_2_clicked()
{
    ui->StackedWidget->setCurrentIndex(2);
}

void MouseControl::on_radioButton_7_clicked()
{
    kylin_hardware_mouse_set_cursorsize(24);
}

void MouseControl::on_radioButton_8_clicked()
{
    kylin_hardware_mouse_set_cursorsize(30);
}

void MouseControl::on_radioButton_9_clicked()
{
    kylin_hardware_mouse_set_cursorsize(36);
}

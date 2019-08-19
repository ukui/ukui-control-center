#ifndef MOUSE_SETTING_H
#define MOUSE_SETTING_H
#include <QObject>
#include <QtPlugin>
#include <QTimer>
#include "mainui/interface.h"
#include "ui_mouse_setting.h"
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>
#include <gio/gio.h>

enum{MOUSE_DBLCLCK_OFF,MOUSE_DBLCLCK_MAYBE,MOUSE_DBLCLCK_ON};

namespace Ui {
class MouseWidget;
}

class MouseControl : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface" )
    Q_INTERFACES(CommonInterface)

public:
    MouseControl();
    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    ~MouseControl();
private Q_SLOTS:
    void on_radioButton_6_clicked();

    void on_radioButton_clicked();

    void on_comboBox_currentIndexChanged(int index);

    void on_radioButton_7_clicked();

    void on_radioButton_8_clicked();

    void on_checkBox_stateChanged(int arg1);

    void on_horizontalSlider_1_valueChanged(int value);

    void on_horizontalSlider_2_valueChanged(int value);

    void on_horizontalSlider_3_valueChanged(int value);

    void on_checkBox_1_stateChanged(int arg1);

    void on_checkBox_2_stateChanged(int arg1);

    void on_checkBox_3_stateChanged(int arg1);

    void on_checkBox_6_stateChanged(int arg1);

    void on_radioButton_2_clicked();

    void on_radioButton_3_clicked();

    void on_radioButton_4_clicked();

    void on_radioButton_5_clicked();

    void reset_click_state_to_off();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_radioButton_9_clicked();

private:
    Ui::MouseWidget * ui;
    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;
    int double_click_state = MOUSE_DBLCLCK_OFF;
    QTimer *timer;
    GSettings *touchpad_settings;

    bool find_synaptics();
    bool device_has_property (XDevice    *device,
                              const char *property_name);
    XDevice* device_is_touchpad (XDeviceInfo *deviceinfo);
    bool supports_xinput_devices (void);
    void init_setting();
    void setup_dialog();
    void draw_cursor();
    QStringList get_cursor_themes();

};

#endif

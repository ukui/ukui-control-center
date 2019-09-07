#ifndef MOUSECONTROL_H
#define MOUSECONTROL_H

#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QWidget>
#include <QAbstractButton>
#include <QDir>

#include <QGSettings/QGSettings>
#include <QX11Info>

#include "kylin-mouse-interface.h"

#include "../../component/switchbutton.h"
#include "../../component/customwidget.h"

#define CURSORS_THEMES_PATH "/usr/share/icons/"

#define TOUCHPAD_SCHEMA "org.ukui.peripherals-touchpad"
#define ACTIVE_TOUCHPAD_KEY "touchpad-enabled"
#define DISABLE_WHILE_TYPING_KEY "disable-while-typing"
#define TOUCHPAD_CLICK_KEY "tap-to-click"
#define V_EDGE_KEY "vertical-edge-scrolling"
#define H_EDGE_KEY "horizontal-edge-scrolling"
#define V_FINGER_KEY "vertical-two-finger-scrolling"
#define H_FINGER_KEY "horizontal-two-finger-scrolling"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

namespace Ui {
class MouseControl;
}

class MouseControl : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    MouseControl();
    ~MouseControl() Q_DECL_OVERRIDE;

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget *get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    void component_init();
    void status_init();

    bool find_synaptics();
    bool _supports_xinput_devices();

private:
    Ui::MouseControl *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

    SwitchButton * activeBtn;

    QGSettings * tpsettings;

    QStringList _get_cursors_themes();
    void _refresh_touchpad_widget_status();
    void _refresh_rolling_btn_status();


private slots:
    void mouseprimarykey_changed_slot(QAbstractButton * button,bool status);
    void cursor_themes_changed_slot(QString text);
    void speed_value_changed_slot(int value);
    void sensitivity_value_changed_slot(int value);
    void show_pointer_position_slot(bool status);

    void active_touchpad_changed_slot(bool status);
    void disable_while_typing_clicked_slot(bool status);
    void touchpad_click_clicked_slot(bool status);
    void rolling_enable_clicked_slot(bool status);
    void rolling_kind_changed_slot(QAbstractButton * basebtn, bool status);

    void cursor_size_changed_slot();
};

#endif // MOUSECONTROL_H

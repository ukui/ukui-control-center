#ifndef DISPLAYSET_H
#define DISPLAYSET_H

#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QGraphicsScene>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#define MATE_DESKTOP_USE_UNSTABLE_API
#include <mate-rr.h>
#include <mate-rr-config.h>
#include <mate-rr-labeler.h>
#include <X11/Xlib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <time.h>
}

#define POWER_MANAGER_SCHEMA "org.ukui.power-manager"
#define BRIGHTNESS_AC_KEY "brightness-ac"

namespace Ui {
class DisplayWindow;
}

typedef struct _Monitor{
    MateRRScreen *screen;
    MateRRConfig  * current_configuration;
    MateRRLabeler * labeler;
    MateRROutputInfo * current_output;

    guint32 ApplyBtnClickTimeStamp;

    DBusGConnection * connection;
    DBusGProxy * proxy;
    DBusGProxyCall * proxy_call;

    GtkWidget * window;
}Monitor;

class DisplaySet : public QObject, CommonInterface{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    DisplaySet();
    ~DisplaySet();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;

    void monitor_init();
    void component_init();
    void status_init();

    void rebuild_monitor_combo();
    void rebuild_resolution_combo();
    void rebuild_rotation_combo();
    void rebuild_refresh_combo();

    MateRROutputInfo * _get_output_for_window (MateRRConfig *configuration, GdkWindow *window);
    MateRRMode ** _get_current_modes();
    void _realign_output_after_resolution_changed(MateRROutputInfo * output_changed, int oldwidth, int oldheight);
    gboolean _sanitize_save_configuration();
    void _ensure_current_configuration_is_saved();
    void _begin_version2_apply_configuration();
    static void _apply_configuration_callback();

private:
    Ui::DisplayWindow * ui;
    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

    GSettings  * brightnessgsettings;

    Monitor monitor;

private slots:
    void brightness_value_changed_slot(int value);
    void refresh_changed_slot(int index);
    void rotation_changed_slot(int index);
    void resolution_changed_slot(int index);
    void monitor_changed_slot(int index);

    void apply_btn_clicked_slot();
};

#endif // DISPLAYSET_H

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2016 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "mouse-setting.h"
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <dbus/dbus-glib.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/Xcursor/Xcursor.h>
#include <mate-settings-daemon/mate-settings-client.h>
#define WID(s) (GTK_WIDGET(gtk_builder_get_object(mousedata.builder, s)))

#define MOUSE_SCHEMA "org.mate.peripherals-mouse"
#define DOUBLE_CLICK_KEY "double-click"
#define TOUCHPAD_SCHEMA "org.mate.peripherals-touchpad"

#define MOUSE_DBLCLCK_OFF "mouse-dblclck-off"
#define MOUSE_DBLCLCK_MAYBE "mouse-dblclck-maybe"
#define MOUSE_DBLCLCK_ON "mouse-dblclck-on"
#define MOUSE_DBLCLCK_ICON_SIZE  67

#define STOCK_ICON_PATH "/usr/share/ukui-control-center/pixmaps/"
typedef struct _MouseData MouseData;
typedef struct {
    char * stock_id;
    char * name;
}StockIcon;

typedef struct {
    GtkWidget * image;
    gint * timeout_id;
}Time_out_data;

static StockIcon items[] = {
    {MOUSE_DBLCLCK_MAYBE, "double-click-maybe.png"},
    {MOUSE_DBLCLCK_OFF, "double-click-off.png"},
    {MOUSE_DBLCLCK_ON, "double-click-on.png"}
};

struct _MouseData {
    // gtk object
    GtkBuilder * builder;
    GtkWidget * right_hand;
    GtkWidget * left_hand;
    GtkWidget * location_pointer_position;
    GtkWidget * movtion_speed;
    GtkWidget * senstive_speed;
    GtkWidget * double_click_speed;
    GtkWidget * test_image;

    //synaptics
    GtkWidget * prefs_widget;
    GtkWidget * disable_typing_toggle;
    GtkWidget * tap_to_click_toggle;
    GtkWidget * horiz_scroll_toggle;

    GtkWidget * scroll_disabled_radio;
    GtkWidget * scroll_edge_radio;
    GtkWidget * scroll_twofinger_radio;
};
MouseData mousedata;

enum {
    DOUBLE_CLICK_TEST_OFF,
    DOUBLE_CLICK_TEST_MAYBE,
    DOUBLE_CLICK_TEST_ON
};

static gint double_click_state = DOUBLE_CLICK_TEST_OFF;
static GtkIconSize stock_icon_size =0;
static GSettings * mouse_settings=NULL;
static GSettings * touchpad_settings=NULL;


static void
scrollmethod_clicked_event (GtkWidget *widget,
                	    gpointer user_data)
{
    GtkToggleButton *disabled = GTK_TOGGLE_BUTTON (mousedata.scroll_disabled_radio);

    gtk_widget_set_sensitive (mousedata.horiz_scroll_toggle,
                  !gtk_toggle_button_get_active (disabled));

    GSList *radio_group;
    int new_scroll_method;
    int old_scroll_method = g_settings_get_int (touchpad_settings, "scroll-method");

    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget)))
        return;

    radio_group = g_slist_copy (gtk_radio_button_get_group
        (GTK_RADIO_BUTTON (mousedata.scroll_disabled_radio)));
    radio_group = g_slist_reverse (radio_group);
    new_scroll_method = g_slist_index (radio_group, widget);
    g_slist_free (radio_group);

    if (new_scroll_method != old_scroll_method)
        g_settings_set_int (touchpad_settings, "scroll-method", new_scroll_method);
}

static void
synaptics_check_capabilities ()
{
    //have xinput
    int numdevices, i;
    XDeviceInfo *devicelist;
    Atom realtype, prop;
    int realformat;
    unsigned long nitems, bytes_after;
    unsigned char *data;

    prop = XInternAtom (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), "Synaptics Capabilities", True);
    if (!prop)
        return;

    devicelist = XListInputDevices (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), &numdevices);
    for (i = 0; i < numdevices; i++) 
    {
        if (devicelist[i].use != IsXExtensionPointer)
            continue;

        gdk_error_trap_push ();
        XDevice *device = XOpenDevice (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()),
                           devicelist[i].id);
        if (gdk_error_trap_pop ())
            continue;

        gdk_error_trap_push ();
        if ((XGetDeviceProperty (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), device, prop, 0, 2, False,
                     XA_INTEGER, &realtype, &realformat, &nitems,
                     &bytes_after, &data) == Success) && (realtype != None)) 
	{
            /* Property data is booleans for has_left, has_middle,
             * has_right, has_double, has_triple */
            if (!data[0]) 
	    {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WID ("tap_to_click_toggle")), TRUE);
                gtk_widget_set_sensitive (WID ("tap_to_click_toggle"), FALSE);
            }

            if (!data[3])
                gtk_widget_set_sensitive (WID ("scroll_twofinger_radio"), FALSE);

            XFree (data);
        }
        gdk_error_trap_pop ();

        XCloseDevice (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), device);
    }
    XFreeDeviceList (devicelist);
}

static void
scrollmethod_gsettings_changed_event (GSettings *setting,gchar * key)
{
    int scroll_method = g_settings_get_int (touchpad_settings, "scroll-method");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mousedata.scroll_disabled_radio),
                scroll_method == 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mousedata.scroll_edge_radio),
                scroll_method == 1);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mousedata.scroll_twofinger_radio),
                scroll_method == 2);
    gtk_widget_set_sensitive (mousedata.horiz_scroll_toggle,
                 scroll_method != 0);
}

static
gboolean find_synaptics()
{
    gboolean ret = FALSE;
    int numdevices, i;
    //we need to install xinput
    XDeviceInfo *devicelist;
    Atom realtype, prop;
    int realformat;
    unsigned long nitems, bytes_after;
    unsigned char *data;
    XExtensionVersion *version;

    /* Input device properties require version 1.5 or higher */
    version = XGetExtensionVersion (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), "XInputExtension");
    if (!version || (version == (XExtensionVersion*) NoSuchExtension))
        return False;

    if (!version->present || (version->major_version * 1000 + version->minor_version) < 1005) 
    {
        XFree (version);
        return False;
    }

    prop = XInternAtom (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), "Synaptics Off", True);
    if (!prop)
        return False;

    devicelist = XListInputDevices (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), &numdevices);
    for (i = 0; i < numdevices; i++) 
    {
        if (devicelist[i].use != IsXExtensionPointer)
            continue;

        gdk_error_trap_push();
        XDevice *device = XOpenDevice (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()),
                           devicelist[i].id);
        if (gdk_error_trap_pop ())
            continue;

        gdk_error_trap_push ();
        if ((XGetDeviceProperty (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), device, prop, 0, 1, False,
                     XA_INTEGER, &realtype, &realformat, &nitems,
                     &bytes_after, &data) == Success) && (realtype != None)) 
	{
            XFree (data);
            ret = TRUE;
        }
        gdk_error_trap_pop ();

        XCloseDevice (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), device);

        if (ret)
            break;
    }

    XFree (version);
    XFreeDeviceList (devicelist);
    return ret;
}

GtkIconSize  mouse_capplet_dblclck_icon_get_size(void)
{
    return stock_icon_size;
}

static void register_stock_icons(GtkIconFactory * factory)
{
    gint i;
    GtkIconSource * source;

    source = gtk_icon_source_new();
    for (i=0; i<G_N_ELEMENTS(items); ++i)
    {
        GtkIconSet * icon_set;
        char * filename;
        filename = g_build_filename(STOCK_ICON_PATH, items[i].name, NULL);
        if (!filename)
	{
            g_warning("Unable to build filename:%s\n",items[i].name);
            icon_set = gtk_icon_factory_lookup_default(GTK_STOCK_MISSING_IMAGE);
            gtk_icon_factory_add(factory, items[i].stock_id, icon_set);
            continue;
        }
        gtk_icon_source_set_filename(source, filename);
        g_free(filename);
        icon_set = gtk_icon_set_new();
        gtk_icon_set_add_source(icon_set, source);
        gtk_icon_factory_add(factory, items[i].stock_id, icon_set);
        gtk_icon_set_unref(icon_set);
    }
    gtk_icon_source_free(source);
}

static void init_stock_icon()
{
    GtkIconFactory *factory;
    static gboolean init_stock= FALSE;
    if (init_stock)
        return;
    init_stock = TRUE;

    factory = gtk_icon_factory_new();
    gtk_icon_factory_add_default(factory);
    register_stock_icons(factory);
    stock_icon_size = gtk_icon_size_register("mouse-dblclck-icon",
                                       MOUSE_DBLCLCK_ICON_SIZE,
                                       MOUSE_DBLCLCK_ICON_SIZE);
    g_object_unref(factory);

}

static gboolean reset_click_state_to_off(Time_out_data *data)
{
    double_click_state = DOUBLE_CLICK_TEST_OFF;
    //set image to double-click-off.png
//    g_warning("reset_click_state");
//    g_warning("image size=%d\n",gtk_image_get_pixel_size(GTK_IMAGE(data->image)));
    gtk_image_set_from_stock(GTK_IMAGE(data->image), MOUSE_DBLCLCK_OFF, mouse_capplet_dblclck_icon_get_size());
    *data->timeout_id = 0;
    return FALSE;
}

static gboolean event_box_button_press(GtkWidget * widget, GdkEventButton *event, gpointer user_data)
{
    GtkWidget * image;
    gint double_click_time;
    static Time_out_data data;
    static gint test_maybe_timeout_id=0;
    static gint test_on_timeout_id=0;
    static gint double_click_timestamp=0;
    if (event->type != GDK_BUTTON_PRESS)
    {
        return FALSE;
    }
    double_click_time = g_settings_get_int(mouse_settings, DOUBLE_CLICK_KEY);
    image = g_object_get_data(G_OBJECT(widget), "image");
    if (test_maybe_timeout_id != 0)
    {
        g_source_remove(test_maybe_timeout_id);
    }
    if (test_on_timeout_id !=0)
    {
        g_source_remove(test_on_timeout_id);
    }

    switch (double_click_state) 
    {
    	case DOUBLE_CLICK_TEST_OFF:
            double_click_state = DOUBLE_CLICK_TEST_MAYBE;
            data.image = image;
            data.timeout_id =&test_maybe_timeout_id;
            test_maybe_timeout_id = g_timeout_add(double_click_time, (GSourceFunc)reset_click_state_to_off,&data);
            break;
    	case DOUBLE_CLICK_TEST_MAYBE:
            if (event->time -double_click_timestamp <double_click_time)
	    {
            	double_click_state = DOUBLE_CLICK_TEST_ON;
            	data.image = image;
            	data.timeout_id =&test_on_timeout_id;
                test_on_timeout_id = g_timeout_add(2500, (GSourceFunc)reset_click_state_to_off, &data);
            }
            break;
    	case DOUBLE_CLICK_TEST_ON:
            double_click_state = DOUBLE_CLICK_TEST_OFF;
            break;
    	default:
            g_warning("Error state occured\n");
            break;
    }
    double_click_timestamp = event->time;
    switch(double_click_state) 
    {
        case DOUBLE_CLICK_TEST_OFF:
            gtk_image_set_from_stock(GTK_IMAGE(image), MOUSE_DBLCLCK_OFF, mouse_capplet_dblclck_icon_get_size());
            break;
        case DOUBLE_CLICK_TEST_MAYBE:
            gtk_image_set_from_stock(GTK_IMAGE(image), MOUSE_DBLCLCK_MAYBE, mouse_capplet_dblclck_icon_get_size());
            break;
        case DOUBLE_CLICK_TEST_ON:
            gtk_image_set_from_stock(GTK_IMAGE(image), MOUSE_DBLCLCK_ON, mouse_capplet_dblclck_icon_get_size());
            break;
    }
    return TRUE;
}


static gboolean active_settings_daemon()
{
    DBusGConnection *connection = NULL;
    DBusGProxy * proxy = NULL;
    GError * error =NULL;

    connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if (connection ==NULL)
    {
        g_warning("connect error:%s\n",error->message);
        g_error_free(error);
        return FALSE;
    }
    proxy  = dbus_g_proxy_new_for_name(connection,
                                       "org.mate.SettingsDaemon",
                                       "/org/mate/SettingsDaemon",
                                       "org.mate.SettingsDaemon");
    if (proxy ==NULL)
    {
        g_warning("get proxy error\n");
        return FALSE;
    }
    if (!org_mate_SettingsDaemon_awake(proxy, &error))
    {
        g_warning("SettingsDaemon awake error\n");
        g_error_free(error);
        return FALSE;
    }
    return TRUE;
}

static void radio_button_toggle(GtkWidget * widget)
{
    gboolean left_handed;
    left_handed = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    g_settings_set_boolean(mouse_settings, "left-handed",left_handed);
}

static void radio_button_release(GtkWidget * widget, GdkEventButton * event, gpointer user_data)
{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
}

static void setup_dialog()
{
    GtkRadioButton *radio;

    radio = GTK_RADIO_BUTTON(mousedata.left_hand);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), g_settings_get_boolean(mouse_settings, "left-handed"));

    g_signal_connect(mousedata.left_hand, "button_release_event", G_CALLBACK(radio_button_release), NULL);
    g_signal_connect(mousedata.right_hand, "button_release_event", G_CALLBACK(radio_button_release), NULL);
    g_signal_connect(mousedata.left_hand, "toggled", G_CALLBACK(radio_button_toggle),NULL);

    g_settings_bind(mouse_settings, "locate-pointer", mousedata.location_pointer_position,
                                                            "active", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(mouse_settings, "motion-acceleration", gtk_range_get_adjustment(GTK_RANGE(mousedata.movtion_speed)),
                                                            "value" , G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(mouse_settings, "motion-threshold", gtk_range_get_adjustment(GTK_RANGE(mousedata.senstive_speed)),
                                                            "value", G_SETTINGS_BIND_DEFAULT);
    g_settings_bind(mouse_settings, DOUBLE_CLICK_KEY, gtk_range_get_adjustment(GTK_RANGE(mousedata.double_click_speed)),
                                                            "value", G_SETTINGS_BIND_DEFAULT);

    gtk_image_set_from_stock(GTK_IMAGE(mousedata.test_image), MOUSE_DBLCLCK_OFF, mouse_capplet_dblclck_icon_get_size() );
    g_object_set_data(G_OBJECT(WID("double_click_eventbox")), "image", mousedata.test_image);
    g_signal_connect(WID("double_click_eventbox"), "button_press_event",G_CALLBACK(event_box_button_press), NULL );

    //judge if we need to show prefs_widget(1)---find_synaptics()
    if (find_synaptics() == FALSE)
    {
        gtk_notebook_remove_page(GTK_NOTEBOOK(mousedata.prefs_widget), 1);
    }
    else 
    {
        g_settings_bind (touchpad_settings, "disable-while-typing",
            mousedata.disable_typing_toggle, "active",
            G_SETTINGS_BIND_DEFAULT);
        g_settings_bind (touchpad_settings, "tap-to-click",
            mousedata.tap_to_click_toggle, "active",
            G_SETTINGS_BIND_DEFAULT);
        g_settings_bind (touchpad_settings, "horiz-scroll-enabled",
            mousedata.horiz_scroll_toggle, "active",
            G_SETTINGS_BIND_DEFAULT);
        scrollmethod_gsettings_changed_event(touchpad_settings, "scroll-method");

        radio = GTK_RADIO_BUTTON (mousedata.scroll_disabled_radio);
        GSList *radio_group = gtk_radio_button_get_group (radio);
        GSList *item = NULL;

        synaptics_check_capabilities ();
        for (item = radio_group; item != NULL; item = item->next) 
	{
            g_signal_connect (G_OBJECT (item->data), "clicked",
                  G_CALLBACK(scrollmethod_clicked_event),
                  NULL);
        }
        g_signal_connect (touchpad_settings,
            "changed::scroll-method",
            G_CALLBACK(scrollmethod_gsettings_changed_event),
            NULL);
    }
}

static void init_setting()
{
    GtkSizeGroup * size_group;

    size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(size_group, WID("movtion_label"));
    gtk_size_group_add_widget(size_group, WID("senstive_label"));
    gtk_size_group_add_widget(size_group, WID("double_click_label"));

    size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(size_group, WID("movtion_slow"));
    gtk_size_group_add_widget(size_group, WID("senstive_low"));
    gtk_size_group_add_widget(size_group, WID("double_click_slow"));

    size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    gtk_size_group_add_widget(size_group, WID("movtion_fast"));
    gtk_size_group_add_widget(size_group, WID("senstive_high"));
    gtk_size_group_add_widget(size_group, WID("double_click_fast"));
}

static void create_dialog(GtkBuilder * builder)
{
    mousedata.builder = builder;
    mousedata.right_hand = WID("right_hand");
    mousedata.left_hand = WID("left_hand");
    mousedata.location_pointer_position = WID("locate_pointer_position");
    mousedata.movtion_speed = WID("movtion_speed");
    mousedata.senstive_speed = WID("senstive_speed");
    mousedata.double_click_speed = WID("double_click_speed");
    mousedata.test_image = WID("test_image");

    mousedata.prefs_widget = WID("prefs_widget");
    mousedata.disable_typing_toggle = WID("disable_typing_toggle");
    mousedata.horiz_scroll_toggle = WID("horiz_scroll_toggle");
    mousedata.tap_to_click_toggle = WID("tap_to_click_toggle");

    mousedata.scroll_disabled_radio = WID("scroll_disabled_radio");
    mousedata.scroll_edge_radio = WID("scroll_edge_radio");
    mousedata.scroll_twofinger_radio = WID("scroll_twofinger_radio");
}

void add_mouse_app(GtkBuilder * builder)
{
    g_warning("add_mouse_app");
    active_settings_daemon();
    mouse_settings = g_settings_new(MOUSE_SCHEMA);
    touchpad_settings = g_settings_new(TOUCHPAD_SCHEMA);
    create_dialog(builder);
    init_setting();
    init_stock_icon();
    setup_dialog();
}

void destory_mouse_app()
{
    g_object_unref(mouse_settings);
    g_object_unref(touchpad_settings);
}

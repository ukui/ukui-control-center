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
#include "mainwindow.h"
#include "spy-time.h"
//#include "../panels/users/users-account.h"
#include "default-app.h"
#include "ukui-keyboard-properties.h"
#include "appearance-main.h"
#include "mouse-setting.h"
#include "network-proxy.h"
#include "gsp-main.h"
#include "xrandr-capplet.h"
#include <glib/gi18n.h>

void app_set_theme(const gchar *theme_path)
{
    static GtkCssProvider *provider = NULL;
    GFile *file;
    GdkScreen *screen;
    screen = gdk_screen_get_default();
    if(theme_path!=NULL)
    {
        file = g_file_new_for_path(theme_path);
        if(file!=NULL)
        {
            if(provider==NULL)
                provider = gtk_css_provider_new();

            gtk_css_provider_load_from_file(provider, file, NULL);
            gtk_style_context_add_provider_for_screen(screen,
                                                      GTK_STYLE_PROVIDER(provider),
                                                      GTK_STYLE_PROVIDER_PRIORITY_USER);
            gtk_style_context_reset_widgets(screen);
        }
    }
    else
    {
        if(provider!=NULL)
        {
            gtk_style_context_remove_provider_for_screen(screen,
                                                         GTK_STYLE_PROVIDER(provider));
            g_object_unref(provider);
            provider = NULL;
        }
        gtk_style_context_reset_widgets(screen);
    }
}

gboolean on_all_quit(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    GtkBuilder * builder = user_data;
    time_data_destory();
//  users_data_destory();
    default_app_destory();
    destory_keyboard_app(builder);
    destory_appearance_app();
    destory_mouse_app();
    destory_network_app();
//  destory_ukui_system_info_app();
//  bug::g_object_unref: assertion 'G_IS_OBJECT(object)' failed
    if (builder){
        g_object_unref(builder);
    }
    g_application_quit(G_APPLICATION(app));
    g_object_unref(app);   
//  destory all panels and shell
    return TRUE;
}

//在页面跳转时，将侧边栏对应的viewport设置为insensitive，其余设置为sensitive
void set_sidebar_sensitive()
{
    gtk_widget_set_sensitive(GTK_WIDGET(vp_theme), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(vp_start), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(vp_app), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(vp_count), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(vp_net), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(vp_power), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(vp_display), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(vp_key), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(vp_mouse), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(vp_sound), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(vp_time), TRUE);
}

//各个页面之间的切换
void show_next_page(GtkWidget *widget, gpointer userdata)
{
    ButtonData *data = (ButtonData *)userdata;
    set_sidebar_sensitive ();
    gtk_widget_set_sensitive(GTK_WIDGET(data->vp), FALSE);
    gtk_notebook_set_current_page(notebook1, (gint)1);
    gtk_notebook_set_current_page(notebook2, data->page);
    gtk_window_set_title(window, data->title);
}

//从主窗口跳转到对应页面
void switch_page(GtkWidget *widget, gpointer userdata)
{
    ButtonData *data = (ButtonData *)userdata;
    set_sidebar_sensitive ();
    gtk_widget_set_sensitive(GTK_WIDGET(data->vp), FALSE);
    gtk_widget_show(GTK_WIDGET(data->vp));
    gtk_notebook_set_current_page(notebook2, data->page);
    gtk_window_set_title(window, data->title);
}

void show_mainpage(GtkWidget *widget, gpointer userdata)
{
    gtk_notebook_set_current_page(notebook1, (gint)0);
    gtk_window_set_title(window, _("Control Center"));
}

static void direct_call_program(GtkWidget * widget, gchar * program_name){
//    in fact, we don't want to use function system(), because this function is dangerous,
//    always we can use it in shell but not in code, function popen() is safeer.but when we use popen in gtk program
//    the screen may not always refresh
    gchar * proname;
    int systemback;
    proname= g_strdup_printf("%s &",program_name);
    systemback = system(proname);
    if (systemback <0)
    {
        g_warning("Call program %s has occured error\n", program_name);
        return;
    }
    else if (systemback ==0)
    {
        g_warning("Call program %s successfully, but no pid return\n", program_name);
    }
    else if (systemback == 127)
    {
        g_warning("Call program %s has occured error\n", program_name);
    }
    else
    {
        g_warning("Successful\n");
    }
}

static void set_button_image(GtkButton * button, gchar * icon_name)
{
    GtkWidget * image;
    const gchar filename[80];
    g_sprintf(filename,"%s/%s.png","/usr/share/ukui-control-center/icons",icon_name);
    //if (!g_file_test(filename, G_FILE_TEST_EXIST))
    image = gtk_image_new_from_file(filename);
    gtk_button_set_image(GTK_BUTTON(button), image);
}

void init_signals()
{
    notebook1 = (GtkNotebook *)GTK_WIDGET(gtk_builder_get_object(builder, "notebook1"));
    notebook2 = (GtkNotebook *)GTK_WIDGET(gtk_builder_get_object(builder, "notebook2"));
    vp_time = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_time"));
    bt_time = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_time"));
    data_time.vp = vp_time;
    data_time.page = 0;
    data_time.title = _("Time and Dates");
    g_signal_connect(G_OBJECT(bt_time), "clicked", G_CALLBACK(switch_page), (gpointer)&data_time);
    button1 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button1"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(button1), _(" Change the time and date of this computer "));
    set_button_image(button1, "日期和时间");
    g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_time);

    vp_count = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_count"));
    bt_count = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_count"));
    data_count.vp = vp_count;
    data_count.page = 1;
    data_count.title = _("User Account");
    g_signal_connect(G_OBJECT(bt_count), "clicked", G_CALLBACK(switch_page), (gpointer)&data_count);
    button2 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button2"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(button2), _(" Change the computer user account \n settings and password"));
    set_button_image(button2, "用户账号");
    g_signal_connect(G_OBJECT(button2), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_count);

    vp_theme = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_theme"));
    bt_theme = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_theme"));
    data_theme.vp = vp_theme;
    data_theme.page = 2;
    data_theme.title = _("Personalization");
    g_signal_connect(G_OBJECT(bt_theme), "clicked", G_CALLBACK(switch_page), (gpointer)&data_theme);
    button3 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button3"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(button3), _(" Change the computer desktop background \n and related theme"));
    set_button_image(button3, "个性化");
    g_signal_connect(G_OBJECT(button3), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_theme);

    vp_app = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_app"));
    bt_app = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_app"));
    data_app.vp = vp_app;
    data_app.page = 3;
    data_app.title = _("Default Aplication");
    g_signal_connect(G_OBJECT(bt_app), "clicked", G_CALLBACK(switch_page), (gpointer)&data_app);
    button4 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button4"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(button4), _(" Change the default application on this computer "));
    set_button_image(button4, "默认应用程序");
    g_signal_connect(G_OBJECT(button4), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_app);

    vp_network = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_network"));
    //隐藏网络连接的选项按钮
    bt_network = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_connection"));
    //gtk_widget_hide(GTK_WIDGET(vp_network));
    gtk_widget_hide(GTK_WIDGET(bt_network));
    /*data_network.vp = vp_network;
    data_network.page = 4;
    data_network.title =_( "网络连接");
    g_signal_connect(G_OBJECT(bt_network), "clicked", G_CALLBACK(switch_page), (gpointer)&data_network);*/
    button5 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button5"));
    //g_signal_connect(G_OBJECT(button5), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_network);
    gtk_widget_set_tooltip_text(GTK_WIDGET(button5), _(" Change the network  on this computer "));
    set_button_image(button5, "网络连接");
    g_signal_connect(G_OBJECT(button5),"clicked", G_CALLBACK(direct_call_program),"nm-connection-editor");

    vp_key = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_key"));
    bt_key = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_key"));
    data_key.vp = vp_key;
    data_key.page = 5;
    data_key.title = _("Keyboard");
    g_signal_connect(G_OBJECT(bt_key), "clicked", G_CALLBACK(switch_page), (gpointer)&data_key);
    button6 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button6"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(button6), _(" Change the keyboard settings on this computer "));
    set_button_image(button6, "键盘");
    g_signal_connect(G_OBJECT(button6), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_key);

    vp_mouse = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_mouse"));
    bt_mouse = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_mouse"));
    data_mouse.vp = vp_mouse;
    data_mouse.page = 6;
    data_mouse.title =_("Mouse");
    g_signal_connect(G_OBJECT(bt_mouse), "clicked", G_CALLBACK(switch_page), (gpointer)&data_mouse);
    button7 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button7"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(button7), _(" Change the mouse settings on this computer  "));
    set_button_image(button7, "鼠标");
    g_signal_connect(G_OBJECT(button7), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_mouse);

    vp_printer = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_printer"));
    bt_printer = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_printer"));
    gtk_widget_hide(GTK_WIDGET(vp_printer));
    gtk_widget_hide(GTK_WIDGET(bt_printer));
    data_printer.vp = vp_printer;
    data_printer.page = 7;
    data_printer.title = "打印机";
    button8 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button8"));
    set_button_image(button8, "打印机");
    gtk_widget_set_tooltip_text(GTK_WIDGET(button8), _(" Configure the connection to the computer printer "));
//    g_signal_connect(G_OBJECT(button8), "clicked", G_CALLBACK(direct_call_program),"system-config-printer");
//    g_signal_connect(G_OBJECT(button8), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_printer);
    g_signal_connect(G_OBJECT(button8), "clicked", G_CALLBACK(direct_call_program),"system-config-printer");

    vp_sound = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_sound"));
    bt_sound = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_sound"));
    data_sound.vp = vp_sound;
    data_sound.page = 8;
    data_sound.title = _("Sound");
    g_signal_connect(G_OBJECT(bt_sound), "clicked", G_CALLBACK(switch_page), (gpointer)&data_sound);
    button9 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button9"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(button9), _(" Change the sound settings on this computer "));
    set_button_image(button9, "声音");
    g_signal_connect(G_OBJECT(button9), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_sound);

    vp_net = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_net"));
    bt_net = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_net"));
    data_net.vp = vp_net;
    data_net.page = 9;
    data_net.title = _("Network Agent");
    g_signal_connect(G_OBJECT(bt_net), "clicked", G_CALLBACK(switch_page), (gpointer)&data_net);
    button10 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button10"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(button10), _(" Change the network agent on this computer "));
    set_button_image(button10, "网络代理");
    g_signal_connect(G_OBJECT(button10), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_net);

    vp_start = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_start"));
    bt_start = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_start"));
    data_start.vp = vp_start;
    data_start.page = 10;
    data_start.title = _("Autostart");
    g_signal_connect(G_OBJECT(bt_start), "clicked", G_CALLBACK(switch_page), (gpointer)&data_start);
    button11 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button11"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(button11), _(" Change the autostart program on this computer "));
    set_button_image(button11, "开机启动");
    g_signal_connect(G_OBJECT(button11), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_start);

    vp_display = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_display"));
    bt_display = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_display"));
    data_display.vp = vp_display;
    data_display.page = 11;
    data_display.title = _("Monitor");
    button12 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button12"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(button12), _(" Change the Monitor settings on this computer "));
    set_button_image(button12, "显示器");
    if(dispalyIsOk)
    {
        g_signal_connect(G_OBJECT(button12), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_display);
        g_signal_connect(G_OBJECT(bt_display), "clicked", G_CALLBACK(switch_page), (gpointer)&data_display);
    }
    vp_power = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_power"));
    bt_power = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_power"));
    data_power.vp = vp_power;
    data_power.page = 12;
    data_power.title = _("Power Management");
    g_signal_connect(G_OBJECT(bt_power), "clicked", G_CALLBACK(switch_page), (gpointer)&data_power);
    button13 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button13"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(button13), _(" Change the power management settings on this computer "));
    set_button_image(button13, "电源管理");
    g_signal_connect(G_OBJECT(button13), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_power);

    bt_startpage = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_startpage"));
    g_signal_connect(G_OBJECT(bt_startpage), "clicked", G_CALLBACK(show_mainpage), NULL);
}

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
#include "mate-keyboard-properties.h"
#include "appearance-main.h"
#include "mouse-setting.h"
#include "network-proxy.h"
#include "gsp-main.h"
#include "xrandr-capplet.h"
#include <glib/gi18n.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#define SIGNAL_TIME_PAGE 3
#define SIGNAL_SOUND_PAGE 2
#define SIGNAL_APPEARANCE_PAGE 0
#define SIGNAL_POWER_PAGE 1
#define SIGNAL_USER_PAGE 4
#define SIGNAL_NO_ARGUMENT 6
#define SIGNAL_KEYBOARD_PAGE 5
gboolean switch_to_timeanddata_page=FALSE;
gboolean switch_to_appearance_page=FALSE;
gboolean switch_to_sound_page=FALSE;
gboolean switch_to_power_page=FALSE;
gboolean switch_to_user_page=FALSE;
gboolean switch_to_keyboard_page=FALSE;
GApplication * unique_app;
GdkColor vp_blue = {0, 0xc0c0, 0xdddd, 0xf9f9};

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

void ukui_init(int *argc, char **argv[])
{
	GOptionContext *context;
	GError *error = NULL;
	gtk_init(argc, argv);
	static GOptionEntry cap_options[] = {
	{
    "time",'t',0,G_OPTION_ARG_NONE, &switch_to_timeanddata_page,"Go to Time and Date settings  page",NULL
	},
	{
    "appearance",'a',0,G_OPTION_ARG_NONE, &switch_to_appearance_page,"Go to Personalization settings page ", NULL
	},
	{
    "power",'p',0,G_OPTION_ARG_NONE, &switch_to_power_page, "Go to Power Management page", NULL
	},
	{
    "sound",'s',0,G_OPTION_ARG_NONE, &switch_to_sound_page, "Go to Sound settings page", NULL
	},
	{
    "user",'u',0,G_OPTION_ARG_NONE, &switch_to_user_page, "Go to User Account page",NULL
	},
	{
    "keyboard", 'k',0,G_OPTION_ARG_NONE, &switch_to_keyboard_page, "Go to Keyboard settings page",NULL
	},
	{
	NULL
	}
	};
    context = g_option_context_new(_("-Kylin Control Center"));
	g_option_context_add_main_entries(context, cap_options,NULL);
	if (context)
	{
		g_option_context_add_group(context, gtk_get_option_group(TRUE));
		if (!g_option_context_parse(context, argc, argv, &error))
		{
			g_warning("Context parse %s\n",error->message);
			exit(EXIT_FAILURE);
		}
	}
}

gboolean on_all_quit(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GtkBuilder * builder = user_data;
	time_data_destory();
	//users_data_destory();
	default_app_destory();
	destory_keyboard_app(builder);
	destory_appearance_app();
	destory_mouse_app();
	destory_network_app();
//	destory_ukui_system_info_app();
	//bug::g_object_unref: assertion 'G_IS_OBJECT(object)' failed
	if (builder)
	{
		g_object_unref(builder);
	}
	g_object_unref(unique_app);
	gtk_main_quit();
	//destory all panels and shell
	return TRUE;
}

void modify_vp_color()
{
	GdkColor white = {0 ,0xf2f2, 0xf2f2, 0xf2f2};
	gtk_widget_modify_bg(GTK_WIDGET(data_theme.vp), GTK_STATE_NORMAL, &white);
	gtk_widget_modify_bg(GTK_WIDGET(data_start.vp), GTK_STATE_NORMAL, &white);
	gtk_widget_modify_bg(GTK_WIDGET(data_app.vp), GTK_STATE_NORMAL, &white);
	gtk_widget_modify_bg(GTK_WIDGET(data_count.vp), GTK_STATE_NORMAL, &white);
	gtk_widget_modify_bg(GTK_WIDGET(data_net.vp), GTK_STATE_NORMAL, &white);
	gtk_widget_modify_bg(GTK_WIDGET(data_power.vp), GTK_STATE_NORMAL, &white);
	gtk_widget_modify_bg(GTK_WIDGET(data_display.vp), GTK_STATE_NORMAL, &white);
	gtk_widget_modify_bg(GTK_WIDGET(data_key.vp), GTK_STATE_NORMAL, &white);
	gtk_widget_modify_bg(GTK_WIDGET(data_mouse.vp), GTK_STATE_NORMAL, &white);
	gtk_widget_modify_bg(GTK_WIDGET(data_sound.vp), GTK_STATE_NORMAL, &white);
	gtk_widget_modify_bg(GTK_WIDGET(data_time.vp), GTK_STATE_NORMAL, &white);
}

void modify_font_color(GtkButton *button, char *textcolor)
{
	GtkWidget *label;
	label = gtk_bin_get_child((GtkBin*)button);
	GdkColor bt_color;
	gdk_color_parse(textcolor, &bt_color);
	gtk_widget_modify_fg(GTK_WIDGET(label), GTK_STATE_NORMAL, &bt_color);
}

void init_button_text_color(char *textcolor)
{
	GtkLabel *label6 = (GtkLabel *)GTK_WIDGET(gtk_builder_get_object(builder, "label6"));
	GdkColor color;
	gdk_color_parse(textcolor, &color);
	gtk_widget_modify_fg(GTK_WIDGET(label6), GTK_STATE_NORMAL, &color);

	modify_font_color(bt_time, textcolor);
	modify_font_color(bt_count, textcolor);
	modify_font_color(bt_theme, textcolor);
	modify_font_color(bt_app, textcolor);
	//modify_font_color(bt_network, textcolor);
	modify_font_color(bt_key, textcolor);
	modify_font_color(bt_mouse, textcolor);
	modify_font_color(bt_printer, textcolor);
	modify_font_color(bt_sound, textcolor);
	modify_font_color(bt_net, textcolor);
	modify_font_color(bt_start, textcolor);
	modify_font_color(bt_display, textcolor);
	modify_font_color(bt_power, textcolor);
	modify_font_color(bt_system, textcolor);
}

void show_next_page(GtkWidget *widget, gpointer userdata)
{
	ButtonData *data = (ButtonData *)userdata;
	modify_vp_color ();
	gtk_widget_modify_bg(GTK_WIDGET(data->vp), GTK_STATE_NORMAL, &vp_blue);
	gtk_notebook_set_current_page(notebook1, (gint)1);
	gtk_notebook_set_current_page(notebook2, data->page);
	gtk_window_set_title(window, data->title);
}

void switch_page(GtkWidget *widget, gpointer userdata)
{
	ButtonData *data = (ButtonData *)userdata;
	modify_vp_color ();
	gtk_widget_modify_bg(GTK_WIDGET(data->vp), GTK_STATE_NORMAL, &vp_blue);
	//gtk_widget_show(GTK_WIDGET(data->vp));
	gtk_notebook_set_current_page(notebook2, data->page);
	gtk_window_set_title(window, data->title);
}

void show_mainpage(GtkWidget *widget, gpointer userdata)
{
        gtk_notebook_set_current_page(notebook1, (gint)0);
        gtk_window_set_title(window, _("Control Center"));
}

static void direct_call_program(GtkWidget * widget, gchar * program_name){
	//in fact, we don't want to use function system(), because this function is dangerous,
	//always we can use it in shell but not in code, function popen() is safeer.but when we use popen in gtk program
	//the screen may not always refresh
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

// deal with UniqueCommand
static int unique_signal_deal(GApplication *application,GApplicationCommandLine *cmdline)
{
  gchar **argv;
  gint argc;

  argv = g_application_command_line_get_arguments (cmdline, &argc);

  int command = atoi(argv[0]);

  g_strfreev (argv);
    if(!gtk_window_is_active(window))
    {
        GdkWindow * gdkwindow = gtk_widget_get_window(GTK_WIDGET(window));
     //   set_active_window(GDK_DRAWABLE_XID(gdkwindow));
        g_printf("------gtk_window_is_no active---------\n");
    }
    int main_window_x,main_window_y;
    gtk_window_get_position (GTK_WINDOW (window),&main_window_x, &main_window_y);
    gtk_window_move (GTK_WINDOW (window),-1000, -1000);
  //gtk_window_set_screen(GTK_WINDOW(window), unique_message_data_get_screen(message));
    gtk_widget_show_all(GTK_WIDGET(window));
    gtk_window_move (GTK_WINDOW (window),main_window_x, main_window_y);
    gtk_widget_grab_focus(window);
    gtk_window_present(window);

    switch (command)
    {
        case SIGNAL_APPEARANCE_PAGE:
            gtk_notebook_set_current_page(notebook1,1);
            gtk_notebook_set_current_page(notebook2,2);
			modify_vp_color ();
			gtk_widget_modify_bg(GTK_WIDGET(data_theme.vp), GTK_STATE_NORMAL, &vp_blue);
//			gtk_window_present_with_time(window,gdk_x11_get_server_time(gtk_widget_get_window(window)));
            gtk_window_set_title(window, data_theme.title);
            break;
        case SIGNAL_POWER_PAGE:
            gtk_notebook_set_current_page(notebook1,1);
            gtk_notebook_set_current_page(notebook2,12);
			modify_vp_color ();
			gtk_widget_modify_bg(GTK_WIDGET(data_power.vp), GTK_STATE_NORMAL, &vp_blue);
//			gtk_window_present_with_time(window,gdk_x11_get_server_time(gtk_widget_get_window(window)));
            gtk_window_set_title(window, data_power.title);
            break;
        case SIGNAL_SOUND_PAGE:
            gtk_notebook_set_current_page(notebook1,1);
            gtk_notebook_set_current_page(notebook2,8);
			modify_vp_color ();
			gtk_widget_modify_bg(GTK_WIDGET(data_sound.vp), GTK_STATE_NORMAL, &vp_blue);
//			gtk_window_present_with_time(window,gdk_x11_get_server_time(gtk_widget_get_window(window)));
            gtk_window_set_title(window, data_sound.title);
            break;
        case SIGNAL_TIME_PAGE:
            gtk_notebook_set_current_page(notebook1,1);
            gtk_notebook_set_current_page(notebook2,0);
			modify_vp_color ();
			gtk_widget_modify_bg(GTK_WIDGET(data_time.vp), GTK_STATE_NORMAL, &vp_blue);
//			gtk_window_present_with_time(window,gdk_x11_get_server_time(gtk_widget_get_window(window)));
            gtk_window_set_title(window, data_time.title);
            break;
        case SIGNAL_USER_PAGE:
            gtk_notebook_set_current_page(notebook1,1);
            gtk_notebook_set_current_page(notebook2,1);
			modify_vp_color ();
			gtk_widget_modify_bg(GTK_WIDGET(data_count.vp), GTK_STATE_NORMAL, &vp_blue);
//			gtk_window_present_with_time(window,gdk_x11_get_server_time(gtk_widget_get_window(window)));
            gtk_window_set_title(window, data_count.title);
            break;
        case SIGNAL_KEYBOARD_PAGE:
            gtk_notebook_set_current_page(notebook1,1);
            gtk_notebook_set_current_page(notebook2,5);
			modify_vp_color ();
			gtk_widget_modify_bg(GTK_WIDGET(data_key.vp), GTK_STATE_NORMAL, &vp_blue);
//			gtk_window_present_with_time(window,gdk_x11_get_server_time(gtk_widget_get_window(window)));
            gtk_window_set_title(window, data_key.title);
            break;
        case SIGNAL_NO_ARGUMENT:
//			gtk_window_present_with_time(window,gdk_x11_get_server_time(gtk_widget_get_window(window)));
            break;
        default:

            break;
    }
	return 0;
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
    gtk_widget_set_tooltip_text(button1, _(" Change the time and date of this computer "));
    set_button_image(button1, "日期和时间");
	g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_time);

	vp_count = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_count"));
	bt_count = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_count"));
	data_count.vp = vp_count;
	data_count.page = 1;
    data_count.title = _("User Account");
	g_signal_connect(G_OBJECT(bt_count), "clicked", G_CALLBACK(switch_page), (gpointer)&data_count);
	button2 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button2"));
    gtk_widget_set_tooltip_text(button2, _(" Change the computer user account \n settings and password"));
    set_button_image(button2, "用户账号");
	g_signal_connect(G_OBJECT(button2), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_count);

	vp_theme = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_theme"));
	bt_theme = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_theme"));
	data_theme.vp = vp_theme;
	data_theme.page = 2;
    data_theme.title = _("Personalization");
	g_signal_connect(G_OBJECT(bt_theme), "clicked", G_CALLBACK(switch_page), (gpointer)&data_theme);
	button3 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button3"));
    gtk_widget_set_tooltip_text(button3, _(" Change the computer desktop background \n and related theme"));
    set_button_image(button3, "个性化");
	g_signal_connect(G_OBJECT(button3), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_theme);

	vp_app = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_app"));
	bt_app = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_app"));
	data_app.vp = vp_app;
	data_app.page = 3;
    data_app.title = _("Default Aplication");
	g_signal_connect(G_OBJECT(bt_app), "clicked", G_CALLBACK(switch_page), (gpointer)&data_app);
	button4 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button4"));
    gtk_widget_set_tooltip_text(button4, _(" Change the default application on this computer "));
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
    gtk_widget_set_tooltip_text(button5, _(" Change the network  on this computer "));
    set_button_image(button5, "网络连接");
	g_signal_connect(G_OBJECT(button5),"clicked", G_CALLBACK(direct_call_program),"nm-connection-editor");

	vp_key = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_key"));
	bt_key = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_key"));
	data_key.vp = vp_key;
	data_key.page = 5;
    data_key.title = _("Keyboard");
	g_signal_connect(G_OBJECT(bt_key), "clicked", G_CALLBACK(switch_page), (gpointer)&data_key);
	button6 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button6"));
    gtk_widget_set_tooltip_text(button6, _(" Change the keyboard settings on this computer "));
    set_button_image(button6, "键盘");
	g_signal_connect(G_OBJECT(button6), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_key);

	vp_mouse = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_mouse"));
	bt_mouse = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_mouse"));
	data_mouse.vp = vp_mouse;
	data_mouse.page = 6;
    data_mouse.title =_("Mouse");
	g_signal_connect(G_OBJECT(bt_mouse), "clicked", G_CALLBACK(switch_page), (gpointer)&data_mouse);
	button7 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button7"));
    gtk_widget_set_tooltip_text(button7, _(" Change the mouse settings on this computer  "));
    set_button_image(button7, "鼠标");
	g_signal_connect(G_OBJECT(button7), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_mouse);

	vp_printer = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_printer"));
	bt_printer = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_printer"));
	gtk_widget_hide(GTK_WIDGET(vp_printer));
	gtk_widget_hide(GTK_WIDGET(bt_printer));
	data_printer.vp = vp_printer;
	data_printer.page = 7;
	data_printer.title = "打印机";
	g_signal_connect(G_OBJECT(bt_printer), "clicked", G_CALLBACK(switch_page), (gpointer)&data_printer);
	button8 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button8"));
	set_button_image(button8, "打印机");
    gtk_widget_set_tooltip_text(button8, _(" Configure the connection to the computer printer "));
	g_signal_connect(G_OBJECT(button8), "clicked", G_CALLBACK(direct_call_program),"system-config-printer");
	//g_signal_connect(G_OBJECT(button8), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_printer);
	g_signal_connect(G_OBJECT(button8), "clicked", G_CALLBACK(direct_call_program),"system-config-printer");

	vp_sound = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_sound"));
	bt_sound = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_sound"));
	data_sound.vp = vp_sound;
	data_sound.page = 8;
    data_sound.title = _("Sound");
	g_signal_connect(G_OBJECT(bt_sound), "clicked", G_CALLBACK(switch_page), (gpointer)&data_sound);
	button9 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button9"));
    gtk_widget_set_tooltip_text(button9, _(" Change the sound settings on this computer "));
    set_button_image(button9, "声音");
	g_signal_connect(G_OBJECT(button9), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_sound);
	//g_signal_connect(G_OBJECT(button9), "clicked", G_CALLBACK(direct_call_program), "mate-volume-control");

	vp_net = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_net"));
	bt_net = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_net"));
	data_net.vp = vp_net;
	data_net.page = 9;
    data_net.title = _("Network Agent");
	g_signal_connect(G_OBJECT(bt_net), "clicked", G_CALLBACK(switch_page), (gpointer)&data_net);
	button10 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button10"));
    gtk_widget_set_tooltip_text(button10, _(" Change the network agent on this computer "));
    set_button_image(button10, "网络代理");
	g_signal_connect(G_OBJECT(button10), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_net);

	vp_start = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_start"));
	bt_start = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_start"));
	data_start.vp = vp_start;
	data_start.page = 10;
    data_start.title = _("Autostart");
	g_signal_connect(G_OBJECT(bt_start), "clicked", G_CALLBACK(switch_page), (gpointer)&data_start);
	button11 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button11"));
    gtk_widget_set_tooltip_text(button11, _(" Change the autostart program on this computer "));
    set_button_image(button11, "开机启动");
	g_signal_connect(G_OBJECT(button11), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_start);

	vp_display = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_display"));
	bt_display = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_display"));
	data_display.vp = vp_display;
	data_display.page = 11;
    data_display.title = _("Monitor");
	button12 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button12"));
    gtk_widget_set_tooltip_text(button12, _(" Change the Monitor settings on this computer "));
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
    gtk_widget_set_tooltip_text(button13, _(" Change the power management settings on this computer "));
    set_button_image(button13, "电源管理");
	g_signal_connect(G_OBJECT(button13), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_power);

	//vp_system = (GtkViewport *)GTK_WIDGET(gtk_builder_get_object(builder, "vp_system"));
	bt_system = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_system"));
	//data_system.vp = vp_system;
	data_system.page = 13;
    data_system.title = _("System Check");
	g_signal_connect(G_OBJECT(bt_system), "clicked", G_CALLBACK(switch_page), (gpointer)&data_system);
	button14 = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "button14"));
	gtk_widget_hide(button14);
    gtk_widget_set_tooltip_text(button14, _("check the system detection information on this computer "));
    set_button_image(button14, "系统检测");
	g_signal_connect(G_OBJECT(button14), "clicked", G_CALLBACK(show_next_page), (gpointer)&data_system);
	GtkWidget *sys_check_layout = GTK_WIDGET(gtk_builder_get_object(builder, "kylin_system_info_layout"));
	gtk_widget_hide(sys_check_layout);

	bt_startpage = (GtkButton *)GTK_WIDGET(gtk_builder_get_object(builder, "bt_startpage"));
    g_signal_connect(G_OBJECT(bt_startpage), "clicked", G_CALLBACK(show_mainpage), NULL);

	//设置按钮上字体的颜色
	init_button_text_color("#074ca6");
	//设置背景色......
	GtkWidget *viewport = GTK_WIDGET(gtk_builder_get_object(builder, "viewport11"));
	GdkColor grey = {0,0xf2f2,0xf2f2,0xf2f2};
	gtk_widget_modify_bg(GTK_WIDGET(viewport), GTK_STATE_NORMAL, &grey);

	//unique app, we need to think about when someone call the other ukuicc
	//first line: if someone call ukuicc first, we can show users the page which they want to see, this depends on the argument.
	//second line: if someone call ukuicc second, we need to prevent the second exe's startup, and then show the exe, do as first line descriptions.
        int status;

        unique_app = g_application_new ("org.gtk.TestApplication",
                           G_APPLICATION_HANDLES_COMMAND_LINE);
        g_signal_connect (unique_app, "command-line", G_CALLBACK (unique_signal_deal), NULL);

//	g_signal_connect(unique_app, "message-received",G_CALLBACK(unique_signal_deal), NULL);
//	unique_app_add_command(unique_app, "time_page", SIGNAL_TIME_PAGE);
//	unique_app_add_command(unique_app, "appearance_page", SIGNAL_APPEARANCE_PAGE);
//	unique_app_add_command(unique_app, "power_page", SIGNAL_POWER_PAGE);
//	unique_app_add_command(unique_app, "sound_page", SIGNAL_SOUND_PAGE);
//	unique_app_add_command(unique_app, "user_page", SIGNAL_USER_PAGE);
//	unique_app_add_command(unique_app, "keyboard_page",SIGNAL_KEYBOARD_PAGE);
//	unique_app_add_command(unique_app, "no_argument",SIGNAL_NO_ARGUMENT);

/*        if (g_application_get_is_remote(unique_app)) {
		if (switch_to_appearance_page)
		{
                        char *my_argv[] = {"0"};
                        status = g_application_run (unique_app, 1, my_argv);     
			switch_to_appearance_page = FALSE;
		}
		else if (switch_to_power_page)
		{
                        char *my_argv[] = {"1"};
                        status = g_application_run (unique_app, 1, my_argv);
			switch_to_power_page = FALSE;
		}	
		else if (switch_to_sound_page)
		{
                        char *my_argv[] = {"2"};
                        status = g_application_run (unique_app, 1, my_argv);
			switch_to_sound_page = FALSE;
		}
		else if (switch_to_timeanddata_page)
		{
                        char *my_argv[] = {"3"};
                        status = g_application_run (unique_app, 1, my_argv);
			switch_to_timeanddata_page = FALSE;
		}
		else if (switch_to_user_page)
		{
                        char *my_argv[] = {"4"};
                        status = g_application_run (unique_app, 1, my_argv);
			switch_to_user_page = FALSE;
		}
		else if (switch_to_keyboard_page)
		{
                        char *my_argv[] = {"5"};
                        status = g_application_run (unique_app, 1, my_argv);
			switch_to_keyboard_page =FALSE;
		}
		g_object_unref(unique_app);
                exit(0);
        }
*/

	//Fixme: this part can write simple, but i don't know how can i do that
	if (switch_to_appearance_page)
	{
		gtk_notebook_set_current_page(notebook1, 1);
		gtk_notebook_set_current_page(notebook2, 2);
		modify_vp_color ();
		gtk_widget_modify_bg(GTK_WIDGET(data_theme.vp), GTK_STATE_NORMAL, &vp_blue);
        gtk_window_set_title(GTK_WINDOW(window), data_theme.title);
        gtk_widget_grab_focus(GTK_WIDGET(window));
		switch_to_appearance_page = FALSE;
		return;
	}
	if (switch_to_power_page)
	{
		gtk_notebook_set_current_page(notebook1, 1);
		gtk_notebook_set_current_page(notebook2, 12);
		modify_vp_color ();
		gtk_widget_modify_bg(GTK_WIDGET(data_power.vp), GTK_STATE_NORMAL, &vp_blue);
        gtk_window_set_title(GTK_WINDOW(window), data_power.title);
        gtk_widget_grab_focus(GTK_WIDGET(window));
		switch_to_power_page = FALSE;
		return;
	}
	if (switch_to_sound_page)
	{
		gtk_notebook_set_current_page(notebook1, 1);
		gtk_notebook_set_current_page(notebook2, 8);
		modify_vp_color ();
		gtk_widget_modify_bg(GTK_WIDGET(data_sound.vp), GTK_STATE_NORMAL, &vp_blue);
        gtk_window_set_title(GTK_WINDOW(window), data_sound.title);
        gtk_widget_grab_focus(GTK_WIDGET(window));
		switch_to_sound_page = FALSE;
		return;
	}
	if (switch_to_timeanddata_page)
	{
		gtk_notebook_set_current_page(notebook1, 1);
		gtk_notebook_set_current_page(notebook2, 0);
		modify_vp_color ();
		gtk_widget_modify_bg(GTK_WIDGET(data_time.vp), GTK_STATE_NORMAL, &vp_blue);
        gtk_window_set_title(GTK_WINDOW(window), data_time.title);
        gtk_widget_grab_focus(GTK_WIDGET(window));
		switch_to_timeanddata_page = FALSE;
		return;
	}
	if (switch_to_user_page)
	{
		gtk_notebook_set_current_page(notebook1, 1);
		gtk_notebook_set_current_page(notebook2, 1);
		modify_vp_color ();
		gtk_widget_modify_bg(GTK_WIDGET(data_count.vp), GTK_STATE_NORMAL, &vp_blue);
        gtk_window_set_title(GTK_WINDOW(window), data_count.title);
        gtk_widget_grab_focus(GTK_WIDGET(window));
		switch_to_user_page = FALSE;
		return;
	}
	if (switch_to_keyboard_page)
	{
                gtk_notebook_set_current_page(notebook1, 1);
		gtk_notebook_set_current_page(notebook2, 5);
		modify_vp_color ();
		gtk_widget_modify_bg(GTK_WIDGET(data_key.vp), GTK_STATE_NORMAL, &vp_blue);
        gtk_window_set_title(GTK_WINDOW(window), data_key.title);
        gtk_widget_grab_focus(GTK_WIDGET(window));
		switch_to_keyboard_page = FALSE;
		return;
	}
}

void init_mainwindow()
{

	GError *err = NULL;
	app_set_theme(UIDIR "/ukcc.css");
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, UIDIR "/shell.ui", &err);
	//gtk_builder_add_from_file(builder, "shell.ui", &err);
	if (err)
	{
		g_warning ("Could not load user interface file: %s", err->message);
		g_error_free (err);
		return;
	}
}

void set_active_window(Window xid)
{
    Display *d = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(d);

    Atom _NET_ACTIVE_WINDOW = XInternAtom(d, "_NET_ACTIVE_WINDOW", False);
    XEvent xev;
    xev.xclient.type = ClientMessage;
    xev.xclient.send_event = True;
    xev.xclient.window = xid;
    xev.xclient.message_type = _NET_ACTIVE_WINDOW;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 2;
    XSendEvent(d, root, False, StructureNotifyMask | SubstructureNotifyMask, &xev);

    XCloseDisplay(d);
}

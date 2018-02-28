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
#include "user-accounts.h"
#include "default-app.h"
#include "ukui-keyboard-properties.h"
#include "appearance-main.h"
#include "mouse-setting.h"
#include "network-proxy.h"
#include "gsp-main.h"
#include "xrandr-capplet.h"
#include "kpm-prefs-core.h"
#include "dialog-main.h"
#include <locale.h>
#include <glib/gi18n.h>

static gboolean switch_to_timeanddata_page = FALSE;
static gboolean switch_to_appearance_page  = FALSE;
static gboolean switch_to_sound_page       = FALSE;
static gboolean switch_to_power_page       = FALSE;
static gboolean switch_to_user_page        = FALSE;
static gboolean switch_to_keyboard_page    = FALSE;
static gboolean switch_to_display_page     = FALSE;
static gboolean show_help                  = FALSE;

const GOptionEntry cap_options[] = {
    {"help", 'h', 0, G_OPTION_ARG_NONE, &show_help, N_("Show help options"), NULL},
    {"time", 't', 0, G_OPTION_ARG_NONE, &switch_to_timeanddata_page, N_("Go to Time and Date settings page"), NULL},
    {"appearance", 'a', 0, G_OPTION_ARG_NONE, &switch_to_appearance_page, N_("Go to Personalization settings page "), NULL},
    {"power", 'p', 0, G_OPTION_ARG_NONE, &switch_to_power_page, N_("Go to Power Management page"), NULL},
    {"sound", 's', 0, G_OPTION_ARG_NONE, &switch_to_sound_page, N_("Go to Sound settings page"), NULL},
    {"user", 'u', 0, G_OPTION_ARG_NONE, &switch_to_user_page, N_("Go to User Account page"), NULL},
    {"keyboard", 'k', 0, G_OPTION_ARG_NONE, &switch_to_keyboard_page, N_("Go to Keyboard settings page"), NULL},
    {"display", 'd', 0, G_OPTION_ARG_NONE, &switch_to_display_page, N_("Go to Display settings page"), NULL},
    {NULL}
};

//带参数命令行(第一个窗口或者后续打开的窗口)的页面跳转
void switch_options_page()
{
    //每次重置为FALSE是为了防止打开第二个窗口时会出现多个TRUE
    gtk_notebook_set_current_page(notebook1, 1);
    set_sidebar_sensitive();
    if(switch_to_appearance_page){
        gtk_notebook_set_current_page(notebook2, 2);
        gtk_window_set_title(GTK_WINDOW(window), data_theme.title);
        gtk_widget_set_sensitive(GTK_WIDGET(vp_app), FALSE);
        switch_to_appearance_page = FALSE;
    }else if(switch_to_timeanddata_page){
        gtk_notebook_set_current_page(notebook2, 0);
        gtk_window_set_title(GTK_WINDOW(window), data_time.title);
        gtk_widget_set_sensitive(GTK_WIDGET(vp_time), FALSE);
        switch_to_timeanddata_page = FALSE;
    }else if(switch_to_display_page){
        gtk_notebook_set_current_page(notebook2, 11);
        gtk_window_set_title(GTK_WINDOW(window), data_display.title);
        gtk_widget_set_sensitive(GTK_WIDGET(vp_display), FALSE);
        switch_to_display_page = FALSE;
    }else if(switch_to_keyboard_page){
        gtk_notebook_set_current_page(notebook2, 5);
        gtk_window_set_title(GTK_WINDOW(window), data_key.title);
        gtk_widget_set_sensitive(GTK_WIDGET(vp_key), FALSE);
        switch_to_keyboard_page = FALSE;
    }else if(switch_to_power_page){
        gtk_notebook_set_current_page(notebook2, 12);
        gtk_window_set_title(GTK_WINDOW(window), data_power.title);
        gtk_widget_set_sensitive(GTK_WIDGET(vp_power), FALSE);
        switch_to_power_page = FALSE;
    }else if(switch_to_sound_page){
        gtk_notebook_set_current_page(notebook2, 8);
        gtk_window_set_title(GTK_WINDOW(window), data_sound.title);
        gtk_widget_set_sensitive(GTK_WIDGET(vp_sound), FALSE);
        switch_to_sound_page = FALSE;
    }else if(switch_to_user_page){
        gtk_notebook_set_current_page(notebook2, 1);
        gtk_window_set_title(GTK_WINDOW(window), data_count.title);
        gtk_widget_set_sensitive(GTK_WIDGET(vp_count), FALSE);
        switch_to_user_page = FALSE;
    }else{
        //无参数时回到主界面
        gtk_notebook_set_current_page(notebook1, 0);
    }
}

//初始化窗口的部件
void init_mainwindow()
{
    add_time_and_data_app(builder);
    init_user_accounts();
    add_appearance_app(builder);
    add_default_app(builder);
    add_keyboard_app(builder);
    add_mouse_app(builder);
    add_network_app(builder);
    add_gsp_app(builder);
    if(0==add_display_app(builder))
        dispalyIsOk = FALSE;
    else
        dispalyIsOk  = TRUE;
    add_volume_control_app(builder);
    init_power();
}

//每次命令行启动时都会激活这个回调
static int
command_line (GApplication            *application,
              GApplicationCommandLine *cmdline)
{
    int argc;
    char **argv;
    GOptionContext *context;
    GError *error = NULL;

    argv = g_application_command_line_get_arguments (cmdline, &argc);

    context = g_option_context_new(_("- Page jump"));
    g_option_context_add_main_entries(context, cap_options, "ukui-control-center");
    g_option_context_set_translation_domain(context, "ukui-control-center");
    g_option_context_add_group(context, gtk_get_option_group(TRUE));
    //禁用默认的命令行help，手动添加了一个help选项，防止打开多个控制面板时前一个会退出
    g_option_context_set_help_enabled(context, FALSE);

    if (context){
        if (!g_option_context_parse(context, &argc, &argv, &error)){
            g_warning(_("%s\nRun '%s --help' to see a full list of available command line options.\n"),
                      error->message, argv[0]);
            g_error_free(error);
            g_option_context_free(context);
            return 1;
        }
    }

    //执行help选项时，显示命令行帮助
    if(show_help){
        gchar *help;
        GOptionGroup *group = NULL;
        //不显示默认的GTK+选项
        help = g_option_context_get_help(context, TRUE, group);
        g_print("%s", help);
        g_free(help);
        g_option_context_free(context);
        return 0;
    }

    GList *list;
    //打开第二个窗口时进行判断，若存在窗口，激活当前的窗口
    list = gtk_application_get_windows(GTK_APPLICATION(application));
    if(list){
        gtk_widget_grab_focus(GTK_WIDGET(window));
        gtk_window_present(GTK_WINDOW(window));
    }else{
        init_mainwindow();
        gtk_window_set_application(window, app);
        gtk_widget_show_all(GTK_WIDGET(window));//如果放到init_aignals后面会导致开始菜单弹出面板时开始菜单不隐藏
        init_signals();
        g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(on_all_quit), builder);
    }
    switch_options_page();
    g_strfreev (argv);
    return 0;
}

int main(int argc, char * argv[])
{
    setlocale(LC_ALL,"");
    bindtextdomain ("ukui-control-center", "/usr/share/locale"); //告诉gettext最终的生成的翻译文件mo的位置
    bind_textdomain_codeset("ukui-control-center","UTF-8");      //指定域消息条目(mo)中消息的字符编码
    textdomain("ukui-control-center");                           //设定翻译环境，即指定使用gettext的翻译。
    gtk_init(&argc, &argv);

    GError *err = NULL;

    builder = gtk_builder_new();
    //加载css
    app_set_theme(UIDIR "/ukcc.css");

    gtk_builder_add_from_file(builder, UIDIR "/shell.ui", &err);
    if (err){
        g_warning ("Could not load user interface file: %s", err->message);
        g_error_free (err);
    }

    window = GTK_WINDOW(gtk_builder_get_object(builder, _("window1")));
    gtk_widget_set_name(GTK_WIDGET(window), "ukuicc");

    app = gtk_application_new ("org.gtk.ukcc", G_APPLICATION_HANDLES_COMMAND_LINE);
    g_application_register(G_APPLICATION(app), NULL, NULL);
    g_signal_connect (app, "command-line", G_CALLBACK(command_line), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    return status;
}

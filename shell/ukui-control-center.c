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
#include "mate-keyboard-properties.h"
#include "appearance-main.h"
#include "mouse-setting.h"
#include "network-proxy.h"
#include "gsp-main.h"
#include "xrandr-capplet.h"
#include "kpm-prefs-core.h"
#include "dialog-main.h"
#include <locale.h>
#include <glib/gi18n.h>

int main(int argc, char * argv[])
{
    setlocale(LC_ALL,"");
    bindtextdomain ("ukui-control-center", "/usr/share/locale"); //告诉gettext最终的生成的翻译文件mo的位置
    bind_textdomain_codeset("ukui-control-center","UTF-8"); //指定域消息条目(mo)中消息的字符编码
    textdomain("ukui-control-center");//设定翻译环境，即指定使用gettext的翻译。
	ukui_init(&argc, &argv);
	init_mainwindow();
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
    //add_ukui_system_info_app(builder);
	init_power();

    window = GTK_WINDOW(gtk_builder_get_object(builder, _("window1")));
    gtk_widget_set_name(GTK_WIDGET(window), _("ukuicc"));
    gtk_widget_show_all(GTK_WIDGET(window));//如果放到init_aignals后面会导致开始菜单弹出面板时开始菜单不隐藏

    init_signals();
	//hide_viewport();
    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(on_all_quit), builder);
	gtk_main();
	return 0;
}

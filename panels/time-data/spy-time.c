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
#include "spy-time.h"
#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#define TZ_DATA_FILE "/usr/share/zoneinfo/zone_utc"
#define _GMT 1970
#define MAXYAERRANGE 100
#define USE_24_FORMAT "use-24h-format"
typedef struct _TzDB TzDB;
typedef struct _TimeDate TimeDate;
typedef struct _TzLocation TzLocation;
typedef struct _TzUTC TzUTC;

struct _TzUTC{
	GPtrArray *tz_rtc;
	gchar *tz_utc;
};

struct _TzDB {
    GPtrArray *locations;
};
struct _TimeDate {
    TzDB *tzdb;
    GtkWidget * tzcombo;
    GtkWidget * hours;
    GtkWidget * minutes;
    GtkWidget * seconds;
    GtkWidget * calendar;
    GtkWidget * viewportlayout;
	GtkWidget * timesetting_checkbutton;
    GtkWidget * td_month_add_button;
    GtkWidget * td_combo_year;
    GtkWidget * td_combo_month;
    GtkWidget * td_month_del_button;
	GtkWidget * ntp_label;

    GtkWidget * hr12_radio;
    GtkWidget * hr24_radio;
    //config
    GDBusProxy *proxy;

    //datetime
    gint year;
    gint month;
    gint day;
    gint hour;
    gint minute;
    gint second;
    gchar ** tmptime;
    //show time component increase
    guint show_timeout_clock;
    //when you change the time,it will be used
    guint apply_timeout_clock;
};
//why we do that? i need to avoid to load timeanddataapp use long time
static const char* year_tab[100]={
    "1970","1971","1972","1973","1974","1975","1976","1977","1978","1979",
    "1980","1981","1982","1983","1984","1985","1986","1987","1988","1989",
    "1990","1991","1992","1993","1994","1995","1996","1997","1998","1999",
    "2000","2001","2002","2003","2004","2005","2006","2007","2008","2009",
    "2010","2011","2012","2013","2014","2015","2016","2017","2018","2019",
    "2020","2021","2022","2023","2024","2025","2026","2027","2028","2029",
    "2030","2031","2032","2033","2034","2035","2036","2037","2038","2039",
    "2040","2041","2042","2043","2044","2045","2046","2047","2048","2049",
    "2050","2051","2052","2053","2054","2055","2056","2057","2058","2059",
    "2060","2061","2062","2063","2064","2065","2066","2067","2068","2069",
};

struct _TzLocation {
    gchar * country;
    gchar * zone;
};
static GSettings *time_format;

TimeDate timedata;
TzDB * init_timedb();
gchar * tz_data_file_get();
void sort_locations_by_country(GPtrArray *locations);
int compare_country_names(const void *a, const void *b);
void init_dbus_proxy();
void set_time_value();
void init_time_config();
void freeze_clock();
void thaw_clock();
void on_editable_changed();
void stop_update_show_time_clock();
void start_update_show_time_clock();
gboolean on_apply_timeout(GtkWidget *widget,gpointer user_data);
gboolean update_show_time(gpointer user_date);
void init_calendar_time(gchar *month,gchar *day, gchar *year);
void on_time_wrapped();
void on_day_selected(GtkWidget *widget, gpointer user_data);
void on_timezone_changed(GtkWidget *widget, gpointer user_data);
void on_timesetting_changed(GtkWidget *widget, gpointer user_data);
void init_calendar();
void dbus_set_answered(GObject *object, GAsyncResult *res, gpointer command);
gchar * get_current_time();
void init_time_setting();

enum{
    Jan,
    Feb,
    Mar,
    Apr,
    May,
    Jun,
    Jul,
    Aug,
    Sep,
    Oct,
    Nov,
    Dec
};

void time_data_destory(){
    g_clear_object(&timedata.proxy);
    g_strfreev(timedata.tmptime);
    g_object_unref(time_format);
}

void init_time_setting(){
    GVariant *value;
    value = g_dbus_proxy_get_cached_property(timedata.proxy, "CanNTP");
    if (value !=NULL) {
        if (g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN)){
			gtk_widget_set_sensitive(timedata.timesetting_checkbutton, TRUE);
        }
    }else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(timedata.timesetting_checkbutton), 0);
		gtk_widget_set_sensitive(timedata.timesetting_checkbutton, FALSE);
    }
    g_variant_unref(value);

    value = g_dbus_proxy_get_cached_property(timedata.proxy, "NTP");
    if (value !=NULL) {
        if (g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN)){
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(timedata.timesetting_checkbutton), g_variant_get_boolean(value));
        }
		//因为ntpd和systemd的网络时间同步会有冲突，所以安装了ntp的话，禁止使用控制面板设置网络时间同步
		if(!access("/usr/sbin/ntpd", F_OK)){
			gtk_widget_set_sensitive(timedata.timesetting_checkbutton, FALSE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(timedata.timesetting_checkbutton), FALSE);
			gtk_widget_show(timedata.ntp_label);
		}
		
        g_variant_unref(value);
        return;
    }
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(timedata.timesetting_checkbutton),TRUE);
}

void on_time_wrapped(){
    init_time_config();
    init_calendar();
}

void init_calendar(){
    if(strcmp(timedata.tmptime[2],"")==0){
        init_calendar_time(timedata.tmptime[1],timedata.tmptime[3],timedata.tmptime[5]);
    }
    else{
        init_calendar_time(timedata.tmptime[1],timedata.tmptime[2],timedata.tmptime[4]);
    }
}

void freeze_clock(){
    g_signal_handlers_block_by_func(timedata.hours, on_editable_changed, NULL);
    g_signal_handlers_block_by_func(timedata.minutes, on_editable_changed, NULL);
    g_signal_handlers_block_by_func(timedata.seconds, on_editable_changed, NULL);
}

void thaw_clock(){
    g_signal_handlers_unblock_by_func(timedata.hours, on_editable_changed, NULL);
    g_signal_handlers_unblock_by_func(timedata.minutes, on_editable_changed, NULL);
    g_signal_handlers_unblock_by_func(timedata.seconds, on_editable_changed, NULL);
}

void dbus_set_answered(GObject *object, GAsyncResult *res, gpointer command){
    GError *error =NULL;
    GVariant *answers;
    answers = g_dbus_proxy_call_finish(G_DBUS_PROXY(object), res, &error);

    if(error !=NULL){
        if(!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)){
            g_warning("Could not set '%s' using timedated:%s",(gchar *)command, error->message);
        }
        g_error_free(error);
        return;
    }
    g_variant_unref(answers);
}

gboolean on_apply_timeout(GtkWidget * widget,gpointer user_data){
    GDateTime *now;
    gint64 newtime;
    guint year,month,day,hour,minute,second;
    hour = gtk_spin_button_get_value((GtkSpinButton *)timedata.hours);
    minute = gtk_spin_button_get_value((GtkSpinButton *)timedata.minutes);
    second = gtk_spin_button_get_value((GtkSpinButton *)timedata.seconds);
    gtk_calendar_get_date((GtkCalendar *)timedata.calendar, &year, &month, &day);
    now = g_date_time_new_local(year,month+1,day,hour,minute,(gdouble)second);
    newtime = g_date_time_to_unix(now);
    g_dbus_proxy_call(timedata.proxy, "SetTime", g_variant_new("(xbb)", (newtime * G_TIME_SPAN_SECOND), FALSE,TRUE),
                      G_DBUS_CALL_FLAGS_NONE,-1,NULL,dbus_set_answered, "time");
    start_update_show_time_clock();
    //return false to end g_timeout_add
    return FALSE;
}

void on_timesetting_changed(GtkWidget *widget, gpointer user_data){
	gint timesetting_check;
	timesetting_check = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(timedata.timesetting_checkbutton));
    if (timesetting_check == 0){
        g_dbus_proxy_call(timedata.proxy, "SetNTP", g_variant_new("(bb)", FALSE, TRUE),
                        G_DBUS_CALL_FLAGS_NONE, -1,NULL, dbus_set_answered, "NTP");
    }else {
        g_dbus_proxy_call(timedata.proxy, "SetNTP", g_variant_new("(bb)", TRUE, TRUE),
                        G_DBUS_CALL_FLAGS_NONE, -1,NULL, dbus_set_answered, "NTP");
    }
    if(!timesetting_check) {
        gtk_widget_set_sensitive(timedata.hours, TRUE);
        gtk_widget_set_sensitive(timedata.minutes, TRUE);
        gtk_widget_set_sensitive(timedata.seconds, TRUE);
        gtk_widget_set_sensitive(timedata.td_combo_month, TRUE);
        gtk_widget_set_sensitive(timedata.td_combo_year, TRUE);
        gtk_widget_set_sensitive(timedata.td_month_add_button, TRUE);
        gtk_widget_set_sensitive(timedata.td_month_del_button, TRUE);
        gtk_widget_set_sensitive(timedata.calendar, TRUE);
    }
    else {
        gtk_widget_set_sensitive(timedata.hours, FALSE);
        gtk_widget_set_sensitive(timedata.minutes, FALSE);
        gtk_widget_set_sensitive(timedata.seconds, FALSE);
        gtk_widget_set_sensitive(timedata.td_combo_month, FALSE);
        gtk_widget_set_sensitive(timedata.td_combo_year, FALSE);
        gtk_widget_set_sensitive(timedata.td_month_add_button, FALSE);
        gtk_widget_set_sensitive(timedata.td_month_del_button, FALSE);
        gtk_widget_set_sensitive(timedata.calendar, FALSE);
    }
}

void on_timezone_changed(GtkWidget *widget, gpointer user_data){
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar * location;
	gchar *rtc;
	TzUTC *tmp;
	GPtrArray *translate;
	translate = timedata.tzdb->locations;

    if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(widget), &iter)) {
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
        gtk_tree_model_get(model, &iter,0 ,&location, -1);
		if(!strcmp(getenv("LANG"), "en_US.UTF-8"))
			rtc = location;
		else{
			for(int i=0; i != timedata.tzdb->locations->len; ++i){
				tmp = g_ptr_array_index(timedata.tzdb->locations, i);
				if(strcmp(tmp->tz_utc, location) == 0){
					rtc = g_ptr_array_index(tmp->tz_rtc,0);
					break;
				}
			}
		}
        g_dbus_proxy_call(timedata.proxy, "SetTimezone", g_variant_new("(sb)", rtc, TRUE),
                            G_DBUS_CALL_FLAGS_NONE, -1,NULL, dbus_set_answered, "timezone");
    }
    g_free(location);
    return;
}

void on_day_selected(GtkWidget *widget, gpointer user_data){
    on_editable_changed();
}

void on_editable_changed(){
    gchar * time;
    time = get_current_time();
     if(timedata.apply_timeout_clock){
         g_source_remove(timedata.apply_timeout_clock);
         timedata.apply_timeout_clock = 0;
     }
     stop_update_show_time_clock();
     timedata.apply_timeout_clock = g_timeout_add(1000,(GSourceFunc)on_apply_timeout,NULL);
 }

 void stop_update_show_time_clock(){
    if(timedata.show_timeout_clock){
        g_source_remove(timedata.show_timeout_clock);
        timedata.show_timeout_clock = 0;
    }
 }

gboolean update_show_time(gpointer user_date){
    init_time_config();
    return TRUE;
}

void start_update_show_time_clock(){
    if (!timedata.show_timeout_clock){
        get_current_time();
        // we need to wait system to set time
        sleep(1);
        set_time_value();
    }
}

void set_time_value(){
        init_time_config();
        timedata.show_timeout_clock= g_timeout_add(1000,(GSourceFunc)update_show_time,NULL);
}

void show_timezone_system_sets(GPtrArray *loc){
    const gchar * timezone;
    GtkTreeModel * model;
    GtkTreeIter iter;
    gboolean valid;
    gchar * location;
    GVariant *str;
	TzUTC *tmp;

	int flag = 0;
    str = g_dbus_proxy_get_cached_property(timedata.proxy,"Timezone");
    timezone = g_variant_get_string(str, NULL);

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(timedata.tzcombo));
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);

	//获取和当前系统rtc时区对应的时区文件中的utc时区
	for(int i =0; i != loc->len; ++i){
			tmp = g_ptr_array_index(loc,i);
			for(int j =0; j != tmp->tz_rtc->len; ++j){
				if(strcmp(timezone, g_ptr_array_index(tmp->tz_rtc, j)) == 0){
					flag = 1;
					break;
				}
			}
			if(flag)
				break;
			//g_free(tmp);
	} 

	char *lang = strcmp(getenv("LANG"), "en_US.UTF-8") == 0 ? g_ptr_array_index(tmp->tz_rtc, 0): tmp->tz_utc;
	while (valid) {
        gtk_tree_model_get(model, &iter, 0, &location, -1);
		
        if (strcmp (location, lang) == 0){
            gtk_combo_box_set_active_iter (GTK_COMBO_BOX(timedata.tzcombo), &iter);
            valid = FALSE;
        }
        else {
            valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
        }
    }
//    g_free(location);
}

void init_dbus_proxy(){

    GError *error=NULL;
    timedata.proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,NULL, "org.freedesktop.timedate1",
    "/org/freedesktop/timedate1",
    "org.freedesktop.timedate1",
    NULL,&error);
    if (error !=NULL) {
        g_warning("Error :%s\n",error->message);
    }
}
void string_to_int(gchar * tmp){
    gchar ** c_time;
    c_time = g_strsplit(tmp, ":",3);
    timedata.hour = atoi(c_time[0]);
    timedata.minute = atoi(c_time[1]);
    timedata.second = atoi(c_time[2]);
    g_strfreev(c_time);
    freeze_clock();
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(timedata.hours),(gfloat)timedata.hour);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(timedata.minutes),(gfloat)timedata.minute);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(timedata.seconds),(gfloat)timedata.second);
    thaw_clock();
}

static void month_combo_changed(GtkComboBox *combobox, gpointer user_data){
    //month combobox changed and then tell calendar
    gint current_month;
    gint current_year;
    current_month  = gtk_combo_box_get_active(GTK_COMBO_BOX(timedata.td_combo_month));
    current_year = gtk_combo_box_get_active(GTK_COMBO_BOX(timedata.td_combo_year));
    if (current_year == -1)
        return;
    current_year    = current_year + _GMT;
    gtk_calendar_select_month((GtkCalendar *)timedata.calendar, current_month, current_year);
    on_editable_changed();
}

static void year_combo_changed(GtkComboBox * combobox, gpointer user_data){
    //year comboboxtext changed and then tell calendar
    gint current_month;
    gint current_year;
    current_month  = gtk_combo_box_get_active(GTK_COMBO_BOX(timedata.td_combo_month));
    if (current_month == -1)
        return;
    current_year = gtk_combo_box_get_active(GTK_COMBO_BOX(timedata.td_combo_year)) +_GMT ;
    gtk_calendar_select_month((GtkCalendar *)timedata.calendar, current_month, current_year);
    on_editable_changed();
}

void init_calendar_time(gchar *month, gchar *day, gchar *year){
    //we need start month from 0 to 11
    if(strcmp(month,"Jan")==0){
        timedata.month = 0;
    }else if(strcmp(month,"Feb")==0){
        timedata.month =1;
    }else if(strcmp(month,"Mar")==0){
        timedata.month =2;
    }else if(strcmp(month,"Apr")==0){
        timedata.month =3;
    }else if(strcmp(month,"May")==0){
        timedata.month =4;
    }else if(strcmp(month,"Jun")==0){
        timedata.month =5;
    }else if(strcmp(month,"Jul")==0){
        timedata.month =6;
    }else if(strcmp(month,"Aug")==0){
        timedata.month =7;
    }else if(strcmp(month,"Sep")==0){
        timedata.month =8;
    }else if(strcmp(month,"Oct")==0){
        timedata.month =9;
    }else if(strcmp(month,"Nov")==0){
        timedata.month =10;
    }else if(strcmp(month,"Dec")==0){
        timedata.month =11;
    }else{
        g_warning("Wrong month");
    }
    timedata.day = atoi(day);
    timedata.year = atoi(year);
    gtk_calendar_select_month((GtkCalendar *)timedata.calendar, timedata.month, timedata.year);
    gtk_calendar_select_day((GtkCalendar *)timedata.calendar, timedata.day);
    gtk_combo_box_set_active(GTK_COMBO_BOX(timedata.td_combo_year),timedata.year - _GMT);
    g_signal_handlers_block_by_func(timedata.td_combo_month,month_combo_changed,NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(timedata.td_combo_month),timedata.month);
    g_signal_handlers_unblock_by_func(timedata.td_combo_month, month_combo_changed,NULL);
}


gchar * get_current_time(){
    gchar * current_time_and_date;
    time_t now;
    struct tm *timenow;
    time(&now);
    timenow = localtime(&now);
    current_time_and_date = asctime(timenow);
    return current_time_and_date;
}

void init_time_config() {
    gchar * current_time_and_date;
    current_time_and_date = get_current_time();
    g_strchomp(current_time_and_date);
    //split to 6,i found the date(for example 2015-07-01,01 will set 1bit and a block)
    timedata.tmptime = g_strsplit(current_time_and_date,"\040",6);
    if(strcmp(timedata.tmptime[2],"")==0){
        string_to_int(timedata.tmptime[4]);
    }
   else {
        string_to_int(timedata.tmptime[3]);
    }

}

gchar *time_location_get_zone(TzUTC *loc){
	char *lang = getenv("LANG");
	if(!strcmp(lang, "en_US.UTF-8"))
		return g_ptr_array_index(loc->tz_rtc,0);
	else
		return loc->tz_utc;
}

int compare_country_names(const void *a, const void *b){
    const TzLocation *tza = * (TzLocation **) a;
    const TzLocation *tzb = * (TzLocation **) b;

    return strcmp(tza->zone, tzb->zone);
}

void sort_locations_by_country(GPtrArray *locations){
    qsort(locations->pdata, locations->len, sizeof(gpointer), compare_country_names);
}

gchar * tz_data_file_get(){
    gchar *file;
    file = g_strdup(TZ_DATA_FILE);
    return file;
}


TzDB *init_timedb (){
	gchar *tz_data_file;
	TzDB *tz_db;
	FILE *tzfile;

	char buf[4096];

	tz_data_file = tz_data_file_get ();
	if (!tz_data_file){
        g_warning("Could not get timedb source\n");
        return NULL;
    }
    tzfile = fopen(tz_data_file, "r");
    if (!tzfile) {
        g_warning("Could not open tzfile *%s*\n",tz_data_file);
        g_free(tz_data_file);
        return NULL;
    }

    tz_db = g_new0(TzDB, 1);
    tz_db->locations = g_ptr_array_new();
	while(fgets(buf, sizeof(buf), tzfile)){
        gchar **tmpstrarr;
		TzUTC *loc;
		loc = g_new0(TzUTC, 1);
		loc->tz_rtc = g_ptr_array_new();
		
		g_strchomp(buf);
        tmpstrarr = g_strsplit(buf, "\t",2);
		gchar ***tmp_rtc = g_strsplit(tmpstrarr[0], " ", 6);
		
		for(int i =0; i!=6; ++i){
			if(tmp_rtc[i] != NULL){
				g_ptr_array_add (loc->tz_rtc, (gpointer)g_strdup(tmp_rtc[i]));
			}
			else
				break;
		}

        loc->tz_utc = g_strdup(tmpstrarr[1]);

        g_ptr_array_add (tz_db->locations, loc);

		g_strfreev(tmp_rtc);
		g_strfreev(tmpstrarr);
    }
    fclose(tzfile);

    //sort_locations_by_country(tz_db->locations);
    g_free(tz_data_file);
    return tz_db;
}

static void init_timedate_data(GtkWidget * widget, gpointer user_data){
    guint i;
    GPtrArray *loc;
	TzUTC *tmp;
    //初始化时区数据库
    timedata.tzdb= init_timedb();
    //init config
    set_time_value();
    //init calendar
    g_signal_handlers_block_by_func(timedata.calendar, on_day_selected, NULL);
    init_calendar();
    g_signal_handlers_unblock_by_func(timedata.calendar, on_day_selected, NULL);
    //init dbus, get proxy
    init_dbus_proxy();
    init_time_setting();
    //初始化时区combobox
    loc = timedata.tzdb->locations;
    for (i=0; i< loc->len; i++){
		//拆开是因为编译时会有一个类型不匹配的错误
		tmp = g_ptr_array_index(loc, i);
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(timedata.tzcombo),time_location_get_zone(tmp));
        //gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(timedata.tzcombo),time_location_get_zone(g_ptr_array_index(loc,i)));
    }
    // Show the timezone what the system sets
    show_timezone_system_sets(loc);
//    g_signal_handlers_block_by_func(timedata.viewportlayout,
//                                    init_timedate_data,NULL);
}

static void add_year_and_month_data(){
    //add td_combo_year comboboxtext data
    gint i;
    for (i=0;i <MAXYAERRANGE; i++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(timedata.td_combo_year),g_strdup_printf("%s%s",year_tab[i],""));
}
//maybe we don't want to make year change
static void month_add_button_clicked(GtkButton * button, gpointer user_data){
    //month add and then tell calendar to change it
    gint current_active;
    gint current_year;
    current_active = gtk_combo_box_get_active(GTK_COMBO_BOX(timedata.td_combo_month));
    if (current_active == 11){
        //add year and then jump to 0
        current_year = gtk_combo_box_get_active(GTK_COMBO_BOX(timedata.td_combo_year));
        if (current_year == 99);
        else{
            g_signal_handlers_block_by_func(timedata.td_combo_year, year_combo_changed, NULL);
            gtk_combo_box_set_active(GTK_COMBO_BOX(timedata.td_combo_year), current_year +1);
            g_signal_handlers_unblock_by_func(timedata.td_combo_year, year_combo_changed, NULL);
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(timedata.td_combo_month), 0);
    }
    else {
        gtk_combo_box_set_active(GTK_COMBO_BOX(timedata.td_combo_month), current_active +1);
    }
}

static void month_del_button_clicked(GtkButton * button, gpointer user_data){
    //month del and then tell calendar to change it
    gint current_active;
    gint current_year;
    current_active = gtk_combo_box_get_active(GTK_COMBO_BOX(timedata.td_combo_month));
    if (current_active == 0){
        //add year and then jump to 0
        current_year = gtk_combo_box_get_active(GTK_COMBO_BOX(timedata.td_combo_year));
        if (current_year == 0);
        else{
            g_signal_handlers_block_by_func(timedata.td_combo_year, year_combo_changed, NULL);
            gtk_combo_box_set_active(GTK_COMBO_BOX(timedata.td_combo_year), current_year -1);
            g_signal_handlers_unblock_by_func(timedata.td_combo_year, year_combo_changed, NULL);
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(timedata.td_combo_month), 11);
    }
    else {
        gtk_combo_box_set_active(GTK_COMBO_BOX(timedata.td_combo_month), current_active -1);
    }
}

static void change_hour_format(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(timedata.hr24_radio)))
        g_settings_set_boolean(time_format, USE_24_FORMAT, TRUE);
    else
        g_settings_set_boolean(time_format, USE_24_FORMAT, FALSE);
}

void add_time_and_data_app(GtkBuilder * builder){
    g_debug("time_and_data");
    //hide calendar head, and then add four components to deal month and year change.
    timedata.td_month_add_button = GTK_WIDGET(gtk_builder_get_object(builder, "td_month_add_button"));
    gtk_widget_set_name(timedata.td_month_add_button, "time-add-button");
    g_signal_connect(timedata.td_month_add_button, "clicked", G_CALLBACK(month_add_button_clicked), NULL);
    timedata.td_combo_year = GTK_WIDGET(gtk_builder_get_object(builder, "td_combo_year"));
    g_signal_connect(timedata.td_combo_year, "changed", G_CALLBACK(year_combo_changed), NULL);
    timedata.td_combo_month = GTK_WIDGET(gtk_builder_get_object(builder, "td_combo_month"));
    g_signal_connect(timedata.td_combo_month, "changed", G_CALLBACK(month_combo_changed), NULL);
    timedata.td_month_del_button = GTK_WIDGET(gtk_builder_get_object(builder, "td_month_del_button"));
    g_signal_connect(timedata.td_month_del_button, "clicked", G_CALLBACK(month_del_button_clicked), NULL);
    add_year_and_month_data();

    //时间制式的相关设置
    timedata.hr12_radio = GTK_WIDGET(gtk_builder_get_object(builder, "radiobutton12"));
    timedata.hr24_radio = GTK_WIDGET(gtk_builder_get_object(builder, "radiobutton24"));
    time_format = g_settings_new("org.ukui.panel.indicator.calendar");
    g_signal_handlers_block_by_func(timedata.hr24_radio,change_hour_format, NULL);
    if(g_settings_get_boolean(time_format, USE_24_FORMAT))
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(timedata.hr24_radio),TRUE);
    else
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(timedata.hr12_radio),TRUE);
    g_signal_handlers_unblock_by_func(timedata.hr24_radio,change_hour_format, NULL);
    g_signal_connect(G_OBJECT(timedata.hr12_radio),"clicked",G_CALLBACK(change_hour_format),NULL);
    g_signal_connect(G_OBJECT(timedata.hr24_radio),"clicked",G_CALLBACK(change_hour_format),NULL);

    timedata.tzcombo = GTK_WIDGET(gtk_builder_get_object(builder, "time_zone_combobox"));
    g_signal_connect(G_OBJECT(timedata.tzcombo),"changed",
                     G_CALLBACK(on_timezone_changed),NULL);
	timedata.timesetting_checkbutton = gtk_builder_get_object(builder, "get_from_network");
	g_signal_connect(G_OBJECT(timedata.timesetting_checkbutton), "clicked", G_CALLBACK(on_timesetting_changed), NULL);
    timedata.calendar = GTK_WIDGET(gtk_builder_get_object(builder,"calendar"));
    g_signal_connect(G_OBJECT(timedata.calendar), "day-selected",
                     G_CALLBACK(on_day_selected),NULL);
    timedata.hours = GTK_WIDGET(gtk_builder_get_object(builder, "hours"));
    gtk_spin_button_set_range((GtkSpinButton *)timedata.hours,0.0,23.0);
//    g_signal_connect(G_OBJECT(timedata.hours),"wrapped",
//                    G_CALLBACK(on_time_wrapped),NULL);
    timedata.minutes = GTK_WIDGET(gtk_builder_get_object(builder, "minutes"));
    gtk_spin_button_set_range((GtkSpinButton *)timedata.minutes,0.0,59.0);
    timedata.seconds = GTK_WIDGET(gtk_builder_get_object(builder, "seconds"));
    gtk_spin_button_set_range((GtkSpinButton *)timedata.seconds,0.0,59.0);
    timedata.viewportlayout = GTK_WIDGET(gtk_builder_get_object(builder, "time_date_layout"));
    //g_signal_connect(G_OBJECT(timedata.viewportlayout), "map",
       //              G_CALLBACK(init_timedate_data),NULL);
    init_timedate_data(timedata.viewportlayout, NULL);
    g_signal_connect(G_OBJECT(timedata.hours),"changed",G_CALLBACK(on_editable_changed),NULL);
    g_signal_connect(G_OBJECT(timedata.minutes), "changed",G_CALLBACK(on_editable_changed),NULL);
    g_signal_connect(G_OBJECT(timedata.seconds), "changed",G_CALLBACK(on_editable_changed),NULL);
    //ntp_label的相关设置
	timedata.ntp_label = GTK_WIDGET(gtk_builder_get_object(builder, "ntp_label"));
	gtk_widget_set_no_show_all(timedata.ntp_label, TRUE);
	gtk_widget_hide(timedata.ntp_label);

}

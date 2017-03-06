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
#include "appearance-font.h"
#include <glib.h>
#include <pango/pangocairo.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <gdk/gdkx.h>
#include <gdk/gdk.h>
#include <X11/Xft/Xft.h>
#define N 3
/*
  设置字体，每套字体包括5个部件，应用程序字体、文档字体、等宽字体、桌面字体
  和窗口标题字体。
  字体设置为预设值，每套字体除大小不固定，其他字体类别固定。
  目前有
  ubuntu字体，分别为"ubuntu" "sans" "ubuntu mono" "' '" "ubuntu Bold"
  mate字体，分别为"ubuntu" "ubuntu" "ubuntu mono" "sans" "ubuntu Bold"
  ukui字体，分别为"ubuntu","ubuntu","ubuntu mono","ubuntu","ubuntu medium"
*/
typedef struct _FontSetting FontSetting;
struct _FontSetting {
    GtkWidget * radiobutton;
    GtkWidget * font_select_combo;
    GtkBuilder * builder;
    GtkWidget * small_radio_button;
    GtkWidget * middle_radio_button;
    GtkWidget * large_radio_button;
    GtkWidget * monochrome_button;
    GtkWidget * monochrome_sample;
    GtkWidget * best_shapes_button;
    GtkWidget * best_shapes_sample;
    GtkWidget * best_contrast_button;
    GtkWidget * best_contrast_sample;
    GtkWidget * subpixel_button;
    GtkWidget * subpixel_sample;
    //current font info
    gint size;
    gchar * font_name;

    float middle;
    float small;
    float large;
	float level;

    //gsettings
    GSettings * g_interface;
    GSettings * g_marco;
    GSettings * g_caja;
};
FontSetting fontsetting;
gint default_font;
gint default_document_font;
gint default_monospace_font;
gint default_caja_font;
gint default_title_font;
//for example, mate
typedef struct _FontInfo FontInfo;
struct _FontInfo{
    char * font_type; //like ubuntu or mate
    char * font_name; // org.mate.interface font-name
    char * document_font_name; //org.mate.interface document-font-name
    char * monospace_font_name; //org.mate.interface monospace-font-name
    char * font; //org.mate.caja.desktop font
    char * titlebar_font; //org.mate.Marco.general
};
//fill struct
FontInfo fontinfo[N] = {
    {"Ubuntu",
    "Ubuntu",
    "Sans",
    "Ubuntu Mono",
    " ",
    "Ubuntu Bold"},
    {"Mate",
    "Ubuntu",
    "Ubuntu",
    "Ubuntu Mono",
    "Sans",
    "Ubuntu Bold"},
	{"Kylin",
	"Ubuntu",
	"Ubuntu",
	"Ubuntu Mono",
	"Ubuntu",
	"Ubuntu Medium"}
};

typedef enum{
    ANTIALIAS_NONE,
    ANTIALIAS_GRAYSCALE,
    ANTIALIAS_RGBA
}Antialiasing;

typedef enum{
    HINT_NONE,
    HINT_SLIGHT,
    HINT_MEDIUM,
    HINT_FULL
}Hinting;

typedef enum{
    RGBA_RGB,
    RGBA_BGR,
    RGBA_VRGB,
    RGBA_VBGR
}RgbaOrder;

typedef struct{
    Antialiasing antialiasing;
    Hinting hinting;
    GtkButton * button;
}FontPair;
static GSList * font_pairs = NULL;
static void font_select_changed(GtkComboBox *widget, gpointer user_data);
/*static void setup_combotext_data(){
    gint i;
    PangoFontFamily ** families;
    gint n_families;
    PangoFontMap * fontmap;

    fontmap = pango_cairo_font_map_get_default();
    pango_font_map_list_families(fontmap, & families, & n_families);
    for (i =0; i<n_families; i++){
        PangoFontFamily * family = families[i];
        const char * family_name;
        family_name = pango_font_family_get_name(family);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(fontsetting.font_select_combo), family_name);
    }
    g_free(families);
}*/

// fill combobox with font type
static void setup_combotext_data(){
    int i;
    for (i=0; i<N; i++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(fontsetting.font_select_combo),fontinfo[i].font_type);
}

static void set_gsetting(){
    int i;
    char fontstring[20];
    for(i=0; i<N;i++){
        if(g_strcmp0(fontinfo[i].font_type,fontsetting.font_name) == 0){
            g_sprintf(fontstring, "%s %d",fontinfo[i].font_name,(gint)(default_font * fontsetting.level));
            g_settings_set_string(fontsetting.g_interface, GTK_FONT_KEY, fontstring);

            g_sprintf(fontstring, "%s %d",fontinfo[i].document_font_name,(gint)(default_document_font * fontsetting.level));
            g_settings_set_string(fontsetting.g_interface,DOCUMENT_FONT_KEY,fontstring);

            g_sprintf(fontstring, "%s %d",fontinfo[i].monospace_font_name,(gint)(default_monospace_font * fontsetting.level));
            g_settings_set_string(fontsetting.g_interface,MONOSPACE_FONT_KEY,fontstring);

            g_sprintf(fontstring, "%s %d",fontinfo[i].font,(gint)(default_caja_font * fontsetting.level));
            g_settings_set_string(fontsetting.g_caja,DESKTOP_FONT_KEY,fontstring);

            g_sprintf(fontstring, "%s %d",fontinfo[i].titlebar_font, (gint)(default_title_font * fontsetting.level));
            g_settings_set_string(fontsetting.g_marco,WINDOW_TITLE_FONT_KEY,fontstring);
            return;
        }
    }
}

static char * get_font_name (gchar * str, gboolean b){
    //font name lenght <30
    gchar * str1;
    str1 = (char *)malloc(sizeof(gchar)* ((int)strlen(str)));
    gint i;
    if (b){
        for (i=0; i < ( (int )strlen(str) -2); i++)
            str1[i] = str[i];
    }
    else {
        for (i=0; i< ( (int)strlen(str) -3); i++)
            str1[i] = str[i];
    }
    str1[i] = '\0';
    return str1;
}

/*static void init_font_info(){
    gchar * font_info;
    gint s_length;
    font_info = g_settings_get_string(fontsetting.g_interface , DOCUMENT_FONT_KEY);
    s_length = strlen(font_info);
    if (font_info[s_length -2] == ' '){
        fontsetting.size = atoi(&font_info[s_length -1]);
        fontsetting.font_name = get_font_name(font_info, TRUE);
    }
    else{
        fontsetting.size = atoi(&font_info[s_length -2]);
        fontsetting.font_name = get_font_name(font_info, FALSE);
    }
}*/

static char * get_string(gchar * gsetting, gint i){
    char * pstring;
    gint s_length;
    s_length = strlen(gsetting);
    if (gsetting[s_length -2] == ' '){
		if (i)
        	fontsetting.size = atoi(&gsetting[s_length -1]);
        pstring = get_font_name(gsetting, TRUE);
    }
    else {
		if (i)
        	fontsetting.size = atoi(&gsetting[s_length -2]);
        pstring = get_font_name(gsetting, FALSE);
    }
    return pstring;
}

// get current font info
static void init_font_info(){
    FontInfo current_info;
    int i;
    current_info.font_name =get_string(g_settings_get_string(fontsetting.g_interface, GTK_FONT_KEY),0);
    current_info.document_font_name = get_string(g_settings_get_string(fontsetting.g_interface, DOCUMENT_FONT_KEY),1);
    current_info.monospace_font_name = get_string(g_settings_get_string(fontsetting.g_interface, MONOSPACE_FONT_KEY),0);
    current_info.font = get_string(g_settings_get_string(fontsetting.g_caja, DESKTOP_FONT_KEY),0);
    current_info.titlebar_font =get_string(g_settings_get_string(fontsetting.g_marco, WINDOW_TITLE_FONT_KEY),0);
    for(i=0; i<N;i++){
        if(g_strcmp0(fontinfo[i].font_name, current_info.font_name) != 0)
            continue;
        if(g_strcmp0(fontinfo[i].document_font_name ,current_info.document_font_name) != 0)
            continue;
        if(g_strcmp0(fontinfo[i].monospace_font_name ,current_info.monospace_font_name) !=0)
            continue;
        if(g_strcmp0(fontinfo[i].font, current_info.font) !=0)
            continue;
        if(g_strcmp0(fontinfo[i].titlebar_font, current_info.titlebar_font) !=0)
            continue;
        fontsetting.font_name = fontinfo[i].font_type;
		return;
    }
}

static void init_font_data(){
    GtkTreeModel * treemodel;
    gboolean valid;
    GtkTreeIter  iter;
    gchar * name=NULL;
	if (default_document_font == 0)
		default_document_font = 10;
    float res = (float)fontsetting.size / (float)default_document_font;
    if (res == fontsetting.small){
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fontsetting.small_radio_button), TRUE);
		fontsetting.level = fontsetting.small;
    }
    else if ( fontsetting.small < res && res <= fontsetting.middle ){
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fontsetting.middle_radio_button), TRUE);
		fontsetting.level = fontsetting.middle;
    }
    else if (res >fontsetting.middle){
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fontsetting.large_radio_button), TRUE);
		fontsetting.level = fontsetting.large;
    }
    treemodel = gtk_combo_box_get_model(GTK_COMBO_BOX(fontsetting.font_select_combo));
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(treemodel),&iter);
    while (valid){
        gtk_tree_model_get(treemodel, &iter,0, &name, -1);
        if (g_strcmp0(name, fontsetting.font_name) == 0){
			g_signal_handlers_block_by_func(fontsetting.font_select_combo, font_select_changed, NULL);
            gtk_combo_box_set_active_iter(GTK_COMBO_BOX(fontsetting.font_select_combo), &iter);
			g_signal_handlers_unblock_by_func(fontsetting.font_select_combo, font_select_changed, NULL);
            valid = FALSE;
        }
        else {
            valid = gtk_tree_model_iter_next(treemodel, &iter);
        }
    }
    g_free(name);
}

static void get_default_setting(AppearanceData *data){
    GVariant * value;
    char * font_value;
    gsize size;
    gint length;
    value = g_settings_get_default_value(data->interface_settings, DOCUMENT_FONT_KEY);
    size = g_variant_get_size (value);
    font_value = g_variant_get_string(value, &size);
    length = (gint)strlen(font_value);
    if (font_value[length -2] == ' '){
        default_document_font = atoi(&font_value[length -1]);
    }
    else {
        default_document_font =  atoi(&font_value[length -2]);
    }
	g_variant_unref(value);

	value = g_settings_get_default_value(data->interface_settings, GTK_FONT_KEY);
	size = g_variant_get_size(value);
	font_value = g_variant_get_string(value, &size);
	length = (gint)strlen(font_value);
	if (font_value[length -2] == ' ')
		default_font = atoi(&font_value[length -1]);
	else 
		default_font = atoi(&font_value[length -2]);
	g_variant_unref(value);

	value = g_settings_get_default_value(data->interface_settings, MONOSPACE_FONT_KEY);
	size = g_variant_get_size(value);
	font_value = g_variant_get_string(value, &size);
	length = (gint)strlen(font_value);
	if (font_value[length -2] == ' ')
		default_monospace_font = atoi(&font_value[length -1]);
	else
		default_monospace_font = atoi(&font_value[length -2]);
	g_variant_unref(value);
	
	value = g_settings_get_default_value(data->caja_settings, DESKTOP_FONT_KEY);
	size = g_variant_get_size(value);
	font_value = g_variant_get_string(value, &size);
	length = (gint)strlen(font_value);
	if (font_value[length -2] == ' ')
		default_caja_font = atoi(&font_value[length -1]);
	else
		default_caja_font = atoi(&font_value[length -2]);
	g_variant_unref(value);
	
	value = g_settings_get_default_value(data->marco_settings, WINDOW_TITLE_FONT_KEY);
	size = g_variant_get_size(value);
	font_value = g_variant_get_string(value, &size);
	length = (gint)strlen(font_value);
	if (font_value[length -2] == ' ')
		default_title_font = atoi(&font_value[length -1]);
	else 
		default_title_font = atoi(&font_value[length -2]);
	g_variant_unref(value);
	
}

static void sample_size_request(GtkWidget * darea, GtkRequisition * requisition){
   // GdkPixbuf * pixbuf = g_object_get_data(G_OBJECT(darea), "sample-pixbuf");
   // requisition->width = gdk_pixbuf_get_width(pixbuf) + 2;
   // requisition->height = gdk_pixbuf_get_height(pixbuf) + 2;
}

static void sample_expose(GtkWidget * darea, GdkEventExpose *expose){
   // GtkAllocation allocation;
   // GdkPixbuf * pixbuf = g_object_get_data(G_OBJECT(darea), "sample-pixbuf");
   // GdkWindow *window = gtk_widget_get_window(darea);
//    GtkStyle * style = gtk_widget_get_style(darea);
   // int width = gdk_pixbuf_get_width(pixbuf);
   // int height = gdk_pixbuf_get_height(pixbuf);
  //  gtk_widget_get_allocation(darea, &allocation);
  //  int x = (allocation.width - width) /2;
   // int y = (allocation.height- height) /2;
   // gdk_draw_rectangle(window, style->white_gc, TRUE, 0,0,allocation.width, allocation.height);
   // gdk_draw_rectangle(window, style->black_gc, FALSE,0,0,allocation.width -1, allocation.height -1);
   // gdk_draw_pixbuf(window, NULL, pixbuf, 0,0,x,y,width, height, GDK_RGB_DITHER_NORMAL,0,0);
   // gtk_widget_show(darea);
}

static XftFont *open_pattern(FcPattern *pattern, Antialiasing antialiasing, Hinting hinting){
    FcPattern * res_pattern;
    FcResult result;
    XftFont * font;

    Display * xdisplay = gdk_x11_get_default_xdisplay();
    int screen = gdk_x11_get_default_screen();

    res_pattern = XftFontMatch(xdisplay, screen, pattern, &result);
    if (res_pattern == NULL){
        return NULL;
    }
    FcPatternDel(res_pattern, FC_HINTING);
    FcPatternAddBool(res_pattern, FC_HINTING, hinting != HINT_NONE);

    FcPatternDel(res_pattern, FC_ANTIALIAS);
    FcPatternAddBool(res_pattern, FC_ANTIALIAS, antialiasing != ANTIALIAS_NONE);

    FcPatternDel(res_pattern, FC_RGBA);
    FcPatternAddInteger(res_pattern, FC_RGBA, antialiasing == ANTIALIAS_RGBA? FC_RGBA_RGB : FC_RGBA_NONE);

    FcPatternDel(res_pattern, FC_DPI);
    FcPatternAddInteger(res_pattern, FC_DPI, 96);

    font = XftFontOpenPattern(xdisplay, res_pattern);
    if (!font)
        FcPatternDestroy(res_pattern);
    return font;
}

static void sample_draw(GtkWidget* darea, cairo_t* cr)
{
    cairo_surface_t* surface = g_object_get_data(G_OBJECT(darea), "sample-surface");
    GtkAllocation allocation;
    int x, y, w, h;

    gtk_widget_get_allocation (darea, &allocation);
    x = allocation.width;
    y = allocation.height;
    w = cairo_image_surface_get_width (surface);
    h = cairo_image_surface_get_height (surface);

    cairo_set_line_width (cr, 1);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);

    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    cairo_rectangle (cr, 0, 0, x, y);
    cairo_fill_preserve (cr);
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_stroke (cr);

    cairo_set_source_surface (cr, surface, (x - w) / 2, (y - h) / 2);

    cairo_paint(cr);
}

static void set_fontoptions(PangoContext *context, Antialiasing antialiasing, Hinting hinting)
{
    cairo_font_options_t *opt;
    cairo_antialias_t aa;
    cairo_hint_style_t hs;

    switch (antialiasing) {
    case ANTIALIAS_NONE:
        aa = CAIRO_ANTIALIAS_NONE;
        break;
    case ANTIALIAS_GRAYSCALE:
        aa = CAIRO_ANTIALIAS_GRAY;
        break;
    case ANTIALIAS_RGBA:
        aa = CAIRO_ANTIALIAS_SUBPIXEL;
        break;
    default:
        aa = CAIRO_ANTIALIAS_DEFAULT;
        break;
    }

    switch (hinting) {
    case HINT_NONE:
        hs = CAIRO_HINT_STYLE_NONE;
        break;
    case HINT_SLIGHT:
        hs = CAIRO_HINT_STYLE_SLIGHT;
        break;
    case HINT_MEDIUM:
        hs = CAIRO_HINT_STYLE_MEDIUM;
        break;
    case HINT_FULL:
        hs = CAIRO_HINT_STYLE_FULL;
        break;
    default:
        hs = CAIRO_HINT_STYLE_DEFAULT;
        break;
    }

    opt = cairo_font_options_create ();
    cairo_font_options_set_antialias (opt, aa);
    cairo_font_options_set_hint_style (opt, hs);
    pango_cairo_context_set_font_options (context, opt);
    cairo_font_options_destroy (opt);
}
static void setup_font_sample(GtkWidget * darea, Antialiasing antialiasing, Hinting hinting){
    const char * string1;
    char *lang = getenv("LANG");
    if (strncmp(lang, "zh_CN", 5) == 0)
        string1 = "欢迎使用麒麟操作系统";
    else
        string1 = "Welcome to Linux";

    const char * string2 = "Welcome to Kylin";

    PangoContext *context;
    PangoLayout *layout;
    PangoFontDescription *fd;
    PangoRectangle extents;
    cairo_surface_t *surface;
    cairo_t *cr;
    int width, height;

    context = gtk_widget_get_pango_context (darea);
    set_fontoptions (context, antialiasing, hinting);
    layout = pango_layout_new (context);

    fd = pango_font_description_from_string ("Serif");
    pango_layout_set_font_description (layout, fd);
    pango_font_description_free (fd);

    pango_layout_set_markup (layout, string1, -1);

    pango_layout_get_extents (layout, NULL, &extents);
    width = PANGO_PIXELS(extents.width) + 4;
    height = PANGO_PIXELS(extents.height) + 2;

    surface = cairo_image_surface_create (CAIRO_FORMAT_A8, width, height);
    cr = cairo_create (surface);

    cairo_move_to (cr, 2, 1);
    pango_cairo_show_layout (cr, layout);
    g_object_unref (layout);
    cairo_destroy (cr);

    g_object_set_data_full(G_OBJECT(darea), "sample-surface", surface, (GDestroyNotify) cairo_surface_destroy);

    gtk_widget_set_size_request (GTK_WIDGET(darea), width + 2, height + 2);
    g_signal_connect(darea, "draw", G_CALLBACK(sample_draw), NULL);
}

static void font_button_clicked(GtkButton *button, FontPair *pair){
    GSettings * settings = g_settings_new(FONT_RENDER_SCHEMA);
    g_settings_set_enum(settings, FONT_ANTIALIASING_KEY, pair->antialiasing);
    g_settings_set_enum(settings, FONT_HINTING_KEY, pair->hinting);

    g_object_unref(settings);
}

static void setup_font_pair(GtkWidget * button, GtkWidget * darea, Antialiasing antialiasing, Hinting hinting){
    FontPair *pair = g_new(FontPair, 1);
    pair->antialiasing = antialiasing;
    pair->hinting = hinting;
    pair->button = button;

    setup_font_sample(darea, antialiasing, hinting);
    font_pairs = g_slist_prepend(font_pairs, pair);
    g_signal_connect(GTK_BUTTON(button), "clicked", G_CALLBACK(font_button_clicked), pair);
}

static void radio_button_clicked(GtkButton *button, GdkEvent *event,gpointer user_data){
    gint new_level;
    gboolean current_state;
    current_state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    if (current_state)
        return;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    //get the font size we need set new
	fontsetting.level = *(float *)user_data;
    set_gsetting();
}

static void font_select_changed(GtkComboBox *widget, gpointer user_data){
    gchar * current_font_name= NULL;
    //gchar current_new_font[30];
    GtkTreeIter iter;
    GtkTreeModel * model;
    gtk_combo_box_get_active_iter(GTK_COMBO_BOX(fontsetting.font_select_combo),&iter);
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
    gtk_tree_model_get(model, &iter, 0, &current_font_name, -1);
    if (current_font_name == NULL)
        return;
    fontsetting.font_name = current_font_name;
    set_gsetting();
}
static gboolean reset_font_default(GtkWidget * widget, GdkEvent *event, gpointer user_data){
    //reset font
    g_settings_reset(fontsetting.g_interface, DOCUMENT_FONT_KEY);
    g_settings_reset(fontsetting.g_interface, GTK_FONT_KEY);
    g_settings_reset(fontsetting.g_interface, MONOSPACE_FONT_KEY);
    g_settings_reset(fontsetting.g_marco,WINDOW_TITLE_FONT_KEY);
    g_settings_reset(fontsetting.g_caja, DESKTOP_FONT_KEY);
    init_font_info();
    init_font_data();
    //reset  font render
    GSettings * settings = g_settings_new(FONT_RENDER_SCHEMA);
    g_settings_reset(settings,FONT_ANTIALIASING_KEY);
    g_settings_reset(settings,FONT_HINTING_KEY);
    g_object_unref(settings);
}

void font_init(AppearanceData * data){
    GtkWidget * default_button;
    fontsetting.builder = data->ui;
    //init data
    fontsetting.small = 1.00;
    fontsetting.middle = 1.25;
    fontsetting.large = 1.50;

    fontsetting.g_interface = data->interface_settings;
    fontsetting.g_marco = data->marco_settings;
    fontsetting.g_caja = data->caja_settings;

    default_button = GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "default_button"));
    g_signal_connect(default_button, "button-press-event",G_CALLBACK(reset_font_default),NULL);

    fontsetting.font_select_combo = GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "font_select_comboboxtext"));
    g_signal_connect(GTK_COMBO_BOX(fontsetting.font_select_combo), "changed", G_CALLBACK(font_select_changed), NULL);
    fontsetting.small_radio_button = GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "small_radio_button"));
    g_signal_connect(GTK_BUTTON(fontsetting.small_radio_button), "button_release_event", G_CALLBACK(radio_button_clicked), &fontsetting.small);
    fontsetting.middle_radio_button = GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "middle_radio_button"));
    g_signal_connect(GTK_BUTTON(fontsetting.middle_radio_button), "button_release_event", G_CALLBACK(radio_button_clicked),&fontsetting.middle);
    fontsetting.large_radio_button =GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "large_radio_button"));
    g_signal_connect(GTK_BUTTON(fontsetting.large_radio_button), "button_release_event", G_CALLBACK(radio_button_clicked),&fontsetting.large);
    //sample
    fontsetting.monochrome_button = GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "monochrome_button"));
    fontsetting.monochrome_sample = GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "monochrome_sample"));
    fontsetting.best_contrast_button = GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "best_contrast_button"));
    fontsetting.best_contrast_sample = GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "best_contrast_sample"));
    fontsetting.best_shapes_button = GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "best_shapes_button"));
    fontsetting.best_shapes_sample = GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "best_shapes_sample"));
    fontsetting.subpixel_button = GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "subpixel_button"));
    fontsetting.subpixel_sample = GTK_WIDGET(gtk_builder_get_object(fontsetting.builder, "subpixel_sample"));

    //init sample
    setup_font_pair(fontsetting.monochrome_button, fontsetting.monochrome_sample, ANTIALIAS_NONE, HINT_FULL);
    setup_font_pair(fontsetting.best_contrast_button, fontsetting.best_contrast_sample, ANTIALIAS_GRAYSCALE, HINT_FULL);
    setup_font_pair(fontsetting.best_shapes_button, fontsetting.best_shapes_sample, ANTIALIAS_GRAYSCALE, HINT_MEDIUM);
    setup_font_pair(fontsetting.subpixel_button, fontsetting.subpixel_sample, ANTIALIAS_RGBA, HINT_FULL);
    //get default value from gsetting
    get_default_setting(data);
    //setup data into font_select_comboboxtext
    setup_combotext_data();
    //get font information, include size and font name
    init_font_info();
    //make font select combobox and font size show the level they should be setted
    init_font_data();
}

void font_shutdown(AppearanceData *data){

}

#ifndef __THEME_THUMBNAIL_H__
#define __THEME_THUMBNAIL_H__


#include <gtk/gtk.h>
#include "ukui-theme-info.h"

typedef void (* ThemeThumbnailFunc)          (GdkPixbuf          *pixbuf,
                                              gchar              *theme_name,
                                              gpointer            data);

GdkPixbuf *generate_meta_theme_thumbnail     (UkuiThemeMetaInfo *theme_info);
GdkPixbuf *generate_gtk_theme_thumbnail      (UkuiThemeInfo     *theme_info);
GdkPixbuf *generate_ukwm_theme_thumbnail (UkuiThemeInfo     *theme_info);
GdkPixbuf *generate_icon_theme_thumbnail     (UkuiThemeIconInfo *theme_info);

void generate_meta_theme_thumbnail_async     (UkuiThemeMetaInfo *theme_info,
                                              ThemeThumbnailFunc  func,
                                              gpointer            data,
                                              GDestroyNotify      destroy);
void generate_gtk_theme_thumbnail_async      (UkuiThemeInfo     *theme_info,
                                              ThemeThumbnailFunc  func,
                                              gpointer            data,
                                              GDestroyNotify      destroy);
void generate_ukwm_theme_thumbnail_async (UkuiThemeInfo     *theme_info,
                                              ThemeThumbnailFunc  func,
                                              gpointer            data,
                                              GDestroyNotify      destroy);
void generate_icon_theme_thumbnail_async     (UkuiThemeIconInfo *theme_info,
                                              ThemeThumbnailFunc  func,
                                              gpointer            data,
                                              GDestroyNotify      destroy);

void theme_thumbnail_factory_init            (int                 argc,
                                              char               *argv[]);

#endif /* __THEME_THUMBNAIL_H__ */

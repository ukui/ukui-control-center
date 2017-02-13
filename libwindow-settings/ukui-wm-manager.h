#ifndef UKUI_WINDOW_MANAGER_LIST_H
#define UKUI_WINDOW_MANAGER_LIST_H

#include <gtk/gtk.h>

#include "ukui-window-manager.h"

void ukui_wm_manager_init (void);

/* gets the currently active window manager */
UkuiWindowManager *ukui_wm_manager_get_current (GdkScreen *screen);

gboolean ukui_wm_manager_spawn_config_tool_for_current (GdkScreen  *screen,
                                                         GError    **error);

#endif

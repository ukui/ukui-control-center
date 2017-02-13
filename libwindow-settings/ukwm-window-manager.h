#ifndef UKWM_WINDOW_MANAGER_H
#define UKWM_WINDOW_MANAGER_H

#include <glib-object.h>
#include "ukui-window-manager.h"

#define UKWM_WINDOW_MANAGER(obj)          G_TYPE_CHECK_INSTANCE_CAST (obj, ukwm_window_manager_get_type (), UkwmWindowManager)
#define UKWM_WINDOW_MANAGER_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST (klass, ukwm_window_manager_get_type (), UkwmWindowManagerClass)
#define IS_UKWM_WINDOW_MANAGER(obj)       G_TYPE_CHECK_INSTANCE_TYPE (obj, ukwm_window_manager_get_type ())

typedef struct _UkwmWindowManager UkwmWindowManager;
typedef struct _UkwmWindowManagerClass UkwmWindowManagerClass;

typedef struct _UkwmWindowManagerPrivate UkwmWindowManagerPrivate;

struct _UkwmWindowManager
{
	UkuiWindowManager parent;
	UkwmWindowManagerPrivate *p;
};

struct _UkwmWindowManagerClass
{
	UkuiWindowManagerClass klass;
};

GType      ukwm_window_manager_get_type             (void);

#endif

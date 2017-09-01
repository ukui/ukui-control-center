#ifndef __KPMPREFSCORE_H
#define __KPMPREFSCORE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define KPM_TYPE_PREFS          (kpm_prefs_get_type ())
#define KPM_PREFS(o)            (G_TYPE_CHECK_INSTANCE_CAST ((o), KPM_TYPE_PREFS, KpmPrefs))
#define KPM_PREFS_CLASS(k)      (G_TYPE_CHECK_CLASS_CAST((k), KPM_TYPE_PREFS, KpmPrefsClass))
#define KPM_IS_PREFS(o)         (G_TYPE_CHECK_INSTANCE_TYPE ((o), KPM_TYPE_PREFS))
#define KPM_IS_PREFS_CLASS(k)   (G_TYPE_CHECK_CLASS_TYPE ((k), KPM_TYPE_PREFS))
#define KPM_PREFS_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), KPM_TYPE_PREFS, KpmPrefsClass))

typedef struct KpmPrefsPrivate KpmPrefsPrivate;

typedef struct
{
        GObject          parent;
        KpmPrefsPrivate *priv;
} KpmPrefs;

typedef struct
{
        GObjectClass    parent_class;
        void            (* action_help)                 (KpmPrefs       *prefs);
        void            (* action_close)                (KpmPrefs       *prefs);
} KpmPrefsClass;

GType            kpm_prefs_get_type                     (void);
KpmPrefs        *kpm_prefs_new                          (void);
void             kpm_prefs_activate_window              (KpmPrefs       *prefs);

void init_power();

G_END_DECLS

#endif  /* __KPMPREFS_H */
                                                    

#ifndef __GSP_APP_MANAGER_H
#define __GSP_APP_MANAGER_H

#include <glib-object.h>

#include <gsp-app.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GSP_TYPE_APP_MANAGER            (gsp_app_manager_get_type ())
#define GSP_APP_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSP_TYPE_APP_MANAGER, GspAppManager))
#define GSP_APP_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GSP_TYPE_APP_MANAGER, GspAppManagerClass))
#define GSP_IS_APP_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSP_TYPE_APP_MANAGER))
#define GSP_IS_APP_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GSP_TYPE_APP_MANAGER))
#define GSP_APP_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GSP_TYPE_APP_MANAGER, GspAppManagerClass))

typedef struct _GspAppManager        GspAppManager;
typedef struct _GspAppManagerClass   GspAppManagerClass;

typedef struct _GspAppManagerPrivate GspAppManagerPrivate;

struct _GspAppManagerClass
{
        GObjectClass parent_class;

        void (* added)   (GspAppManager *manager,
                          GspApp        *app);
        void (* removed) (GspAppManager *manager,
                          GspApp        *app);
};

struct _GspAppManager
{
        GObject parent_instance;

        GspAppManagerPrivate *priv;
};

GType           gsp_app_manager_get_type               (void);

GspAppManager  *gsp_app_manager_get                    (void);

void            gsp_app_manager_fill                   (GspAppManager *manager);

GSList         *gsp_app_manager_get_apps               (GspAppManager *manager);

GspApp         *gsp_app_manager_find_app_with_basename (GspAppManager *manager,
                                                        const char    *basename);

GspApp         *gsp_app_manager_find_app_with_name (GspAppManager *manager,
                                                        const char    *name);

GspApp         *gsp_app_manager_find_local_app_with_name_exec(GspAppManager *manager,
                                                         const char *name, const char *exec,const char *icon);

const char     *gsp_app_manager_get_dir                (GspAppManager *manager,
                                                        unsigned int   index);

void            gsp_app_manager_add                    (GspAppManager *manager,
                                                        GspApp        *app);

#ifdef __cplusplus
}
#endif

#endif /* __GSP_APP_MANAGER_H */

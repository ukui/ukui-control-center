#ifndef __GSM_APP_DIALOG_H
#define __GSM_APP_DIALOG_H

#include <glib-object.h>
#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GSM_TYPE_APP_DIALOG         (gsm_app_dialog_get_type ())
#define GSM_APP_DIALOG(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GSM_TYPE_APP_DIALOG, GsmAppDialog))
#define GSM_APP_DIALOG_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), GSM_TYPE_APP_DIALOG, GsmAppDialogClass))
#define GSM_IS_APP_DIALOG(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GSM_TYPE_APP_DIALOG))
#define GSM_IS_APP_DIALOG_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), GSM_TYPE_APP_DIALOG))
#define GSM_APP_DIALOG_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GSM_TYPE_APP_DIALOG, GsmAppDialogClass))

typedef struct GsmAppDialogPrivate GsmAppDialogPrivate;

typedef struct
{
        GtkDialog            parent;
        GsmAppDialogPrivate *priv;
} GsmAppDialog;

typedef struct
{
        GtkDialogClass   parent_class;
} GsmAppDialogClass;

GType                  gsm_app_dialog_get_type           (void);

GtkWidget            * gsm_app_dialog_new                (const char   *name,
                                                          const char   *command,
                                                          const char   *comment);

gboolean               gsm_app_dialog_run               (GsmAppDialog  *dialog,
                                                         char         **name_p,
                                                         char         **command_p,
                                                         char         **comment_p,
                                                         char         **icon_p);

const char *           gsm_app_dialog_get_name           (GsmAppDialog *dialog);
const char *           gsm_app_dialog_get_command        (GsmAppDialog *dialog);
const char *           gsm_app_dialog_get_comment        (GsmAppDialog *dialog);

#ifdef __cplusplus
}
#endif

#endif /* __GSM_APP_DIALOG_H */

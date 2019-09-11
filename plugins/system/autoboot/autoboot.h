#ifndef AUTOBOOT_H
#define AUTOBOOT_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include <QTableWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QSignalMapper>

#include <QDebug>

#include "mainui/interface.h"
#include "../../pluginsComponent/switchbutton.h"
#include "../../pluginsComponent/customwidget.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
}

namespace Ui {
class AutoBoot;
}

#define APP_KEY_FILE_DESKTOP_KEY_AUTOSTART_ENABLE "X-UKUI-Autostart-enabled"

typedef struct _AutoApp{

    QString bname;
    QString path;

    bool enable;
    bool no_display;
    bool shown;
    bool hidden;

    QString name;
    QString comment;
    QIcon qicon;
    QString exec;
    QString description;
}AutoApp;

class AutoBoot : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    AutoBoot();
    ~AutoBoot();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    void initUI();
    void update_app_status();


private:
    Ui::AutoBoot *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

    QMap<QString, AutoApp> appMaps;
    QMap<QString, AutoApp> localappMaps;
    QMap<QString, AutoApp> statusMaps;
    QMultiMap<QString, QWidget *> appgroupMultiMaps;

    void _walk_config_dirs();
    AutoApp _app_new(const char * path);
    gboolean _key_file_get_boolean(GKeyFile * keyfile, const gchar * key, gboolean defaultvalue);
    gboolean _key_file_get_shown(GKeyFile * keyfile, const char * currentdesktop);
    gboolean _stop_autoapp(QString bname);
    gboolean _key_file_to_file(GKeyFile * keyfile, const gchar * path);

public slots:
    void checkbox_changed_cb(QString bname);
};

#endif // AUTOBOOT_H

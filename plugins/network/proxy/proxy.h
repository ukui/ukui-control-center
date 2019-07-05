#ifndef PROXY_H
#define PROXY_H


#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QWidget>
#include <QDialog>
#include <QLineEdit>
#include "certificationdialog.h"

#include <QGSettings/QGSettings>



/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
}

#define PROXY_SCHEMA              "org.gnome.system.proxy"
#define PROXY_MODE_KEY            "mode"
#define PROXY_AUTOCONFIG_URL_KEY  "autoconfig-url"
#define IGNORE_HOSTS_KEY          "ignore-hosts"

#define HTTP_PROXY_SCHEMA         "org.gnome.system.proxy.http"
#define HTTP_PROXY_HOST_KEY       "host"
#define HTTP_PROXY_PORT_KEY       "port"
#define HTTP_USE_AUTH_KEY         "use-authentication"
#define HTTP_AUTH_USER_KEY        "authentication-user"
#define HTTP_AUTH_PASSWD_KEY      "authentication-password"

#define SECURE_PROXY_SCHEMA        "org.gnome.system.proxy.https"
#define SECURE_PROXY_HOST_KEY     "host"
#define SECURE_PROXY_PORT_KEY     "port"

#define FTP_PROXY_SCHEMA          "org.gnome.system.proxy.ftp"
#define FTP_PROXY_HOST_KEY        "host"
#define FTP_PROXY_PORT_KEY        "port"

#define SOCKS_PROXY_SCHEMA        "org.gnome.system.proxy.socks"
#define SOCKS_PROXY_HOST_KEY      "host"
#define SOCKS_PROXY_PORT_KEY      "port"

struct KeyValue : QObjectUserData {
    QString key;
    QString schema;
};


typedef enum{
    NONE,
    MANUAL,
    AUTO
}ProxyMode;

//自定义类型使用QVariant需要使用 Q_DECLARE_METATYPE 注册
Q_DECLARE_METATYPE(ProxyMode)

namespace Ui {
class Proxy;
}

class Proxy : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Proxy();
    ~Proxy();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;

    void component_init();
    void status_init();

private:
    Ui::Proxy *ui;

    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

    QGSettings * proxysettings;
    QGSettings * httpsettings;
    QGSettings * securesettings;
    QGSettings * ftpsettings;
    QGSettings * sockssettings;


    int _get_current_proxy_mode();
    void  _refreshUI();

private slots:
    void mode_changed_slot(int id);
    void url_edit_changed_slot(QString edit);
    void ignore_host_edit_changed_slot();
    void manual_component_changed_slot(QString edit);

    void certification_dialog_show_slot();
};

#endif // PROXY_H

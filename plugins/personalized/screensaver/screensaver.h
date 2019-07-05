#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QMap>
#include <QGSettings/QGSettings>

#include "plugins/component/switchbutton.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
}

typedef struct _SSThemeInfo{
    QString name;
    QString exec;
    QString id;
}SSThemeInfo;

//自定义数据类型，如果要使用QVariant，就必须使用Q_DECLARE_METATYPE注册。
Q_DECLARE_METATYPE(SSThemeInfo);

namespace Ui {
class Screensaver;
}

class Screensaver : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Screensaver();
    ~Screensaver();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;

    void component_init();
    void status_init();

    void set_idle_gsettings_value(int value);

private:
    Ui::Screensaver *ui;

    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

    SSThemeInfo _info_new(const char * path);
    void init_theme_info_map();

    QMap<QString, SSThemeInfo> infoMap;

    GSettings * screensaver_settings;
    GSettings * session_settings;

    SwitchButton * lockscreenBtn;

private slots:
    void combobox_changed_slot(int index);
    void activebtn_changed_slot(int status);
    void lockbtn_changed_slot(int status);
    void slider_changed_slot(int value);
    void slider_released_slot();

};

#endif // SCREENSAVER_H

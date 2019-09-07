#ifndef DATETIME_H
#define DATETIME_H

#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QWidget>
#include <QAbstractButton>

#include <QGSettings/QGSettings>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QFileInfo>
#include <QFile>
#include <QDateTime>
#include <QTimer>

#include "../../component/switchbutton.h"
#include "../../component/customwidget.h"

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

#define FORMAT_SCHEMA "org.ukui.panel.indicator.calendar"
#define TIME_FORMAT_KEY "use-24h-format"
#define WEEK_FORMAT_KEY "sunday-as-first"
#define SHOW_SECOND "show-second"

#define TZ_DATA_FILE "/usr/share/zoneinfo/zoneUtc"
#define DEFAULT_TZ "Asia/Shanghai"

namespace Ui {
class DateTime;
}

class DateTime : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    DateTime();
    ~DateTime();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    void component_init();
    void status_init();
    void hour_combobox_setup();

private:
    Ui::DateTime *ui;

    SwitchButton * ntpSwitchBtn;
    SwitchButton * longtimeSwitchBtn;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

    QGSettings * formatsettings;

    QDBusInterface * datetimeiface;
    QDBusInterface * datetimeiproperties;

    QMap<QString, int> tzindexMap;

    QTimer * chtimer;

private slots:
    void time_format_clicked_slot(int id);
    void datetime_update_slot();
    void sub_time_update_slot();
    void rsync_with_network_slot(bool status);
    void tz_combobox_changed_slot(int index);
    void longdt_changed_slot(bool status);
    void apply_btn_clicked_slot();
};

#endif // DATETIME_H

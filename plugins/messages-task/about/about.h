#ifndef ABOUT_H
#define ABOUT_H

#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"
#include "../../component/customwidget.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
//#include <QDBusPendingCall>
//#include <QDBusPendingCallWatcher>
//#include <QDBusPendingReply>

#include <QSysInfo>
#include <QLabel>

#define MANUFACTURER "Manufacturer"
#define VERSION "Version"
#define PRODUCTNAME "Product Name"
#define SERIALNUMBER "Serial Number"

namespace Ui {
class About;
}

class About : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    About();
    ~About();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    void initUI();

    void _call_dbus_get_computer_info();
    void _data_init();

private:
    Ui::About *ui;
    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

    QLabel * envlogoLabel;
    QLabel * logoLabel;

    QDBusInterface * interface;

    QString computerinfo;
    QMap<QString, QString> infoMap;

private slots:
    void call_finished_slot(QDBusPendingCallWatcher * call);
};

#endif // ABOUT_H

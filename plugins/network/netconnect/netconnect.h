#ifndef NETCONNECT_H
#define NETCONNECT_H


#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QWidget>
#include <QNetworkInterface>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QtNetwork/QtNetwork>
#include <QProcess>
#include <QToolButton>
#include <QTimer>

#include "../../component/customwidget.h"

enum {
    DISCONNECTED,
    NOINTERNET,
    CONNECTED
};

typedef enum {
    LOOPBACK,
    ETHERNET,
    WIFI
}NetType;

typedef struct _CardInfo{
    QString name;
    NetType type;
    bool status;
}CardInfo;

namespace Ui {
class NetConnect;
}

class NetConnect : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    NetConnect();
    ~NetConnect();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    void refreshUI();
    void acquire_cardinfo();
    void component_init();
    void status_init();
    void begin_timer();
    void reset_lookuphostid();

//protected:
//    void timerEvent(QTimerEvent * event);

private:
    Ui::NetConnect *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

    QNetworkConfigurationManager * nmg;

    QTimer * timer;

    int timerid; //计时器id
    int lookhostid; //hostlookup id

    int netstatus;

    QList<CardInfo> cardinfoQList;

private slots:
    void internet_status_slot(QHostInfo host);
    void internet_status_changed_slot(bool status);
    void run_external_app_slot();

    void updatevalue();
};

#endif // NETCONNECT_H

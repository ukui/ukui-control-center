#ifndef VPN_H
#define VPN_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QProcess>

namespace Ui {
class Vpn;
}

class Vpn : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Vpn();
    ~Vpn();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;

private:
    Ui::Vpn *ui;

    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

private slots:
    void run_external_app_slot();
};

#endif // VPN_H

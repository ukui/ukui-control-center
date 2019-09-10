#ifndef AREA_H
#define AREA_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include "../../component/customwidget.h"
#include "../../component/publicdata.h"

#include <QProcess>

namespace Ui {
class Area;
}

class Area : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Area();
    ~Area();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

private:
    Ui::Area *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

private slots:
    void run_external_app_slot();
};

#endif // AREA_H

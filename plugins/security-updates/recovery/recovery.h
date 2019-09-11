#ifndef RECOVERY_H
#define RECOVERY_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include "../../pluginsComponent/customwidget.h"

namespace Ui {
class Recovery;
}

class Recovery : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Recovery();
    ~Recovery();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

private:
    Ui::Recovery *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;
};

#endif // RECOVERY_H

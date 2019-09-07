#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include "../../component/customwidget.h"

#include <QWidget>
#include <QLabel>

namespace Ui {
class Template;
}

class Template : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Template();
    ~Template() Q_DECL_OVERRIDE;

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

private:
    Ui::Template *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;
};

#endif // TEMPLATE_H

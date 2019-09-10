#ifndef LOGINOPTIONS_H
#define LOGINOPTIONS_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include "../../component/customwidget.h"

namespace Ui {
class LoginOptions;
}

class LoginOptions : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    LoginOptions();
    ~LoginOptions();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

private:
    Ui::LoginOptions *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;
};

#endif // LOGINOPTIONS_H

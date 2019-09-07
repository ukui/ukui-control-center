#ifndef UPDATE_H
#define UPDATE_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include "../../component/customwidget.h"

#include <QProcess>

#include <QDateTime>

namespace Ui {
class Update;
}

class Update : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Update();
    ~Update();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    void ui_init();

private:
    Ui::Update *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

private slots:
    void update_btn_clicked();
    void update_settings_btn_clicked();
};

#endif // UPDATE_H

#ifndef UPDATE_H
#define UPDATE_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

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
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;

private:
    Ui::Update *ui;

    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;
};

#endif // UPDATE_H

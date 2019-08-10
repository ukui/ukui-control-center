#ifndef ABOUT_H
#define ABOUT_H

#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QDBusConnection>
#include "runroot.h"

#include <QSysInfo>
#include <QLabel>

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
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;

    void initUI();

private:
    Ui::About *ui;
    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

    QLabel * logoLabel;
};

#endif // ABOUT_H

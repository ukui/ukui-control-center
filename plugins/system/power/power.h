#ifndef POWER_H
#define POWER_H

#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QWidget>
#include <QGSettings/QGSettings>

namespace Ui {
class Power;
}

class Power : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    explicit Power();
    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    ~Power();

    void component_init();
    void status_init();
    void refreshUI();

private:
    Ui::Power *ui;
    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

    QGSettings * settings;

    QStringList sleepStringList;
    QStringList closeStringList;
    QStringList lidStringList;
    QStringList buttonStringList;

public slots:
//    void icondisplayBtnGroup_changed_slot(int index);
    void powerBtnGroup_changed_slot(int index);

    void ac_sleep_changed_slot(QString value);
    void bat_sleep_changed_slot(QString value);
    void ac_close_changed_slot(QString value);
    void bat_close_changed_slot(QString value);
    void ac_lid_changed_slot(QString value);
    void bat_lid_changed_slot(QString value);
    void power_btn_changed_slot(QString value);
    void suspend_btn_changed_slot(QString value);
};

#endif // POWER_H

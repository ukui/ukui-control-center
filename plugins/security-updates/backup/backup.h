#ifndef BACKUP_H
#define BACKUP_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include "../../component/customwidget.h"

#include <QProcess>

namespace Ui {
class Backup;
}

class Backup : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Backup();
    ~Backup();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

private:
    Ui::Backup *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

private slots:
    void backup_btn_clicked_slot();
};

#endif // BACKUP_H

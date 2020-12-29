#ifndef UPGRADE_H
#define UPGRADE_H

#include <QObject>

#include "shell/interface.h"
#include "src/mainwindow.h"

class Upgrade : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)
public:
    Upgrade();
    ~Upgrade();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type()     Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;

private:
    QString pluginName;
    int pluginType;
    MainWindow *pluginWidget;
    bool mFirstLoad;    
};

#endif // UPGRADE_H

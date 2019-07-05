#ifndef DISPLAY_H
#define DISPLAY_H

#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QGraphicsScene>

namespace Ui {
class DisplayWindow;
}

class Display : public QObject, CommonInterface{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Display();
    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;

    ~Display();

private:
    Ui::DisplayWindow * ui;
    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;
};

#endif // DISPLAY_H

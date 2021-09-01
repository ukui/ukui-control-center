#ifndef GESTURE_H
#define GESTURE_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include <QSettings>
#include "shell/interface.h"
#include "Label/titlelabel.h"
#include "itemwidget.h"
#include "FlowLayout/flowlayout.h"
#include <QDebug>
namespace Ui {
class Gesture;
}

class Gesture : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)


public:
    explicit Gesture();
    ~Gesture();
public:
    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;


    void init();
    void additem();
private:
    Ui::Gesture *ui;
    QWidget * pluginWidget=nullptr;

    QString pluginName;
    int pluginType;
    TitleLabel * currentLabel;
private:
    bool mFirstLoad;
    QSettings *gesturesetting;
    FlowLayout * gifFlowLayout;
};

#endif // GESTURE_H

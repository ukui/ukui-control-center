#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <QObject>
#include <QWidget>

#include "shell/interface.h"
#include "bluetoothmain.h"
#include "bluetoothmainwindow.h"

class Bluetooth : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)
public:
    Bluetooth();
    ~Bluetooth();

    QString plugini18nName()   Q_DECL_OVERRIDE;
    int pluginTypes()       Q_DECL_OVERRIDE;
    QWidget * pluginUi()   Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const          Q_DECL_OVERRIDE;
    bool isEnable() const       Q_DECL_OVERRIDE;

private:
    QString pluginName;
    int pluginType;
    BlueToothMain * pluginWidget;
//    BlueToothMainWindow *pluginWidget;
    bool mFirstLoad;

};

#endif // BLUETOOTH_H

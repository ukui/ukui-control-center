#ifndef UPGRADE_H
#define UPGRADE_H

#include <QObject>

#include "shell/interface.h"
#include "src/upgrademain.h"

class Upgrade : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)
public:
    Upgrade();
    ~Upgrade();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes()     Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    QString pluginName;
    int pluginType;
    UpgradeMain *pluginWidget;
    bool mFirstLoad;    
};

#endif // UPGRADE_H

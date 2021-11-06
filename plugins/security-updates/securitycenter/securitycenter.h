#ifndef SECURITYCENTER_H
#define SECURITYCENTER_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QList>

#include <QMouseEvent>
#include <QLabel>
#include <QTimer>

#include "shell/interface.h"
#include "FlowLayout/flowlayout.h"
#include "ksc_main_page_widget.h"

class SecurityCenter : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    SecurityCenter();
    ~SecurityCenter();

public:
    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    ksc_main_page_widget *ui;

    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

    bool mFirstLoad;

};
#endif // SECURITYCENTER_H

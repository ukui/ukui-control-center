#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>

#include "shell/interface.h"
#include "widget.h"


namespace Ui { class TouchScreen; }


class TouchScreen : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    TouchScreen();
    ~TouchScreen();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget * pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    Ui::TouchScreen *ui;
    QString pluginName;
    int pluginType;
    Widget * pluginWidget;

    bool mFirstLoad;
};
#endif // TOUCHSCREEN_H

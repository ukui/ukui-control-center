#ifndef THEME_H
#define THEME_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"
#include <QToolButton>
#include <QSignalMapper>
#include <QMap>

#include <QGSettings/QGSettings>

#include "../../pluginsComponent/switchbutton.h"
#include "../../pluginsComponent/customwidget.h"

#define INTERFACE_SCHEMA "org.mate.interface"
#define MARCO_SCHEMA "org.gnome.desktop.wm.preferences"

#define GTK_THEME_KEY "gtk-theme"
#define ICON_THEME_KEY "icon-theme"
#define MARCO_THEME_KEY "theme"

namespace Ui {
class Theme;
}

class Theme : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Theme();
    ~Theme();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    void status_init();
    void component_init();
    void refresh_btn_select_status();

private:
    Ui::Theme *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

    QGSettings * ifsettings;
    QGSettings * marcosettings;
    QGSettings * desktopsettings;

    QStringList themeList;
    QMap<QString, QToolButton *> delbtnMap;
    QMap<QString, SwitchButton *> delsbMap;

public slots:
    void set_theme_slots(QString value);
    void desktop_icon_settings_btn_clicked_slots();
    void desktop_icon_settings_slots(QString key);
};

#endif // THEME_H

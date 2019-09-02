#ifndef KEYBOARDCONTROL_H
#define KEYBOARDCONTROL_H

#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QWidget>

#include "kylin-keyboard-interface.h"
#include "kylin-interface-interface.h"
#include <QGSettings/QGSettings>

#include "../../component/switchbutton.h"
//#include "keyboardlayout.h"
#include "kbdlayoutmanager.h"

namespace Ui {
class KeyboardControl;
}

class KeyboardControl : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    KeyboardControl();
    ~KeyboardControl() Q_DECL_OVERRIDE;

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;

    void component_init();
    void status_init();

    void rebuild_layouts_component(QString id);

private:
    Ui::KeyboardControl *ui;

    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

    QGSettings * kbdsettings;

    SwitchButton * keySwitchBtn;
    SwitchButton * cursorSwitchBtn;

//    KeyboardLayout * kbl;
    KbdLayoutManager * layoutmanagerObj;

private slots:
//    void activate_key_repeat_changed_slot(bool status);
    void layout_combobox_changed_slot(int index);
};

#endif // KEYBOARDCONTROL_H

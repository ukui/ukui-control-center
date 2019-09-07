#ifndef DEFAULTAPP_H
#define DEFAULTAPP_H

#include <QObject>
#include <QtPlugin>
#include <QComboBox>
#include <QIcon>

#include "mainui/interface.h"
#include "../../component/customwidget.h"


//libs
#include "kylin-defaultprograms-interface.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
}

namespace Ui {
class DefaultAppWindow;
}

class DefaultApp : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    DefaultApp();
    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    ~DefaultApp();

    void initUI();

private:
    Ui::DefaultAppWindow * ui;
    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

public slots:
    void browserComBoBox_changed_cb(int index);
    void mailComBoBox_changed_cb(int index);
    void imageComBoBox_changed_cb(int index);
    void audioComBoBox_changed_cb(int index);
    void videoComBoBox_changed_cb(int index);
    void textComBoBox_changed_cb(int index);
};

#endif // DEFAULTAPP_H

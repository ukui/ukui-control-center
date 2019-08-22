#ifndef SCREENLOCK_H
#define SCREENLOCK_H

#include <QWidget>
#include <QFileDialog>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include "../../component/switchbutton.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
}


namespace Ui {
class Screenlock;
}

class Screenlock : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Screenlock();
    ~Screenlock();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;

    void component_init();
    void status_init();

private:
    Ui::Screenlock *ui;

    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

    GSettings * bggsettings;

    SwitchButton * setloginbgBtn;

    QSize lockbgSize;

private slots:
    void openpushbtn_clicked_slot();
};

#endif // SCREENLOCK_H

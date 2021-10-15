#include "securitycenter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QProcess>
#include <QDebug>
#include <QFontMetrics>
#include <locale.h>
#include <libintl.h>

SecurityCenter::SecurityCenter() : mFirstLoad(true)
{
    pluginName = tr("Security Center");
    pluginType = SECURITY;

    setlocale(LC_ALL, "");
    bindtextdomain("ksc-defender", "/usr/share/locale");
    bind_textdomain_codeset("ksc-defender", "UTF-8");
    textdomain("ksc-defender");
}

SecurityCenter::~SecurityCenter()
{
    if (!mFirstLoad) {

    }
}

QString SecurityCenter::get_plugin_name(){
    return pluginName;
}

int SecurityCenter::get_plugin_type(){
    return pluginType;
}

QWidget * SecurityCenter::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;

        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui = new ksc_main_page_widget(pluginWidget);

    }else{
        ui->refresh_data();
    }

    return pluginWidget;
}

void SecurityCenter::plugin_delay_control(){

}

const QString SecurityCenter::name() const {

    return QStringLiteral("securitycenter");
}

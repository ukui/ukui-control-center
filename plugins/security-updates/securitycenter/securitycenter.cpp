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

QString SecurityCenter::plugini18nName(){
    return pluginName;
}

int SecurityCenter::pluginTypes(){
    return pluginType;
}

QWidget * SecurityCenter::pluginUi(){
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

const QString SecurityCenter::name() const {

    return QStringLiteral("SecurityCenter");
}

bool SecurityCenter::isShowOnHomePage() const
{
    return true;
}

QIcon SecurityCenter::icon() const
{
    return QIcon();
}

bool SecurityCenter::isEnable() const
{
    return false;
}

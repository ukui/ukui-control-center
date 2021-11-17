#include "ukccplugin.h"

#include <ukcc/widgets/switchbutton.h>

UkccPlugin::UkccPlugin() : mFirstLoad(true)
{
    QTranslator *translator = new QTranslator(this);
    translator->load("/usr/share/plugin-test/translations/" + QLocale::system().name());
    QApplication::installTranslator(translator);

    pluginName = tr("UkccPlugin");
    pluginType = SYSTEM;
}

UkccPlugin::~UkccPlugin()
{

}

QString UkccPlugin::plugini18nName()
{
    return pluginName;
}

int UkccPlugin::pluginTypes()
{
    return pluginType;
}

QWidget *UkccPlugin::pluginUi()
{
    // 需要加上这个判断标志位，否则每次点击都会新建一个QWidget(只加载一次)
    if (mFirstLoad) {
        widget = new QWidget;
        mFirstLoad = false;
    }
    return widget;
}

bool UkccPlugin::isEnable() const
{
    return true;
}

const QString UkccPlugin::name() const
{
    return QStringLiteral("UkccPlugin");
}

bool UkccPlugin::isShowOnHomePage() const
{
    return true;
}

QIcon UkccPlugin::icon() const
{
    return QIcon();
}

QString UkccPlugin::translationPath() const
{
    return "/usr/share/plugin-test/translations/%1.ts";
}

void UkccPlugin::initSearchText()
{
    //~ contents_path /UkccPlugin/UkccPlugin
    tr("UkccPlugin");
    //~ contents_path /UkccPlugin/ukccplugin test
    tr("ukccplugin test");
}


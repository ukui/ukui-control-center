#include "touchscreen.h"
#include "ui_touchscreen.h"


#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/output.h>
#include <QDebug>
#include <QThread>

TouchScreen::TouchScreen() : mFirstLoad(true)
{
    pluginName = tr("TouchScreen");
    pluginType = SYSTEM;
}

TouchScreen::~TouchScreen() {
}

QWidget *TouchScreen::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new Widget;
        QObject::connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
                         [&](KScreen::ConfigOperation *op) {
            QThread::usleep(20000);
            pluginWidget->setConfig(qobject_cast<KScreen::GetConfigOperation*>(op)->config());
        });
    }
    return pluginWidget;
}

QString TouchScreen::plugini18nName() {
    return pluginName;
}

int TouchScreen::pluginTypes() {
    return pluginType;
}

const QString TouchScreen::name() const {

    return QStringLiteral("Touchscreen");
}

QIcon TouchScreen::icon() const
{
    return QIcon();
}

bool TouchScreen::isShowOnHomePage() const
{
    return false;
}

bool TouchScreen::isEnable() const
{
    return true;
}

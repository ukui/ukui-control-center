#include "upgrade.h"

#include <QWidget>
#include <QMainWindow>

Upgrade::Upgrade() :mFirstLoad(true) {
    pluginName = tr("Upgrade");
    pluginType = UPDATE;
}

Upgrade::~Upgrade() {

}

QString Upgrade::plugini18nName() {
    return pluginName;
}

int Upgrade::pluginTypes() {
    return pluginType;
}

QWidget *Upgrade::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;
        // will delete by takewidget
        pluginWidget = new UpgradeMain("");
    }

    return pluginWidget;
}

const QString Upgrade::name() const {
    return QStringLiteral("Upgrade");
}

bool Upgrade::isShowOnHomePage() const
{
    return true;
}

QIcon Upgrade::icon() const
{
    return QIcon();
}

bool Upgrade::isEnable() const
{
    return true;
}

#include "addappdialog.h"
#include "ui_addappdialog.h"

#include <QDebug>

AddAppDialog::AddAppDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddAppDialog)
{
    ui->setupUi(this);

    connect(ui->cancelPushBtn, SIGNAL(clicked(bool)), this, SLOT(close()));

    apps = QDir("/usr/share/applications/");
    QStringList filters;
    filters << "*.desktop";
    apps.setNameFilters(filters);

    foreach (QString name, apps.entryList(QDir::Files)) {
        qDebug() << name;
        QByteArray ba = apps.absoluteFilePath(name).toUtf8();
        GDesktopAppInfo * appinfo = g_desktop_app_info_new_from_filename(ba.constData());
        QString appname = g_app_info_get_name(G_APP_INFO(appinfo));

        const char * iconname = g_icon_to_string(g_app_info_get_icon(G_APP_INFO(appinfo)));
        QIcon appicon;
        if (QIcon::hasThemeIcon(QString(iconname)))
            appicon = QIcon::fromTheme(QString(iconname));

        QListWidgetItem * item = new QListWidgetItem(appicon, appname, ui->appListWidget);
        ui->appListWidget->addItem(item);
    }
}

AddAppDialog::~AddAppDialog()
{
    delete ui;
}

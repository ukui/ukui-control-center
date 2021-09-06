#ifndef OPERATINGMODE_H
#define OPERATINGMODE_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QDebug>
#include <QString>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGSettings>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnection>

#include "shell/interface.h"
#include "SwitchButton/switchbutton.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class OperatingMode;
}
QT_END_NAMESPACE

class OperatingMode : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    explicit OperatingMode();
    ~OperatingMode();

    QString get_plugin_name()   Q_DECL_OVERRIDE;
    int get_plugin_type()       Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui()   Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    void initConnection();
protected:
    bool eventFilter(QObject *w,QEvent *e);

private:
    Ui::OperatingMode *ui;
    QString pluginName;
    int pluginType;
    void initComponent();
    QWidget * pluginWidget;
    QFrame * titleFrame;
    QFrame * modeFrame;
    QFrame * autoFrame;
    QFrame * pcFrame;
    QFrame * padFrame;
    QString cur_mode;
    bool mFirstLoad;

    QDBusInterface *m_statusSessionDbus = nullptr;

    void dbusConnect(bool tablet_mode);
};
#endif // OPERATINGMODE_H

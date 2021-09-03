#ifndef DEVICEINFOITEM_H
#define DEVICEINFOITEM_H

#include "config.h"

#include <QFrame>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/pendingcall.h>
#include <QLabel>
#include <QIcon>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QString>

#include <QGSettings/QGSettings>

//#include "../config/config.h"

class BluezQt::Device;

class DeviceInfoItem : public QFrame
{
    Q_OBJECT
public:
    DeviceInfoItem(QWidget *parent = nullptr,BluezQt::DevicePtr dev = nullptr);
    ~DeviceInfoItem();
    void initInfoPage(QString d_name = "",DEVICE_STATUS status = DEVICE_STATUS::NOT,BluezQt::DevicePtr device = nullptr);
    void refresh_device_icon(BluezQt::Device::Type changeType);

    void setDeviceCurrentStatus();

    enum Status{
        Hover = 0,
        Nomal,
    };
    Q_ENUM(Status)

    void InitMemberVariables();

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

private slots:
    void GSettingsChanges(const QString &key);
    void MenuSignalDeviceFunction(QAction *action);

signals:

    void devPaired(QString);
    void devConnect(QString);
    void devDisconnect(QString);
    void devRemove(QString);
    void devSendFiles(QString);

private:
    QColor getPainterBrushColor();
    QColor getDevStatusColor();
    QPixmap getDevTypeIcon();

    void DrawBackground(QPainter &);
    void DrawStatusIcon(QPainter &);
    void DrawText(QPainter &);
    void DrawStatusText(QPainter &);
    void DrawFuncBtn(QPainter &);

    void MouseClickedFunc();
    void MouseClickedDevFunc();
    void DevConnectFunc();
    void setDeviceConnectSignals();

    int iconFlag = 7;

    DEVICE_STATUS _DevStatus;
    Status        _MStatus;
    QString        devName;

    bool _clicked   ;
    bool _pressFlag ;
    bool _connDevTimeOutFlag ;
    bool _rightFlag ;
    bool _removeDevFlag ;

    QMenu  *dev_Menu         = nullptr;


    QTimer *_iconTimer       = nullptr;
    QTimer *_devConnTimer    = nullptr;
    BluezQt::DevicePtr _MDev = nullptr;

    QGSettings *item_gsettings = nullptr;
};
#endif // DEVICEINFOITEM_H

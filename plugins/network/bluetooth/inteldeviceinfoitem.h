#ifndef DEVICEINFOITEM_H
#define DEVICEINFOITEM_H

#include "config.h"
#include "devicebase.h"
#include "ImageUtil/imageutil.h"

#include <QFrame>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/services.h>
#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/pendingcall.h>
#include <QLabel>
#include <QIcon>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QPoint>
#include <QString>
#include <QRgb>
#include <QColor>
#include <QSize>
#include <QToolButton>
#include <QImage>
#include <QPixmap>
#include <QPushButton>
#include <QFont>
#include <QFontMetrics>
#include <QDateTime>

#include <QGSettings/QGSettings>

class BluezQt::Device;

class IntelDeviceInfoItem : public QFrame
{
    Q_OBJECT
public:
    enum Status{
        Hover = 0,
        Nomal,
        Check,

    };
    Q_ENUM(Status)

    enum DEVSTATUS{
        Connected,
        Connecting,
        DisConnecting,
        ConnectFailed,
        DisConnectFailed,
        NoPaired,
        Paired,
    };
    Q_ENUM(DEVSTATUS)

    IntelDeviceInfoItem(QWidget *parent = nullptr,bluetoothdevice * dev = nullptr);
    ~IntelDeviceInfoItem();
    void refresh_device_icon(bluetoothdevice::DEVICE_TYPE changeType);

    void InitMemberVariables();

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

private slots:
    void GSettingsChanges(const QString &key);
    void MenuSignalDeviceFunction(QAction *action);
    void MouseClickedDevFunc();

signals:

    void devPaired(QString);
    void devConnect(QString);
    void devDisconnect(QString);
    void devRemove(QString);
    void devSendFiles(QString);
    void devConnectionComplete();

private:
    QColor getPainterBrushColor();
    QColor getDevStatusColor();
    QPixmap getDevTypeIcon();

    void DrawBackground(QPainter &);
    void DrawDevTypeIcon(QPainter &);
    void DrawText(QPainter &);
    void DrawStatusText(QPainter &);
    void DrawFuncBtn(QPainter &);

    void MouseClickedFunc();
    void DevConnectFunc();
    void setDeviceConnectSignals();
    bool mouseEventIntargetAera(QPoint);
    QRect getStatusTextRect(QRect);
    QRect getStatusIconRect(QRect);

    QColor getStatusColor(DEVSTATUS);
    QPixmap getDevConnectedIcon(DEVSTATUS,QSize);
    QPixmap convertIconColor(QPixmap,QColor);
    QRect  getFontPixelQPoint(QString);

    QString getDeviceName(QString devName);
    int ShowNameTextNumberMax();
    void TimedRestoreConnectionErrorDisplay();

    bool toptipflag = false;

    int iconFlag = 7;

    DEVSTATUS   _DevStatus = DEVSTATUS::NoPaired;
    Status        _MStatus;
    QString        devName;

    bool _clicked   ;
    bool _pressFlag ;
    bool _connDevTimeOutFlag ;
    bool _pressBtnFlag ;
    bool _removeDevFlag ;
    bool _inBtn = false;
    bool _themeIsBlack = false;
    long long _pressCurrentTime;

    QString _fontFamily;
    int _fontSize = 0;

    QMenu  *dev_Menu         = nullptr;


    QTimer *_iconTimer       = nullptr;
    QTimer *_devConnTimer    = nullptr;
    bluetoothdevice * _MDev = nullptr;

    QGSettings *item_gsettings = nullptr;


    //#define DEV_CONNECTING_TEXT       "Connecting"
    QString m_str_dev_connecting ;
    //#define DEV_DISCONNECTING_TEXT    "Disconnecting"
    QString m_str_dev_disconnecting ;
    //#define DEV_CONNECTED_TEXT        "Connected"
    QString m_str_dev_connected ;
    //#define DEV_UNUNITED_TEXT         "Ununited"
    QString m_str_dev_ununited ;
    //#define DEV_CONNECTION_FAIL_TEXT  "Connect fail"
    QString m_str_dev_conn_fail;
    //#define DEV_CONNECTION_FAIL_TEXT  "Connect fail"
    QString m_str_dev_disconn_fail;

    QToolButton * devFuncBtn = nullptr;

};
#endif // DEVICEINFOITEM_H

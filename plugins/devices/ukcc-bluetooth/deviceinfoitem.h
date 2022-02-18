#ifndef DEVICEINFOITEM_H
#define DEVICEINFOITEM_H

#include "devicebase.h"
#include "config.h"


#include <QFrame>
#include <QLabel>
#include <QIcon>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QFont>
#include <QFontMetrics>
#include <QString>
#include <QGSettings/QGSettings>



class DeviceInfoItem : public QFrame
{
    Q_OBJECT
public:
    DeviceInfoItem(QWidget *parent = nullptr,bluetoothdevice * dev = nullptr);
    ~DeviceInfoItem();

    void refresh_device_icon(bluetoothdevice::DEVICE_TYPE changeType);
    void setDeviceCurrentStatus();

    enum Status{
        Hover = 0,
        Nomal,
        Check,
    };
    Q_ENUM(Status)

    enum DEVSTATUS{
        None,
        Connected,
        Connecting,
        DisConnecting,
        ConnectFailed,
        DisConnectFailed,
        NoPaired,
        Paired,
        Error
    };
    Q_ENUM(DEVSTATUS)

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
    void DrawStatusIcon(QPainter &);
    void DrawText(QPainter &);
    void DrawStatusText(QPainter &);
    void DrawFuncBtn(QPainter &);
    void DrawLoadingIcon(QPainter &);

    void  MouseClickedFunc();
    void  MouseClickedDevFunc();
    bool  mouseEventIntargetAera(QPoint);
    void  DevConnectFunc();
    void  setDeviceConnectSignals();
    QRect getStatusTextRect(QRect);
    QRect getLoadIconRect();
    QRect getStatusIconRect(QRect);
    QRect getFontPixelQPoint(QString);

    //int NameTextWidthMax();
    QString getDeviceName(QString);
    int ShowNameTextNumberMax();
    int NameTextCoordinate_Y_offset();
    void TimedRestoreConnectionErrorDisplay();
    void showDeviceRemoveWidget();
    bool isDisplayPINCodeWidget();

    bool toptipflag = false;
    int iconFlag = 7;
    int m_devConnect_fail_count = 0;

    DEVSTATUS  _DevStatus;
    Status     _MStatus;
    QString    devName;
    QString    _fontFamily;

    int  _fontSize;
    bool _inBtn = false;
    bool _clicked;
    bool _pressFlag ;
    bool _connDevTimeOutFlag ;
    bool _rightFlag ;
    bool _removeDevFlag ;
    bool _pressBtnFlag = false;
    bool _themeIsBlack = false;
    long long _pressCurrentTime;

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
    QString m_str_dev_disconn_fail;
};
#endif // DEVICEINFOITEM_H

#ifndef BLUETOOTHMAIN_H
#define BLUETOOTHMAIN_H

#include "SwitchButton/switchbutton.h"
//#include "TitleLabel/titlelabel.h"
//#include <ukcc/widgets/titlelabel.h>

//#include <polkit-qt5-1/PolkitQt1/Authority>

#include <QGSettings/QGSettings>

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QString>
#include <QDebug>
#include <QLabel>
#include <QFrame>
#include <QSystemTrayIcon>
#include <QFont>
#include <QMetaType>
#include <QSizePolicy>
#include <QWidget>
#include <QScrollArea>
#include <QMenu>
#include <QTimer>
#include <QVariant>
#include <QProcess>
#include <QComboBox>
#include <QStringView>
#include <QLayoutItem>
#include <QMouseEvent>
#include <QMetaEnum>
#include <QList>
#include <QVector>

#include <QStackedWidget>

#include "ukccbluetoothconfig.h"
#include "intelcustomizenamelabel.h"
#include "deviceinfoitem.h"
#include "inteldeviceinfoitem.h"
#include "bluetoothnamelabel.h"
#include "devicebase.h"
#include "config.h"

class BlueToothMain : public QMainWindow
{
    Q_OBJECT
public:
    enum DevTypeShow {
        All = 0,
        Audio,
        Peripherals,
        PC,
        Phone,
        Other,
    };
    Q_ENUM(DevTypeShow)

    BlueToothMain(QWidget *parent = nullptr);
    ~BlueToothMain();
protected:
    void leaveEvent(QEvent *event);

//dev-3.1 界面服务分离

public:
    static bool m_device_operating ;
    static QString m_device_operating_address ;

    static bool m_device_pin_flag ;
    void setbluetoothAdapterDiscoveringStatus(bool);
signals:
    void sendAdapterNameChangeSignal(const QString &value);
    void defaultAdapterNameChanged(const QString &name);

    void adapter_name_changed(const QString &name);
private slots:
    //report
    void reportDefaultAdapterChanged(QString);
    void reportDefaultAdapterNameChanged(QString);
    void reportAdapterAddSignal(QString );
    void reportAdapterRemoveSignal(QString);
    void reportDefaultAdapterPowerChanged(bool);
    void reportDefaultDiscoverableChanged(bool);
    void reportDefaultScanStatusChanged(bool);
    void reportDeviceScanResult(QString,QString);
    //void reportDeviceScanResult(QString,QString,QString,bool.bool);

    void reportDevRemoveSignal(QString);
    void reportDevPairSignal(QString,bool);
    void reportDevConnectStatusSignal(QString,bool);
    void reportDevNameChangedSignal(QString,QString);
    void reportDevTypeChangedSignal(QString,QString);
    void reportRequestConfirmation(QString,QString);
    void reportReplyRequestConfirmation(bool);
    void reportDevOperateErrorSignal(QString,int,QString);

    //receive
    void receiveConnectsignal(QString);
    void receiveDisConnectSignal(QString);
    void receiveRemoveSignal(QString);

    //gsetting
    void setTrayVisible(bool);
    void onClick_Open_Bluetooth(bool);

    void changeDeviceParentWindow(const QString &address);
    void refreshLoadLabelIcon();

    void monitorSleepSlot(bool value);
    void changeListOfDiscoveredDevices(int);
    void receiveSendFileSignal(QString);
    void gSettingsChanged(const QString &);
    void adapterPoweredChanged(bool value);
    void adapterComboxChanged(int i);

    void setTipTextSlot(int);

private:
    QStackedWidget      * _MCentralWidget = nullptr;

    QWidget             * frame_top = nullptr;
    QWidget             * frame_middle = nullptr;
    QWidget             * frame_bottom = nullptr;
    QWidget             * m_normal_main_widget = nullptr;
    QWidget             * m_error_main_widget = nullptr;
    QVBoxLayout         * errorWidgetLayout = nullptr;
    QLabel              * errorWidgetIcon = nullptr;
    QLabel              * errorWidgetTip0 = nullptr;
    QFrame              * line_frame2 = nullptr;
    QFrame              * mDev_frame = nullptr;

    QFrame             * device_list = nullptr;
    //QWidget             * paired_device_list = nullptr;
    QVBoxLayout         * device_list_layout = nullptr;

    //CustomizeNameLabel  * bluetooth_name = nullptr;
    BluetoothNameLabel  * bluetooth_name = nullptr;
    QLabel *tipTextLabel = nullptr;
    QString _tip1;
    QString _tip2;

    QVBoxLayout         * paired_dev_layout = nullptr;
    QVBoxLayout         * main_layout = nullptr;
    QFrame              * frame_2 = nullptr;
    QComboBox           * cacheDevTypeList = nullptr;
    QLabel              * label_2 = nullptr;
    QLabel              * loadLabel = nullptr;

    //QString     Default_Adapter;
    QGSettings * m_settings = nullptr;
    QGSettings * styleSettings = nullptr;

    int         i = 7;
    bool        sleep_status = false;
    bool        m_myDev_show_flag = false;
    bool        isblack = false;


    bool        m_service_dbus_adapter_power_change_flag = false;

    bool        m_current_adapter_power_swtich;
    bool        m_current_adapter_disconvery_swtich;
    bool        m_current_bluetooth_block_status;
    bool        m_current_adapter_scan_status;          //正在扫描：true，否则false

    QString     m_default_adapter_name;
    QString     m_default_adapter_address;

    QStringList m_adapter_name_list;
    QStringList m_adapter_address_list;

    QStringList last_discovery_device_address;
    QStringList m_discovery_device_address_all_list;        //所有device mac list
    //QStringList m_discovery_device_address_effective_list;  //有效device mac list
    //QStringList m_paired_device_address_list;               //已配对device mac list
    //QStringList m_paired_device_name_list;

    //QWidget * m_all_device_list = nullptr;
    //QWidget * m_audio_device_list = nullptr; // include :
    //QWidget * m_other_device_list = nullptr;

    SwitchButton * m_open_bluetooth_btn = nullptr;
    SwitchButton * m_show_panel_btn = nullptr;
    SwitchButton * m_discover_switch_btn = nullptr;

    QComboBox    * m_adapter_list_cmbox = nullptr;
    QComboBox    * m_cache_dev_type_list_cmbox = nullptr;

    bluetoothadapter * m_default_bluetooth_adapter = nullptr ;
    QList <bluetoothadapter *> m_bluetooth_adapter_list;
    //QList <bluetoothdevice  *> m_bluetooth_device_list;

    DevTypeShow discoverDevFlag = DevTypeShow::All;

    QTimer   * m_timer = nullptr;
    QTimer   * delayStartDiscover_timer = nullptr;

private:
    void    mDevFrameAddLineFrame(QString,QString);
    void    removeMDevFrameLineFrame(QString);
    void    monitorBluetoothDbusConnection();
    void    removeDeviceItemUI(QString address);
    void    addMyDeviceItemUI(bluetoothdevice *);
    void    monitorSleepSignal();
    void    cleanPairDevices();
    void    addDiscoverDevListByFlag(DevTypeShow);

    //get adapter data
    void    connectBluetoothServiceSignal();
    void    getAllAdapterData();
    void    getDefaultAdapterData(QString);
    QStringList getAdapterDevAddressList();
    QStringList getAdapterDevNameList();

    QString getDefaultAdapterAddress();
    QString getAdapterName(QString);

    bool    getBluetoothBlock();
    bool    getDefaultAdapterPower();
    bool    getDefaultAdapterDiscoverable();
    bool    getDefaultAdapterScanStatus();

    QStringList getDefaultAdapterCacheDevAddress();
    QString getDevName(QString);
    bluetoothdevice::DEVICE_TYPE getDeviceType(QString,QString);
    QString getDevType(QString);
    bool    getDevPairStatus(QString);
    bool    getDevConnectStatus(QString);
    void    clearAllUnPairedDevicelist();
    //QStringList getAdapterNameList();

    void    setBluetoothBlock(bool);
    void    setDefaultAdapterPower(bool);
    void    setDefaultAdapter(QString);
    void    setDefaultAdapterName(QString);
    void    setDefaultAdapterDiscoverable(bool);
    void    setDefaultAdapterScanOn(bool);

    //
    void    showBluetoothNormalMainWindow();
    void    initMainWindowTopUI();
    void    initMainWindowMiddleUI();
    void    initMainWindowbottomUI();

    void    showBluetoothErrorMainWindow();

    void    refreshUIWhenAdapterChanged();
    void    refreshBluetoothAdapterInterfaceUI();
    bool    isInvalidDevice(QString , bluetoothdevice::DEVICE_TYPE);

    bluetoothdevice  * createOneBluetoothDevice(QString);
    bluetoothadapter * createOneBluetoothAdapter(QString);
    bool whetherToAddCurrentInterface(bluetoothdevice *);
    void addOneBluetoothDeviceItemUi(bluetoothdevice *);

    void addAdapterDataList(QString);
    void removeAdapterDataList(QString);
    void stopAllTimer();
};

#endif // BLUETOOTHMAIN_H

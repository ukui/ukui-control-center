#ifndef BLUETOOTHMAIN_H
#define BLUETOOTHMAIN_H

#include "SwitchButton/switchbutton.h"
#include <string>

#include <polkit-qt5-1/PolkitQt1/Authority>

#include <string>
#include <glib.h>
#include <glib/gprintf.h>

#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/manager.h>
#include <KF5/BluezQt/bluezqt/initmanagerjob.h>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/agent.h>
#include <KF5/BluezQt/bluezqt/pendingcall.h>

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
#include <QStackedWidget>
#include <QLayoutItem>
#include <QMouseEvent>

#include "deviceinfoitem.h"
#include "bluetoothnamelabel.h"

class BlueToothMain : public QMainWindow
{
    Q_OBJECT
public:
    BlueToothMain(QWidget *parent = nullptr);
    ~BlueToothMain();
    void InitMainTopUI();
    void InitMainMiddleUI();
    void InitMainbottomUI();
    void startDiscovery();
    void stopDiscovery();
    void adapterChanged();
    void updateUIWhenAdapterChanged();
    void removeDeviceItemUI(QString address);
    void addMyDeviceItemUI(BluezQt::DevicePtr);
    void MonitorSleepSignal();
    //void showNormalMainWindow();
    void showMainWindowError();
    void showSpeNoteMainWindow();
    void updateAdaterInfoList();
    BluezQt::AdapterPtr getDefaultAdapter();
    void adapterConnectFun();
    void cleanPairDevices();

    bool isSpebluetooth();
protected:
    void leaveEvent(QEvent *event);

signals:
    void adapter_name_changed(const QString &name);
    void timerStatusChanged(bool on);

private slots:
    void onClick_Open_Bluetooth(bool);
    void serviceDiscovered(BluezQt::DevicePtr);
    void serviceDiscoveredChange(BluezQt::DevicePtr);
    void receiveConnectsignal(QString);
    void receiveDisConnectSignal(QString);
    void receiveRemoveSignal(QString);
    void Refresh_load_Label_icon();
    void set_tray_visible(bool);
    void set_discoverable(bool);
    void change_adapter_name(const QString &name);
    void change_device_parent(const QString &address);
    void adapterPoweredChanged(bool value);
    //void delay_adapterPoweredChanged(bool value);
    void adapterComboxChanged(int i);
    void adapterNameChanged(const QString &name);
    void adapterDeviceRemove(BluezQt::DevicePtr ptr);
    void MonitorSleepSlot(bool value);

    void startBluetoothDiscovery();

private:

//============================================================================

    QGSettings *settings = nullptr;
    QString Default_Adapter;
    QStringList paired_device_address;
    QString finally_connect_the_device;
    QStringList Discovery_device_address;
    QStringList last_discovery_device_address;


    QLabel *label_2 = nullptr;
    QLabel *loadLabel = nullptr;

    QPushButton *discover_refresh;
    QTimer *m_timer = nullptr;
    QTimer *discovering_timer =nullptr;
    QTimer *delayStartDiscover_timer =nullptr;
    QTimer *IntermittentScann_timer =nullptr;
    QTimer *poweronAgain_timer =nullptr;
    QTimer *restartDiscover_timer =nullptr;
    int IntermittentScann_timer_count = 0 ;

    int i = 7;
    bool myDevShowFlag = false;
    bool sleep_status = false;

    void clearUiShowDeviceList();
    void addOneBluetoothDeviceItemUi(BluezQt::DevicePtr);

    void clearAllDeviceItemUi();
    void clearAllTimer();


//============================================================================

//new
    QStackedWidget *StackedWidget = nullptr;
    BluezQt::Manager *m_manager = nullptr;
    BluezQt::InitManagerJob  *job = nullptr;
    BluezQt::AdapterPtr m_localDevice = nullptr;

    SwitchButton *open_bluetooth = nullptr;
    SwitchButton *show_panel = nullptr;
    SwitchButton *switch_discover = nullptr;


    QVBoxLayout *main_layout = nullptr;
    QFrame *frame_2 = nullptr;
    QComboBox *adapter_list = nullptr;

    QWidget *normal_main_widget = nullptr;
    QWidget *frame_top = nullptr;
    QWidget *frame_middle = nullptr;
    QVBoxLayout *paired_dev_layout = nullptr;
    QWidget *frame_bottom = nullptr;
    BluetoothNameLabel *bluetooth_name = nullptr;

    QVBoxLayout *bottom_layout = nullptr;
    QScrollArea *device_area = nullptr;
    QWidget * device_list = nullptr;
    QVBoxLayout *device_list_layout = nullptr;

    QStringList adapter_address_list;
    QStringList adapter_name_list;


    QTimer * btPowerBtnTimer = nullptr;

    void InitMainWindowUi();
    void InitMainWindowTopUi();
    void InitMainWindowMiddleUi();
    void InitMainWindowBottomUi();
    void ShowNormalMainWindow();
    void ShowSpecialMainWindow();

    QWidget *errorWidget ;
    void InitMainWindowError();
    void ShowErrorMainWindow();

    void RefreshWindowUiState();
    void RefreshMainWindowTopUi();
    void RefreshMainWindowMiddleUi();
    void RefreshMainWindowBottomUi();

    void InitBluetoothManager();
    //void connectAdapterChanged();
    void connectManagerChanged();
    void addAdapterList(QString newAdapterAddress,QString newAdapterName);
    void removeAdapterList(QString adapterAddress,QString adapterName);
    void InitAllTimer();

};

#endif // BLUETOOTHMAIN_H

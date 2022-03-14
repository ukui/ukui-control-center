#include "bluetoothmain.h"

#include "bluetoothdbusservice.h"
#include "devicebase.h"
#include "ImageUtil/imageutil.h"

#include <QDBusObjectPath>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include <QDBusMessage>
#include <QMessageLogger>
#include <QMetaEnum>

bool BlueToothMain::m_device_operating = false;
QString BlueToothMain::m_device_operating_address = "";
bool BlueToothMain::m_device_pin_flag = false;

//dev-3.1 界面服务-分离
BlueToothMain::BlueToothMain(QWidget *parent)
    : QMainWindow(parent)
{
    this->setAttribute(Qt::WA_StyledBackground,true);
    this->setAttribute(Qt::WA_DeleteOnClose);
    if(QGSettings::isSchemaInstalled("org.ukui.bluetooth"))
    {
        m_settings = new QGSettings("org.ukui.bluetooth");
    }

    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        styleSettings = new QGSettings("org.ukui.style");
        if(styleSettings->get("style-name").toString() == "ukui-black" ||
           styleSettings->get("style-name").toString() == "ukui-dark")
            isblack = true;
        else
            isblack = false;
        connect(styleSettings,&QGSettings::changed,this,&BlueToothMain::gSettingsChanged);
    }

    getAllAdapterData();

    _MCentralWidget = new QStackedWidget(this);
    this->setCentralWidget(_MCentralWidget);
    showBluetoothNormalMainWindow();
    showBluetoothErrorMainWindow();

    qDebug() << "adapter (addresss/name) size:" << m_adapter_address_list.size() << m_adapter_name_list.size();

    if (m_adapter_address_list.size() > 0 && m_adapter_name_list.size() == m_adapter_address_list.size())
    {
        _MCentralWidget->setCurrentWidget(m_normal_main_widget);
        refreshUIWhenAdapterChanged();
    }
    else
    {
        _MCentralWidget->setCurrentWidget(m_error_main_widget);
    }
    monitorBluetoothDbusConnection();
}

BlueToothMain::~BlueToothMain()
{
    //rfkill_exit();
    setDefaultAdapterScanOn(false);
    clearAllUnPairedDevicelist();

    delete m_settings;
    m_settings = nullptr;
    //delete paired_device_list;
    //paired_device_list = nullptr;
    delete device_list;
    device_list = nullptr;

}

void BlueToothMain::leaveEvent(QEvent *event)
{
    //qDebug() << Q_FUNC_INFO;
}

void BlueToothMain::monitorBluetoothDbusConnection()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    //    bool connect(const QString &service, const QString &path, const QString &interface,
    //                                         const QString &name, QObject *receiver, const char *slot);
    //1.适配器数据上报

    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "defaultAdapterChanged",this, SLOT(reportDefaultAdapterChanged(QString)));

    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "defaultAdapterNameChanged",this, SLOT(reportDefaultAdapterNameChanged(QString)));

    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "adapterAddSignal",this, SLOT(reportAdapterAddSignal(QString)));
    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "adapterRemoveSignal",this, SLOT(reportAdapterRemoveSignal(QString)));

    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "defaultAdapterPowerChanged",this, SLOT(reportDefaultAdapterPowerChanged(bool)));
    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "defaultDiscoverableChanged",this, SLOT(reportDefaultDiscoverableChanged(bool)));

    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "defaultScanStatusChanged",this, SLOT(reportDefaultScanStatusChanged(bool)));

    //2.蓝牙设备数据上报
    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "deviceScanResult",this, SLOT(reportDeviceScanResult(QString,QString)));

    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "devPairSignal",this, SLOT(reportDevPairSignal(QString,bool)));

    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "devConnectStatusSignal",this, SLOT(reportDevConnectStatusSignal(QString,bool)));

    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "devRemoveSignal",this, SLOT(reportDevRemoveSignal(QString)));

    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "devNameChangedSignal",this, SLOT(reportDevNameChangedSignal(QString,QString)));

    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "devTypeChangedSignal",this, SLOT(reportDevTypeChangedSignal(QString,QString)));

    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "requestConfirmation",this, SLOT(reportRequestConfirmation(QString,QString)));

    //PIN窗口是否操作
    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "replyRequestConfirmation",this, SLOT(reportReplyRequestConfirmation(bool)));



    QDBusConnection::sessionBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "devOperateErrorSignal",this, SLOT(reportDevOperateErrorSignal(QString,int,QString)));

}

//adapter report start
void BlueToothMain::reportDefaultAdapterChanged(QString )
{
    qDebug () << Q_FUNC_INFO << __LINE__;

}

void BlueToothMain::reportDefaultAdapterNameChanged(QString adapter_name)
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    if(nullptr != m_default_bluetooth_adapter)
    {
        m_default_bluetooth_adapter->resetDeviceName(adapter_name);
        for(bluetoothadapter * adapter_dev : m_bluetooth_adapter_list)
        {
            if (adapter_dev->getDevAddress() == m_default_bluetooth_adapter->getDevAddress())
            {
                adapter_dev->resetDeviceName(adapter_name);
                break;
            }
        }
        emit defaultAdapterNameChanged(adapter_name);
    }
}

void BlueToothMain::reportDefaultDiscoverableChanged(bool discoverable)
{
    qDebug () << Q_FUNC_INFO << discoverable << __LINE__;
    m_current_adapter_disconvery_swtich = discoverable;
    m_discover_switch_btn->setChecked(discoverable);
}

void BlueToothMain::reportDefaultScanStatusChanged(bool isScan)
{
    qDebug () << Q_FUNC_INFO << isScan << __LINE__;
    m_current_adapter_scan_status = isScan;

    if (isScan)
    {
        if(!loadLabel->isVisible())
            loadLabel->setVisible(true);
        if (!m_timer->isActive())
            m_timer->start();

        if(delayStartDiscover_timer->isActive())
            delayStartDiscover_timer->stop();
    }
    else
    {
        if(loadLabel->isVisible())
            loadLabel->setVisible(false);
        if (m_timer->isActive())
            m_timer->stop();
    }
}

bluetoothadapter * BlueToothMain::createOneBluetoothAdapter(QString adapter_address)
{
    qDebug () << Q_FUNC_INFO << adapter_address << __LINE__;
    bluetoothadapter * adapter_dev = nullptr ;

    //将等待值修改为300ms
    //建议基于事件定时获取，防止出现异常
    QString adapter_dev_name = getAdapterName(adapter_address);
    qDebug() << Q_FUNC_INFO << adapter_dev_name << __LINE__;
    if (adapter_dev_name.isEmpty())
    {
        if (adapter_dev_name.isEmpty())
        {
            qDebug() << Q_FUNC_INFO << "name is null!" << __LINE__;
            adapter_dev->deleteLater();
            return nullptr;
        }
    }

    //设备列表已经存在该设备
    for (bluetoothadapter * tmpDev : m_bluetooth_adapter_list)
    {
        if (adapter_address == tmpDev->getDevAddress())
        {
            qDebug() << Q_FUNC_INFO << "device existing!" << __LINE__;
            adapter_dev->deleteLater();
            return nullptr;
        }
    }

    adapter_dev = new bluetoothadapter(adapter_dev_name,
                                       adapter_address,
                                       false,
                                       false,
                                       false);

    qDebug () << Q_FUNC_INFO << adapter_address << "create ok!" << __LINE__;
    return adapter_dev;
}


void BlueToothMain::refreshBluetoothAdapterInterfaceUI()
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    qDebug () << Q_FUNC_INFO << m_adapter_address_list << m_adapter_name_list <<  __LINE__;

    if(1 == m_adapter_address_list.size())// ||
      //(m_adapter_address_list.size() > 0 && m_adapter_address_list.size() != m_adapter_name_list.size()))
    {
        //m_default_bluetooth_adapter = m_bluetooth_adapter_list.at(0);
        if("normalWidget" != _MCentralWidget->currentWidget()->objectName())
        {
            _MCentralWidget->setCurrentWidget(m_normal_main_widget);
        }
        if(frame_2->isVisible()) {
            frame_2->setVisible(false);
            line_frame2->setVisible(false);
        }

//#ifdef DEVICE_IS_INTEL
        if(ukccbluetoothconfig::m_isIntel)
        {
            frame_top->setMinimumSize(582,153);
            frame_top->setMaximumSize(1800,153);
        }
        else
        {
            frame_top->setMinimumSize(582,217);
            frame_top->setMaximumSize(1800,217);
        }
    }
    else if (m_adapter_address_list.size() > 1)
    {
        if("normalWidget" != _MCentralWidget->currentWidget()->objectName())
        {
            _MCentralWidget->setCurrentWidget(m_normal_main_widget);
        }
        if(!frame_2->isVisible())
            frame_2->setVisible(true);
//#ifdef DEVICE_IS_INTEL
        if(ukccbluetoothconfig::m_isIntel)
        {
            frame_top->setMinimumSize(582,215);
            frame_top->setMaximumSize(1800,215);
        }
        else
        {
            frame_top->setMinimumSize(582,279);
            frame_top->setMaximumSize(1800,279);
        }
//#endif
    }
    else
    {
        if("errorWidget" != _MCentralWidget->currentWidget()->objectName())
        {
            _MCentralWidget->setCurrentWidget(m_error_main_widget);
        }
    }

    refreshUIWhenAdapterChanged();
}

//void BlueToothMain::addBluetoothAdapterComboBox(QString adapterName)
//{

//}

//void BlueToothMain::removeBluetoothAdapterComboBox(int index)
//{

//}

void BlueToothMain::addAdapterDataList(QString adapterAddress)
{
    qDebug () << Q_FUNC_INFO  << adapterAddress << __LINE__;

    bluetoothadapter * adapter_dev = createOneBluetoothAdapter(adapterAddress);
    if (nullptr == adapter_dev)
    {
        qDebug () << Q_FUNC_INFO  << "adapter dev not add!" << __LINE__;
        return;
    }
    qDebug () << Q_FUNC_INFO  << "what's =======?" << __LINE__;
    qDebug () << Q_FUNC_INFO  << m_adapter_address_list << m_adapter_name_list << __LINE__;

    if (m_adapter_address_list.indexOf(adapterAddress) == -1)
    {
        m_adapter_address_list << adapterAddress;
        m_adapter_name_list << getAdapterName(adapterAddress);

        if(m_adapter_list_cmbox)
            m_adapter_list_cmbox->addItem(m_adapter_name_list.last());
    }
    qDebug () << Q_FUNC_INFO  << m_adapter_address_list << m_adapter_name_list << __LINE__;


    m_bluetooth_adapter_list.append(adapter_dev);
}

void BlueToothMain::removeAdapterDataList(QString adapterAddress)
{
    qDebug () << Q_FUNC_INFO  << adapterAddress << __LINE__;
    int i = m_adapter_address_list.indexOf(adapterAddress);
    if (-1 == i)
    {
        qDebug () << Q_FUNC_INFO  << "Not device :" << adapterAddress << __LINE__;
        return;
    }
    qDebug() << Q_FUNC_INFO << __LINE__ << m_adapter_address_list << m_adapter_name_list << i;
    m_adapter_name_list.removeAt(i);
    m_adapter_address_list.removeAll(adapterAddress);
    m_bluetooth_adapter_list.removeAt(i);

    //if (m_adapter_address_list.size())
    //{
    disconnect(m_adapter_list_cmbox,SIGNAL(currentIndexChanged(int)),NULL,NULL);
    if(m_adapter_list_cmbox != nullptr)
    {
        m_adapter_list_cmbox->clear();
        m_adapter_list_cmbox->addItems(m_adapter_name_list);
        //m_adapter_list_cmbox->removeItem(i);
        connect(m_adapter_list_cmbox,SIGNAL(currentIndexChanged(int)),this,SLOT(adapterComboxChanged(int)));
    }
    //}
}
void BlueToothMain::reportAdapterAddSignal(QString adapterAddress)
{
    qDebug () << Q_FUNC_INFO  << adapterAddress << __LINE__;
//    if("normalWidget" != this->centralWidget()->objectName())
//    {
//        _MCentralWidget->setCurrentWidget(m_normal_main_widget);
//    }

    addAdapterDataList(adapterAddress);
    refreshBluetoothAdapterInterfaceUI();
}

void BlueToothMain::reportAdapterRemoveSignal(QString adapterAddress)
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    removeAdapterDataList(adapterAddress);
    refreshBluetoothAdapterInterfaceUI();
}

void BlueToothMain::reportDefaultAdapterPowerChanged(bool isPower)
{
    qDebug () << Q_FUNC_INFO << isPower << __LINE__;
    if("normalWidget" != _MCentralWidget->currentWidget()->objectName())
    {
        qDebug () << Q_FUNC_INFO << "is not normalWidget" << __LINE__;
        return;
    }

    m_current_adapter_power_swtich = isPower;

    if (isPower && !m_open_bluetooth_btn->isChecked())
    {
        m_service_dbus_adapter_power_change_flag = true;
        m_open_bluetooth_btn->setChecked(true);
    }
    if (!isPower && m_open_bluetooth_btn->isChecked())
    {
        m_service_dbus_adapter_power_change_flag = true;
        m_open_bluetooth_btn->setChecked(false);
    }
}


//device report start
void BlueToothMain::reportDevPairSignal(QString deviceAddress,bool isPaired)
{
    qDebug() << Q_FUNC_INFO << deviceAddress << "isPaired:"<< isPaired << __LINE__;
    for (bluetoothdevice * dev :m_default_bluetooth_adapter->m_bluetooth_device_list)
    {
        qDebug() << Q_FUNC_INFO << dev->getDevAddress() << __LINE__;
        if (deviceAddress == dev->getDevAddress())
        {
            dev->devPairedChanged(isPaired);
            return;
        }
    }
}

void BlueToothMain::reportDevConnectStatusSignal(QString deviceAddress,bool isConnected)
{
    qDebug() << Q_FUNC_INFO << deviceAddress << "isConnected:"<< isConnected << __LINE__;
    for (bluetoothdevice * dev :m_default_bluetooth_adapter->m_bluetooth_device_list)
    {
        if (deviceAddress == dev->getDevAddress())
        {
            dev->devConnectedChanged(isConnected);
            return;
        }
    }
}

void BlueToothMain::reportDevRemoveSignal(QString deviceAddress)
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    removeDeviceItemUI(deviceAddress);

    if(!m_device_operating_address.isNull() &&
        m_device_operating &&
       (m_device_operating_address == deviceAddress))
    {
        m_device_operating = false;
        m_device_operating_address.clear();
    }
    for (int i = 0 ; i < m_default_bluetooth_adapter->m_bluetooth_device_list.size() ; i++)
    {
        if(deviceAddress == m_default_bluetooth_adapter->m_bluetooth_device_list.at(i)->getDevAddress())
        {
            //m_default_bluetooth_adapter->m_bluetooth_device_list.removeAll(m_default_bluetooth_adapter->m_bluetooth_device_list.at(i));
            m_default_bluetooth_adapter->m_bluetooth_device_list.removeAt(i);
            break ;
        }
    }
}

void BlueToothMain::reportDevNameChangedSignal(QString deviceAddress,QString changedName)
{
    qDebug() << Q_FUNC_INFO << deviceAddress << "changedName:"<< changedName << __LINE__;
    for (bluetoothdevice * dev :m_default_bluetooth_adapter->m_bluetooth_device_list)
    {
        if (deviceAddress == dev->getDevAddress())
        {
            dev->resetDeviceName(changedName);
            return;
        }
    }
}

void BlueToothMain::reportDevTypeChangedSignal(QString deviceAddress,QString changedType)
{
    qDebug() << Q_FUNC_INFO << deviceAddress << "changedType:"<< changedType << __LINE__;
    for (bluetoothdevice * dev :m_default_bluetooth_adapter->m_bluetooth_device_list)
    {
        if (deviceAddress == dev->getDevAddress())
        {
            bluetoothdevice::DEVICE_TYPE devType = getDeviceType(deviceAddress,changedType);
            dev->setDevType(devType);
            return;
        }
    }
}


void BlueToothMain::reportRequestConfirmation(QString deviceName,QString disPinCode)
{
    qDebug() << Q_FUNC_INFO << deviceName << disPinCode << __LINE__;
    m_device_pin_flag = true;
}
void BlueToothMain::reportReplyRequestConfirmation(bool userCheck)
{
    qDebug() << Q_FUNC_INFO << "User check：" << userCheck << __LINE__;
    m_device_pin_flag = false;
}

void BlueToothMain::reportDevOperateErrorSignal(QString deviceAddress , int ErrorId , QString ErrorText)
{
    qDebug() << Q_FUNC_INFO << deviceAddress << ErrorId << ErrorText << __LINE__;
    //m_device_pin_flag = false;
    for (bluetoothdevice * dev :m_default_bluetooth_adapter->m_bluetooth_device_list)
    {
        if (deviceAddress == dev->getDevAddress())
        {
            dev->setErrorInfo(ErrorId,ErrorText);
            return;
        }
    }
}

bluetoothdevice * BlueToothMain::createOneBluetoothDevice(QString devAddress)
{
    qDebug() << Q_FUNC_INFO << devAddress << __LINE__;
    bluetoothdevice * dev = nullptr ;

    QString dev_name = getDevName(devAddress);
    qDebug() << Q_FUNC_INFO << dev_name << __LINE__;
    if (dev_name.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << "name is null!" << __LINE__;
        dev->deleteLater();
        return nullptr;
    }

    bluetoothdevice::DEVICE_TYPE dev_type = getDeviceType(devAddress,"");
    bool isPaired = getDevPairStatus(devAddress);
    bool isConnected = getDevConnectStatus(devAddress);
    qDebug() << Q_FUNC_INFO << devAddress << isPaired << isConnected << __LINE__;

    if(isInvalidDevice(dev_name,dev_type))
    {
        qDebug() << Q_FUNC_INFO << "isInvalidDevice!" << __LINE__;
        dev->deleteLater();
        return nullptr;
    }

    //设备列表已经存在该设备
    for (bluetoothdevice * tmpDev : m_default_bluetooth_adapter->m_bluetooth_device_list)
    {
        if (devAddress == tmpDev->getDevAddress())
        {
            qDebug() << Q_FUNC_INFO << "device existing!" << __LINE__;
            dev->deleteLater();
            return nullptr;
        }
    }

    dev = new bluetoothdevice(dev_name,
                              devAddress,
                              dev_type,
                              isPaired,
                              isConnected,
                              isPaired);



    return dev;
}

void BlueToothMain::reportDeviceScanResult(QString devAddress,QString devName)
{

    if ("normalWidget" != _MCentralWidget->currentWidget()->objectName())
    {
        qDebug() << Q_FUNC_INFO << "errorWidget not add device!!" << __LINE__;
        return;
    }

    qDebug () << Q_FUNC_INFO << devName << devAddress << __LINE__;
    qDebug () << Q_FUNC_INFO << devName << "m_current_adapter_scan_status =========" << m_current_adapter_scan_status << __LINE__;

    //当前是扫描状态，且扫描延时也在计时
    if (m_current_adapter_scan_status && delayStartDiscover_timer && delayStartDiscover_timer->isActive())
    {
        delayStartDiscover_timer->stop();
    }
    //设备列表已经存在该设备,添加到界面中去
    for (bluetoothdevice * tmpDev : m_default_bluetooth_adapter->m_bluetooth_device_list)
    {
        if (devAddress == tmpDev->getDevAddress())
        {
            if(whetherToAddCurrentInterface(tmpDev))
            {
                addOneBluetoothDeviceItemUi(tmpDev);
                return;
            }
        }
    }

    bluetoothdevice * device = createOneBluetoothDevice(devAddress);
    if (device == nullptr)
    {
        qDebug () << Q_FUNC_INFO << "device not add!";
        return;
    }

    qDebug() << "effective device";
    device->setObjectName(devAddress);
    m_default_bluetooth_adapter->m_bluetooth_device_list.append(device);
    if(device->isPaired())
    {
        this->addMyDeviceItemUI(device);
    }
    else
    {
        if(this->whetherToAddCurrentInterface(device))
            this->addOneBluetoothDeviceItemUi(device);
    }
}

void BlueToothMain::addMyDeviceItemUI(bluetoothdevice * device)
{
    qDebug() << __FUNCTION__ << device->getDevName() << device->getDevAddress() << device->getDevType() << __LINE__;
//#ifdef DEVICE_IS_INTEL
//    IntelDeviceInfoItem *item  = frame_middle->findChild<IntelDeviceInfoItem *>(device->getDevAddress());
//#else
    DeviceInfoItem *item  = frame_middle->findChild<DeviceInfoItem *>(device->getDevAddress());
//#endif
    if (item)
    {
        m_myDev_show_flag = true;
        qDebug () << Q_FUNC_INFO << "device is exist" << __LINE__;
        return;
    }

    if (device && device->isPaired()) {

        mDevFrameAddLineFrame("paired",device->getDevAddress());

        //IntelDeviceInfoItem *item = new IntelDeviceInfoItem(this,device);
        DeviceInfoItem *item = new DeviceInfoItem(this,device);
        item->setObjectName(device->getDevAddress());
        connect(item,SIGNAL(devPaired(QString)),this,SLOT(changeDeviceParentWindow(QString)));
        connect(item,SIGNAL(devConnect(QString)),this,SLOT(receiveConnectsignal(QString)));
        connect(item,SIGNAL(devDisconnect(QString)),this,SLOT(receiveDisConnectSignal(QString)));
        connect(item,SIGNAL(devRemove(QString)),this,SLOT(receiveRemoveSignal(QString)));
        connect(item,SIGNAL(devSendFiles(QString)),this,SLOT(receiveSendFileSignal(QString)));
        //connect(item,&IntelDeviceInfoItem::devConnectionComplete,this,[=]
        connect(item,&DeviceInfoItem::devConnectionComplete,this,[=]
        {
            m_device_operating = false;
            m_device_operating_address.clear();
            cacheDevTypeList->setEnabled(true);
        });
        m_myDev_show_flag = true;

        paired_dev_layout->addWidget(item,Qt::AlignTop);
    }
    return;
}
bool BlueToothMain::whetherToAddCurrentInterface(bluetoothdevice * device)
{
    switch (discoverDevFlag) {
    case DevTypeShow::All:
        return true;
    case DevTypeShow::Audio:
        if (device->getDevType() == bluetoothdevice::DEVICE_TYPE::headset ||
            device->getDevType() == bluetoothdevice::DEVICE_TYPE::headphones ||
            device->getDevType() == bluetoothdevice::DEVICE_TYPE::audiovideo) {
            return true;
        } else {
            return false;
        }
        break;
    case DevTypeShow::Peripherals:
        if (device->getDevType() == bluetoothdevice::DEVICE_TYPE::mouse ||
            device->getDevType() == bluetoothdevice::DEVICE_TYPE::keyboard) {
            return true;
        } else {
            return false;
        }
        break;
    case DevTypeShow::PC:
        if (device->getDevType() == bluetoothdevice::DEVICE_TYPE::computer) {
            return true;
        } else {
            return false;
        }
        break;
    case DevTypeShow::Phone:
        if (device->getDevType() == bluetoothdevice::DEVICE_TYPE::phone) {
            return true;
        } else {
            return false;
        }
        break;
    default:
        if (device->getDevType() == bluetoothdevice::DEVICE_TYPE::headset ||
            device->getDevType() == bluetoothdevice::DEVICE_TYPE::headphones ||
            device->getDevType() == bluetoothdevice::DEVICE_TYPE::audiovideo ||
            device->getDevType() == bluetoothdevice::DEVICE_TYPE::phone ||
            device->getDevType() == bluetoothdevice::DEVICE_TYPE::mouse ||
            device->getDevType() == bluetoothdevice::DEVICE_TYPE::keyboard ||
            device->getDevType() == bluetoothdevice::DEVICE_TYPE::computer) {
            return false;
        }
        return true;
    }
}

void BlueToothMain::addOneBluetoothDeviceItemUi(bluetoothdevice * device)
{
    //IntelDeviceInfoItem * item  = frame_middle->findChild<IntelDeviceInfoItem *>(device->getDevAddress());
    DeviceInfoItem * item  = frame_middle->findChild<DeviceInfoItem *>(device->getDevAddress());
    if (item)
    {
        qDebug () << Q_FUNC_INFO << "device is exist" << __LINE__;
        return;
    }

    //item = device_list->findChild<IntelDeviceInfoItem *>(device->getDevAddress());
    item = device_list->findChild<DeviceInfoItem *>(device->getDevAddress());
    if (item)
    {
        qDebug () << Q_FUNC_INFO << "device is exist" << __LINE__;
        return ;
    }


    //item = new IntelDeviceInfoItem(device_list,device);
    item = new DeviceInfoItem(device_list,device);
    item->setObjectName(device->getDevAddress());
    connect(item,SIGNAL(devPaired(QString)),this,SLOT(changeDeviceParentWindow(QString)));
    connect(item,SIGNAL(devConnect(QString)),this,SLOT(receiveConnectsignal(QString)));
    connect(item,SIGNAL(devDisconnect(QString)),this,SLOT(receiveDisConnectSignal(QString)));
    connect(item,SIGNAL(devRemove(QString)),this,SLOT(receiveRemoveSignal(QString)));
    connect(item,SIGNAL(devSendFiles(QString)),this,SLOT(receiveSendFileSignal(QString)));
    //connect(item,&IntelDeviceInfoItem::devConnectionComplete,this,[=]
    connect(item,&DeviceInfoItem::devConnectionComplete,this,[=]
    {
        m_device_operating = false;
        m_device_operating_address.clear();
        cacheDevTypeList->setEnabled(true);
//        if (delayStartDiscover_timer->isActive())
//            delayStartDiscover_timer->stop();
//        delayStartDiscover_timer->start();
    });

    mDevFrameAddLineFrame("other",device->getDevAddress());

    if (!device_list->isVisible())
        device_list->setVisible(true);

    device_list_layout->insertWidget(0,item,0,Qt::AlignTop);

    last_discovery_device_address << device->getDevAddress();
}

void BlueToothMain::getDefaultAdapterData(QString adapter_address)
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    m_current_bluetooth_block_status    = getBluetoothBlock();
    m_default_adapter_name              = getAdapterName(adapter_address);
    m_current_adapter_power_swtich      = getDefaultAdapterPower();
    m_current_adapter_scan_status       = getDefaultAdapterScanStatus();
    //当前扫描状态是开启，则关闭扫描状态，并清除所有未匹配的蓝牙设备，
    if (m_current_adapter_scan_status)
    {
        setDefaultAdapterScanOn(false);
        clearAllUnPairedDevicelist();
    }

    m_current_adapter_disconvery_swtich = getDefaultAdapterDiscoverable();


    m_default_bluetooth_adapter         = new bluetoothadapter(m_default_adapter_name,
                                                               m_default_adapter_address,
                                                               m_current_adapter_power_swtich,
                                                               m_current_adapter_scan_status,
                                                               m_current_adapter_disconvery_swtich);
    m_default_bluetooth_adapter->m_bluetooth_device_list.clear();
    m_discovery_device_address_all_list = getDefaultAdapterCacheDevAddress();
    //qDebug () << Q_FUNC_INFO << "m_discovery_device_address_all_list:" <<  m_discovery_device_address_all_list ;
    for (auto deviceAddress: m_discovery_device_address_all_list)
    {
        bluetoothdevice * dev = createOneBluetoothDevice(deviceAddress);
        if (nullptr == dev)
            continue ;
        dev->setObjectName(deviceAddress);
        m_default_bluetooth_adapter->m_bluetooth_device_list.append(dev);
    }
    qDebug () << Q_FUNC_INFO << "end" << __LINE__;

}

void BlueToothMain::connectBluetoothServiceSignal()
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    iface.setTimeout(20);
    iface.call("connectSignal");
}

void BlueToothMain::getAllAdapterData()
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    m_default_bluetooth_adapter = nullptr;

    m_default_adapter_address           = getDefaultAdapterAddress();
    if(m_default_adapter_address.isEmpty())
    {
        m_default_bluetooth_adapter = nullptr;
        return;
    }

    getDefaultAdapterData(m_default_adapter_address);

    qDebug () << Q_FUNC_INFO << "m_default_bluetooth_adapter->m_bluetooth_device_list.size():" << m_default_bluetooth_adapter->m_bluetooth_device_list.size();

    m_adapter_address_list              = getAdapterDevAddressList();
    m_adapter_name_list                 = getAdapterDevNameList();

    //m_adapter_address_list << m_default_adapter_address;
    //m_adapter_name_list << m_default_adapter_name;

    qDebug () << Q_FUNC_INFO << "m_adapter_address_list size :" << m_adapter_address_list.size() << __LINE__;
    for(QString adapterAddress:m_adapter_address_list)
    {
        QString adapterName = getAdapterName(adapterAddress);
        if(adapterAddress == m_default_adapter_address)
            m_bluetooth_adapter_list.append(m_default_bluetooth_adapter);
        else
        {
            bluetoothadapter * adapterdev = new bluetoothadapter(adapterName,adapterAddress,false,false,false);
            m_bluetooth_adapter_list.append(adapterdev);
        }
        //m_adapter_address_list << adapterAddress;
        //m_adapter_name_list << adapterName;
    }

    qDebug () << Q_FUNC_INFO << "m_current_bluetooth_block_status: " << m_current_bluetooth_block_status << __LINE__;
    qDebug () << Q_FUNC_INFO << "m_default_adapter_address: " << m_default_adapter_address << __LINE__;
    qDebug () << Q_FUNC_INFO << "m_adapter_address_list: " << m_adapter_address_list << __LINE__;
    qDebug () << Q_FUNC_INFO << "m_default_adapter_name: " << m_default_adapter_name << __LINE__;
    qDebug () << Q_FUNC_INFO << "m_adapter_name_list: " << m_adapter_name_list << __LINE__;
    qDebug () << Q_FUNC_INFO << "m_current_adapter_power_swtich: " << m_current_adapter_power_swtich << __LINE__;
    qDebug () << Q_FUNC_INFO << "m_current_adapter_disconvery_swtich: " << m_current_adapter_disconvery_swtich << __LINE__;
    qDebug () << Q_FUNC_INFO << "m_current_adapter_scan_status: " << m_current_adapter_scan_status << __LINE__;
}

bool BlueToothMain::getBluetoothBlock()
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间
    iface.setTimeout(100);
    QDBusReply<bool> reply=iface.call("getBluetoothBlock");
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;
    return reply;
}

QString BlueToothMain::getDefaultAdapterAddress()
{
    qDebug () << Q_FUNC_INFO << __LINE__;

    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间
    iface.setTimeout(100);
    QDBusReply<QString> reply=iface.call("getDefaultAdapterAddress");
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;
    return reply;
}

QStringList BlueToothMain::getAdapterDevAddressList()
{
    qDebug () << Q_FUNC_INFO << __LINE__;

    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间
    iface.setTimeout(100);
    QDBusReply<QStringList> reply=iface.call("getAdapterDevAddressList");
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;
    return reply;
}

QStringList BlueToothMain::getAdapterDevNameList()
{
    qDebug () << Q_FUNC_INFO << __LINE__;

    QStringList adapterNameList ;
    adapterNameList.clear();
    for (QString adapterAddress : m_adapter_address_list)
    {
        adapterNameList << getAdapterName(adapterAddress);
    }
    qDebug () << Q_FUNC_INFO << adapterNameList << __LINE__;
    return adapterNameList;
}

QString BlueToothMain::getAdapterName(QString adapterAddress)
{
    qDebug () << Q_FUNC_INFO << __LINE__;

    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间, 从20ms增加300ms,减少出现异常的几率
    iface.setTimeout(300);
    QDBusReply<QString> reply=iface.call("getAdapterNameByAddr", adapterAddress);
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;
    return reply;
}

bool BlueToothMain::getDefaultAdapterPower()
{
    qDebug () << Q_FUNC_INFO << __LINE__;

    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间 ms
    iface.setTimeout(100);
    QDBusReply<bool> reply=iface.call("getDefaultAdapterPower");
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;
    return reply;
}

QStringList BlueToothMain::getDefaultAdapterCacheDevAddress()
{
    qDebug () << Q_FUNC_INFO << __LINE__;

    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间, ms
    iface.setTimeout(100);
    QDBusReply<QStringList> reply=iface.call("getDefaultAdapterCacheDevAddress");
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;
    return reply;
}

bool BlueToothMain::getDefaultAdapterDiscoverable()
{
    qDebug () << Q_FUNC_INFO << __LINE__;

    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间
    iface.setTimeout(100);
    QDBusReply<bool> reply=iface.call("getDefaultAdapterDiscoverable");
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;
    return reply;
}

bool BlueToothMain::getDefaultAdapterScanStatus()
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间,从20ms延迟至100ms
    iface.setTimeout(100);
    QDBusReply<bool> reply=iface.call("getDefaultAdapterScanStatus");
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;
    return reply;
}


QString BlueToothMain::getDevName(QString deviceAddress)
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间,从20ms延长至300ms
    iface.setTimeout(300);
    QDBusReply<QString> reply=iface.call("getDevName",deviceAddress);
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;
    return reply;
}

bool BlueToothMain::getDevPairStatus(QString deviceAddress)
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间
    iface.setTimeout(100);
    QDBusReply<bool> reply=iface.call("getDevPairStatus",deviceAddress);
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;
    return reply;
}
bool BlueToothMain::getDevConnectStatus(QString deviceAddress)
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间
    iface.setTimeout(100);
    QDBusReply<bool> reply=iface.call("getDevConnectStatus",deviceAddress);
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;
    return reply;
}

bluetoothdevice::DEVICE_TYPE BlueToothMain::getDeviceType(QString deviceAddress , QString devType)
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    QString dev_type_str = devType ;
    if (dev_type_str.isEmpty())
        dev_type_str = getDevType(deviceAddress);

    qDebug() << Q_FUNC_INFO << "Type:" << dev_type_str << __LINE__;
    QByteArray enum_str_to_char = dev_type_str.toLatin1();
    const char * enum_char = enum_str_to_char.data();
    QMetaEnum metaEnum = QMetaEnum::fromType<bluetoothdevice::DEVICE_TYPE>();
    qDebug() << Q_FUNC_INFO << "enum index:" << metaEnum.keysToValue(enum_char);

    return (metaEnum.keysToValue(enum_char) == -1 ? bluetoothdevice::DEVICE_TYPE::uncategorized : bluetoothdevice::DEVICE_TYPE(metaEnum.keysToValue(enum_char)));
}
QString BlueToothMain::getDevType(QString deviceAddress)
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间
    iface.setTimeout(100);
    QDBusReply<QString> reply=iface.call("getDevType",deviceAddress);
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;

    return reply;
}

void BlueToothMain::clearAllUnPairedDevicelist()
{
    qDebug () << Q_FUNC_INFO << __LINE__;

    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::sessionBus());
    //设置超时时间
    iface.call(QDBus::NoBlock, "clearAllUnPairedDevicelist");
    //qDebug () << Q_FUNC_INFO << "==========================" << reply << __LINE__;
    //return reply;
}

void BlueToothMain::setTrayVisible(bool value)
{
    qDebug() << Q_FUNC_INFO << "user check: " << value << __LINE__;
    m_settings->set("tray-show",QVariant::fromValue(value));
}

void BlueToothMain::setBluetoothBlock(bool value)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "setBluetoothBlock");
    m << value;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<bool>() <<__LINE__;
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);
}

void BlueToothMain::setDefaultAdapterName(QString adapterName)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "setDefaultAdapterName");//setBluetoothName
    m << adapterName;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<QString>() <<__LINE__;
    // 发送Message
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);

}

void BlueToothMain::setDefaultAdapterPower(bool value)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "setDefaultAdapterPower");
    m << value;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<bool>() <<__LINE__;
    // 发送Message
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);
}

void BlueToothMain::setDefaultAdapter(QString adapterAddress)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "setDefaultAdapter");
    m << adapterAddress;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<QString>() <<__LINE__;
    // 发送Message
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);
}

void BlueToothMain::setDefaultAdapterDiscoverable(bool value)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "setDefaultAdapterDiscoverable");
    m << value;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<bool>() <<__LINE__;
    // 发送Message
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);
}
void BlueToothMain::setbluetoothAdapterDiscoveringStatus(bool isScan)
{
    qDebug() << Q_FUNC_INFO << __LINE__;

    if ("normalWidget" != _MCentralWidget->currentWidget()->objectName())
    {
        qDebug() << Q_FUNC_INFO << "errorWidget not open scan!!" << __LINE__;
        return;
    }
    setDefaultAdapterScanOn(isScan);
}

void BlueToothMain::setDefaultAdapterScanOn(bool value)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "setDefaultAdapterScanOn");
    m << value;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<bool>() <<__LINE__;
    // 发送Message
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);
}

void BlueToothMain::receiveConnectsignal(QString address)
{
    //停止扫描，连接完成或连接超时后继续开启扫描
    //setDefaultAdapterScanOn(false);
    cacheDevTypeList->setEnabled(false);
    qDebug() << Q_FUNC_INFO << __LINE__;
    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "devConnect");
    m << address;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<QString>() <<__LINE__;
    // 发送Message
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);
}

void BlueToothMain::receiveDisConnectSignal(QString address)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    cacheDevTypeList->setEnabled(false);
    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "devDisconnect");
    m << address;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<QString>() <<__LINE__;
    // 发送Message
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);
}

void BlueToothMain::receiveRemoveSignal(QString address)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "devRemove");
    m << address;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<QString>() <<__LINE__;
    // 发送Message
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);

    removeDeviceItemUI(address);
}

void BlueToothMain::receiveSendFileSignal(QString address)
{
    qDebug() << Q_FUNC_INFO << address;
    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "setSendTransferDeviceMesg");
    m << address;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<QString>() <<__LINE__;
    // 发送Message
    QDBusConnection::sessionBus().call(m, QDBus::NoBlock);

}

void BlueToothMain::showBluetoothNormalMainWindow()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    m_normal_main_widget = new QWidget(_MCentralWidget);
    m_normal_main_widget->setObjectName("normalWidget");
    //this->setCentralWidget(m_normal_main_widget);

    main_layout = new QVBoxLayout(m_normal_main_widget);
    main_layout->setSpacing(40);
    main_layout->setContentsMargins(0,0,0,10);
    frame_top    = new QWidget(m_normal_main_widget);
    frame_top->setObjectName("frame_top");
    if ((m_adapter_address_list.size() > 1) &&
        (m_adapter_address_list.size() == m_adapter_name_list.size()))//用什么作为适配数量的判断
    {
        if(ukccbluetoothconfig::m_isIntel)
        {
            frame_top->setMinimumSize(582,215);
            frame_top->setMaximumSize(1800,215);
        }
        else
        {
            frame_top->setMinimumSize(582,279);
            frame_top->setMaximumSize(1800,279);
        }
    }
    else
    {
        if(ukccbluetoothconfig::m_isIntel)
        {
            frame_top->setMinimumSize(582,153);
            frame_top->setMaximumSize(1800,153);
        }
        else
        {
            frame_top->setMinimumSize(582,217);
            frame_top->setMaximumSize(1800,217);
        }
    }

    frame_middle = new QWidget(m_normal_main_widget);
    frame_middle->setObjectName("frame_middle");
    frame_bottom = new QWidget(m_normal_main_widget);
    frame_bottom->setObjectName("frame_bottom");
    frame_bottom->setMinimumWidth(582);
    frame_bottom->setMaximumWidth(1800);

    main_layout->addWidget(frame_top,1,Qt::AlignTop);
    main_layout->addWidget(frame_middle,1,Qt::AlignTop);
    main_layout->addWidget(frame_bottom,1,Qt::AlignTop);
    main_layout->addStretch(10);

    initMainWindowTopUI();
    initMainWindowMiddleUI();
    initMainWindowbottomUI();
    this->setLayout(main_layout);
    _MCentralWidget->addWidget(m_normal_main_widget);
}


void BlueToothMain::initMainWindowTopUI()
{
    qDebug() << Q_FUNC_INFO << __LINE__;

    //~ contents_path /bluetooth/Bluetooth
    QLabel *label_1 = new QLabel(tr("Bluetooth"),frame_top);
    label_1->setFixedSize(120,25);
    label_1->setStyleSheet("QLabel{\
                           font: Noto Sans CJK SC;\
                           font-size: 14px;\
                           font-weight: 400;\
                           line-height: 20.72px;}");
                           //font-size: 14px;\
//    label_1->setFont(QFont("Noto Sans CJK SC", 12, QFont::Normal));
    label_1->setContentsMargins(16,0,0,0);

    QVBoxLayout *top_layout = new QVBoxLayout();
    top_layout->setSpacing(8);
    top_layout->setContentsMargins(0,0,16,0);
    top_layout->addWidget(label_1);

    QFrame *top_frame = new QFrame(frame_top);
    top_frame->setMinimumWidth(582);
    top_frame->setFrameShape(QFrame::Shape::Box);
    top_layout->addWidget(top_frame);

    QVBoxLayout *layout = new QVBoxLayout(top_frame);
    layout->setSpacing(2);
    layout->setContentsMargins(0,0,0,0);

    QFrame *frame_1 = new QFrame(top_frame);
    frame_1->setMinimumWidth(582);
    frame_1->setFrameShape(QFrame::Shape::Box);
    frame_1->setFixedHeight(60);
    layout->addWidget(frame_1);

    QFrame *line_frame1 = new QFrame(top_frame);
    line_frame1->setFixedHeight(1);
    line_frame1->setMinimumWidth(582);
    line_frame1->setFrameStyle(QFrame::HLine);
    layout->addWidget(line_frame1);

    QHBoxLayout *frame_1_layout = new QHBoxLayout(frame_1);
    frame_1_layout->setSpacing(10);
    frame_1_layout->setContentsMargins(16,0,16,0);
    //~ contents_path /bluetooth/Local device:
    label_2 = new QLabel(tr("Local device:"),frame_1);
    label_2->setStyleSheet("QLabel{\
                           font-size: 16px;\
                           width: 56px;\
                           height: 20px;\
                           font-weight: 400;\
                           line-height: 23.68px;}");
    frame_1_layout->addWidget(label_2);


    bluetooth_name = new BluetoothNameLabel(frame_1,300,38);
    connect(bluetooth_name,&BluetoothNameLabel::sendAdapterName,this,&BlueToothMain::setDefaultAdapterName);
    connect(this,&BlueToothMain::defaultAdapterNameChanged,bluetooth_name,&BluetoothNameLabel::setLabelText);
    frame_1_layout->addWidget(bluetooth_name);
    frame_1_layout->addStretch();

//#ifdef DEVICE_IS_INTEL
//    bluetooth_name = new CustomizeNameLabel(frame_1);
//    bluetooth_name->setFixedSize(420,40);
//    connect(bluetooth_name,&CustomizeNameLabel::setTipText,this,&BlueToothMain::setTipTextSlot);
//    connect(bluetooth_name,&CustomizeNameLabel::sendAdapterName,this,&BlueToothMain::setDefaultAdapterName);
//    connect(this,&BlueToothMain::defaultAdapterNameChanged,bluetooth_name,&CustomizeNameLabel::setAdapterNameText);
//    frame_1_layout->addWidget(bluetooth_name);
//    frame_1_layout->addStretch();


//    _tip1 = tr("* Nothing entered, please re-enter");
//    _tip2 = tr("* Up to 30 characters can be entered");
//    tipTextLabel = new QLabel(top_frame);
//    tipTextLabel->resize(300,40);
////    tipTextLabel->setText(_tip2);
//    frame_1_layout->addWidget(tipTextLabel);
//#endif

    m_open_bluetooth_btn = new SwitchButton(frame_1);
    connect(m_open_bluetooth_btn,SIGNAL(checkedChanged(bool)),this,SLOT(onClick_Open_Bluetooth(bool)));

    frame_1_layout->addWidget(m_open_bluetooth_btn);

    line_frame2 = new QFrame(top_frame);
    line_frame2->setFixedHeight(1);
    line_frame2->setMinimumWidth(582);
    line_frame2->setFrameStyle(QFrame::HLine);

    frame_2 = new QFrame(top_frame);
    frame_2->setMinimumWidth(582);
    frame_2->setFrameShape(QFrame::Shape::Box);
    frame_2->setFixedHeight(60);
    layout->addWidget(frame_2);
    layout->addWidget(line_frame2);
    if(m_adapter_address_list.size() <= 1) {
        frame_2->setVisible(false);
        line_frame2->setVisible(false);
    }

    QHBoxLayout *frame_2_layout = new QHBoxLayout(frame_2);
    frame_2_layout->setSpacing(10);
    frame_2_layout->setContentsMargins(16,0,16,0);
    //~ contents_path /bluetooth/Bluetooth adapter
    QLabel *label_3 = new QLabel(tr("Bluetooth adapter"),frame_2);
    label_3->setStyleSheet("QLabel{\
                           width: 56px;\
                           height: 20px;\
                           font-weight: 400;\
                           line-height: 20px;}");
    frame_2_layout->addWidget(label_3);
    frame_2_layout->addStretch();

    m_adapter_list_cmbox = new QComboBox(frame_2);
    m_adapter_list_cmbox->clear();
    m_adapter_list_cmbox->setMinimumWidth(300);
    m_adapter_list_cmbox->addItems(m_adapter_name_list);
    //m_adapter_list_cmbox->setCurrentIndex(adapter_address_list.indexOf(m_default_adapter_address));
    m_adapter_list_cmbox->setCurrentIndex(m_adapter_name_list.indexOf(m_default_adapter_name));
    connect(m_adapter_list_cmbox,SIGNAL(currentIndexChanged(int)),this,SLOT(adapterComboxChanged(int)));
    frame_2_layout->addWidget(m_adapter_list_cmbox);

//#ifndef DEVICE_IS_INTEL
    if(!ukccbluetoothconfig::m_isIntel)
    {
        QFrame *frame_3 = new QFrame(top_frame);
        frame_3->setMinimumWidth(582);
        frame_3->setFrameShape(QFrame::Shape::Box);
        frame_3->setFixedHeight(60);
        layout->addWidget(frame_3);

        QFrame *line_frame3 = new QFrame(top_frame);
        line_frame3->setFixedHeight(1);
        line_frame3->setMinimumWidth(582);
        line_frame3->setFrameStyle(QFrame::HLine);
        layout->addWidget(line_frame3);

        QHBoxLayout *frame_3_layout = new QHBoxLayout(frame_3);
        frame_3_layout->setSpacing(10);
        frame_3_layout->setContentsMargins(16,0,16,0);

        //~ contents_path /bluetooth/Show icon on taskbar
        QLabel *label_4 = new QLabel(tr("Show icon on taskbar"),frame_3);
        label_4->setStyleSheet("QLabel{\
                               width: 56px;\
                               height: 20px;\
                               font-weight: 400;\
                               line-height: 20px;}");
        frame_3_layout->addWidget(label_4);
        frame_3_layout->addStretch();

        m_show_panel_btn = new SwitchButton(frame_3);
        frame_3_layout->addWidget(m_show_panel_btn);
        if(m_settings)
        {
            m_show_panel_btn->setChecked(m_settings->get("tray-show").toBool());
        }
        else
        {
            m_show_panel_btn->setChecked(false);
            m_show_panel_btn->setDisabledFlag(false);
        }
        connect(m_show_panel_btn,&SwitchButton::checkedChanged,this,&BlueToothMain::setTrayVisible);
    }
//#endif
    QFrame *frame_4 = new QFrame(top_frame);
    frame_4->setMinimumWidth(582);
    frame_4->setFrameShape(QFrame::Shape::Box);
    frame_4->setFixedHeight(60);
    layout->addWidget(frame_4);

    QHBoxLayout *frame_4_layout = new QHBoxLayout(frame_4);
    frame_4_layout->setSpacing(10);
    frame_4_layout->setContentsMargins(16,0,16,0);

    //~ contents_path /bluetooth/Discoverable by nearby Bluetooth devices
    QLabel *label_5 = new QLabel(tr("Discoverable by nearby Bluetooth devices"),frame_4);
    label_5->setStyleSheet("QLabel{\
                           width: 56px;\
                           height: 20px;\
                           font-weight: 400;\
                           line-height: 20px;}");
    frame_4_layout->addWidget(label_5);
    frame_4_layout->addStretch();

    m_discover_switch_btn = new SwitchButton(frame_4);
    frame_4_layout->addWidget(m_discover_switch_btn);
    //switch_discover->setChecked(m_localDevice->isDiscoverable());
    m_discover_switch_btn->setChecked(m_current_adapter_disconvery_swtich);
    connect(m_discover_switch_btn,&SwitchButton::checkedChanged,this,&BlueToothMain::setDefaultAdapterDiscoverable);

    frame_top->setLayout(top_layout);
    qDebug() << Q_FUNC_INFO << "end" << __LINE__;

}


void BlueToothMain::initMainWindowMiddleUI()
{
    QVBoxLayout *middle_layout = new QVBoxLayout(frame_middle);
    middle_layout->setSpacing(8);
    //middle_layout->setContentsMargins(0,0,16,0);
    middle_layout->setContentsMargins(0,0,0,0);

    QHBoxLayout *label_layout = new QHBoxLayout();
    label_layout->setSpacing(0);
    label_layout->setContentsMargins(16,0,0,0);


    QLabel *middle_label = new QLabel(tr("My Devices"),frame_middle);
    middle_label->setStyleSheet("QLabel{width: 72px;\
                                height: 25px;\
                                font-size: 14px;\
                                font-weight: 400;\
                                line-height: 20.72px;}");
    middle_label->setContentsMargins(0,0,0,0);
    //middle_label->setEnabled(false);
    label_layout->addWidget(middle_label);

    QFrame *mDev_frame = new QFrame(frame_middle);
    mDev_frame->setObjectName("mDev_frame");
    mDev_frame->setMinimumWidth(582);
    mDev_frame->setFrameShape(QFrame::Shape::Box);

    paired_dev_layout = new QVBoxLayout(mDev_frame);
    paired_dev_layout->setSpacing(2);
    paired_dev_layout->setContentsMargins(0,0,0,0);

    middle_layout->addLayout(label_layout,Qt::AlignTop);
    middle_layout->addWidget(mDev_frame,Qt::AlignTop);

    frame_middle->setLayout(middle_layout);
}

void BlueToothMain::initMainWindowbottomUI()
{
    QHBoxLayout *title_layout = new QHBoxLayout();
    title_layout->setSpacing(10);
    title_layout->setContentsMargins(16,0,16,0);

    //~ contents_path /bluetooth/Other Devices
    QLabel *label_1 = new QLabel(tr("Other Devices"),frame_bottom);
    label_1->setStyleSheet("QLabel{\
                          width: 72px;\
                          height: 25px;\
                          font-size: 14px;\
                          font-weight: 400;\
                          line-height: 20.72px;}");
    label_1->setContentsMargins(0,0,0,0);
    //label_1->setEnabled(false);

    loadLabel = new QLabel(frame_bottom);
    loadLabel->setFixedSize(18,18);

    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(100);
        connect(m_timer,&QTimer::timeout,this,&BlueToothMain::refreshLoadLabelIcon);
    }

    //开启时延迟1.8s后开启扫描，留点设备回连时间
    delayStartDiscover_timer = new QTimer(this);
    delayStartDiscover_timer->setInterval(2000);
    connect(delayStartDiscover_timer,&QTimer::timeout,this,[=]
    {
        qDebug() << "delay Start Discover time out:" << __LINE__;
        delayStartDiscover_timer->stop();
        if (!m_current_adapter_scan_status)
        {
            setDefaultAdapterScanOn(true);

            //QTimer::singleShot(500,this,[=]
            //{
            //    if (!m_current_adapter_scan_status)
            //    {
            //        delayStartDiscover_timer->start();
            //    }
            //});
        }
    });

    title_layout->addWidget(label_1);
    title_layout->addWidget(loadLabel);
    title_layout->addStretch();

    cacheDevTypeList = new QComboBox(frame_bottom);
    cacheDevTypeList->clear();
    cacheDevTypeList->setMinimumWidth(120);
    QStringList devStrList;
    devStrList << tr("All");
    devStrList << tr("Audio");
    devStrList << tr("Peripherals");
    devStrList << tr("PC");
    devStrList << tr("Phone");
    devStrList << tr("Other");

    cacheDevTypeList->addItems(devStrList);

    connect(cacheDevTypeList,SIGNAL(currentIndexChanged(int)),this,SLOT(changeListOfDiscoveredDevices(int)));
    title_layout->addWidget(cacheDevTypeList);

    QVBoxLayout *bottom_layout = new QVBoxLayout(frame_bottom);
    bottom_layout->setSpacing(8);
    //bottom_layout->setContentsMargins(0,0,16,0);
    bottom_layout->setContentsMargins(0,0,0,0);
    bottom_layout->addLayout(title_layout);

    device_list = new QFrame();
    device_list->setObjectName("device_list");
    device_list->setMinimumWidth(582);
    device_list->setFrameShape(QFrame::Shape::Box);
    device_list->setVisible(false);

    bottom_layout->addWidget(device_list);

    device_list_layout  = new QVBoxLayout(device_list);
    device_list_layout->setSpacing(2);
    device_list_layout->setContentsMargins(0,0,0,0);
    device_list_layout->setAlignment(Qt::AlignTop);
    device_list->setLayout(device_list_layout);

    frame_bottom->setLayout(bottom_layout);
}

void BlueToothMain::stopAllTimer()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    if (delayStartDiscover_timer && delayStartDiscover_timer->isActive())
        delayStartDiscover_timer->stop();
}

void BlueToothMain::showBluetoothErrorMainWindow()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    stopAllTimer();
    m_error_main_widget       = new QWidget(_MCentralWidget);
    errorWidgetLayout = new QVBoxLayout(m_error_main_widget);
    errorWidgetIcon   = new QLabel(m_error_main_widget);
    errorWidgetTip0   = new QLabel(m_error_main_widget);

    m_error_main_widget->setObjectName("errorWidget");

    errorWidgetLayout->setSpacing(10);
    errorWidgetLayout->setMargin(0);
    errorWidgetLayout->setContentsMargins(0,0,0,0);

    QPalette pe;
    pe.setColor(QPalette::WindowText,"#818181");
    errorWidgetIcon->setFixedSize(54,88);
    errorWidgetTip0->resize(200,30);
    errorWidgetTip0->setFont(QFont("Noto Sans CJK SC",18,QFont::Medium));
    errorWidgetTip0->setPalette(pe);
    if (QIcon::hasThemeIcon("bluetooth-disabled-symbolic")) {
        if (isblack)
            errorWidgetIcon->setPixmap(ImageUtil::drawSymbolicColoredPixmap(QIcon::fromTheme("bluetooth-disabled-symbolic").pixmap(54,88),"white"));
        else
            errorWidgetIcon->setPixmap(QIcon::fromTheme("bluetooth-disabled-symbolic").pixmap(54,88));
    }

    errorWidgetTip0->setText(tr("Bluetooth driver abnormal"));

    errorWidgetLayout->addStretch(7);
    errorWidgetLayout->addWidget(errorWidgetIcon,1,Qt::AlignCenter);
    errorWidgetLayout->addWidget(errorWidgetTip0,1,Qt::AlignCenter);
    errorWidgetLayout->addStretch(10);

    _MCentralWidget->addWidget(m_error_main_widget);
}


void BlueToothMain::refreshUIWhenAdapterChanged()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    // =============清空我的设备和蓝牙发现设备栏布局下的所有设备item=================
    cleanPairDevices();
    m_default_adapter_address = getDefaultAdapterAddress();
    getDefaultAdapterData(m_default_adapter_address);
    // ========================END===========================================
    m_myDev_show_flag = false;
    m_discovery_device_address_all_list.clear();

    for (int i = 0 ; i < m_default_bluetooth_adapter->m_bluetooth_device_list.size(); i++)
    {
        if(m_default_bluetooth_adapter->m_bluetooth_device_list.at(i)->isPaired())
            addMyDeviceItemUI(m_default_bluetooth_adapter->m_bluetooth_device_list.at(i));
        else
            addOneBluetoothDeviceItemUi(m_default_bluetooth_adapter->m_bluetooth_device_list.at(i));
    }

    if (m_current_adapter_scan_status)
    {
        if(!loadLabel->isVisible())
            loadLabel->setVisible(true);
        if (!m_timer->isActive())
            m_timer->start();
    }

    //==============初始化蓝牙信息和基础信息====================================

//#ifdef DEVICE_IS_INTEL
//    bluetooth_name->setAdapterNameText(m_default_adapter_name);
//#else
    bluetooth_name->set_dev_name(m_default_adapter_name);

//#endif
    qDebug() << Q_FUNC_INFO << "m_current_adapter_power_swtich:" << m_current_adapter_power_swtich ;
    if(m_current_adapter_power_swtich)
    {
        if (!m_open_bluetooth_btn->isChecked())
        {
            //添加标识，从蓝牙服务获取的数据
            m_service_dbus_adapter_power_change_flag = true;
            m_open_bluetooth_btn->setChecked(true);
        }
//        bluetooth_name->setVisible(true);
        frame_middle->setVisible(m_myDev_show_flag);
        if (!frame_bottom->isVisible() && frame_bottom->children().size())
            frame_bottom->setVisible(true);
    }
    else
    {
        if (m_open_bluetooth_btn->isChecked())
        {
            //添加标识，从蓝牙服务获取的数据
            m_service_dbus_adapter_power_change_flag = true;
            m_open_bluetooth_btn->setChecked(false);
        }
        frame_bottom->setVisible(false);
        frame_middle->setVisible(false);
    }

    if (m_open_bluetooth_btn->isChecked())
    {
        qDebug() << Q_FUNC_INFO << "m_open_bluetooth_btn->isChecked() == " << m_open_bluetooth_btn->isChecked() << "stary discover" ;
        if (delayStartDiscover_timer->isActive())
            delayStartDiscover_timer->stop();
        delayStartDiscover_timer->start();
    }

    //===========================END==========================================
    qDebug() << Q_FUNC_INFO << "end" << __LINE__;

    //return ;
}

void BlueToothMain::changeDeviceParentWindow(const QString &address)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    if(!frame_middle->isVisible()){
        frame_middle->setVisible(true);
    }

    //IntelDeviceInfoItem *item = device_list->findChild<IntelDeviceInfoItem *>(address);
    DeviceInfoItem *item = device_list->findChild<DeviceInfoItem *>(address);
    QFrame *line_item = device_list->findChild<QFrame *>("line-"+address);
    if(item){


        device_list_layout->removeWidget(item);
        item->setParent(frame_middle);

        if(line_item) {
            device_list_layout->removeWidget(line_item);
            line_item->setParent(NULL);
            line_item->deleteLater();
        }

        //设备配对成功后，先读取设备的连接状态，设置对应的设备状态
        bool isConnect = getDevConnectStatus(address);
        reportDevConnectStatusSignal(address,isConnect);

        mDevFrameAddLineFrame("paired",address);
        paired_dev_layout->addWidget(item);
        m_myDev_show_flag = true;
    }else{
        return;
    }
}

void BlueToothMain::onClick_Open_Bluetooth(bool ischeck)
{
    qDebug() << Q_FUNC_INFO << ischeck << (dynamic_cast<SwitchButton*>(sender()) == nullptr);

//    if (m_service_dbus_adapter_power_change_flag)
//        disconnect(m_open_bluetooth_btn,SIGNAL(checkedChanged(bool)),nullptr,nullptr);

    if (dynamic_cast<SwitchButton*>(sender()) == nullptr)
    {
        m_open_bluetooth_btn->setChecked(ischeck);
    }
    else
    {
        if (!m_service_dbus_adapter_power_change_flag) //不由服务发送的设置
        {
            setDefaultAdapterPower(ischeck);
        }
        //        bluetooth_name->setVisible(ischeck);
    }
    if(ischeck && m_myDev_show_flag)
        frame_middle->setVisible(ischeck);
    else
        frame_middle->setVisible(false);

    frame_bottom->setVisible(ischeck);
    if(ischeck && loadLabel->isVisible())
    {
        loadLabel->setVisible(false);
    }

    if(ischeck)
    {
        if(delayStartDiscover_timer->isActive())
            delayStartDiscover_timer->stop();
        delayStartDiscover_timer->start();
    }
    else
    {
        if (loadLabel->isVisible())
            loadLabel->setVisible(false);
        if (m_timer->isActive())
            m_timer->stop();

        stopAllTimer();
    }

    if (m_service_dbus_adapter_power_change_flag)
    {
        m_service_dbus_adapter_power_change_flag = false;
    }

    return;
}

bool BlueToothMain::isInvalidDevice(QString devName , bluetoothdevice::DEVICE_TYPE devType)
{
    //qDebug() << Q_FUNC_INFO << __LINE__;
    if (devName.split("-").length() == 6 && bluetoothdevice::DEVICE_TYPE::uncategorized == devType)
        return true;
    else
        return false;
}

void BlueToothMain::refreshLoadLabelIcon()
{
    if("normalWidget" == _MCentralWidget->currentWidget()->objectName())
    {
        if(i > 7)
            i = 0;
        if (isblack)
            loadLabel->setPixmap(ImageUtil::drawSymbolicColoredPixmap(QIcon::fromTheme("ukui-loading-"+QString::number(i,10)).pixmap(18,18),"white"));
        else
            loadLabel->setPixmap(QIcon::fromTheme("ukui-loading-"+QString::number(i,10)).pixmap(18,18));

        loadLabel->update();
        i++;
    }
}

void BlueToothMain::removeDeviceItemUI(QString address)
{
    qDebug() << Q_FUNC_INFO << address << __LINE__;
    //qDebug() << Q_FUNC_INFO << address << last_discovery_device_address.indexOf(address) <<__LINE__;

    //if(last_discovery_device_address.indexOf(address) != -1)
    //{
    //IntelDeviceInfoItem *item = device_list->findChild<IntelDeviceInfoItem *>(address);
        DeviceInfoItem *item = device_list->findChild<DeviceInfoItem *>(address);
        QFrame *line_item = device_list->findChild<QFrame *>("line-"+address);
        if(item)
        {
            device_list_layout->removeWidget(item);
            item->setParent(NULL);
            item->deleteLater();

            if(line_item) {
                device_list_layout->removeWidget(line_item);
                line_item->setParent(NULL);
                line_item->deleteLater();
            }
            //m_discovery_device_address_all_list.removeAll(address);
            //last_discovery_device_address.removeAll(address);

        }
        //else
        //{
        //    qDebug() << Q_FUNC_INFO << "NULL"<<__LINE__;
        //    return;
        //}

    //}else{
        //IntelDeviceInfoItem *item = frame_middle->findChild<IntelDeviceInfoItem *>(address);
        //item = frame_middle->findChild<IntelDeviceInfoItem *>(address);
        item = frame_middle->findChild<DeviceInfoItem *>(address);
        line_item = frame_middle->findChild<QFrame *>("line-"+address);
        if(item){


            paired_dev_layout->removeWidget(item);
            item->setParent(NULL);
            item->deleteLater();

            if (line_item) {
                paired_dev_layout->removeWidget(line_item);
                line_item->setParent(NULL);
                line_item->deleteLater();
            } else
                removeMDevFrameLineFrame("paired");

            qDebug() << Q_FUNC_INFO << "paired_dev_layout->count():" << paired_dev_layout->count() <<__LINE__;
            if(!paired_dev_layout->count()){
                m_myDev_show_flag = false;
                frame_middle->setVisible(false);
            }
            else
            {
                m_myDev_show_flag = true;
            }

//            qDebug() << Q_FUNC_INFO << " frame_middle->children().size() :"<<frame_middle->children().size() <<__LINE__;

//            if(frame_middle->children().size() == 3)
//            {
//                frame_middle->setVisible(false);
//            }
        }
        else
        {
            qDebug() << Q_FUNC_INFO << "NULL"<<__LINE__;
            return;
        }
    //}
    qDebug() << Q_FUNC_INFO << "remove Item UI end" <<__LINE__;

}


void BlueToothMain::changeListOfDiscoveredDevices(int index)
{
    //qDebug() << Q_FUNC_INFO << index << m_discovery_device_address_all_list;
    qDebug() << Q_FUNC_INFO << index << __LINE__;
    discoverDevFlag = DevTypeShow(index);

    //清空列表数据
    QLayoutItem *child;
    while ((child = device_list_layout->takeAt(0)) != 0)
    {
        if(child->widget())
        {
            child->widget()->setParent(NULL);
        }

        QFrame *line_item = device_list->findChild<QFrame *>("line-"+child->widget()->objectName());
        if(line_item) {
            device_list_layout->removeWidget(line_item);
            line_item->setParent(NULL);
            line_item->deleteLater();
        }
        delete child;
        child = nullptr;
    }

    device_list->setVisible(false);

    addDiscoverDevListByFlag(discoverDevFlag);
}


void BlueToothMain::addDiscoverDevListByFlag(BlueToothMain::DevTypeShow flag)
{
    qDebug() << Q_FUNC_INFO << flag;
    for (bluetoothdevice *dev : m_default_bluetooth_adapter->m_bluetooth_device_list)
    {
        if (isInvalidDevice(dev->getDevName(),dev->getDevType()))
            continue;
        switch (flag) {
        case DevTypeShow::All:
            addOneBluetoothDeviceItemUi(dev);
            break;
        case DevTypeShow::Audio:
            if (dev->getDevType() == bluetoothdevice::DEVICE_TYPE::headset ||
                dev->getDevType() == bluetoothdevice::DEVICE_TYPE::headphones ||
                dev->getDevType() == bluetoothdevice::DEVICE_TYPE::audiovideo) {
                addOneBluetoothDeviceItemUi(dev);
            }
            break;
        case DevTypeShow::Peripherals:
            if (dev->getDevType() == bluetoothdevice::DEVICE_TYPE::mouse ||
                dev->getDevType() == bluetoothdevice::DEVICE_TYPE::keyboard) {
                addOneBluetoothDeviceItemUi(dev);
            }
            break;
        case DevTypeShow::PC:
            if (dev->getDevType() == bluetoothdevice::DEVICE_TYPE::computer) {
                addOneBluetoothDeviceItemUi(dev);
            }
            break;
        case DevTypeShow::Phone:
            if (dev->getDevType() == bluetoothdevice::DEVICE_TYPE::phone) {
                addOneBluetoothDeviceItemUi(dev);
            }
            break;
        case DevTypeShow::Other:
            if (dev->getDevType() == bluetoothdevice::DEVICE_TYPE::headset ||
                dev->getDevType() == bluetoothdevice::DEVICE_TYPE::headphones ||
                dev->getDevType() == bluetoothdevice::DEVICE_TYPE::audiovideo ||
                dev->getDevType() == bluetoothdevice::DEVICE_TYPE::phone ||
                dev->getDevType() == bluetoothdevice::DEVICE_TYPE::mouse ||
                dev->getDevType() == bluetoothdevice::DEVICE_TYPE::keyboard ||
                dev->getDevType() == bluetoothdevice::DEVICE_TYPE::computer)
                break;
            else
                addOneBluetoothDeviceItemUi(dev);
            break;
        default:
            addOneBluetoothDeviceItemUi(dev);
            break;
        }
    }
}

void BlueToothMain::monitorSleepSignal()
{
    if (QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                             "/org/freedesktop/login1",
                                             "org.freedesktop.login1.Manager",
                                             "PrepareForSleep",
                                             this,
                                             SLOT(monitorSleepSlot(bool)))){
        qDebug() << Q_FUNC_INFO << "PrepareForSleep signal connected successfully to slot";
    } else {
        qDebug() << Q_FUNC_INFO << "PrepareForSleep signal connected was not successful";
    }
}

void BlueToothMain::monitorSleepSlot(bool value)
{
    //connectBluetoothServiceSignal();
    if (!value) {
        if (sleep_status)
        {
//            if (this->centralWidget()->objectName() == "normalWidget")
//                adapterPoweredChanged(true);
        }
    } else {
        sleep_status = m_current_adapter_power_swtich;
    }
}

void BlueToothMain::adapterPoweredChanged(bool value)
{
    qDebug() << Q_FUNC_INFO << value;
//    if(m_settings)
//        m_settings->set("switch",QVariant::fromValue(value));

    if(value)
    {
        //bluetooth_name->setAdapterNameText(m_default_adapter_name);
        bluetooth_name->set_dev_name(m_default_adapter_name);
        bluetooth_name->setVisible(true);
        frame_bottom->setVisible(true);

        if(m_myDev_show_flag)
            frame_middle->setVisible(true);

//         if(!frame_middle->isVisible())
//         {
//            frame_middle->setVisible(true);
//         }
        if (!m_open_bluetooth_btn->isChecked())
            m_open_bluetooth_btn->setChecked(true);

        //延时2S开启扫描，给用户回连缓冲
        delayStartDiscover_timer->start();

    }
    else
    {
//        if (bluetooth_name->isVisible())
//            bluetooth_name->setVisible(false);

        if (m_open_bluetooth_btn->isChecked())
            m_open_bluetooth_btn->setChecked(false);

        if (frame_bottom->isVisible())
            frame_bottom->setVisible(false);

        if(frame_middle->isVisible())
            frame_middle->setVisible(false);

        if (!paired_dev_layout->isEmpty())
            m_myDev_show_flag = true ;
        else
            m_myDev_show_flag = false ;
    }
}

void BlueToothMain::adapterComboxChanged(int i)
{
    qDebug() << Q_FUNC_INFO << i
             << m_adapter_address_list.at(i)
             << m_adapter_name_list.at(i)
             << m_adapter_address_list
             << m_adapter_name_list;

    if(m_adapter_address_list.size() != m_adapter_name_list.size())
        return;

    if(i != -1)
    {
        qDebug() << Q_FUNC_INFO << "=================3" << __LINE__;
        setDefaultAdapterScanOn(false);
        clearAllUnPairedDevicelist();

        if(i < m_bluetooth_adapter_list.size())
            setDefaultAdapter(m_bluetooth_adapter_list.at(i)->getDevAddress());

        refreshUIWhenAdapterChanged();
    }else{
        if(m_open_bluetooth_btn->isChecked()){
            qDebug() << __FUNCTION__<< "index - i : "<< i << __LINE__ ;
            m_open_bluetooth_btn->setChecked(false);
#ifndef DEVICE_IS_INTEL
            m_open_bluetooth_btn->setDisabledFlag(false);
#endif
        }
        if(frame_middle->isVisible()){
            frame_middle->setVisible(false);
        }
    }
    qDebug() << Q_FUNC_INFO << "end";
}

void BlueToothMain::mDevFrameAddLineFrame(QString str,QString addr)
{
    qDebug() << Q_FUNC_INFO << "#########################" << str << addr;

    if ("paired" == str) {

//        if (!frame_middle->findChildren<IntelDeviceInfoItem *>().size()) {
//            return;
//        }
        if (!frame_middle->findChildren<DeviceInfoItem *>().size()) {
            return;
        }

        QFrame *line_frame = new QFrame(mDev_frame);
        line_frame->setObjectName("line-"+addr);
        line_frame->setFixedHeight(1);
        line_frame->setMinimumWidth(582);
        line_frame->setFrameStyle(QFrame::HLine);
        paired_dev_layout->addWidget(line_frame,Qt::AlignTop);

    } else if ("other" == str) {

//        if (frame_bottom->findChildren<IntelDeviceInfoItem *>().size() <= 1) {
//            return;
//        }
        if (frame_bottom->findChildren<DeviceInfoItem *>().size() <= 1) {
            return;
        }
        QFrame *line_frame = new QFrame(device_list);
        line_frame->setObjectName("line-"+addr);
        line_frame->setFixedHeight(1);
        line_frame->setMinimumWidth(582);
        line_frame->setFrameStyle(QFrame::HLine);


        if (!device_list->isVisible())
            device_list->setVisible(true);

        device_list_layout->insertWidget(0,line_frame,Qt::AlignTop);

    } else {
        return;
    }
}

void BlueToothMain::removeMDevFrameLineFrame(QString str)
{
    qDebug() << Q_FUNC_INFO ;

    if ("paired" == str) {

        //if ((frame_middle->findChildren<QFrame *>().size() - frame_middle->findChildren<IntelDeviceInfoItem *>().size() - 2) == frame_middle->findChildren<IntelDeviceInfoItem *>().size()) {
        if ((frame_middle->findChildren<QFrame *>().size() - frame_middle->findChildren<DeviceInfoItem *>().size() - 2) == frame_middle->findChildren<DeviceInfoItem *>().size()) {
            QLayoutItem *child;
            if ((child = paired_dev_layout->takeAt(0)) != 0) {
                if (child->widget()->objectName().startsWith("line-")) {

                    child->widget()->setParent(NULL);

                    delete child;
                    child = nullptr;
                }
            }
        }
    } else if ("other" == str) {

    } else {
        return;
    }
}

void BlueToothMain::cleanPairDevices()
{
    qDebug() << Q_FUNC_INFO << "=============111111111" ;

    QLayoutItem *child;
    while ((child = paired_dev_layout->takeAt(0)) != 0)
    {
        qDebug() << Q_FUNC_INFO << __LINE__;
        if(child->widget())
        {
            child->widget()->setParent(NULL);
        }
        paired_dev_layout->removeItem(child);

        QFrame *line_item = frame_middle->findChild<QFrame *>("line-"+child->widget()->objectName());
        if(line_item) {
            paired_dev_layout->removeWidget(line_item);
            line_item->setParent(NULL);
            line_item->deleteLater();
        }else
            removeMDevFrameLineFrame("paired");

        delete child;
        child = nullptr;


    }
    qDebug() << Q_FUNC_INFO << "=============22222222" ;

    while ((child = device_list_layout->takeAt(0)) != 0)
    {
        qDebug() << Q_FUNC_INFO << __LINE__;
        if(child->widget())
        {
            child->widget()->setParent(NULL);
        }
        device_list_layout->removeItem(child);

        QFrame *line_item = device_list->findChild<QFrame *>("line-"+child->widget()->objectName());
        if(line_item) {
            device_list_layout->removeWidget(line_item);
            line_item->setParent(NULL);
            line_item->deleteLater();
        }

        delete child;
        child = nullptr;


    }
    device_list->setVisible(false);
    qDebug() << Q_FUNC_INFO << "=============33333333" ;

}

void BlueToothMain::gSettingsChanged(const QString &key)
{
    if ("styleName" == key) {
        if(styleSettings->get("style-name").toString() == "ukui-black" ||
           styleSettings->get("style-name").toString() == "ukui-dark")
            isblack = true;
        else
            isblack = false;

        if (isblack)
            errorWidgetIcon->setPixmap(ImageUtil::drawSymbolicColoredPixmap(QIcon::fromTheme("bluetooth-disabled-symbolic").pixmap(54,88),"white"));
        else
            errorWidgetIcon->setPixmap(QIcon::fromTheme("bluetooth-disabled-symbolic").pixmap(54,88));
    }
}

void BlueToothMain::setTipTextSlot(int i)
{
    if (i == 1) {
        tipTextLabel->setText(_tip1);
        QPalette textLabelColor;
        textLabelColor.setColor(QPalette::WindowText,Qt::red);
        tipTextLabel->setPalette(textLabelColor);
    } else if (i == 2){
        tipTextLabel->setText(_tip2);
        QPalette textLabelColor;
        textLabelColor.setColor(QPalette::WindowText,Qt::red);
        tipTextLabel->setPalette(textLabelColor);
    } else {
        tipTextLabel->clear();
    }
}

//add function before

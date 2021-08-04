#include "bluetoothmain.h"

#include <QDBusObjectPath>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include <QDBusMessage>
//#include <polkit-qt5-1/PolkitQt1/Authority>
#include <QMessageLogger>
#include <QFileInfo>


BlueToothMain::BlueToothMain(QWidget *parent)
    : QMainWindow(parent)
{
    if(QGSettings::isSchemaInstalled("org.ukui.bluetooth")){
        settings = new QGSettings("org.ukui.bluetooth");

//        paired_device_address.clear();
//        paired_device_address = settings->get("paired-device-address").toStringList();
//        finally_connect_the_device = settings->get("finally-connect-the-device").toString();
        Default_Adapter = settings->get("adapter-address").toString();

        qDebug() << "GSetting Value: " << Default_Adapter/* << finally_connect_the_device << paired_device_address*/;
    }
//    this->setMinimumSize(582,500);

    m_manager = new BluezQt::Manager(this);
    job = m_manager->init();
    job->exec();
    qDebug() << m_manager->isOperational() << m_manager->isBluetoothBlocked();
    updateAdaterInfoList();

    adapterChanged();
    m_localDevice = getDefaultAdapter();
    if(m_manager->adapters().size() == 0){
        showMainWindowError();
        return;
    } 
    qDebug() << m_localDevice->name() << m_localDevice->isPowered() << m_localDevice->isDiscoverable() << m_localDevice->isDiscovering() << m_localDevice->address();

    showNormalMainWindow();
}

/*
 * Initialize the fixed UI in the upper half of the interface
 *
 */
void BlueToothMain::InitMainTopUI()
{
    //~ contents_path /bluetooth/Bluetooth
    QLabel *label_1 = new QLabel(tr("Bluetooth"),frame_top);
    label_1->setFixedSize(100,25);
    label_1->setStyleSheet("QLabel{\
                           font-size: 18px;\
                           font-weight: 500;\
                           line-height: 25px;}");

    QVBoxLayout *top_layout = new QVBoxLayout();
    top_layout->setSpacing(8);
    top_layout->setContentsMargins(0,0,0,0);
    top_layout->addWidget(label_1);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setContentsMargins(0,0,0,0);
    top_layout->addLayout(layout);

    QFrame *frame_1 = new QFrame(frame_top);
    frame_1->setMinimumWidth(582);
    frame_1->setFrameShape(QFrame::Shape::Box);
    frame_1->setFixedHeight(50);
    frame_1->setAutoFillBackground(true);
    layout->addWidget(frame_1);

    QHBoxLayout *frame_1_layout = new QHBoxLayout(frame_1);
    frame_1_layout->setSpacing(10);
    frame_1_layout->setContentsMargins(16,0,16,0);

    //~ contents_path /bluetooth/Turn on Bluetooth
    label_2 = new QLabel(tr("Turn on :"),frame_1);
    label_2->setStyleSheet("QLabel{\
                           width: 56px;\
                           height: 20px;\
                           font-weight: 400;\
                           line-height: 20px;}");
    frame_1_layout->addWidget(label_2);

    bluetooth_name = new BluetoothNameLabel(frame_1,300,38);
    connect(bluetooth_name,&BluetoothNameLabel::send_adapter_name,this,&BlueToothMain::change_adapter_name);
    connect(this,&BlueToothMain::adapter_name_changed,bluetooth_name,&BluetoothNameLabel::set_label_text);
    frame_1_layout->addWidget(bluetooth_name);
    frame_1_layout->addStretch();

    open_bluetooth = new SwitchButton(frame_1);

    frame_1_layout->addWidget(open_bluetooth);

    frame_2 = new QFrame(frame_top);
    frame_2->setMinimumWidth(582);
    frame_2->setFrameShape(QFrame::Shape::Box);
    frame_2->setFixedHeight(50);
    if(adapter_address_list.size() <= 1)
        frame_2->setVisible(false);
    layout->addWidget(frame_2);

    QHBoxLayout *frame_2_layout = new QHBoxLayout(frame_2);
    frame_2_layout->setSpacing(10);
    frame_2_layout->setContentsMargins(16,0,16,0);

    QLabel *label_3 = new QLabel(tr("Bluetooth adapter"),frame_2);
    label_3->setStyleSheet("QLabel{\
                           width: 56px;\
                           height: 20px;\
                           font-weight: 400;\
                           line-height: 20px;}");
    frame_2_layout->addWidget(label_3);
    frame_2_layout->addStretch();

    adapter_list = new QComboBox(frame_2);
    adapter_list->clear();
    adapter_list->addItems(adapter_name_list);
    adapter_list->setMinimumWidth(300);
    adapter_list->setCurrentIndex(adapter_address_list.indexOf(m_localDevice->address()));
    connect(adapter_list,SIGNAL(currentIndexChanged(int)),this,SLOT(adapterComboxChanged(int)));
    frame_2_layout->addWidget(adapter_list);

    QFrame *frame_3 = new QFrame(frame_top);
    frame_3->setMinimumWidth(582);
    frame_3->setFrameShape(QFrame::Shape::Box);
    frame_3->setFixedHeight(50);
    layout->addWidget(frame_3);

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

    show_panel = new SwitchButton(frame_3);
    frame_3_layout->addWidget(show_panel);
    if(settings){
        show_panel->setChecked(settings->get("tray-show").toBool());
    }else{
        show_panel->setChecked(false);
        show_panel->setDisabledFlag(false);
    }
    connect(show_panel,&SwitchButton::checkedChanged,this,&BlueToothMain::set_tray_visible);

    QFrame *frame_4 = new QFrame(frame_top);
    frame_4->setMinimumWidth(582);
    frame_4->setFrameShape(QFrame::Shape::Box);
    frame_4->setFixedHeight(50);
    layout->addWidget(frame_4);

    QHBoxLayout *frame_4_layout = new QHBoxLayout(frame_4);
    frame_4_layout->setSpacing(10);
    frame_4_layout->setContentsMargins(16,0,16,0);

    //~ contents_path /bluetooth/Discoverable
    QLabel *label_5 = new QLabel(tr("Discoverable by nearby Bluetooth devices"),frame_4);
    label_5->setStyleSheet("QLabel{\
                           width: 56px;\
                           height: 20px;\
                           font-weight: 400;\
                           line-height: 20px;}");
    frame_4_layout->addWidget(label_5);
    frame_4_layout->addStretch();

    switch_discover = new SwitchButton(frame_4);
    frame_4_layout->addWidget(switch_discover);
    switch_discover->setChecked(m_localDevice->isDiscoverable());
    connect(switch_discover,&SwitchButton::checkedChanged,this,&BlueToothMain::set_discoverable);
    connect(m_localDevice.data(), &BluezQt::Adapter::discoverableChanged, this, [=](bool discoverable){
        switch_discover->setChecked(discoverable);
    });

    connect(open_bluetooth,SIGNAL(checkedChanged(bool)),this,SLOT(onClick_Open_Bluetooth(bool)));
    frame_top->setLayout(top_layout);
}

void BlueToothMain::InitMainMiddleUI()
{
    QVBoxLayout *middle_layout = new QVBoxLayout(frame_middle);
    middle_layout->setSpacing(8);
    middle_layout->setContentsMargins(0,0,0,0);

    paired_dev_layout = new QVBoxLayout();
    paired_dev_layout->setSpacing(2);
    paired_dev_layout->setContentsMargins(0,0,0,0);

    QLabel *middle_label = new QLabel(tr("My Devices"),frame_middle);
    middle_label->setStyleSheet("QLabel{width: 72px;\
                                height: 25px;\
                                font-size: 18px;\
                                font-weight: 500;\
                                line-height: 25px;}");

    middle_layout->addWidget(middle_label,Qt::AlignTop);
    middle_layout->addLayout(paired_dev_layout,Qt::AlignTop);

    frame_middle->setLayout(middle_layout);
}

void BlueToothMain::InitMainbottomUI()
{
    QHBoxLayout *title_layout = new QHBoxLayout();
    title_layout->setSpacing(10);
    title_layout->setContentsMargins(0,0,10,0);

    //~ contents_path /bluetooth/Other Devices
    QLabel *label_1 = new QLabel(tr("Other Devices"),frame_bottom);
    label_1->setStyleSheet("QLabel{\
                          width: 72px;\
                          height: 25px;\
                          font-size: 18px;\
                          font-weight: 500;\
                          line-height: 25px;}");

    loadLabel = new QLabel(frame_bottom);
    loadLabel->setFixedSize(24,24);

    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(100);
        connect(m_timer,&QTimer::timeout,this,&BlueToothMain::Refresh_load_Label_icon);
    }

    discovering_timer = new QTimer(this);
    discovering_timer->setInterval(18000);
    connect(discovering_timer,&QTimer::timeout,this,[=]{
        qDebug() << __FUNCTION__ << "discovering_timer:timeout" << __LINE__ ;
        if(!m_localDevice->isDiscovering()){
            m_localDevice->startDiscovery();
        }
        discovering_timer->stop();
        clearUiShowDeviceList();

        QTimer::singleShot(2000,this,[=]{
            Discovery_device_address.clear();
            discovering_timer->start();
            //this->startDiscovery();
        });
    });


    IntermittentScann_timer_count = 0;
    IntermittentScann_timer= new QTimer(this);
    IntermittentScann_timer->setInterval(2000);
    connect(IntermittentScann_timer,&QTimer::timeout,this,[=]
    {
        qDebug() << __FUNCTION__ << "IntermittentScann_timer_count:" << IntermittentScann_timer_count << __LINE__ ;

        if (IntermittentScann_timer_count >= 2)
        {
            IntermittentScann_timer_count = 0;
            IntermittentScann_timer->stop();
            if(!m_localDevice->isDiscovering())
            {
                m_localDevice->startDiscovery();
            }
            //this->startDiscovery();
            discovering_timer->start();
        }
        else
        {
            if (1 == IntermittentScann_timer_count%2)
            {
                if(m_localDevice->isDiscovering())
                {
                    m_localDevice->stopDiscovery();
                }
            }
            else
            {
                if(!m_localDevice->isDiscovering())
                {
                    m_localDevice->startDiscovery();
                }

            }

        }
        IntermittentScann_timer_count++;

    });

    //开启时延迟1.8s后开启扫描，留点设备回连时间
    delayStartDiscover_timer = new QTimer(this);
    delayStartDiscover_timer->setInterval(2000);
    connect(delayStartDiscover_timer,&QTimer::timeout,this,[=]
    {
        qDebug() << __FUNCTION__ << "delayStartDiscover_timer:timeout" << __LINE__ ;
        delayStartDiscover_timer->stop();
        if(!m_localDevice->isDiscovering())
        {
            m_localDevice->startDiscovery();
        }
        IntermittentScann_timer->start();
        IntermittentScann_timer_count = 0;

    });

    title_layout->addWidget(label_1);
    title_layout->addStretch();
    title_layout->addWidget(loadLabel);

    QVBoxLayout *bottom_layout = new QVBoxLayout(frame_bottom);
    bottom_layout->setSpacing(8);
    bottom_layout->setContentsMargins(0,0,0,0);
    bottom_layout->addLayout(title_layout);

    device_list = new QWidget();

    bottom_layout->addWidget(device_list);

    device_list_layout  = new QVBoxLayout(device_list);
    device_list_layout->setSpacing(2);
    device_list_layout->setContentsMargins(0,0,0,0);
    device_list_layout->setAlignment(Qt::AlignTop);
    device_list->setLayout(device_list_layout);

    frame_bottom->setLayout(bottom_layout);
}

void BlueToothMain::startDiscovery()
{
    if(m_localDevice->isDiscovering()){
        m_localDevice->stopDiscovery();
    }
    m_localDevice->startDiscovery();
}

void BlueToothMain::adapterChanged()
{
    connect(m_manager,&BluezQt::Manager::adapterRemoved,this,[=](BluezQt::AdapterPtr adapter){
//        qDebug() << Q_FUNC_INFO << adapter_address_list.indexOf(adapter.data()->address());
//        qDebug() << Q_FUNC_INFO << adapter_list->currentIndex() << adapter_address_list.at(adapter_list->currentIndex()) << adapter.data()->address() <<__LINE__;
        qDebug() << Q_FUNC_INFO << __LINE__;
        int i = adapter_address_list.indexOf(adapter.data()->address());
        qDebug() << Q_FUNC_INFO << __LINE__ << i;
        adapter_name_list.removeAt(i);
        adapter_address_list.removeAll(adapter.data()->address());
        adapter_list->removeItem(i);

        if((adapter_address_list.size() == adapter_name_list.size())&&(adapter_name_list.size() == 1)){
            frame_2->setVisible(false);
            frame_top->setMinimumSize(582,135);
            frame_top->setMaximumSize(1000,135);
        }
        qDebug() << Q_FUNC_INFO << adapter_address_list.size();
        if (adapter_address_list.size() == 0) {
            if (this->centralWidget()->objectName() == "normalWidget") {
                showMainWindowError();
            }
        }
        qDebug() << Q_FUNC_INFO << __LINE__;
    });

    connect(m_manager,&BluezQt::Manager::adapterAdded,this,[=](BluezQt::AdapterPtr adapter){

        adapter_address_list << adapter.data()->address();
        adapter_name_list << adapter.data()->name();
        qDebug() << Q_FUNC_INFO << adapter_address_list << "===" << adapter_name_list;

        m_localDevice = getDefaultAdapter();

        if (this->centralWidget()->objectName() == "errorWidget") {
            showNormalMainWindow();
        }

        adapter_list->addItem(adapter.data()->name());

        if((adapter_address_list.size() == adapter_name_list.size()) && (adapter_address_list.size() == 1)){
            frame_top->setMinimumSize(582,187);
            frame_top->setMaximumSize(1000,187);
        }else if((adapter_address_list.size() == adapter_name_list.size()) && (adapter_address_list.size() > 1)){
            if(!frame_2->isVisible())
                frame_2->setVisible(true);
            frame_top->setMinimumSize(582,239);
            frame_top->setMaximumSize(1000,239);
        }
    });
}

void BlueToothMain::updateUIWhenAdapterChanged()
{
    qDebug() << Q_FUNC_INFO << __LINE__;

    connect(m_localDevice.data(),&BluezQt::Adapter::poweredChanged,this,&BlueToothMain::adapterPoweredChanged);
    connect(m_localDevice.data(),&BluezQt::Adapter::deviceAdded,this,&BlueToothMain::serviceDiscovered);
    connect(m_localDevice.data(),&BluezQt::Adapter::deviceChanged,this,&BlueToothMain::serviceDiscoveredChange);
    connect(m_localDevice.data(),&BluezQt::Adapter::nameChanged,this,&BlueToothMain::adapterNameChanged);
    connect(m_localDevice.data(),&BluezQt::Adapter::deviceRemoved,this,&BlueToothMain::adapterDeviceRemove);
    connect(m_localDevice.data(),&BluezQt::Adapter::discoveringChanged,this,[=](bool discover){
       if(discover){
           m_timer->start();
           loadLabel->setVisible(true);
       }
       else
       {
           m_timer->stop();
           loadLabel->setVisible(false);
       }
    });
    qDebug() << Q_FUNC_INFO << __LINE__;
    if (m_localDevice->isDiscovering())
    {
        loadLabel->setVisible(true);
        if (!m_timer->isActive())
            m_timer->start();
        //if (!discovering_timer->isActive())
        //    discovering_timer->start();
    }

    connect(m_localDevice.data(),&BluezQt::Adapter::uuidsChanged,this,[=](const QStringList &uuids){
        for (int i = 0 ; i < uuids.size() ; i++)
        {
            qDebug()<< uuids.at(i);
        }
    });


    //==============初始化蓝牙信息和基础信息====================================
     bluetooth_name->set_dev_name(m_localDevice->name());

     if(m_localDevice->isPowered()){
         qDebug() << Q_FUNC_INFO << __LINE__;
         open_bluetooth->setChecked(true);
         bluetooth_name->setVisible(true);
         if(!frame_bottom->isVisible())
             frame_bottom->setVisible(true);
     }else{
         qDebug() << Q_FUNC_INFO << m_manager->isBluetoothBlocked() << __LINE__;
         open_bluetooth->setChecked(false);
         bluetooth_name->setVisible(false);
         frame_bottom->setVisible(false);
         frame_middle->setVisible(false);
     }
     //===========================END==========================================

     // =============清空我的设备和蓝牙发现设备栏布局下的所有设备item=================
     QLayoutItem *child;
     while ((child = paired_dev_layout->takeAt(0)) != 0)
     {
         qDebug() << Q_FUNC_INFO << __LINE__;
         if(child->widget())
         {
             child->widget()->setParent(NULL);
         }
         delete child;
         child = nullptr;
     }
     while ((child = device_list_layout->takeAt(0)) != 0)
     {
         qDebug() << Q_FUNC_INFO << __LINE__;
         if(child->widget())
         {
             child->widget()->setParent(NULL);
         }
         delete child;
         child = nullptr;
     }
     // ========================END===========================================
     qDebug() << Q_FUNC_INFO <<m_localDevice->devices().size();
     show_flag = false;
     Discovery_device_address.clear();
     last_discovery_device_address.clear();

     for(int i = 0;i < m_localDevice->devices().size(); i++)
     {
        qDebug() << m_localDevice->devices().at(i)->name() << m_localDevice->devices().at(i)->type();
        addMyDeviceItemUI(m_localDevice->devices().at(i));
     }
     device_list_layout->addStretch();

     qDebug() << Q_FUNC_INFO << m_localDevice->devices().size() << show_flag;
     if(m_localDevice->isPowered()){
         if(show_flag)
             frame_middle->setVisible(true);
         else
             frame_middle->setVisible(false);
     }

     if(m_localDevice->isPowered())
     {
         m_timer->start();
         delayStartDiscover_timer->start();
     }
}

void BlueToothMain::removeDeviceItemUI(QString address)
{
    qDebug() << Q_FUNC_INFO << address << last_discovery_device_address.indexOf(address) <<__LINE__;

    if(last_discovery_device_address.indexOf(address) != -1){
        DeviceInfoItem *item = device_list->findChild<DeviceInfoItem *>(address);
        if(item){
            device_list_layout->removeWidget(item);
            item->setParent(NULL);
            delete item;
            item = nullptr;
            Discovery_device_address.removeAll(address);
            last_discovery_device_address.removeAll(address);
        }else{
            qDebug() << Q_FUNC_INFO << "NULL"<<__LINE__;
            return;
        }
    }else{
        DeviceInfoItem *item = frame_middle->findChild<DeviceInfoItem *>(address);
        if(item){
            paired_dev_layout->removeWidget(item);
            item->setParent(NULL);
            delete item;
            item = nullptr;

            if(frame_middle->children().size() == 2){
                frame_middle->setVisible(false);
            }
        }else{
            qDebug() << Q_FUNC_INFO << "NULL"<<__LINE__;

            return;
        }
    }
}

void BlueToothMain::addMyDeviceItemUI(BluezQt::DevicePtr device)
{
    qDebug() << __FUNCTION__ << device->name() << device->address() << device->type() << __LINE__;

    DeviceInfoItem *item  = frame_middle->findChild<DeviceInfoItem *>(device->address());
    if (item)
    {
        qDebug() << __FUNCTION__ << device->name()  <<"paired_dev_layout exist" << __LINE__;
        return;
    }

    connect(device.data(),&BluezQt::Device::typeChanged,this,[=](BluezQt::Device::Type  changeType){

        DeviceInfoItem *item = device_list->findChild<DeviceInfoItem *>(device->address());
        if (item)
        {
            item->refresh_device_icon(changeType);
        }
    });

    if (device && device->isPaired()) {
        DeviceInfoItem *item = new DeviceInfoItem();
        connect(item,SIGNAL(sendConnectDevice(QString)),this,SLOT(receiveConnectsignal(QString)));
        connect(item,SIGNAL(sendDisconnectDeviceAddress(QString)),this,SLOT(receiveDisConnectSignal(QString)));
        connect(item,SIGNAL(sendDeleteDeviceAddress(QString)),this,SLOT(receiveRemoveSignal(QString)));
        connect(item,SIGNAL(sendPairedAddress(QString)),this,SLOT(change_device_parent(QString)));
        if(device->isConnected())
            item->initInfoPage(device->name(), DEVICE_STATUS::LINK, device);
        else
            item->initInfoPage(device->name(), DEVICE_STATUS::MATCHED, device);

        show_flag = true;
        paired_dev_layout->addWidget(item,Qt::AlignTop);
    }
    return;
}

void BlueToothMain::MonitorSleepSignal()
{
    if (QDBusConnection::systemBus().connect("org.freedesktop.login1",
                                             "/org/freedesktop/login1",
                                             "org.freedesktop.login1.Manager",
                                             "PrepareForSleep",
                                             this,
                                             SLOT(MonitorSleepSlot(bool)))){
        qDebug() << Q_FUNC_INFO << "PrepareForSleep signal connected successfully to slot";
    } else {
        qDebug() << Q_FUNC_INFO << "PrepareForSleep signal connected was not successful";
    }
}

void BlueToothMain::showNormalMainWindow()
{

    main_widget = new QWidget(this);
    main_widget->setObjectName("normalWidget");
    this->setCentralWidget(main_widget);

    main_layout = new QVBoxLayout(main_widget);
    main_layout->setSpacing(40);
    main_layout->setContentsMargins(0,0,30,10);
    frame_top    = new QWidget(main_widget);
    frame_top->setObjectName("frame_top");
    if(m_manager->adapters().size() > 1){
        frame_top->setMinimumSize(582,239);
        frame_top->setMaximumSize(1000,239);
    }else{
        frame_top->setMinimumSize(582,187);
        frame_top->setMaximumSize(1000,187);
    }
    frame_middle = new QWidget(main_widget);
    frame_middle->setObjectName("frame_middle");
    frame_bottom = new QWidget(main_widget);
    frame_bottom->setObjectName("frame_bottom");
    frame_bottom->setMinimumWidth(582);
    frame_bottom->setMaximumWidth(1000);

    main_layout->addWidget(frame_top,1,Qt::AlignTop);
    main_layout->addWidget(frame_middle,1,Qt::AlignTop);
    main_layout->addWidget(frame_bottom,1,Qt::AlignTop);
    main_layout->addStretch(10);

    Discovery_device_address.clear();
    last_discovery_device_address.clear();


    poweronAgain_timer = new QTimer();
    poweronAgain_timer->setInterval(3000);
    connect(poweronAgain_timer,&QTimer::timeout,this,[=]{
        qDebug() << __FUNCTION__ << "adapterPoweredChanged again" << __LINE__;
        poweronAgain_timer->stop();
        adapterPoweredChanged(true);
    });

    InitMainTopUI();
    InitMainMiddleUI();
    InitMainbottomUI();
    this->setLayout(main_layout);
    MonitorSleepSignal();
    updateUIWhenAdapterChanged();
}

void BlueToothMain::showMainWindowError()
{
    QWidget     *errorWidget       = new QWidget();
    QVBoxLayout *errorWidgetLayout = new QVBoxLayout(errorWidget);
    QLabel      *errorWidgetIcon   = new QLabel(errorWidget);
    QLabel      *errorWidgetTip0   = new QLabel(errorWidget);
    //QLabel      *errorWidgetTip1   = new QLabel(errorWidget);

    errorWidget->setObjectName("errorWidget");

    errorWidgetLayout->setSpacing(10);
    errorWidgetLayout->setMargin(0);
    errorWidgetLayout->setContentsMargins(0,0,0,0);

    errorWidgetIcon->setFixedSize(56,56);
    errorWidgetTip0->resize(200,30);
    errorWidgetTip0->setFont(QFont("Noto Sans CJK SC",18,QFont::Bold));
    //errorWidgetTip1->resize(200,30);

    if (QIcon::hasThemeIcon("dialog-warning")) {
        errorWidgetIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(56,56));
    }

    errorWidgetTip0->setText(tr("No Bluetooth adapter detected !"));
    //errorWidgetTip1->setText(tr("You can refer to the rfkill command for details."));

    errorWidgetLayout->addStretch(10);
    errorWidgetLayout->addWidget(errorWidgetIcon,1,Qt::AlignCenter);
    errorWidgetLayout->addWidget(errorWidgetTip0,1,Qt::AlignCenter);
    //errorWidgetLayout->addWidget(errorWidgetTip1,1,Qt::AlignCenter);
    errorWidgetLayout->addStretch(10);

    this->setCentralWidget(errorWidget);
    delete main_widget;
    main_widget = NULL;
}

void BlueToothMain::updateAdaterInfoList()
{
    adapter_address_list.clear();
    adapter_name_list.clear();
    if(m_manager->adapters().size()){
        for(int i = 0; i < m_manager->adapters().size(); i++){
            adapter_address_list << m_manager->adapters().at(i)->address();
            adapter_name_list    << m_manager->adapters().at(i)->name();
        }
    }
}

BluezQt::AdapterPtr BlueToothMain::getDefaultAdapter()
{
    BluezQt::AdapterPtr value = nullptr;
    if (!m_manager->adapters().size()) {
        return nullptr;
    } else {
        if(m_manager->adapters().size() == 1){
            value = m_manager->adapters().at(0);
        }else{
            if(adapter_address_list.indexOf(Default_Adapter) != -1){
                value = m_manager->adapterForAddress(Default_Adapter);
            }else{
                value = m_manager->adapterForAddress(adapter_address_list.at(0));
            }
        }
    }
    if(settings)
        settings->set("adapter-address",QVariant::fromValue(value->address()));

    return value;
}

void BlueToothMain::MonitorSleepSlot(bool value)
{
    if (!value) {
        if (sleep_status)
        {
            adapterPoweredChanged(true);
            poweronAgain_timer->start();
        }
        else
            adapterPoweredChanged(false);

    } else {
        sleep_status = m_localDevice->isPowered();
    }
}

void BlueToothMain::leaveEvent(QEvent *event)
{
    qDebug() << Q_FUNC_INFO;
}

BlueToothMain::~BlueToothMain()
{
    delete settings;
    settings = nullptr;
    delete device_list;
    device_list = nullptr;
    clearAllDeviceItemUi();

}
void BlueToothMain::clearAllDeviceItemUi()
{
    qDebug() << Q_FUNC_INFO  << m_localDevice->isPowered() <<__LINE__;

    if (m_manager->adapters().size()) {
        foreach (BluezQt::DevicePtr dev, m_localDevice->devices())
        {
            qDebug() << Q_FUNC_INFO  << dev->name();
            if (!dev->isPaired())
                m_localDevice->removeDevice(dev);
        }
    }
}

void BlueToothMain::clearTimer()
{
    if (discovering_timer->isActive())
        discovering_timer->stop();

    if (delayStartDiscover_timer->isActive())
        delayStartDiscover_timer->stop();

    if (IntermittentScann_timer->isActive())
        IntermittentScann_timer->stop();

    if (poweronAgain_timer->isActive())
        poweronAgain_timer->stop();
}

void BlueToothMain::onClick_Open_Bluetooth(bool ischeck)
{
    if(ischeck)
    {
        if (m_manager->isBluetoothBlocked())
            m_manager->setBluetoothBlocked(false);
        BluezQt::PendingCall *call = m_localDevice->setPowered(true);
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *p){
            if(p->error() == 0){
                qDebug() << Q_FUNC_INFO << m_localDevice->isPowered();
            }
            else
            {
                poweronAgain_timer->start();
                qDebug() << "Failed to turn off Bluetooth:" << p->errorText();
            }
        });
    }
    else
    {

        BluezQt::PendingCall *call = m_localDevice->setPowered(false);
        //断电后先删除所有扫描到的蓝牙设备
        clearAllDeviceItemUi();
        clearTimer();

        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *p){
            if(p->error() == 0){
                qDebug() << Q_FUNC_INFO << m_localDevice->isPowered();
                m_manager->setBluetoothBlocked(true);
            }else
                qDebug() << "Failed to turn off Bluetooth:" << p->errorText();
        });
    }
//    qDebug() << Q_FUNC_INFO << ischeck << m_localDevice->isPowered() <<__LINE__;
//    if(ischeck)
//    {
//        if(m_manager->isBluetoothBlocked())
//            m_manager->setBluetoothBlocked(false);
//    }
//    else
//    {
//        clearAllDeviceItemUi();
//        if (!m_manager->isBluetoothBlocked())
//            m_manager->setBluetoothBlocked(true);
//    }
}

void BlueToothMain::addOneBluetoothDeviceItemUi(BluezQt::DevicePtr device)
{

    DeviceInfoItem *item = device_list->findChild<DeviceInfoItem *>(device->address());
    if (item)
    {
        qDebug() << __FUNCTION__ << device->name() <<"device_list exist" << __LINE__;
        return ;
    }

    connect(device.data(),&BluezQt::Device::typeChanged,this,[=](BluezQt::Device::Type  changeType){
        DeviceInfoItem *item = device_list->findChild<DeviceInfoItem *>(device->address());
        if (item)
        {
            item->refresh_device_icon(changeType);
        }
    });

    if(!last_discovery_device_address.contains(device->address()))
    {
        DeviceInfoItem *item = new DeviceInfoItem(device_list);
        connect(item,SIGNAL(sendConnectDevice(QString)),this,SLOT(receiveConnectsignal(QString)));
        connect(item,SIGNAL(sendDisconnectDeviceAddress(QString)),this,SLOT(receiveDisConnectSignal(QString)));
        connect(item,SIGNAL(sendDeleteDeviceAddress(QString)),this,SLOT(receiveRemoveSignal(QString)));
        connect(item,SIGNAL(sendPairedAddress(QString)),this,SLOT(change_device_parent(QString)));
        item->initInfoPage(device->name(), DEVICE_STATUS::UNLINK, device);
        if(device->name() == device->address())
            device_list_layout->addWidget(item,Qt::AlignTop);
        else
        {
            if (device->rssi() > MaxRssiValue)
            {
                qDebug() << __FUNCTION__ << "MaxRssiValue:" << MaxRssiValue << __LINE__;
                MaxRssiValue = device->rssi();
                device_list_layout->insertWidget(0,item,0,Qt::AlignTop);
            }
            else
                device_list_layout->insertWidget(1,item,0,Qt::AlignTop);

        }

        last_discovery_device_address << device->address();
    }
}

void BlueToothMain::serviceDiscovered(BluezQt::DevicePtr device)
{
    qDebug() << Q_FUNC_INFO << device->type() << device->name() << device->address() << device->uuids().size();
    if(device->isPaired()){
        addMyDeviceItemUI(device);
        return;
    }

    if(device->uuids().size() == 0 && device->name().split("-").length() == 6 && device->type() == BluezQt::Device::Uncategorized){
        qDebug() << Q_FUNC_INFO << device->name() << device->type();
        return;
    }
    if(Discovery_device_address.contains(device->address())){
        addOneBluetoothDeviceItemUi(device);
        return;
    }

    addOneBluetoothDeviceItemUi(device);
    Discovery_device_address << device->address();
}


void BlueToothMain::clearUiShowDeviceList()
{
    qDebug() << Q_FUNC_INFO << __LINE__ ;

    for (int i = 0  ; i < last_discovery_device_address.size() ; i++) {

        if (m_manager->adapters().size() > 0)
        {
            BluezQt::DevicePtr device = m_localDevice->deviceForAddress(last_discovery_device_address.at(i));
            if (device->isPaired() || device->isConnected())
                continue;
        }

        //剔除重新开始扫描时，不在设备列表中的device
        if (! Discovery_device_address.contains(last_discovery_device_address.at(i))){
            //removeDeviceItemUI(last_discovery_device_address.at(i));
            receiveRemoveSignal(last_discovery_device_address.at(i));//没有改变动作从列表中删除
        }
    }
}


void BlueToothMain::serviceDiscoveredChange(BluezQt::DevicePtr device)
{
    qDebug() << Q_FUNC_INFO << device->type() << device->name() << device->address() << device->uuids().size() << device->rssi();
    if(device->uuids().size() == 0 && device->name().split("-").length() == 6 && device->type() == BluezQt::Device::Uncategorized){
        return;
    }

    if(device->isPaired() || device->isConnected()) {
        qDebug() << Q_FUNC_INFO << "device is Paired or Connected" << __LINE__;
        //addMyDeviceItemUI(device);
        return;
    }

    if(Discovery_device_address.contains(device->address())){
        addOneBluetoothDeviceItemUi(device);
        return;
    }

    addOneBluetoothDeviceItemUi(device);
    Discovery_device_address << device->address();
}

void BlueToothMain::receiveConnectsignal(QString device)
{

    qDebug() <<__FUNCTION__ << " device name :" << device << __LINE__ ;

    if (m_localDevice->isDiscovering())
    {
        clearTimer();
        m_localDevice->stopDiscovery();
        if (!delayStartDiscover_timer->isActive())
            delayStartDiscover_timer->start();
    }

    int ps_bluetooth = system("ps aux|grep ukui-bluetooth|grep -v \"grep\" ");
    qDebug() <<__FUNCTION__ <<  ps_bluetooth << __LINE__ ;

    if (256 == ps_bluetooth)
    {
        QString cmd_path = "/usr/bin/ukui-bluetooth";
        QFileInfo fileInfo(cmd_path);

        if(fileInfo.exists())
        {
            QProcess *process = new QProcess(this);
            qDebug() << Q_FUNC_INFO;
            process->start(cmd_path,NULL);
        }
    }

    QDBusMessage m = QDBusMessage::createMethodCall("org.ukui.bluetooth","/org/ukui/bluetooth","org.ukui.bluetooth","connectToDevice");
    m << device;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<QString>() <<__LINE__;
    // 发送Message
    QDBusMessage response = QDBusConnection::sessionBus().call(m);
    // 判断Method是否被正确返回
//    if (response.type()== QDBusMessage::ReplyMessage)
//    {
//        // QDBusMessage的arguments不仅可以用来存储发送的参数，也用来存储返回值;
//        bRet = response.arguments().at(0).toBool();
//    }
//    else {
//        qDebug()<<"Message In fail!\n";
//    }
//    qDebug() << Q_FUNC_INFO << __LINE__;
    //    qDebug()<<"bRet:"<<bRet;
}

void BlueToothMain::receiveDisConnectSignal(QString address)
{
    QDBusMessage m = QDBusMessage::createMethodCall("org.ukui.bluetooth","/org/ukui/bluetooth","org.ukui.bluetooth","disConnectToDevice");
    m << address;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<QString>() <<__LINE__;
    // 发送Message
    QDBusMessage response = QDBusConnection::sessionBus().call(m);
}

void BlueToothMain::receiveRemoveSignal(QString address)
{
//    QDBusMessage m = QDBusMessage::createMethodCall("org.ukui.bluetooth","/org/ukui/bluetooth","org.ukui.bluetooth","removeDevice");
//    m << address;
//    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<QString>() <<__LINE__;
//    // 发送Message
//    QDBusMessage response = QDBusConnection::sessionBus().call(m);
    qDebug() << Q_FUNC_INFO << address;
    removeDeviceItemUI(address);
    m_localDevice->removeDevice(m_localDevice->deviceForAddress(address));
}

void BlueToothMain::Refresh_load_Label_icon()
{
    if(this->centralWidget()->objectName() == "normalWidget") {
        if(i == 0)
            i = 7;
        loadLabel->setPixmap(QIcon::fromTheme("ukui-loading-"+QString::number(i,10)).pixmap(24,24));
        loadLabel->update();
        i--;
    }
}

void BlueToothMain::set_tray_visible(bool value)
{
    settings->set("tray-show",QVariant::fromValue(value));
}

void BlueToothMain::set_discoverable(bool value)
{
    if(value){
        if(m_localDevice->discoverableTimeout() != 0)
            m_localDevice->setDiscoverableTimeout(0);
    }
    m_localDevice->setDiscoverable(value);
}

void BlueToothMain::change_adapter_name(const QString &name)
{
    m_localDevice->setName(name);
}

void BlueToothMain::change_device_parent(const QString &address)
{
    qDebug() << Q_FUNC_INFO ;
    if(!frame_middle->isVisible()){
        frame_middle->setVisible(true);
    }

    DeviceInfoItem *item = device_list->findChild<DeviceInfoItem *>(address);
    if(item){
        device_list_layout->removeWidget(item);
        item->setParent(frame_middle);
        paired_dev_layout->addWidget(item);
        Discovery_device_address.removeAll(address);
        last_discovery_device_address.removeAll(address);
    }else{
        return;
    }
}

void BlueToothMain::adapterPoweredChanged(bool value)
{
    qDebug() << Q_FUNC_INFO <<value;
    if(settings)
        settings->set("switch",QVariant::fromValue(value));

    if(value)
    {
        bluetooth_name->set_dev_name(m_localDevice->name());
        bluetooth_name->setVisible(true);
        frame_bottom->setVisible(true);

        if(show_flag)
            frame_middle->setVisible(true);

//         if(!frame_middle->isVisible())
//         {
//            frame_middle->setVisible(true);
//         }

        if (!open_bluetooth->isChecked())
            open_bluetooth->setChecked(true);

        //延时2S开启扫描，给用户回连缓冲
        delayStartDiscover_timer->start();

        //this->startDiscovery();
    }
    else
    {
        if (bluetooth_name->isVisible())
            bluetooth_name->setVisible(false);

        if (open_bluetooth->isChecked())
            open_bluetooth->setChecked(false);

        if (frame_bottom->isVisible())
            frame_bottom->setVisible(false);

        if(frame_middle->isVisible())
            frame_middle->setVisible(false);

        if (!paired_dev_layout->isEmpty())
            show_flag = true ;
        else
            show_flag = false ;


        if(m_localDevice->isDiscovering()){
            m_localDevice->stopDiscovery();
        }
    }

    //switch_discover->setChecked(value);
}



void BlueToothMain::adapterComboxChanged(int i)
{
//    qDebug() << Q_FUNC_INFO << i << adapter_address_list.at(i) << adapter_name_list.at(i) << adapter_address_list << adapter_name_list;
    if(i != -1){
        m_localDevice = m_manager->adapterForAddress(adapter_address_list.at(i));

        if (m_localDevice.isNull())
	    return;
        m_localDevice->stopDiscovery();
        updateUIWhenAdapterChanged();
        if(settings)
            settings->set("adapter-address",QVariant::fromValue(adapter_address_list.at(i)));
            Default_Adapter = adapter_address_list.at(i);
    }else{

        if(open_bluetooth->isChecked()){
            qDebug() << __FUNCTION__<< "index - i : "<< i << __LINE__ ;

            open_bluetooth->setChecked(false);
            open_bluetooth->setDisabledFlag(false);
        }

        if(frame_middle->isVisible()){
            frame_middle->setVisible(false);
        }
    }
}

void BlueToothMain::adapterNameChanged(const QString &name)
{
    emit this->adapter_name_changed(name);

    //设备名字改变，同时改变combox的当前设备名字
     int index;
    index = adapter_address_list.indexOf(m_localDevice->address());
    adapter_name_list.removeAt(index);
    adapter_name_list.insert(index,name);
    adapter_list->setItemText(index,name);
}

void BlueToothMain::adapterDeviceRemove(BluezQt::DevicePtr ptr)
{
//    qDebug() << Q_FUNC_INFO << ptr.data()->address() << ptr.data()->name();
    removeDeviceItemUI(ptr.data()->address());
}


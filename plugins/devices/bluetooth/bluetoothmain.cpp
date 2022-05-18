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
#include "Label/titlelabel.h"

#include <fcntl.h>

enum rfkill_type {
    RFKILL_TYPE_ALL = 0,
    RFKILL_TYPE_WLAN,
    RFKILL_TYPE_BLUETOOTH,
    RFKILL_TYPE_UWB,
    RFKILL_TYPE_WIMAX,
    RFKILL_TYPE_WWAN,
};

enum rfkill_operation {
    RFKILL_OP_ADD = 0,
    RFKILL_OP_DEL,
    RFKILL_OP_CHANGE,
    RFKILL_OP_CHANGE_ALL,
};

struct rfkill_event {
    uint32_t idx;
    uint8_t  type;
    uint8_t  op;
    uint8_t  soft;
    uint8_t  hard;
};

enum {
    OPT_b = (1 << 0), /* must be = 1 */
    OPT_u = (1 << 1),
    OPT_l = (1 << 2),
};

static guint watch    = 0;
bool   spe_bt_node    = false;
bool   not_hci_node   = true;
bool   M_power_on     = false;
bool   M_adapter_flag = false;

static gboolean rfkill_event(GIOChannel *chan,
                GIOCondition cond, gpointer data)
{
    unsigned char buf[32];
    struct rfkill_event *event = (struct rfkill_event *)buf;
    char sysname[PATH_MAX];
    ssize_t len;
    int fd, id;

    if (cond & (G_IO_NVAL | G_IO_HUP | G_IO_ERR))
        return FALSE;

    fd = g_io_channel_unix_get_fd(chan);

    memset(buf, 0, sizeof(buf));
    len = read(fd, buf, sizeof(buf));
    if (len < 0) {
        if (errno == EAGAIN)
            return TRUE;
        return FALSE;
    }

    if (len != sizeof(struct rfkill_event))
        return TRUE;

    qDebug("RFKILL event idx %u type %u op %u soft %u hard %u",
                    event->idx, event->type, event->op,
                        event->soft, event->hard);

    if (event->type != RFKILL_TYPE_BLUETOOTH &&
                    event->type != RFKILL_TYPE_ALL)
    {
        qDebug() << Q_FUNC_INFO << "Not bt====" ;
        return TRUE;
    }
    memset(sysname, 0, sizeof(sysname));
    snprintf(sysname, sizeof(sysname) - 1,
            "/sys/class/rfkill/rfkill%u/name", event->idx);

    fd = open(sysname, O_RDONLY);
    if (fd < 0)
    {
        qDebug () << Q_FUNC_INFO  << __LINE__;

        return TRUE;
    }

    if (read(fd, sysname, sizeof(sysname) - 1) < 4) {
        close(fd);
        qDebug () << Q_FUNC_INFO  << __LINE__;

        return TRUE;
    }

    close(fd);

    if (g_str_has_prefix(sysname, "tpacpi_bluetooth_sw") == TRUE ||
        g_str_has_prefix(sysname, "ideapad_bluetooth") == TRUE   ||
        g_str_has_prefix(sysname, "dell-bluetooth") == TRUE)
    {
        spe_bt_node = true;
        qDebug () << Q_FUNC_INFO <<  "spe_bt_node:" << spe_bt_node  << __LINE__;
        if (event->soft)
        {
            not_hci_node = true ;
            qDebug () << Q_FUNC_INFO <<  "event->soft:" << event->soft  << __LINE__;
        }
        else
            not_hci_node = false ;
    }
    else if (g_str_has_prefix(sysname, "hci") == TRUE)
    {
        qDebug () << Q_FUNC_INFO <<  "not_hci_node:FALSE"  << __LINE__;
        not_hci_node = false;
    }
    else
    {
        qDebug () << Q_FUNC_INFO  << "not_hci_node:TRUE" << __LINE__;
        not_hci_node = true;
    }

    return TRUE;
}

void rfkill_init(void)
{
    qDebug () << Q_FUNC_INFO << __LINE__;

    int fd;
    GIOChannel *channel;

    fd = open("/dev/rfkill", O_RDWR);
    if (fd < 0) {
        return;
    }

    channel = g_io_channel_unix_new(fd);
    g_io_channel_set_close_on_unref(channel, TRUE);

    watch = g_io_add_watch(channel,
                GIOCondition(G_IO_IN | G_IO_NVAL | G_IO_HUP | G_IO_ERR),
                rfkill_event, NULL);

    g_io_channel_unref(channel);
}

void rfkill_set_idx(void)
{
    qDebug () << Q_FUNC_INFO  << __LINE__;
    struct rfkill_event event;

    int rf_fd;
    int mode;
    int rf_type;
    int rf_idx;
    unsigned rf_opt = 0;

    /* Must have one or two params */
    mode = O_RDWR | O_NONBLOCK;

    rf_type = RFKILL_TYPE_BLUETOOTH;
    rf_idx = -1;

    rf_fd = open("/dev/rfkill", mode);

    memset(&event, 0, sizeof(event));
    if (rf_type >= 0) {
        event.type = rf_type;
        event.op = RFKILL_OP_CHANGE_ALL;
    }

    if (rf_idx >= 0) {
        event.idx = rf_idx;
        event.op = RFKILL_OP_CHANGE;
    }

    /* Note: OPT_b == 1 */
    event.soft = (rf_opt & OPT_b);

    write(rf_fd, &event, sizeof(event));
}

void rfkill_exit(void)
{
    if (watch == 0)
        return;
    g_source_remove(watch);
    watch = 0;
}

bool BlueToothMain::isSpebluetooth()
{
    QProcess process;
    process.start("rfkill list");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QString str_output = output;
    bool isDevice1 = str_output.contains(QString("tpacpi_bluetooth_sw"), Qt::CaseInsensitive);
    qDebug() << Q_FUNC_INFO << isDevice1 <<__LINE__;
    bool isDevice2 = str_output.contains(QString("ideapad_bluetooth"), Qt::CaseInsensitive);
    qDebug() << Q_FUNC_INFO << isDevice2 <<__LINE__;
    bool isDevice3 = str_output.contains(QString("dell-bluetooth"), Qt::CaseInsensitive);
    qDebug() << Q_FUNC_INFO << isDevice3 <<__LINE__;

    return isDevice1||isDevice2||isDevice3;
}

BlueToothMain::BlueToothMain(QWidget *parent)
    : QMainWindow(parent)
{
    rfkill_init();
    //rfkill_set_idx();//会导致蓝牙自启动
    spe_bt_node = isSpebluetooth();
    if(QGSettings::isSchemaInstalled("org.ukui.bluetooth"))
    {
        settings = new QGSettings("org.ukui.bluetooth");
        Default_Adapter = settings->get("adapter-address").toString();
        qDebug() << "GSetting Value: " << Default_Adapter/* << finally_connect_the_device << paired_device_address*/;
    }
    StackedWidget = new QStackedWidget(this);
    this->setCentralWidget(StackedWidget);

    //初始化正常蓝牙界面
    InitMainWindowUi();
    //初始化异常蓝牙界面
    InitMainWindowError();
    //初始化所有计时器
    InitAllTimer();
    //初始化蓝牙管理器
    InitBluetoothManager();
    //刷新界面状态
    RefreshWindowUiState();

    //MonitorSleepSignal();
}

BlueToothMain::~BlueToothMain()
{
    delete settings;
    settings = nullptr;
    delete device_list;
    device_list = nullptr;

    //clearAllDeviceItemUi();
}

void BlueToothMain::InitAllTimer()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    btPowerBtnTimer = new QTimer();
    btPowerBtnTimer->setInterval(8000);
    connect(btPowerBtnTimer,&QTimer::timeout,this,[=]
    {
        qDebug() << Q_FUNC_INFO << "btPowerBtnTimer:timeout ";
        btPowerBtnTimer->stop();
        open_bluetooth->setEnabled(true);
    });

    //30s扫描清理一次
    discovering_timer = new QTimer(this);
    discovering_timer->setInterval(28000);
    connect(discovering_timer,&QTimer::timeout,this,[=]{
        qDebug() << __FUNCTION__ << "discovering_timer:timeout" << __LINE__ ;
        discovering_timer->stop();
        clearUiShowDeviceList();
        QTimer::singleShot(2000,this,[=]{
            Discovery_device_address.clear();
            discovering_timer->start();
        });
    });

    m_timer = new QTimer(this);
    m_timer->setInterval(100);
    connect(m_timer,&QTimer::timeout,this,&BlueToothMain::Refresh_load_Label_icon);

//    IntermittentScann_timer_count = 0;
//    IntermittentScann_timer= new QTimer(this);
//    IntermittentScann_timer->setInterval(2000);
//    connect(IntermittentScann_timer,&QTimer::timeout,this,[=]
//    {
//        qDebug() << __FUNCTION__ << "IntermittentScann_timer_count:" << IntermittentScann_timer_count << __LINE__ ;
//        IntermittentScann_timer->stop();
//        if (IntermittentScann_timer_count >= 2)
//        {
//            IntermittentScann_timer_count = 0;
//            IntermittentScann_timer->stop();
//            this->startDiscovery();
//            discovering_timer->start();
//        }
//        else
//        {
//            if (1 == IntermittentScann_timer_count%2)
//            {
//                this->stopDiscovery();
//            }
//            else
//            {
//                this->startDiscovery();
//            }
//            IntermittentScann_timer->start();
//        }
//        IntermittentScann_timer_count++;

//    });

    //开启时延迟2s后开启扫描，留点设备回连时间
    delayStartDiscover_timer = new QTimer(this);
    delayStartDiscover_timer->setInterval(2000);
    connect(delayStartDiscover_timer,&QTimer::timeout,this,[=]
    {
        qDebug() << __FUNCTION__ << "delayStartDiscover_timer:timeout" << __LINE__ ;
        delayStartDiscover_timer->stop();
        this->startDiscovery();
        emit timerStatusChanged(false);

        //IntermittentScann_timer->start();
        //IntermittentScann_timer_count = 0;
    });

    powerOnScanTimer = new QTimer(this);
    powerOnScanTimer->setInterval(1000);
    connect(powerOnScanTimer, &QTimer::timeout, this, [=] {
        bool jud = false;
        QDBusMessage m = QDBusMessage::createMethodCall("org.ukui.bluetooth","/org/ukui/bluetooth","org.ukui.bluetooth","getDiscovering");
        QDBusMessage response = QDBusConnection::sessionBus().call(m);
        if (response.type() == QDBusMessage::ReplyMessage) {
            jud = response.arguments().takeFirst().toBool();
        }
        if (jud) {
            powerOnScanTimer->stop();
            this->startDiscovery();
            emit timerStatusChanged(false);
        }
    });


    poweronAgain_timer = new QTimer();
    poweronAgain_timer->setInterval(3000);
    connect(poweronAgain_timer,&QTimer::timeout,this,[=]{
        qDebug() << __FUNCTION__ << "adapterPoweredChanged again" << __LINE__;
        poweronAgain_timer->stop();
        adapterPoweredChanged(true);
    });

}

void BlueToothMain::InitBluetoothManager()
{
    m_manager = new BluezQt::Manager(this);
    job = m_manager->init();
    job->exec();

    qDebug() << m_manager->isOperational() << m_manager->isBluetoothBlocked();
    updateAdaterInfoList();
    m_localDevice = getDefaultAdapter();

    if (nullptr == m_localDevice)
    {
        qDebug() << Q_FUNC_INFO << "m_localDevice is nullptr";
    }
    connectManagerChanged();
}
/*
 * InitMainWindowUi:
 */

void BlueToothMain::InitMainWindowUi()
{
    //初始化显示框架
    normal_main_widget = new QWidget(this);
    normal_main_widget->setObjectName("normalWidget");

    main_layout = new QVBoxLayout(normal_main_widget);
    main_layout->setSpacing(40);
    main_layout->setContentsMargins(0,0,30,10);
    frame_top = new QWidget(normal_main_widget);
    frame_top->setObjectName("frame_top");
    frame_top->setMinimumSize(582,239);
    frame_top->setMaximumSize(1000,239);

    frame_middle = new QWidget(normal_main_widget);
    frame_middle->setObjectName("frame_middle");
    frame_bottom = new QWidget(normal_main_widget);
    frame_bottom->setObjectName("frame_bottom");
    frame_bottom->setMinimumWidth(582);
    frame_bottom->setMaximumWidth(1000);

    main_layout->addWidget(frame_top,1,Qt::AlignTop);
    main_layout->addWidget(frame_middle,1,Qt::AlignTop);
    main_layout->addWidget(frame_bottom,1,Qt::AlignTop);
    main_layout->addStretch(10);

    InitMainWindowTopUi();
    InitMainWindowMiddleUi();
    InitMainWindowBottomUi();

    StackedWidget->addWidget(normal_main_widget);
}


void BlueToothMain::RefreshWindowUiState()
{
    qDebug() << Q_FUNC_INFO << __LINE__;

    if (nullptr != m_manager)
    {
        RefreshMainWindowTopUi();
        RefreshMainWindowMiddleUi();
        RefreshMainWindowBottomUi();

        if(m_manager->adapters().size() == 0)
        {
            not_hci_node = true;
            M_adapter_flag = false;
            if (spe_bt_node)
                ShowSpecialMainWindow();
            else
                ShowErrorMainWindow();
            return;
        }
        else
        {
            not_hci_node = false;
            M_adapter_flag = true;
            ShowNormalMainWindow();
        }
        qDebug() << Q_FUNC_INFO << spe_bt_node << M_adapter_flag << not_hci_node << __LINE__;

        adapterConnectFun();
    }

}

void BlueToothMain::InitMainWindowTopUi()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    //~ contents_path /bluetooth/Bluetooth
    TitleLabel *label_1 = new TitleLabel(frame_top);
    label_1->setText(tr("Bluetooth"));
    label_1->resize(100,25);

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
    open_bluetooth->setEnabled(true);
    connect(open_bluetooth,SIGNAL(checkedChanged(bool)),this,SLOT(onClick_Open_Bluetooth(bool)));
    frame_1_layout->addWidget(open_bluetooth);

    frame_2 = new QFrame(frame_top);
    frame_2->setMinimumWidth(582);
    frame_2->setFrameShape(QFrame::Shape::Box);
    frame_2->setFixedHeight(50);
    //if(adapter_address_list.size() <= 1)
        frame_2->setVisible(false);
    frame_top->setMinimumSize(582,187);
    frame_top->setMaximumSize(1000,187);
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

    adapter_name_list.clear();
    adapter_list = new QComboBox(frame_2);
    adapter_list->clear();
    adapter_list->setMinimumWidth(300);
    adapter_list->addItems(adapter_name_list);
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
    if(settings)
        show_panel->setChecked(settings->get("tray-show").toBool());
    else
    {
        show_panel->setChecked(false);
        show_panel->setDisabledFlag(false);
    }
    connect(show_panel,&SwitchButton::checkedChanged,this,&BlueToothMain::set_tray_visible);

    qDebug () << Q_FUNC_INFO << "spe_bt_node:" << spe_bt_node << "not_hci_node:" << not_hci_node;

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
    connect(switch_discover,&SwitchButton::checkedChanged,this,&BlueToothMain::set_discoverable);
    frame_top->setLayout(top_layout);
}

void BlueToothMain::RefreshMainWindowTopUi()
{
    qDebug () << Q_FUNC_INFO << "in" ;

    if (spe_bt_node && not_hci_node)
    {
        bluetooth_name->setVisible(false);
    }
    else
    {
        if (nullptr != m_localDevice)
        {
            bluetooth_name->set_dev_name(m_localDevice->name());
            bluetooth_name->setVisible(m_localDevice->isPowered());
        }
    }

    if (nullptr != m_localDevice)
    {
        open_bluetooth->setChecked(m_localDevice->isPowered());
        adapterPoweredChanged(m_localDevice->isPowered());

        switch_discover->setChecked(m_localDevice->isDiscoverable());
        frame_bottom->setVisible(m_localDevice->isPowered());
        frame_middle->setVisible(m_localDevice->isPowered());
    }
    else
    {
        frame_bottom->setVisible(false);
        frame_middle->setVisible(false);
    }

    if((adapter_address_list.size() == adapter_name_list.size()) && (adapter_address_list.size() == 1))
    {
        frame_2->setVisible(false);
        frame_top->setMinimumSize(582,187);
        frame_top->setMaximumSize(1000,187);
    }
    else if((adapter_address_list.size() == adapter_name_list.size()) && (adapter_address_list.size() > 1))
    {
        if(!frame_2->isVisible())
            frame_2->setVisible(true);
        frame_top->setMinimumSize(582,239);
        frame_top->setMaximumSize(1000,239);
    }
    qDebug () << Q_FUNC_INFO << "end" ;

}

void BlueToothMain::addAdapterList(QString newAdapterAddress,QString newAdapterName)
{
    qDebug () << Q_FUNC_INFO << __LINE__ ;
    if (adapter_address_list.indexOf(newAdapterAddress) == -1)
    {
        adapter_address_list << newAdapterAddress ;
        adapter_name_list << newAdapterName ;
        qDebug () << Q_FUNC_INFO << "";
        adapter_list->addItem(newAdapterName);
        if (nullptr != m_localDevice)
        {
            int current_index = adapter_address_list.indexOf(m_localDevice->address());
            adapter_list->setCurrentIndex(current_index);
        }
    }

    qDebug () << Q_FUNC_INFO << adapter_address_list << __LINE__ ;
    qDebug () << Q_FUNC_INFO << adapter_name_list << __LINE__ ;
}

void BlueToothMain::removeAdapterList(QString adapterAddress,QString adapterName)
{
    qDebug () << Q_FUNC_INFO << adapterName  << adapterAddress <<__LINE__ ;
    qDebug () << Q_FUNC_INFO << adapter_address_list << __LINE__ ;
    qDebug () << Q_FUNC_INFO << adapter_name_list << __LINE__ ;

    int index = adapter_address_list.indexOf(adapterAddress);
    if(index < adapter_address_list.size() && index < adapter_name_list.size())
    {
        qDebug () << Q_FUNC_INFO << "removeAdapterList index:" << index ;
        if (index != -1)
        {
            adapter_address_list.removeAt(index);
            adapter_name_list.removeAt(index);
            //
            qDebug () << Q_FUNC_INFO << "disconnect of adapter_list" ;
            disconnect(adapter_list, SIGNAL(currentIndexChanged(int)), nullptr, nullptr);
            adapter_list->clear();
            adapter_list->addItems(adapter_name_list);
            //adapter_list->removeItem(index);
            qDebug () << Q_FUNC_INFO << "add connect of adapter_list" ;
            connect(adapter_list,SIGNAL(currentIndexChanged(int)),this,SLOT(adapterComboxChanged(int)));
            if (adapter_address_list.size() >= 1 && adapter_name_list.size() >= 1)
                adapterComboxChanged(0);
        }
    }

    qDebug () << Q_FUNC_INFO << adapter_address_list << __LINE__ ;
    qDebug () << Q_FUNC_INFO << adapter_name_list << __LINE__ ;
}

void BlueToothMain::RefreshMainWindowMiddleUi()
{
    qDebug () << Q_FUNC_INFO << "in" ;
    if (nullptr == m_localDevice)
        return;

    myDevShowFlag = false;
    Discovery_device_address.clear();
    last_discovery_device_address.clear();

    for(int i = 0;i < m_localDevice->devices().size(); i++)
    {
       qDebug() << m_localDevice->devices().at(i)->name() << m_localDevice->devices().at(i)->type();
       addMyDeviceItemUI(m_localDevice->devices().at(i));
    }
    device_list_layout->addStretch();
    qDebug() << Q_FUNC_INFO << m_localDevice->devices().size() << myDevShowFlag;
    qDebug() << Q_FUNC_INFO << m_localDevice->isPowered();
    if(m_localDevice->isPowered())
    {
        if(myDevShowFlag)
            frame_middle->setVisible(true);
        else
            frame_middle->setVisible(false);
    }
}

void  BlueToothMain::RefreshMainWindowBottomUi()
{
    qDebug () << Q_FUNC_INFO << "in" ;
    if (nullptr == m_localDevice)
        return;

    if(m_localDevice->isPowered())
    {

        frame_bottom->setVisible(true);
        if (m_localDevice->isDiscovering())
           m_timer->start();
        //delayStartDiscover_timer->start();
    }
    else
    {
        frame_bottom->setVisible(false);
    }
}

/*
 *
 *
 */

void BlueToothMain::InitMainWindowMiddleUi()
{
    qDebug()<< Q_FUNC_INFO << __LINE__;
    QVBoxLayout *middle_layout = new QVBoxLayout(frame_middle);
    middle_layout->setSpacing(8);
    middle_layout->setContentsMargins(0,0,0,0);

    paired_dev_layout = new QVBoxLayout();
    paired_dev_layout->setSpacing(2);
    paired_dev_layout->setContentsMargins(0,0,0,0);

    TitleLabel *middle_label = new TitleLabel(frame_middle);
    middle_label->setText(tr("My Devices"));
    middle_label->resize(72,25);

    middle_layout->addWidget(middle_label,Qt::AlignTop);
    middle_layout->addLayout(paired_dev_layout,Qt::AlignTop);

    frame_middle->setLayout(middle_layout);
}


void BlueToothMain::InitMainWindowBottomUi()
{
    qDebug()<< Q_FUNC_INFO << __LINE__;
    QHBoxLayout *title_layout = new QHBoxLayout();
    title_layout->setSpacing(10);
    title_layout->setContentsMargins(0,0,10,0);

    //~ contents_path /bluetooth/Other Devices
    TitleLabel *label_1 = new TitleLabel(frame_bottom);
    label_1->setText(tr("Other Devices"));
    label_1->resize(72,25);

    loadLabel = new QLabel(frame_bottom);
    loadLabel->setFixedSize(24,24);

    if (!m_timer)
    {
        m_timer = new QTimer(this);
        m_timer->setInterval(100);
        connect(m_timer,&QTimer::timeout,this,&BlueToothMain::Refresh_load_Label_icon);
    }

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

void BlueToothMain::ShowNormalMainWindow()
{
    qDebug()<< Q_FUNC_INFO << __LINE__;
    normal_main_widget->setObjectName("normalWidget");

    if (m_manager->adapters().size() > 1)
    {
        if(!frame_2->isVisible())
            frame_2->setVisible(true);
        frame_top->setMinimumSize(582,239);
        frame_top->setMaximumSize(1000,239);
    }
    else
    {
        if(frame_2->isVisible())
            frame_2->setVisible(false);
        frame_top->setMinimumSize(582,187);
        frame_top->setMaximumSize(1000,187);
    }

    StackedWidget->setCurrentWidget(normal_main_widget);
}
void BlueToothMain::ShowSpecialMainWindow()
{
    qDebug()<< Q_FUNC_INFO << __LINE__;
    normal_main_widget->setObjectName("SpeNoteWidget");

    bluetooth_name->setVisible(false);

    if((adapter_address_list.size() == adapter_name_list.size()) && (adapter_address_list.size() == 1))
    {
        frame_top->setMinimumSize(582,187);
        frame_top->setMaximumSize(1000,187);
    }
    else if((adapter_address_list.size() == adapter_name_list.size()) && (adapter_address_list.size() > 1))
    {
        if(!frame_2->isVisible())
            frame_2->setVisible(true);
        frame_top->setMinimumSize(582,239);
        frame_top->setMaximumSize(1000,239);
    }

    frame_middle->setVisible(false);
    frame_bottom->setVisible(false);

    //修复部分机型rfkill block后没有power off信号导致，显示开关为开的问题
    disconnect(open_bluetooth,SIGNAL(checkedChanged(bool)),nullptr,nullptr);
    open_bluetooth->setChecked(false);
    connect(open_bluetooth,SIGNAL(checkedChanged(bool)),this,SLOT(onClick_Open_Bluetooth(bool)));

    StackedWidget->setCurrentWidget(normal_main_widget);
}

void BlueToothMain::InitMainWindowError()
{
    qDebug()<< Q_FUNC_INFO << __LINE__;
    errorWidget = new QWidget();
    QVBoxLayout *errorWidgetLayout = new QVBoxLayout(errorWidget);
    QLabel      *errorWidgetIcon   = new QLabel(errorWidget);
    QLabel      *errorWidgetTip0   = new QLabel(errorWidget);

    errorWidget->setObjectName("errorWidget");

    errorWidgetLayout->setSpacing(10);
    errorWidgetLayout->setMargin(0);
    errorWidgetLayout->setContentsMargins(0,0,0,0);

    errorWidgetIcon->setFixedSize(56,56);
    errorWidgetTip0->resize(200,30);
    errorWidgetTip0->setFont(QFont("Noto Sans CJK SC",18,QFont::Bold));

    if (QIcon::hasThemeIcon("dialog-warning")) {
        errorWidgetIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(56,56));
    }

    errorWidgetTip0->setText(tr("Bluetooth adapter is abnormal !"));

    errorWidgetLayout->addStretch(10);
    errorWidgetLayout->addWidget(errorWidgetIcon,1,Qt::AlignCenter);
    errorWidgetLayout->addWidget(errorWidgetTip0,1,Qt::AlignCenter);
    errorWidgetLayout->addStretch(10);

    StackedWidget->addWidget(errorWidget);
}

void BlueToothMain::ShowErrorMainWindow()
{
    qDebug()<< Q_FUNC_INFO << __LINE__;
    this->setObjectName("errorWidget");

    StackedWidget->setCurrentWidget(errorWidget);
}

void BlueToothMain::connectManagerChanged()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
//    &BluezQt::Manager::operationalChanged
//    &BluezQt::Manager::bluetoothOperationalChanged
//    &BluezQt::Manager::bluetoothBlockedChanged
//    &BluezQt::Manager::adapterAdded
//    &BluezQt::Manager::adapterRemoved
//    &BluezQt::Manager::adapterChanged
//    &BluezQt::Manager::deviceAdded
//    &BluezQt::Manager::deviceRemoved
//    &BluezQt::Manager::deviceChanged
//    &BluezQt::Manager::usableAdapterChanged
//    &BluezQt::Manager::allAdaptersRemoved

    connect(m_manager,&BluezQt::Manager::adapterAdded,this,[=](BluezQt::AdapterPtr adapter)
    {
        qDebug() << Q_FUNC_INFO << "adapterAdded";

        m_localDevice = getDefaultAdapter();
        adapterConnectFun();

        addAdapterList(adapter->address(),adapter->name());
        qDebug() << Q_FUNC_INFO << adapter_address_list << "===" << adapter_name_list;

        not_hci_node = false;
        M_adapter_flag = true;
        if (spe_bt_node && M_power_on)
        {
            //not_hci_node = false;
            if(m_manager->adapters().size() == 1)
                onClick_Open_Bluetooth(true);
        }

        qDebug() << Q_FUNC_INFO << StackedWidget->currentWidget()->objectName() << __LINE__; ;
        ShowNormalMainWindow();

//        if (this->centralWidget()->objectName() == "errorWidget" ||
//            this->centralWidget()->objectName() == "SpeNoteWidget")
//        {
//            ShowNormalMainWindow();
//            //RefreshMainWindowMiddleUi();
//        }
//        else
//        {
//            ShowNormalMainWindow();
//            //RefreshWindowUiState();
//        }

    });

    connect(m_manager,&BluezQt::Manager::adapterRemoved,this,[=](BluezQt::AdapterPtr adapter)
    {
        qDebug() << Q_FUNC_INFO << "adapterRemoved";
        qDebug() << Q_FUNC_INFO << "Removed" << adapter->address() << adapter->name();
        removeAdapterList(adapter->address(),adapter->name());
        qDebug() << Q_FUNC_INFO << __LINE__ << adapter_list->count() << adapter_address_list << adapter_name_list;

        m_localDevice = getDefaultAdapter();
        adapterConnectFun();

        qDebug() << Q_FUNC_INFO << "adapter_address_list : " << adapter_address_list.size() << __LINE__;
        if (adapter_address_list.size() == 0)
        {
            M_adapter_flag = false;
            not_hci_node = true;
            qDebug() << Q_FUNC_INFO << StackedWidget->currentWidget()->objectName() << __LINE__; ;
            if (StackedWidget->currentWidget()->objectName() == "normalWidget")
            {
                if (spe_bt_node)
                    ShowSpecialMainWindow();
                else
                    ShowErrorMainWindow();
            }
        }
        else if (adapter_address_list.size() >= 1)
        {
            ShowNormalMainWindow();
        }
    });


    connect(m_manager,&BluezQt::Manager::adapterChanged,this,[=](BluezQt::AdapterPtr adapter)
    {
        //qDebug() << Q_FUNC_INFO << "adapterChanged" ;
        //qDebug() << Q_FUNC_INFO << adapter->name() << adapter->address();
        if(nullptr != m_localDevice)
        {
            //qDebug() << Q_FUNC_INFO << "adapterChanged=======" ;
            if (m_localDevice->address() == adapter.data()->address())
                m_localDevice = adapter;
        }
        else
        {
            //m_localDevice = adapter;
        }
    });

    connect(m_manager,&BluezQt::Manager::usableAdapterChanged,this,[=](BluezQt::AdapterPtr adapter)
    {
        qDebug() << Q_FUNC_INFO << "usableAdapterChanged" ;
    });

    connect(m_manager,&BluezQt::Manager::allAdaptersRemoved,this,[=]
    {
        qDebug() << Q_FUNC_INFO << "allAdaptersRemoved" ;
        clearAllTimer();
    });
}


void BlueToothMain::adapterConnectFun()
{
    qDebug() << Q_FUNC_INFO << __LINE__;

    if (nullptr == m_localDevice)
    {
        qDebug() << Q_FUNC_INFO << "error: m_localDevice is nullptr";
        return;
    }

    connect(m_localDevice.data(),&BluezQt::Adapter::nameChanged,this,&BlueToothMain::adapterNameChanged);
    connect(m_localDevice.data(),&BluezQt::Adapter::poweredChanged,this,&BlueToothMain::adapterPoweredChanged);
    connect(m_localDevice.data(),&BluezQt::Adapter::deviceAdded,this,&BlueToothMain::serviceDiscovered);
    connect(m_localDevice.data(),&BluezQt::Adapter::deviceChanged,this,&BlueToothMain::serviceDiscoveredChange);
    connect(m_localDevice.data(),&BluezQt::Adapter::deviceRemoved,this,&BlueToothMain::adapterDeviceRemove);
    connect(m_localDevice.data(),&BluezQt::Adapter::discoverableChanged, this, [=](bool discoverable)
    {
        switch_discover->setChecked(discoverable);
    });
    connect(m_localDevice.data(),&BluezQt::Adapter::discoveringChanged,this,[=](bool discover)
    {
        qDebug() << Q_FUNC_INFO << discover << __LINE__;
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
    qDebug() << Q_FUNC_INFO << "end";

}




//============================================================DT
/*
 * Initialize the fixed UI in the upper half of the interface
 *
 */
void BlueToothMain::InitMainTopUI()
{
    qDebug() << Q_FUNC_INFO << __LINE__;

    //~ contents_path /bluetooth/Bluetooth
    TitleLabel *label_1 = new TitleLabel(frame_top);
    label_1->setText(tr("Bluetooth"));
    label_1->resize(100,25);


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
    if (spe_bt_node && not_hci_node)
    {
        bluetooth_name->setVisible(false);
    }
    //if (NULL == open_bluetooth)
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
    adapter_list->setMinimumWidth(300);
    adapter_list->addItems(adapter_name_list);
    if (spe_bt_node && not_hci_node)
    {
        adapter_list->setCurrentIndex(0);
    }
    else
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

    qDebug () << Q_FUNC_INFO << "spe_bt_node:" << spe_bt_node << "not_hci_node:" << not_hci_node;

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
    if (spe_bt_node && not_hci_node)
    {
        //switch_discover->setVisible(false);
    }
    else
    {
        //switch_discover->setVisible(true);

        switch_discover->setChecked(m_localDevice->isDiscoverable());
        connect(switch_discover,&SwitchButton::checkedChanged,this,&BlueToothMain::set_discoverable);
        connect(m_localDevice.data(), &BluezQt::Adapter::discoverableChanged, this, [=](bool discoverable){
        switch_discover->setChecked(discoverable);
        });
    }
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

    TitleLabel *middle_label = new TitleLabel(frame_middle);
    middle_label->setText(tr("My Devices"));
    middle_label->resize(72,25);

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
    TitleLabel *label_1 = new TitleLabel(frame_bottom);
    label_1->setText(tr("Other Devices"));
    label_1->resize(72,25);

    loadLabel = new QLabel(frame_bottom);
    loadLabel->setFixedSize(24,24);

    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(100);
        connect(m_timer,&QTimer::timeout,this,&BlueToothMain::Refresh_load_Label_icon);
    }

    discovering_timer = new QTimer(this);
    discovering_timer->setInterval(28000);
    connect(discovering_timer,&QTimer::timeout,this,[=]{
        qDebug() << __FUNCTION__ << "discovering_timer:timeout" << __LINE__ ;
        discovering_timer->stop();
        clearUiShowDeviceList();

        QTimer::singleShot(2000,this,[=]{
            Discovery_device_address.clear();
            discovering_timer->start();
        });
    });


//    IntermittentScann_timer_count = 0;
//    IntermittentScann_timer= new QTimer(this);
//    IntermittentScann_timer->setInterval(2000);
//    connect(IntermittentScann_timer,&QTimer::timeout,this,[=]
//    {
//        qDebug() << __FUNCTION__ << "IntermittentScann_timer_count:" << IntermittentScann_timer_count << __LINE__ ;
//        IntermittentScann_timer->stop();
//        if (IntermittentScann_timer_count >= 2)
//        {
//            IntermittentScann_timer_count = 0;
//            IntermittentScann_timer->stop();

//            this->startDiscovery();
//            discovering_timer->start();
//        }
//        else
//        {
//            if (1 == IntermittentScann_timer_count%2)
//            {

//                this->stopDiscovery();

//            }
//            else
//            {

//                this->startDiscovery();
//            }

//            IntermittentScann_timer->start();

//        }
//        IntermittentScann_timer_count++;

//    });

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
    qDebug() << Q_FUNC_INFO << __LINE__;
    if (nullptr == m_localDevice)
    {
        qDebug() << Q_FUNC_INFO << "m_localDevice is nullptr !!!" << __LINE__ ;
    }

    if (!m_localDevice->isDiscovering())
    {
        BluezQt::PendingCall *call = m_localDevice->startDiscovery();
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *p)
        {
            if(p->error() == 0)
            {
                qDebug() << Q_FUNC_INFO << "startDiscovery Success " << m_localDevice->isDiscovering();
            }
            else
            {
                qDebug() << Q_FUNC_INFO << "startDiscovery fail " << m_localDevice->isDiscovering();
            }
        });
    }
}

void BlueToothMain::stopDiscovery()
{
    qDebug() << Q_FUNC_INFO << __LINE__;

    if (nullptr == m_localDevice)
    {
        qDebug() << Q_FUNC_INFO << "m_localDevice is nullptr !!!" << __LINE__ ;
    }

    if (m_localDevice->isDiscovering())
    {
        m_localDevice->stopDiscovery();
    }
}

void BlueToothMain::adapterChanged()
{
    connect(m_manager,&BluezQt::Manager::adapterRemoved,this,[=](BluezQt::AdapterPtr adapter){
        qDebug() << Q_FUNC_INFO << __LINE__;
        int i = adapter_address_list.indexOf(adapter.data()->address());
        qDebug() << Q_FUNC_INFO << __LINE__ << adapter_list->count() << adapter_address_list << adapter_name_list << i;
        adapter_name_list.removeAt(i);
        adapter_address_list.removeAll(adapter.data()->address());

        if (m_manager->adapters().size()) {
            adapter_list->removeItem(i);
        }
        qDebug() << Q_FUNC_INFO << __LINE__;

        if((adapter_address_list.size() == adapter_name_list.size())&&(adapter_name_list.size() == 1)){
            frame_2->setVisible(false);
            frame_top->setMinimumSize(582,135);
            frame_top->setMaximumSize(1000,135);
        }
        qDebug() << Q_FUNC_INFO << adapter_address_list.size();
        if (adapter_address_list.size() == 0) {
            not_hci_node = true;
            if (this->centralWidget()->objectName() == "normalWidget") {
                if (spe_bt_node)
                    showSpeNoteMainWindow();
                else
                    showMainWindowError();
            }
        }
        qDebug() << Q_FUNC_INFO << __LINE__;
    });

    connect(m_manager,&BluezQt::Manager::adapterAdded,this,[=](BluezQt::AdapterPtr adapter){

        if (adapter_address_list.indexOf(adapter.data()->address()) == -1) {
            adapter_address_list << adapter.data()->address();
            adapter_name_list << adapter.data()->name();
        }
        qDebug() << Q_FUNC_INFO << adapter_address_list << "===" << adapter_name_list;

        m_localDevice = getDefaultAdapter();
        M_adapter_flag = true ;
        if (spe_bt_node && M_power_on)
        {
            onClick_Open_Bluetooth(true);
        }

//        cleanPairDevices();
        adapterConnectFun();

        if (this->centralWidget()->objectName() == "errorWidget" ||
            this->centralWidget()->objectName() == "SpeNoteWidget") {
            ShowNormalMainWindow();
        }

        if (m_manager->adapters().size() > 1) {
            adapter_list->clear();
            adapter_list->addItems(adapter_name_list);
            adapter_list->setCurrentIndex(adapter_address_list.indexOf(m_localDevice.data()->name()));
        }

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

    connect(m_manager,&BluezQt::Manager::adapterChanged,this,[=](BluezQt::AdapterPtr adapter){
        qDebug() << Q_FUNC_INFO <<__LINE__;

        if (m_localDevice->address() == adapter.data()->address())
            m_localDevice = adapter;
    });

    connect(m_manager,&BluezQt::Manager::usableAdapterChanged,this,[=](BluezQt::AdapterPtr adapter){
        qDebug() << Q_FUNC_INFO <<__LINE__;
    });
}

void BlueToothMain::updateUIWhenAdapterChanged()
{
    qDebug() << Q_FUNC_INFO << __LINE__;

    adapterConnectFun();

    //==============初始化蓝牙信息和基础信息====================================
    bluetooth_name->set_dev_name(m_localDevice->name());

     if(m_localDevice->isPowered())
     {
         qDebug() << Q_FUNC_INFO << __LINE__;
         open_bluetooth->setChecked(true);
         bluetooth_name->setVisible(true);
         if(!frame_bottom->isVisible())
             frame_bottom->setVisible(true);
     }
     else
     {
         qDebug() << Q_FUNC_INFO << m_manager->isBluetoothBlocked() << __LINE__;
         //open_bluetooth->setChecked(false);
         bluetooth_name->setVisible(false);
         frame_bottom->setVisible(false);
         frame_middle->setVisible(false);
     }
     //===========================END==========================================

     // =============清空我的设备和蓝牙发现设备栏布局下的所有设备item=================
     cleanPairDevices();
     // ========================END===========================================
     qDebug() << Q_FUNC_INFO <<m_localDevice->devices().size();
     myDevShowFlag = false;
     Discovery_device_address.clear();
     last_discovery_device_address.clear();

     for(int i = 0;i < m_localDevice->devices().size(); i++)
     {
        qDebug() << m_localDevice->devices().at(i)->name() << m_localDevice->devices().at(i)->type();
        addMyDeviceItemUI(m_localDevice->devices().at(i));
     }
     device_list_layout->addStretch();

     qDebug() << Q_FUNC_INFO << m_localDevice->devices().size() << myDevShowFlag;
     if(m_localDevice->isPowered()){
         if(myDevShowFlag)
             frame_middle->setVisible(true);
         else
             frame_middle->setVisible(false);
     }

     if(m_localDevice->isPowered())
     {

         if (m_localDevice->isDiscovering())
            m_timer->start();
         emit timerStatusChanged(true);
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
    last_discovery_device_address.removeAll(address);
}

void BlueToothMain::addMyDeviceItemUI(BluezQt::DevicePtr device)
{
    qDebug() << __FUNCTION__ << device->name() << device->address() << device->type() << __LINE__;

    DeviceInfoItem *item  = frame_middle->findChild<DeviceInfoItem *>(device->address());
    if (item)
    {
        myDevShowFlag = true;
        return;
    }

    if ((m_localDevice != nullptr) && m_localDevice->isPowered() && !frame_middle->isVisible())
        frame_middle->setVisible(true);
    connect(device.data(),&BluezQt::Device::typeChanged,this,[=](BluezQt::Device::Type  changeType){

        DeviceInfoItem *item = device_list->findChild<DeviceInfoItem *>(device->address());
        if (item)
        {
            item->refresh_device_icon(changeType);
        }
    });

    if (device && device->isPaired()) {
        DeviceInfoItem *item = new DeviceInfoItem();
        item->setObjectName(device->address());
        item->waitForDiscover(delayStartDiscover_timer->isActive());
        connect(item,SIGNAL(sendConnectDevice(QString)),this,SLOT(receiveConnectsignal(QString)));
        connect(item,SIGNAL(sendDisconnectDeviceAddress(QString)),this,SLOT(receiveDisConnectSignal(QString)));
        connect(item,SIGNAL(sendDeleteDeviceAddress(QString)),this,SLOT(receiveRemoveSignal(QString)));
        connect(item,SIGNAL(sendPairedAddress(QString)),this,SLOT(change_device_parent(QString)));

        connect(this, &BlueToothMain::timerStatusChanged, item, [=](bool on) {
            item->waitForDiscover(on);
            item->update();
        });
        connect(item,SIGNAL(connectComplete()),this,SLOT(startBluetoothDiscovery()));
        QGSettings *changeTheme;
        const QByteArray id_Theme("org.ukui.style");
        if (QGSettings::isSchemaInstalled(id_Theme))
            changeTheme = new QGSettings(id_Theme);

        connect(changeTheme, &QGSettings::changed, this, [=] (const QString &key){
           if (key == "iconThemeName"){
               DeviceInfoItem *item = frame_middle->findChild<DeviceInfoItem *>(device->address());
               if (item)
                   item->refresh_device_icon(device->type());
           }
        });
        if(device->isConnected())
            item->initInfoPage(device->name(), DEVICE_STATUS::LINK, device);
        else
            item->initInfoPage(device->name(), DEVICE_STATUS::UNLINK, device);

        myDevShowFlag = true;
        paired_dev_layout->addWidget(item,Qt::AlignTop);
    }
    return;
}

void BlueToothMain::startBluetoothDiscovery()
{

    if (!m_localDevice->isDiscovering())
        m_localDevice->startDiscovery();
    if(!discovering_timer->isActive())
    {
        discovering_timer->start();
    }
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

//void BlueToothMain::showNormalMainWindow()
//{
//    qDebug() << Q_FUNC_INFO << __LINE__;
//    normal_main_widget = new QWidget(this);
//    normal_main_widget->setObjectName("normalWidget");
//    this->setCentralWidget(normal_main_widget);

//    main_layout = new QVBoxLayout(normal_main_widget);
//    main_layout->setSpacing(40);
//    main_layout->setContentsMargins(0,0,30,10);
//    frame_top    = new QWidget(normal_main_widget);
//    frame_top->setObjectName("frame_top");
//    if(m_manager->adapters().size() > 1){
//        frame_top->setMinimumSize(582,239);
//        frame_top->setMaximumSize(1000,239);
//    }else{
//        frame_top->setMinimumSize(582,187);
//        frame_top->setMaximumSize(1000,187);
//    }
//    frame_middle = new QWidget(normal_main_widget);
//    frame_middle->setObjectName("frame_middle");
//    frame_bottom = new QWidget(normal_main_widget);
//    frame_bottom->setObjectName("frame_bottom");
//    frame_bottom->setMinimumWidth(582);
//    frame_bottom->setMaximumWidth(1000);

//    main_layout->addWidget(frame_top,1,Qt::AlignTop);
//    main_layout->addWidget(frame_middle,1,Qt::AlignTop);
//    main_layout->addWidget(frame_bottom,1,Qt::AlignTop);
//    main_layout->addStretch(10);

//    Discovery_device_address.clear();
//    last_discovery_device_address.clear();

//    InitMainTopUI();
//    InitMainMiddleUI();
//    InitMainbottomUI();
//    this->setLayout(main_layout);
//    MonitorSleepSignal();
//    updateUIWhenAdapterChanged();
//}

void BlueToothMain::showMainWindowError()
{
    errorWidget = new QWidget();
    QVBoxLayout *errorWidgetLayout = new QVBoxLayout(errorWidget);
    QLabel      *errorWidgetIcon   = new QLabel(errorWidget);
    QLabel      *errorWidgetTip0   = new QLabel(errorWidget);
    QLabel      *errorWidgetTip1   = new QLabel(errorWidget);

    errorWidget->setObjectName("errorWidget");

    errorWidgetLayout->setSpacing(10);
    errorWidgetLayout->setMargin(0);
    errorWidgetLayout->setContentsMargins(0,0,0,0);

    errorWidgetIcon->setFixedSize(56,56);
    errorWidgetTip0->resize(200,30);
    errorWidgetTip0->setFont(QFont("Noto Sans CJK SC",18,QFont::Bold));
    errorWidgetTip1->resize(200,30);

    if (QIcon::hasThemeIcon("dialog-warning")) {
        errorWidgetIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(56,56));
    }

    errorWidgetTip0->setText(tr("Bluetooth adapter is abnormal !"));
    errorWidgetTip1->setText(tr("You can refer to the rfkill command for details."));

    errorWidgetLayout->addStretch(10);
    errorWidgetLayout->addWidget(errorWidgetIcon,1,Qt::AlignCenter);
    errorWidgetLayout->addWidget(errorWidgetTip0,1,Qt::AlignCenter);
    errorWidgetLayout->addWidget(errorWidgetTip1,1,Qt::AlignCenter);
    errorWidgetLayout->addStretch(10);

    //this->setCentralWidget(errorWidget);
    //delete normal_main_widget;
    //normal_main_widget = NULL;
}

void BlueToothMain::showSpeNoteMainWindow()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    QWidget     *SpeNoteWidget  = new QWidget();
    SpeNoteWidget->setObjectName("SpeNoteWidget");
    this->setCentralWidget(SpeNoteWidget);
    main_layout = new QVBoxLayout(SpeNoteWidget);
    main_layout->setSpacing(40);
    main_layout->setContentsMargins(0,0,30,10);
    frame_top    = new QWidget(SpeNoteWidget);
    frame_top->setObjectName("frame_top");
    if(m_manager->adapters().size() > 1){
        frame_top->setMinimumSize(582,239);
        frame_top->setMaximumSize(1000,239);
    }else{
        frame_top->setMinimumSize(582,187);
        frame_top->setMaximumSize(1000,187);
    }

    main_layout->addWidget(frame_top,1,Qt::AlignTop);
    main_layout->addStretch(10);

    InitMainTopUI();
    this->setLayout(main_layout);

}

void BlueToothMain::updateAdaterInfoList()
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    adapter_address_list.clear();
    adapter_name_list.clear();
    if(m_manager->adapters().size())
    {
        for(int i = 0; i < m_manager->adapters().size(); i++)
        {
            qDebug() << Q_FUNC_INFO << m_manager->adapters().at(i)->address()
                     << m_manager->adapters().at(i)->name() << __LINE__;

            adapter_address_list << m_manager->adapters().at(i)->address();
            adapter_name_list    << m_manager->adapters().at(i)->name();
        }
        //加入adapter_list
        adapter_list->addItems(adapter_name_list);
        if(nullptr != m_localDevice)
            adapter_list->setCurrentIndex(adapter_name_list.indexOf(m_localDevice->name()));
    }
    qDebug () << Q_FUNC_INFO << "adapter_address_list:" << adapter_address_list;
    qDebug () << Q_FUNC_INFO << "adapter_name_list:" << adapter_name_list;

}

BluezQt::AdapterPtr BlueToothMain::getDefaultAdapter()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    BluezQt::AdapterPtr value = nullptr;
    if (!m_manager->adapters().size())
    {
        return nullptr;
    }
    else
    {
        if(m_manager->adapters().size() == 1)
        {
            value = m_manager->adapters().at(0);
        }
        else
        {
            if(adapter_address_list.indexOf(Default_Adapter) != -1)
            {
                value = m_manager->adapterForAddress(Default_Adapter);
            }
            else
            {
                value = m_manager->adapterForAddress(adapter_address_list.at(0));
            }
        }
    }

    if(settings)
        settings->set("adapter-address",QVariant::fromValue(value->address()));
    qDebug() << Q_FUNC_INFO << value.data()->name() << value.data()->address();

    return value;
}


void BlueToothMain::cleanPairDevices()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
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
}

void BlueToothMain::MonitorSleepSlot(bool value)
{
    qDebug() << Q_FUNC_INFO << value;
    if (!value)
    {
        if (sleep_status)
        {
            adapterPoweredChanged(true);
            poweronAgain_timer->start();
        }
        else
            adapterPoweredChanged(false);

    }
    else
    {
        sleep_status = m_localDevice->isPowered();
        qDebug() << Q_FUNC_INFO << "The state before sleep:"<<sleep_status;

    }
}

void BlueToothMain::leaveEvent(QEvent *event)
{
    qDebug() << Q_FUNC_INFO;
}


void BlueToothMain::clearAllDeviceItemUi()
{
    qDebug() << Q_FUNC_INFO <<__LINE__;

    if (m_manager->adapters().size())
    {
        foreach (BluezQt::DevicePtr dev, m_localDevice->devices())
        {
            qDebug() << Q_FUNC_INFO  << dev->name();
            if (!dev->isPaired())
                m_localDevice->removeDevice(dev);
        }
    }
}

void BlueToothMain::clearAllTimer()
{
    qDebug() << Q_FUNC_INFO  << __LINE__;

    //IntermittentScann_timer_count = 0;

    if (discovering_timer->isActive())
        discovering_timer->stop();
    if (delayStartDiscover_timer->isActive())
        delayStartDiscover_timer->stop();
    //if (IntermittentScann_timer->isActive())
    //    IntermittentScann_timer->stop();
    if (poweronAgain_timer->isActive())
        poweronAgain_timer->stop();
    if(m_timer->isActive())
        m_timer->stop();

}

void BlueToothMain::onClick_Open_Bluetooth(bool ischeck)
{
    if (ischeck)
        qDebug() << Q_FUNC_INFO << "User Turn on bluetooth" << __LINE__ ;
    else
        qDebug() << Q_FUNC_INFO << "User Turn off bluetooth" << __LINE__ ;

    open_bluetooth->setEnabled(false);
    btPowerBtnTimer->start();

    if(ischeck)
    {
        if (spe_bt_node && not_hci_node)
        {
            M_power_on = true;
            rfkill_set_idx();
        }

        qDebug() << Q_FUNC_INFO << "spe_bt_node:"   << spe_bt_node;
        qDebug() << Q_FUNC_INFO << "not_hci_node"   << not_hci_node;
        qDebug() << Q_FUNC_INFO << "M_adapter_flag" << M_adapter_flag;

        if (!not_hci_node && M_adapter_flag && (nullptr != m_localDevice))
        {
            if (!spe_bt_node && m_manager->isBluetoothBlocked())
                m_manager->setBluetoothBlocked(false);
            BluezQt::PendingCall *call = m_localDevice->setPowered(true);
            connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *p)
            {
                if(p->error() == 0)
                {
                    qDebug() << Q_FUNC_INFO << "Success to turn on Bluetooth:" << m_localDevice->isPowered();
                }
                else
                {
                    //poweronAgain_timer->start();
                    qDebug() << "Failed to turn on Bluetooth:" << p->errorText();
                }
                //switch_discover->setEnabled(true);
            });
        }
    }
    else
    {

        if (nullptr == m_localDevice)
        {
            qDebug() << Q_FUNC_INFO << "m_localDevice is nullptr!!!" << __LINE__ ;
            return ;
        }

        stopDiscovery();
        M_power_on = false;
        adapterPoweredChanged(false);
        //断电后先删除所有扫描到的蓝牙设备
        clearAllDeviceItemUi();
        clearAllTimer();

        BluezQt::PendingCall *call = m_localDevice->setPowered(false);
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *p){
            if(p->error() == 0)
            {
                qDebug() << Q_FUNC_INFO << "Success to turn off Bluetooth:" << m_localDevice->isPowered();
                if (!spe_bt_node)
                    m_manager->setBluetoothBlocked(true);
            }
            else
                qDebug() << "Failed to turn off Bluetooth:" << p->errorText();
        });
    }

    qDebug() << Q_FUNC_INFO << "end" << __LINE__ ;
}

void BlueToothMain::addOneBluetoothDeviceItemUi(BluezQt::DevicePtr device)
{

    DeviceInfoItem *item = device_list->findChild<DeviceInfoItem *>(device->address());
    if (item)
    {
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
        item->setObjectName(device->address());
        item->waitForDiscover(delayStartDiscover_timer->isActive());
        connect(item,SIGNAL(sendConnectDevice(QString)),this,SLOT(receiveConnectsignal(QString)));
        connect(item,SIGNAL(sendDisconnectDeviceAddress(QString)),this,SLOT(receiveDisConnectSignal(QString)));
        connect(item,SIGNAL(sendDeleteDeviceAddress(QString)),this,SLOT(receiveRemoveSignal(QString)));
        connect(item,SIGNAL(sendPairedAddress(QString)),this,SLOT(change_device_parent(QString)));
        connect(this, &BlueToothMain::timerStatusChanged, item, [=](bool on) {
            item->waitForDiscover(on);
            item->update();
        });
        connect(item,SIGNAL(connectComplete()),this,SLOT(startBluetoothDiscovery()));

        QGSettings *changeTheme;
        const QByteArray id_Theme("org.ukui.style");
        if (QGSettings::isSchemaInstalled(id_Theme))
            changeTheme = new QGSettings(id_Theme);
        connect(changeTheme, &QGSettings::changed, this, [=] (const QString &key){
           if (key == "iconThemeName"){
               DeviceInfoItem *item = device_list->findChild<DeviceInfoItem *>(device->address());
               if (item)
                   item->refresh_device_icon(device->type());
           }
        });

        item->initInfoPage(device->name(), DEVICE_STATUS::UNLINK, device);
        if(device->name() == device->address())
            device_list_layout->addWidget(item,Qt::AlignTop);
        else
        {
            device_list_layout->insertWidget(0,item,0,Qt::AlignTop);
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
    for (int i = 0  ; i < last_discovery_device_address.size() ; i++) {
        //剔除重新开始扫描时，不在设备列表中的device
        if (! Discovery_device_address.contains(last_discovery_device_address.at(i))){
            //removeDeviceItemUI(last_discovery_device_address.at(i));
            receiveRemoveSignal(last_discovery_device_address.at(i));
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

    if (m_localDevice->isDiscovering())
    {
        clearAllTimer();
        m_localDevice->stopDiscovery();
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
    if(StackedWidget->currentWidget()->objectName() == "normalWidget") {
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
    qDebug() << Q_FUNC_INFO << value;
    if (nullptr == m_localDevice)
        return;

    if(value)
    {
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

//void BlueToothMain::delay_adapterPoweredChanged(bool value)
//{
//    if (value)
//    {
//        QTimer::singleShot(1000,this,[=]{
//            adapterPoweredChanged(value);
//        });
//    }
//    else
//    {
//        adapterPoweredChanged(value);
//    }
//}

void BlueToothMain::adapterPoweredChanged(bool value)
{
    qDebug() << Q_FUNC_INFO << value;

    if(m_localDevice == nullptr)
    {
        qDebug() << Q_FUNC_INFO << value;
        return;
    }

    btPowerBtnTimer->stop();
    open_bluetooth->setEnabled(true);

    if(settings)
        settings->set("switch",QVariant::fromValue(value));

    if(value)
    {
        bluetooth_name->set_dev_name(m_localDevice->name());
        bluetooth_name->setVisible(true);
        frame_bottom->setVisible(true);

        if(myDevShowFlag)
            frame_middle->setVisible(true);

        if (!open_bluetooth->isChecked())
            open_bluetooth->setChecked(true);

        //延时2S开启扫描，给用户回连缓冲
        powerOnScanTimer->start();

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
            myDevShowFlag = true ;
        else
            myDevShowFlag = false ;

        emit timerStatusChanged(true);

        if(nullptr != m_localDevice && m_localDevice->isDiscovering()){
            m_localDevice->stopDiscovery();
        }
    }

    //switch_discover->setChecked(value);
    qDebug() << Q_FUNC_INFO << "end" << __LINE__;

}



void BlueToothMain::adapterComboxChanged(int i)
{
    qDebug() << Q_FUNC_INFO << i <<
                adapter_address_list.at(i) <<
                adapter_name_list.at(i) <<
                adapter_address_list <<
                adapter_name_list;

    if(i != -1)
    {
        if (i >= m_manager->adapters().size())
        {
            return;
        }
        else
        {

            m_localDevice = m_manager->adapterForAddress(adapter_address_list.at(i));
            if (m_localDevice.isNull())
                return;
            m_localDevice->stopDiscovery();

            updateUIWhenAdapterChanged();
            if(settings)
                settings->set("adapter-address",QVariant::fromValue(adapter_address_list.at(i)));
                Default_Adapter = adapter_address_list.at(i);
        }
    }
    else
    {
        if(open_bluetooth->isChecked())
        {
            qDebug() << __FUNCTION__<< "index - i : "<< i << __LINE__ ;
            open_bluetooth->setChecked(false);
            open_bluetooth->setDisabledFlag(false);
        }

        if(frame_middle->isVisible())
        {
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


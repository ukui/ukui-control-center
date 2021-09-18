#include "mobilehotspotwidget.h"
#include <QDebug>

#define LABEL_RECT 17, 0, 105, 23
#define CONTENTS_MARGINS 0, 0, 40, 0
#define FRAME_MIN_SIZE 550, 0
#define FRAME_MAX_SIZE 16777215, 16777215
#define CONTECT_FRAME_MAX_SIZE 16777215, 60
#define LABLE_MIN_WIDTH 140
#define COMBOBOX_MIN_WIDTH 600
#define LINE_MAX_SIZE 16777215, 1
#define LINE_MIN_SIZE 0, 1

#define WIRELESS   1

void showDesktopNotify(const QString &message)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QList<QVariant> args;
    args<<("Kylin Cloud Account")
    <<((unsigned int) 0)
    <<QString("kylin-cloud-account")
    <<"Cloud ID desktop message" //显示的是什么类型的信息
    <<message //显示的具体信息
    <<QStringList()
    <<QVariantMap()
    <<(int)-1;
    iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
}

MobileHotspotWidget::MobileHotspotWidget(QWidget *parent) : QWidget(parent)
{
    mVlayout = new QVBoxLayout(this);
    mVlayout->setContentsMargins(CONTENTS_MARGINS);

    qDBusRegisterMetaType<QMap<QString, bool> >();
    qDBusRegisterMetaType<QVector<QStringList> >();
    qDBusRegisterMetaType<QMap<QString, QVector<QStringList> >>();

    initUI();
    m_switchBtn->installEventFilter(this);
    m_interface = new QDBusInterface("com.kylin.network", "/com/kylin/network",
                                     "com.kylin.network",
                                     QDBusConnection::sessionBus());
    if(!m_interface->isValid()) {
        qDebug() << "dbus interface com.kylin.network is invaild";
        m_switchBtn->setChecked(false);
        setUiEnabled(false);
    }

    initInterfaceInfo();
    getApInfo();

    initDbusConnect();

    connect(m_switchBtn, &SwitchButton::checkedChanged, this, &MobileHotspotWidget::setUiEnabled);
    connect(m_interfaceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() {
        m_interfaceName = m_interfaceComboBox->currentText();
    });
    connect(m_pwdShowBox, &QCheckBox::clicked, this, [=]() {
        if (m_pwdNameLine->echoMode() == QLineEdit::Password) {
            m_pwdNameLine->setEchoMode(QLineEdit::Normal);
        } else {
            m_pwdNameLine->setEchoMode(QLineEdit::Password);
        }
    });
}

MobileHotspotWidget::~MobileHotspotWidget()
{
    delete m_interface;
}

bool MobileHotspotWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (watched == m_switchBtn) {
            if (!m_interface->isValid()) {
                return true;
            }
            if (m_switchBtn->getDisabledFlag()) {
                return true;
            }
            if (m_switchBtn->isChecked()) {
                QDBusReply<void> reply = m_interface->call("deactiveWirelessAp",
                                  m_apNameLine->text(),
                                  m_pwdNameLine->text(),
                                  m_interfaceComboBox->currentText());
                if (!reply.isValid()) {
                    qDebug() << "[MobileHotspotWidget] call deactiveWirelessAp failed ";
                    return true;
                }
//                showDesktopNotify(tr("start to deactive hotspot ") + m_apNameLine->text());
            } else {
                if (m_apNameLine->text().isEmpty() || m_interfaceName.isEmpty())
                {
                    return true;
                }
                QDBusReply<void> reply = m_interface->call("activeWirelessAp",
                                  m_apNameLine->text(),
                                  m_pwdNameLine->text(),
                                  m_interfaceComboBox->currentText());
                if (!reply.isValid()) {
                    qDebug() << "[MobileHotspotWidget] call deactiveWirelessAp failed ";
                    return true;
                }
//                showDesktopNotify(tr("start to create hotspot ") + m_apNameLine->text());
            }
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void MobileHotspotWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void MobileHotspotWidget::initUI()
{
    QFrame *hotspotFrame = new QFrame(this);
    hotspotFrame->setMinimumSize(FRAME_MIN_SIZE);
    hotspotFrame->setMaximumSize(FRAME_MAX_SIZE);
    hotspotFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *hotspotLyt = new QVBoxLayout(hotspotFrame);
    hotspotLyt->setContentsMargins(0, 0, 0, 0);

    m_hotspotTitleLabel = new TitleLabel(this);
    m_hotspotTitleLabel->setText(tr("Hotspot"));

    setSwitchFrame();
    setApNameFrame();
    setPasswordFrame();
    setFreqBandFrame();
    setInterFaceFrame();

    switchAndApNameLine = myLine();
    apNameAndPwdLine = myLine();
    pwdAndfreqBandLine = myLine();
    freqBandAndInterfaceLine = myLine();

    /* add widget */
    hotspotLyt->addWidget(m_switchFrame);
    hotspotLyt->addWidget(switchAndApNameLine);
    hotspotLyt->addWidget(m_ApNameFrame);
    hotspotLyt->addWidget(apNameAndPwdLine);
    hotspotLyt->addWidget(m_passwordFrame);
    hotspotLyt->addWidget(pwdAndfreqBandLine);
    hotspotLyt->addWidget(m_freqBandFrame);
    hotspotLyt->addWidget(freqBandAndInterfaceLine);
    hotspotLyt->addWidget(m_interfaceFrame);
    hotspotLyt->setSpacing(0);

    mVlayout->addWidget(m_hotspotTitleLabel);
    mVlayout->setSpacing(8);
    mVlayout->addWidget(hotspotFrame);
    mVlayout->addStretch();
}

void MobileHotspotWidget::initDbusConnect()
{
    if(m_interface->isValid()) {
        connect(m_interface,SIGNAL(activateFailed(QString)), this, SLOT(onActivateFailed(QString)));
        connect(m_interface,SIGNAL(deactivateFailed(QString)), this, SLOT(onDeactivateFailed(QString)));
        connect(m_interface,SIGNAL(deviceStatusChanged()), this, SLOT(onDeviceStatusChanged()));
        connect(m_interface,SIGNAL(deviceNameChanged(QString, QString)), this, SLOT(onDeviceNameChanged(QString, QString)));
        connect(m_interface,SIGNAL(hotspotDeactivated(QString, QString)), this, SLOT(onHotspotDeactivated(QString, QString)));
        connect(m_interface,SIGNAL(hotspotActivated(QString, QString)), this, SLOT(onHotspotActivated(QString, QString)));
        connect(m_interface,SIGNAL(listUpdate(QString)), this, SLOT(onListUpdate(QString)));
    }
}

void MobileHotspotWidget::initInterfaceInfo()
{
    if(!m_interface->isValid()) {
        return;
    }
    m_interfaceComboBox->clear();
    QDBusReply<QMap<QString, bool> > reply = m_interface->call("getDeviceListAndEnabled",WIRELESS);

    if (!reply.isValid()) {
        qDebug()<<"execute dbus method 'getDeviceListAndEnabled' is invalid in func getObjectPath()";
        setWidgetHidden(true);
        return;
    }
    QMap<QString, bool> devMap = reply.value();

    if (devMap.isEmpty()) {
        qDebug() << "no wireless device";
        setWidgetHidden(true);
    } else {
        QMap<QString, bool>::Iterator iter = devMap.begin();
        while (iter != devMap.end()) {
            if (iter.value()) {
                QString interfaceName = iter.key();
                m_interfaceComboBox->addItem(interfaceName);
            }

            iter++;
        }
        if (m_interfaceComboBox->count() > 0) {
            setWidgetHidden(false);
            m_interfaceName = m_interfaceComboBox->currentText();
        } else {
            qDebug() << "no useable wireless device";
            setWidgetHidden(true);
        }
    }
}

void MobileHotspotWidget::getApInfo()
{
    if(!m_interface->isValid()) {
        return;
    }
    QDBusReply<QStringList> reply = m_interface->call("getStoredApInfo");
    if (!reply.isValid()) {
        qDebug()<<"execute dbus method 'getStoredApInfo' is invalid in func getObjectPath()";
    }

    QStringList apInfo = reply.value();

    if (apInfo.isEmpty()) {
        qDebug() << "no stored hotspot info";
        return;
    } else {
        int index = m_interfaceComboBox->findText(apInfo.at(2));
        if (index >= 0) {
            m_apNameLine->setText(apInfo.at(0));
            m_pwdNameLine->setText(apInfo.at(1));
            m_interfaceComboBox->setCurrentIndex(index);
            m_interfaceName = apInfo.at(2);
            if (apInfo.at(3) == "true") {
                m_switchBtn->setChecked(true);
                setUiEnabled(false);
            } else {
                m_switchBtn->setChecked(false);
                setUiEnabled(true);
            }
        } else {
            qDebug() << "no such interface " << apInfo.at(2);
        }
    }
}

void MobileHotspotWidget::setSwitchFrame()
{
    /* Open */
    m_switchFrame = new QFrame(this);
    m_switchFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_switchFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_switchFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *switchLayout = new QHBoxLayout();

    m_switchLabel = new QLabel(tr("Open"), this);
    m_switchLabel->setMinimumWidth(LABLE_MIN_WIDTH);
    m_switchBtn = new SwitchButton(this);
    switchLayout->addWidget(m_switchLabel);
    switchLayout->addStretch();
    switchLayout->addWidget(m_switchBtn);

    m_switchFrame->setLayout(switchLayout);
}

void MobileHotspotWidget::setApNameFrame()
{
    /* ApName */
    m_ApNameFrame = new QFrame(this);
    m_ApNameFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_ApNameFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_ApNameFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *apNameHLayout = new QHBoxLayout();

    m_apNameLabel = new QLabel(tr("Wi-Fi Name"), this);
    m_apNameLabel->setMinimumWidth(LABLE_MIN_WIDTH);
    m_apNameLine = new QLineEdit(this);
    m_apNameLine->setMinimumWidth(COMBOBOX_MIN_WIDTH);
    apNameHLayout->addWidget(m_apNameLabel);
    apNameHLayout->addWidget(m_apNameLine);
    m_ApNameFrame->setLayout(apNameHLayout);

}

void MobileHotspotWidget::setPasswordFrame()
{
    /* Password */
    m_passwordFrame = new QFrame(this);
    m_passwordFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_passwordFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_passwordFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *passwordHLayout = new QHBoxLayout();

    m_pwdLabel = new QLabel(tr("Password"), this);
    m_pwdLabel->setMinimumWidth(LABLE_MIN_WIDTH);
    m_pwdNameLine = new QLineEdit(this);
    m_pwdNameLine->setMinimumWidth(COMBOBOX_MIN_WIDTH);
    m_pwdNameLine->setEchoMode(QLineEdit::Password);
    passwordHLayout->addWidget(m_pwdLabel);
    passwordHLayout->addWidget(m_pwdNameLine);

    m_passwordFrame->setLayout(passwordHLayout);

    m_pwdShowBox = new QCheckBox(this);
    m_pwdShowBox->setStyleSheet("QCheckBox::indicator {width: 18px; height: 9px;}"
                                       "QCheckBox::indicator:checked {image: url(:/img/plugins/mobilehotspot/show-pwd.png);}"
                                       "QCheckBox::indicator:unchecked {image: url(:/img/plugins/mobilehotspot/hide-pwd.png);}");
    m_pwdShowBox->setCursor(Qt::PointingHandCursor);
    m_pwdShowBox->setFixedSize(30, m_pwdNameLine->height());
    //防止文本框输入内容位于按钮之下
    QMargins margins = m_pwdNameLine->textMargins();
    m_pwdNameLine->setTextMargins(margins.left(), margins.top(), m_pwdShowBox->width(), margins.bottom());
    QHBoxLayout *pSearchLayout = new QHBoxLayout();
    pSearchLayout->addStretch();
    pSearchLayout->addWidget(m_pwdShowBox);
    pSearchLayout->setSpacing(0);
    pSearchLayout->setContentsMargins(0, 0, 0, 0);
    m_pwdNameLine->setLayout(pSearchLayout);
    m_pwdNameLine->setEchoMode(QLineEdit::Password);
}

void MobileHotspotWidget::setFreqBandFrame()
{
    /* frequency band */
    m_freqBandFrame = new QFrame(this);
    m_freqBandFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_freqBandFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_freqBandFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *freqBandHLayout = new QHBoxLayout();

    m_freqBandLabel = new QLabel(tr("Frequency band"), this);
    m_freqBandLabel->setMinimumWidth(LABLE_MIN_WIDTH);
    m_freqBandComboBox = new QComboBox(this);
    m_freqBandComboBox->setInsertPolicy(QComboBox::NoInsert);
    m_freqBandComboBox->setMinimumWidth(COMBOBOX_MIN_WIDTH);
    m_freqBandComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    m_freqBandComboBox->addItem("2.4Ghz");
    m_freqBandComboBox->addItem("5Ghz");
    freqBandHLayout->addWidget(m_freqBandLabel);
    freqBandHLayout->addWidget(m_freqBandComboBox);

    m_freqBandFrame->setLayout(freqBandHLayout);
}

void MobileHotspotWidget::setInterFaceFrame()
{
    /* key tips */
    m_interfaceFrame = new QFrame(this);
    m_interfaceFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_interfaceFrame->setMinimumSize(FRAME_MIN_SIZE);
    m_interfaceFrame->setMaximumSize(CONTECT_FRAME_MAX_SIZE);

    QHBoxLayout *interfaceHLayout = new QHBoxLayout;

    m_interfaceLabel = new QLabel(tr("Net card"), this);
    m_interfaceLabel->setMinimumWidth(LABLE_MIN_WIDTH);
    m_interfaceComboBox = new QComboBox(this);
    m_interfaceComboBox->setInsertPolicy(QComboBox::NoInsert);
    m_interfaceComboBox->setMinimumWidth(COMBOBOX_MIN_WIDTH);
    m_interfaceComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    interfaceHLayout->addWidget(m_interfaceLabel);
    interfaceHLayout->addWidget(m_interfaceComboBox);

    m_interfaceFrame->setLayout(interfaceHLayout);
}

void MobileHotspotWidget::onListUpdate(QString devName)
{
    if (!m_switchBtn->isChecked()) {
        return;
    }
    if (!devName.isEmpty()) {
        if (devName  != m_interfaceName) {
            return;
        }
    }

    QDBusReply<QMap<QString, QVector<QStringList>> > reply = m_interface->call("getWirelessList");
    if (!reply.isValid()) {
        qDebug()<<"execute dbus method 'getWirelessList' is invalid in func onListUpdate()";
    }
    m_interfaceComboBox->clear();
    QMap<QString, QVector<QStringList> > wifiMap = reply.value();

    bool isFind = false;
    if (wifiMap.contains(m_interfaceName)) {
        for (int index = 0; index < wifiMap[m_interfaceName].size(); ++index) {
            if (wifiMap[m_interfaceName][index].contains(m_apNameLine->text())) {
                isFind = true;
                break;
            }
        }
    }

    if (!isFind) {
        onHotspotDeactivated(m_interfaceName, m_apNameLine->text());
        initInterfaceInfo();
        getApInfo();
    }

}

void MobileHotspotWidget::onActivateFailed(QString errorMessage)
{
    if (errorMessage.indexOf("hotspot")) {
        //todo
//        showDesktopNotify(errorMessage);
    }
}

void MobileHotspotWidget::onDeactivateFailed(QString errorMessage)
{
    if (errorMessage.indexOf("hotspot")) {
//        showDesktopNotify(errorMessage);
    }
}

//设备插拔
void MobileHotspotWidget::onDeviceStatusChanged()
{
    initInterfaceInfo();
    getApInfo();
}

void MobileHotspotWidget::onDeviceNameChanged(QString oldName, QString newName)
{
    int index = m_interfaceComboBox->findText(oldName);
    if (index >= 0) {
        m_interfaceComboBox->setItemData(index, newName);
        if (m_interfaceName == oldName) {
            m_interfaceName = newName;
        }
    }
}

//热点断开
void MobileHotspotWidget::onHotspotDeactivated(QString devName, QString ssid)
{
    if (!m_switchBtn->isChecked()) {
        return;
    }
    if (devName == m_interfaceComboBox->currentText() && ssid == m_apNameLine->text()) {
        m_switchBtn->setChecked(false);
        setUiEnabled(true);
//        showDesktopNotify(tr("deactive hotspot ") + ssid + tr(" success"));
    }
}

//热点连接
void MobileHotspotWidget::onHotspotActivated(QString devName, QString ssid)
{
    if (m_switchBtn->isChecked()) {
        return;
    }
    if (devName == m_interfaceComboBox->currentText() && ssid == m_apNameLine->text()) {
        m_switchBtn->setChecked(true);
        setUiEnabled(false);
//        showDesktopNotify(tr("create hotspot ") + ssid + tr(" success"));
    }
}

void MobileHotspotWidget::setUiEnabled(bool enable)
{
    if (!enable) {
        m_apNameLine->setEnabled(false);
        m_pwdNameLine->setEnabled(false);
        m_freqBandComboBox->setEnabled(false);
        m_interfaceComboBox->setEnabled(false);
    } else {
        m_apNameLine->setEnabled(true);
        m_pwdNameLine->setEnabled(true);
        m_freqBandComboBox->setEnabled(true);
        m_interfaceComboBox->setEnabled(true);
    }
}

void MobileHotspotWidget::setWidgetHidden(bool isHidden)
{
    m_ApNameFrame->setHidden(isHidden);
    m_passwordFrame->setHidden(isHidden);
    m_freqBandFrame->setHidden(isHidden);
    m_interfaceFrame->setHidden(isHidden);

    switchAndApNameLine->setHidden(isHidden);
    apNameAndPwdLine->setHidden(isHidden);
    pwdAndfreqBandLine->setHidden(isHidden);
    freqBandAndInterfaceLine->setHidden(isHidden);

    if (isHidden) {
        m_switchBtn->setChecked(false);
        m_interfaceName = "";
    }
}

QFrame* MobileHotspotWidget::myLine()
{
    QFrame *line = new QFrame(this);
    line->setMinimumSize(QSize(LINE_MIN_SIZE));
    line->setMaximumSize(QSize(LINE_MAX_SIZE));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

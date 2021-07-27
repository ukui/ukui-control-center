#include "netlistitem.h"
#include <QIcon>

NetListItem::NetListItem(const QString &icon_path, const QString &net_name, QWidget *parent) : QWidget(parent)
{
    if (icon_path.contains("wifi")) {
        initWifiWidget(icon_path, net_name);
    } else {
        initLanWidget(icon_path, net_name);
    }
}

NetListItem::~NetListItem()
{
    if (m_iconLabel) {
        delete m_iconLabel;
        m_iconLabel = NULL;
    }
    if (m_nameLabel) {
        delete m_nameLabel;
        m_nameLabel = NULL;
    }
    if (m_statusLabel) {
        delete m_statusLabel;
        m_statusLabel = NULL;
    }
    if (m_circleLabel) {
        delete m_circleLabel;
        m_circleLabel = NULL;
    }
    if (m_devHorLayout) {
        delete m_devHorLayout;
        m_devHorLayout = NULL;
    }
    if (m_devFrame) {
        delete m_devFrame;
        m_devFrame = NULL;
    }
    if (m_baseVerLayout) {
        delete m_baseVerLayout;
        m_baseVerLayout = NULL;
    }
}

void NetListItem::setActive(const bool &isAct)
{
    if (isAct) {
        if (!m_active) { //原来未连接，现要将此wifi显示为已连接
            m_statusLabel->show();
            if (m_secu) {
                m_iconLabel->setPixmap(QIcon(QString(":/img/plugins/netconnect/wifionpwd%1.svg").arg(m_signal)).pixmap(QSize(24, 24)));
            } else {
                m_iconLabel->setPixmap(QIcon(QString(":/img/plugins/netconnect/wifion%1.png").arg(m_signal)).pixmap(QSize(24, 24)));
            }
        }
    } else {
        if (m_active) { //原来已连接，现要将此wifi显示为未连接
            m_statusLabel->hide();
            if (m_secu) {
                m_iconLabel->setPixmap(QIcon(QString(":/img/plugins/netconnect/wifipwd%1.svg").arg(m_signal)).pixmap(QSize(24, 24)));
            } else {
                m_iconLabel->setPixmap(QIcon(QString(":/img/plugins/netconnect/wifi%1.png").arg(m_signal)).pixmap(QSize(24, 24)));
            }
        }
    }
    m_active = isAct;
}

bool NetListItem::isActive()
{
    return m_active;
}

void NetListItem::initWifiWidget(const QString &icon_path, const QString &net_name)
{
    m_signal = icon_path.mid(icon_path.indexOf(".") - 1, 1).toInt();
    if (icon_path.contains("pwd")) m_secu = true;
    else m_secu = false;
    m_name = net_name;
    if (icon_path.contains("wifion")) m_active = true;
    else m_active = false;

    m_baseVerLayout = new QVBoxLayout(this);
    m_baseVerLayout->setSpacing(0);
    m_baseVerLayout->setContentsMargins(0, 0, 0, 0);

    m_devFrame = new QFrame(this);
    m_devFrame->setAutoFillBackground(true);
    m_devFrame->setObjectName("devFrame");
    m_devFrame->setStyleSheet("QFrame#devFrame{background: palette(base);}");
    m_devFrame->setMinimumWidth(550);
    m_devFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_devFrame->setFixedHeight(56);

    m_devHorLayout = new QHBoxLayout(m_devFrame);
    m_devHorLayout->setSpacing(16);
    m_devHorLayout->setContentsMargins(16, 0, 16, 0);

    m_iconLabel = new QLabel(m_devFrame);
    QSizePolicy iconSizePolicy = m_iconLabel->sizePolicy();
    iconSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    iconSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    m_iconLabel->setSizePolicy(iconSizePolicy);
    m_iconLabel->setScaledContents(true);
    m_iconLabel->setPixmap(QIcon(icon_path).pixmap(QSize(24, 24)));

    m_nameLabel = new QLabel(m_devFrame);
    QSizePolicy nameSizePolicy = m_nameLabel->sizePolicy();
    nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    m_nameLabel->setSizePolicy(nameSizePolicy);
    m_nameLabel->setScaledContents(true);
    m_nameLabel->setText(net_name);

    //由于设计稿调整，去掉详情按钮
//    QPushButton * detailBtn = new QPushButton;
//    detailBtn->setFixedSize(24, 24);
//    detailBtn->setIconSize(QSize(24, 24));
//    detailBtn->setAutoFillBackground(false);
//    detailBtn->setCursor(QCursor(Qt::PointingHandCursor));
//    detailBtn->setStyleSheet("QPushButton{border-radius: 12px;}");
//    detailBtn->setIcon(QIcon("://img/plugins/netconnect/info.svg"));
//    QDBusInterface *interface = new QDBusInterface("org.ukui.KylinNM",
//                                                       "/MainWindow",
//                                                       "org.ukui.KylinNM",
//                                                       QDBusConnection::sessionBus(),
//                                                       NULL);
//    //无接口时隐藏详情按钮
//    if(!interface->isValid()){
//        detailBtn->hide();
//    }
//    connect(detailBtn, &QPushButton::clicked, this, [=](){
//        QDBusMessage msg = interface->call(QStringLiteral("ShowPb"),netName,false);
//    });
    m_statusLabel = new QLabel(m_devFrame);
    m_statusLabel->setText(tr("connected"));
    m_statusLabel->setEnabled(false);
    m_statusLabel->hide();

    if (icon_path.contains(":/img/plugins/netconnect/wifion")) {
        m_statusLabel->show();
    }

    m_devHorLayout->addWidget(m_iconLabel);
    m_devHorLayout->addWidget(m_nameLabel);
    m_devHorLayout->addStretch();
    m_devHorLayout->addWidget(m_statusLabel);

    m_devFrame->setLayout(m_devHorLayout);
    m_baseVerLayout->addWidget(m_devFrame);
}

void NetListItem::initLanWidget(const QString &icon_path, const QString &net_name)
{
    if (icon_path.contains("offline")) m_active = false;
    else m_active = true;

    m_baseVerLayout = new QVBoxLayout(this);
    m_baseVerLayout->setSpacing(0);
    m_baseVerLayout->setContentsMargins(0, 0, 0, 2);
    m_devFrame = new QFrame(this);
//    m_devFrame->setAutoFillBackground(false);
    m_devFrame->setAutoFillBackground(true);
    m_devFrame->setObjectName("devFrame");
    m_devFrame->setStyleSheet("QFrame#devFrame{background: palette(base);}");
    m_devFrame->setMinimumWidth(550);
    m_devFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_devFrame->setFixedHeight(56);
    m_devHorLayout = new QHBoxLayout(m_devFrame);
    m_devHorLayout->setSpacing(16);
    m_devHorLayout->setContentsMargins(16, 0, 16, 0);
    m_iconLabel = new QLabel(m_devFrame);
    m_iconLabel->setFixedSize(24, 24);
    m_iconLabel->setScaledContents(true);
    m_iconLabel->setPixmap(QPixmap(icon_path));

    m_nameLabel = new QLabel(m_devFrame);
    QSizePolicy nameSizePolicy = m_nameLabel->sizePolicy();
    nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    m_nameLabel->setSizePolicy(nameSizePolicy);
    m_nameLabel->setScaledContents(true);
    if ("No Net" != net_name) {
        m_nameLabel->setText(net_name);
    }

    m_statusLabel = new QLabel(m_devFrame);
    QSizePolicy statusSizePolicy = m_statusLabel->sizePolicy();
    statusSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    statusSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    m_statusLabel->setSizePolicy(statusSizePolicy);
    m_statusLabel->setScaledContents(true);
    m_circleLabel = new QLabel();
    m_circleLabel->setFixedSize(8, 8);
    if ("No Net" != net_name) {
        m_statusLabel->setText(tr("connected"));
        m_circleLabel->setStyleSheet("QLabel{border-radius: 4px; background-color: #49E17B;}");
    } else {
        m_statusLabel->setText(tr("No LAN available"));
        m_circleLabel->setStyleSheet("QLabel{border-radius: 4px; background-color: red;}");
    }

    if (!m_active && net_name != "No Net") {
        m_statusLabel->hide();
        m_circleLabel->hide();
    }
    m_devHorLayout->addWidget(m_iconLabel);
    m_devHorLayout->addWidget(m_nameLabel);
    m_devHorLayout->addStretch();
    m_devHorLayout->addWidget(m_circleLabel);
    m_devHorLayout->addWidget(m_statusLabel);
    m_devFrame->setLayout(m_devHorLayout);
    m_baseVerLayout->addWidget(m_devFrame);
    m_baseVerLayout->addStretch();
    this->setLayout(m_baseVerLayout);
}

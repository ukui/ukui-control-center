#include "deviceinfoitem.h"
#include <QStyle>

DeviceInfoItem::DeviceInfoItem(QWidget *parent, BluezQt::DevicePtr dev):
    QFrame(parent),
    _MDev(dev)
{
    qDebug() << Q_FUNC_INFO << QIcon::themeSearchPaths() << QIcon::themeName();

    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        item_gsettings = new QGSettings("org.ukui.style");
        connect(item_gsettings,&QGSettings::changed,this,&DeviceInfoItem::GSettingsChanges);
    }

    m_str_dev_connecting    = tr("Connecting");
    m_str_dev_disconnecting = tr("Disconnecting");
    m_str_dev_connected     = tr("Connected");
    m_str_dev_ununited      = tr("Not connected");
    m_str_dev_conn_fail     = tr("Connect fail");
    m_str_dev_disconn_fail  = tr("Disconnect fail");

    this->setMinimumSize(580,64);
    this->setMaximumSize(1800,64);
    //this->setFixedSize(parent->width(),64);
    this->setObjectName(_MDev? _MDev.data()->address(): "null");

    InitMemberVariables();
    setDeviceConnectSignals();

}

DeviceInfoItem::~DeviceInfoItem()
{

}


void DeviceInfoItem::InitMemberVariables()
{
    _MStatus = Status::Nomal;

    devName    = _MDev.data()->name();

    if (_MDev.data()->isPaired()) {

        _DevStatus = DEVSTATUS::Paired;

        if (_MDev.data()->isConnected()) {
            _DevStatus = DEVSTATUS::Connected;
        }
    } else {
        _DevStatus = DEVSTATUS::NoPaired;
    }

    if(item_gsettings->get("style-name").toString() == "ukui-black" ||
       item_gsettings->get("style-name").toString() == "ukui-dark")
    {
        _themeIsBlack = true;
    }
    else
    {
        _themeIsBlack = false;
    }


    _clicked = false;
    _pressFlag = false;

    _iconTimer = new QTimer(this);
    _iconTimer->setInterval(110);
    connect(_iconTimer,&QTimer::timeout,this,[=]{
        if (iconFlag == 0)
            iconFlag = 7;
        iconFlag--;
        update();
    });

    _devConnTimer = new QTimer(this);
    _devConnTimer->setInterval(5000);
    connect(_devConnTimer,&QTimer::timeout,this,[=]{

        _devConnTimer->stop();
        _iconTimer->stop();
        _clicked = false;

        if (DEVSTATUS::Connecting == _DevStatus) {
            _DevStatus = DEVSTATUS::ConnectFailed;
        } else if (DEVSTATUS::DisConnecting == _DevStatus) {
            _DevStatus = DEVSTATUS::DisConnectFailed;
        }

        //错误信息显示超时后，做一下操作
        QTimer::singleShot(2000,this,[=]{
            if (_MDev.data()->isPaired())
            {
                _DevStatus = DEVSTATUS::Paired;

                if (_MDev.data()->isConnected())
                    _DevStatus = DEVSTATUS::Connected;

            } else {
                _DevStatus = DEVSTATUS::NoPaired;
            }
            update();
        });

        update();

        emit devConnectComplete();
    });

    dev_Menu = new QMenu(this);
    connect(dev_Menu,&QMenu::triggered,this,&DeviceInfoItem::MenuSignalDeviceFunction);

}

void DeviceInfoItem::MenuSignalDeviceFunction(QAction *action)
{
    if(NULL == _MDev)
        return;

    if(action->text() == tr("Send files"))
    {
        qDebug() << Q_FUNC_INFO << "To :" << _MDev->name() << "Send files" << __LINE__;
        emit devSendFiles(_MDev.data()->address());
        //Send_files_by_address(action->statusTip());
    }
    else if(action->text() == tr("Remove"))
    {
        QPushButton * cancel_btn = new QPushButton(tr("cancel"));
        QPushButton * remove_btn = new QPushButton(tr("remove"));
        QMessageBox * removeBox = new QMessageBox;
        QIcon icon;
        icon = QIcon::fromTheme("bluetooth");
        removeBox->setIcon(QMessageBox::Warning);
        removeBox->setWindowIcon(icon);

        QString title_text = tr("Sure to remove,")+devName+"?";
        removeBox->setWindowTitle(title_text);
        removeBox->setText(tr("After removal, the next connection requires matching PIN code!"));
        removeBox->addButton(cancel_btn,QMessageBox::ButtonRole::RejectRole);
        removeBox->addButton(remove_btn,QMessageBox::ButtonRole::AcceptRole);

        removeBox->show();
        removeBox->exec();

        if (removeBox->clickedButton() == remove_btn)
        {
            qDebug() << Q_FUNC_INFO << "To :" << _MDev->name() << "Remove" << __LINE__;
            emit devRemove(_MDev.data()->address());
        }
        else
        {
            qDebug() << Q_FUNC_INFO << "To :" << _MDev->name() << "Cancel" << __LINE__;
        }
    }
}

void DeviceInfoItem::GSettingsChanges(const QString &key)
{
    if(key == "styleName"){
        if(item_gsettings->get("style-name").toString() == "ukui-black" ||
           item_gsettings->get("style-name").toString() == "ukui-dark")
        {
            _themeIsBlack = true;
        }
        else
        {
            _themeIsBlack = false;

        }
    }
}

void DeviceInfoItem::setDeviceConnectSignals()
{
    if (_MDev)
    {
        connect(_MDev.data(),&BluezQt::Device::nameChanged,this,[=](const QString &name)
        {
            devName = name ;
            update();
        });

        connect(_MDev.data(),&BluezQt::Device::typeChanged,this,[=](BluezQt::Device::Type type)
        {
            qDebug() << Q_FUNC_INFO << "typeChanged" << __LINE__;
            update();
        });

        connect(_MDev.data(),&BluezQt::Device::pairedChanged,this,[=](bool paired)
        {
            qDebug() << Q_FUNC_INFO << "pairedChanged" << __LINE__;
            if(_devConnTimer->isActive())
                _devConnTimer->stop();

            if (_iconTimer->isActive())
                _iconTimer->stop();

            if (paired)
            {
                _DevStatus = DEVSTATUS::Paired;
                emit devPaired(_MDev->address());

                if (_MDev.data()->isConnected())
                    _DevStatus = DEVSTATUS::Connected;

            } else {
                _DevStatus = DEVSTATUS::NoPaired;
            }

            _clicked = false;

            update();
        });

        connect(_MDev.data(),&BluezQt::Device::connectedChanged,this,[=](bool connected)
        {
            qDebug() << Q_FUNC_INFO << "connectedChanged" << __LINE__;
            if(_devConnTimer->isActive())
                _devConnTimer->stop();

            if (_iconTimer->isActive())
                _iconTimer->stop();

            _clicked = false;

            if (_MDev.data()->isPaired() && connected) {
                _DevStatus = DEVSTATUS::Connected;
            } else if (!_MDev.data()->isPaired() && connected) {
                _DevStatus = DEVSTATUS::Connecting;
            } else if (_MDev.data()->isPaired() && !connected){
                _DevStatus = DEVSTATUS::Paired;
            } else {
                _DevStatus = DEVSTATUS::ConnectFailed;
            }

            update();

            emit devConnectComplete();
        });
    }
}

bool DeviceInfoItem::mouseEventIntargetAera(QPoint p)
{
    QRect *targte = new QRect(this->width()-55,14,36,36);
    if (targte->contains(p)) {
        return true;
    } else {
        return false;
    }
}

QRect DeviceInfoItem::getStatusTextRect()
{
    if (_MDev && _MDev.data()->isPaired()) {
        return QRect(this->width()-226,20,150,24);
    } else {
        return QRect(this->width()-120,20,105,24);
    }
}

QRect DeviceInfoItem::getStatusIconRect()
{
    if (_MDev && _MDev.data()->isPaired()) {
        return QRect(this->width()-250,15,25,25);
    } else {
        return QRect(this->width()-145,15,25,25);
    }
}

QColor DeviceInfoItem::getStatusColor(DEVSTATUS status)
{
    QColor color;
    switch (status) {
    case DEVSTATUS::Paired:
        color = QColor("#A8A8A8");
        break;
    case DEVSTATUS::Connected:
        color = QColor("#2FB3E8");
        break;
    case DEVSTATUS::DisConnectFailed:
    case DEVSTATUS::ConnectFailed:
        color = QColor("#FB5050");
        break;
    case DEVSTATUS::Connecting:
    case DEVSTATUS::DisConnecting:
        color = QColor("#F8A34C");
        break;
    default:
        color = QColor();
        break;
    }

    return color;
}

QPixmap DeviceInfoItem::getDevConnectedIcon(DEVSTATUS status,QSize size)
{
    QString iconName;
    switch (status) {
    case DEVSTATUS::Paired:
        iconName = ":/img/bluetooth/plugins/not-connected.svg";
        break;
    case DEVSTATUS::Connected:
        iconName = ":/img/plugins/bluetooth/connected.svg";
        break;
    case DEVSTATUS::DisConnectFailed:
    case DEVSTATUS::ConnectFailed:
        iconName = ":/img/bluetooth/plugins/connect-fail.svg";
        break;
    case DEVSTATUS::Connecting:
    case DEVSTATUS::DisConnecting:
        iconName = ":/img/bluetooth/plugins/connecting.svg";
        break;
    default:
        iconName = "";
        break;
    }



    return iconName.isEmpty() ? \
                QPixmap() : \
                ImageUtil::loadSvg(iconName,"default",size.width());
}

QPixmap DeviceInfoItem::convertIconColor(QPixmap icon, QColor rgb)
{
    QImage targetImage = icon.toImage();
    for (int x = 0; x < targetImage.width(); x++) {
        for (int y = 0; y <targetImage.height(); y++) {
            auto colorPoint = targetImage.pixelColor(x,y);

            if (colorPoint.alpha() > 0) {
                targetImage.setPixelColor(x,y,rgb);
            }
        }
    }

    return QPixmap::fromImage(targetImage);
}

void DeviceInfoItem::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    _MStatus = Status::Hover;
    update();
}

void DeviceInfoItem::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    _MStatus = Status::Nomal;
    _inBtn = false;
    update();
}

void DeviceInfoItem::mousePressEvent(QMouseEvent *event)
{
//    if (event->button() == Qt::LeftButton && mouseEventIntargetAera(event->pos())) {
//        _pressFlag = true;
//    }
//    if (event->button() == Qt::LeftButton)
//    {
//        _pressBtnFlag = true;
//    }

    if (event->button() == Qt::LeftButton) {
        if (mouseEventIntargetAera(event->pos())) {
            _pressBtnFlag = true;
        } else {
            _pressFlag = true;
        }
    }
}

void DeviceInfoItem::MouseClickedDevFunc()
{
    qDebug () << Q_FUNC_INFO << __LINE__;
    if (!_MDev->isPaired())
        return;

    dev_Menu->clear();
    QPoint currentWPoint = this->pos();
    QPoint sreenPoint = QWidget::mapFromGlobal(currentWPoint);
    //qDebug () <<Q_FUNC_INFO << "allPoint--x--y:" << sreenPoint.x() << sreenPoint.y() <<__LINE__;

    QPalette palette;
    if(_themeIsBlack)
    {
        palette.setBrush(QPalette::Base,QColor(Qt::black));
        palette.setBrush(QPalette::Active, QPalette::Text,QColor(Qt::white));
    }
    else
    {
        palette.setBrush(QPalette::Base,QColor(Qt::white));
        palette.setBrush(QPalette::Active, QPalette::Text,QColor(Qt::black));
    }
    dev_Menu->setProperty("setIconHighlightEffectDefaultColor", dev_Menu->palette().color(QPalette::Active, QPalette::Base));
    dev_Menu->setPalette(palette);
    dev_Menu->setMinimumWidth(180);

    if (-1 != _MDev.data()->uuids().indexOf(BluezQt::Services::ObexObjectPush)) {
        QAction *sendfile = new QAction(dev_Menu);
        sendfile->setText(tr("Send files"));
        dev_Menu->addAction(sendfile);
        dev_Menu->addSeparator();
    }

    QAction *remove = new QAction(dev_Menu);
    remove->setText(tr("Remove"));
    dev_Menu->addAction(remove);

    dev_Menu->move(qAbs(sreenPoint.x())+this->width()-200,qAbs(sreenPoint.y())+this->y()+40);
    dev_Menu->exec();

}

void DeviceInfoItem::mouseReleaseEvent(QMouseEvent *event)
{
//    if (event->button() == Qt::LeftButton && _pressFlag) {
//        MouseClickedFunc();
//    }

//    if (event->button() == Qt::RightButton && _pressBtnFlag) {
//        MouseClickedDevFunc();
//    }


    if (event->button() == Qt::LeftButton) {
        if (mouseEventIntargetAera(event->pos()) && _pressBtnFlag) {
            MouseClickedDevFunc();
            _pressBtnFlag = false;
        } else {
            MouseClickedFunc();
            _pressFlag = false;
        }
    }
}

void DeviceInfoItem::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseEventIntargetAera(event->pos())) {
        _inBtn = true;
        this->update();
    } else {
        if (_inBtn) {
            _inBtn = false;
            this->update();
        }
    }

}

void DeviceInfoItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor(Qt::transparent));

    DrawBackground(painter);
    DrawDevTypeIcon(painter);
    DrawText(painter);

    DrawStatusText(painter);

    if (_MDev.data()->isPaired())
    {
        this->setMouseTracking(true);
        DrawFuncBtn(painter);
    }
}

/************************************************
 * @brief  getPainterBrushColor
 * @param  null
 * @return QColor
*************************************************/
QColor DeviceInfoItem::getPainterBrushColor()
{
    QColor color;
    switch (_MStatus) {
    case Status::Nomal:

        if(_themeIsBlack)
            color = QColor(Qt::black);//("#EBEBEB");
        else
            color = QColor(Qt::white);//("#EBEBEB");

        //color = QColor(Qt::black);//("#EBEBEB");

        break;
    case Status::Hover:
        color = QColor("#D7D7D7");
        break;

    default:
        if(_themeIsBlack)
            color = QColor(Qt::black);//("#EBEBEB");
        else
            color = QColor(Qt::white);//("#EBEBEB");
        //color = QColor(Qt::white);//("#EBEBEB");
        break;
    }
    return color;
}

/************************************************
 * @brief  getDevStatusColor
 * @param  null
 * @return QColor
*************************************************/
QColor DeviceInfoItem::getDevStatusColor()
{
    QColor color;

    if (_MDev) {
        if (_MDev.data()->isPaired() && _MDev.data()->isConnected()) {
            color = QColor("#2FB3E8");
        } else {
            color = QColor("#F4F4F4");
        }
    } else {
        color = QColor("#F4F4F4");
    }

    return color;
}

/************************************************
 * @brief  getDevTypeIcon
 * @param  null
 * @return QPixmap
*************************************************/
QPixmap DeviceInfoItem::getDevTypeIcon()
{
    QIcon icon;
    if (_MDev) {
//        if (_clicked) {
//            icon = QIcon::fromTheme("ukui-loading-" + QString::number(iconFlag));
//        } else {
        switch (_MDev.data()->type()) {
        case BluezQt::Device::Phone:
            icon = QIcon::fromTheme("phone");
            break;
        case BluezQt::Device::Computer:
            icon = QIcon::fromTheme("computer-symbolic");
            break;
        case BluezQt::Device::Headset:
            icon = QIcon::fromTheme("audio-headset-symbolic");
            break;
        case BluezQt::Device::Headphones:
            icon = QIcon::fromTheme("audio-headphones-symbolic");
            break;
        case BluezQt::Device::AudioVideo:
            icon = QIcon::fromTheme("audio-speakers-symbolic");
            break;
        case BluezQt::Device::Keyboard:
            icon = QIcon::fromTheme("input-keyboard-symbolic");
            break;
        case BluezQt::Device::Mouse:
            icon = QIcon::fromTheme("input-mouse-symbolic");
            break;
        default:
            icon = QIcon::fromTheme("bluetooth-active-symbolic");
            break;
        }
//        }
    } else {
//        if (_clicked) {
//            icon = QIcon::fromTheme("ukui-loading-" + QString::number(iconFlag));
//        } else {
        icon = QIcon::fromTheme("bluetooth-active-symbolic");
//        }
    }

    return icon.pixmap(18,18);
}

/************************************************
 * @brief  DrawBackground
 * @param  painter
 * @return null
*************************************************/
void DeviceInfoItem::DrawBackground(QPainter &painter)
{
    painter.save();
    painter.setBrush(getPainterBrushColor());
    painter.drawRoundedRect(this->rect(),6,6,Qt::AbsoluteSize);
    painter.restore();
}

/************************************************
 * @brief  DrawDevTypeIcon
 * @param  painter
 * @return null
*************************************************/
void DeviceInfoItem::DrawDevTypeIcon(QPainter &painter)
{
    painter.save();
//    painter.setBrush(getDevStatusColor());
//    painter.drawEllipse(14,9,45,45);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    style()->drawItemPixmap(&painter, QRect(19, 13, 36, 36), Qt::AlignCenter, getDevTypeIcon());
    painter.restore();
}

/************************************************
 * @brief  DrawText
 * @param  painter
 * @return null
*************************************************/
void DeviceInfoItem::DrawText(QPainter &painter)
{
    painter.save();
    if(_themeIsBlack)
        painter.setPen(QColor(Qt::white));
    else
        painter.setPen(QColor(Qt::black));

    painter.drawText(70,20,350,24,Qt::AlignLeft,_MDev? _MDev.data()->name(): QString("Example"));
    painter.restore();
}

void DeviceInfoItem::DrawStatusText(QPainter &painter)
{
    painter.save();
    if(_themeIsBlack)
        painter.setPen(QColor(Qt::white));
    else
        painter.setPen(QColor(Qt::black));

    switch (_DevStatus) {
    case DEVSTATUS::Paired:
        painter.setPen(getStatusColor(DEVSTATUS::Paired));
        style()->drawItemPixmap(&painter,getStatusIconRect(), Qt::AlignCenter, getDevConnectedIcon(DEVSTATUS::Paired,QSize(25,25)));
        painter.drawText(getStatusTextRect(),Qt::AlignRight,(m_str_dev_ununited));
        break;
    case DEVSTATUS::Connected:
        painter.setPen(getStatusColor(DEVSTATUS::Connected));
        style()->drawItemPixmap(&painter,getStatusIconRect(), Qt::AlignCenter, getDevConnectedIcon(DEVSTATUS::Connected,QSize(15,15)));
        painter.drawText(getStatusTextRect(),Qt::AlignRight,(m_str_dev_connected));
        break;
    case DEVSTATUS::Connecting:
        painter.setPen(getStatusColor(DEVSTATUS::Connecting));
        style()->drawItemPixmap(&painter,getStatusIconRect(), Qt::AlignCenter, getDevConnectedIcon(DEVSTATUS::Connecting,QSize(15,15)));
        painter.drawText(getStatusTextRect(),Qt::AlignRight,(m_str_dev_connecting));
        break;
    case DEVSTATUS::DisConnecting:
        painter.setPen(getStatusColor(DEVSTATUS::DisConnecting));
        style()->drawItemPixmap(&painter,getStatusIconRect(), Qt::AlignCenter, getDevConnectedIcon(DEVSTATUS::DisConnecting,QSize(15,15)));
        painter.drawText(getStatusTextRect(),Qt::AlignRight,(m_str_dev_disconnecting));
        break;
    case DEVSTATUS::DisConnectFailed:
        painter.setPen(getStatusColor(DEVSTATUS::DisConnectFailed));
        style()->drawItemPixmap(&painter,getStatusIconRect(), Qt::AlignCenter, getDevConnectedIcon(DEVSTATUS::DisConnectFailed,QSize(15,15)));
        painter.drawText(getStatusTextRect(),Qt::AlignRight,(m_str_dev_disconn_fail));
        break;
    case DEVSTATUS::ConnectFailed:
        painter.setPen(getStatusColor(DEVSTATUS::ConnectFailed));
        style()->drawItemPixmap(&painter,getStatusIconRect(), Qt::AlignCenter, getDevConnectedIcon(DEVSTATUS::ConnectFailed,QSize(15,15)));
        painter.drawText(getStatusTextRect(),Qt::AlignRight,(m_str_dev_conn_fail));
        break;
    default:
        break;
    }


    painter.restore();
}

void DeviceInfoItem::DrawFuncBtn(QPainter &painter)
{
//    devFuncBtn->setGeometry(this->width()-55,12,40,35);

//    if (_MDev)
//        devFuncBtn->setVisible(_MDev->isPaired());

    painter.save();

    if (_inBtn) {
//        painter.setBrush(QColor(55, 144, 250));
        painter.setPen(QColor("#2FB3E8"));
    } else {
        painter.setBrush(getPainterBrushColor());
    }

    painter.drawRoundRect(this->width()-55,14,36,36,30,30);

    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    style()->drawItemPixmap(&painter, QRect(this->width()-48,23,20,20), Qt::AlignCenter, QIcon::fromTheme("content-loading-symbolic").pixmap(20,20));

    painter.restore();
}

/************************************************
 * @brief  MouseClickedFunc
 * @param  null
 * @return null
*************************************************/
void DeviceInfoItem::MouseClickedFunc()
{
    _clicked = true;
    _pressFlag = false;

    update();

    if (_MDev) {
        if (_MDev.data()->isConnected()) {
            qDebug() << Q_FUNC_INFO << "devDisconnect: "<< _MDev.data()->name() <<  _MDev.data()->address() << __LINE__;
            //_MDev.data()->disconnectFromDevice();
            _DevStatus = DEVSTATUS::DisConnecting;
            emit devDisconnect(_MDev.data()->address());
        } else {
            qDebug() << Q_FUNC_INFO << "devConnect: "<< _MDev.data()->name() <<  _MDev.data()->address() << __LINE__;
           // _MDev.data()->connectToDevice();
            _DevStatus = DEVSTATUS::Connecting;
            emit devConnect(_MDev.data()->address());
        }
    }


    if (!_devConnTimer->isActive())
    {
        _devConnTimer->start();
    }

    //_iconTimer loading
//    if (!_iconTimer->isActive())
//    {
//        _iconTimer->start();
//        _devConnTimer->start();
//    }
}

/************************************************
 * @brief  ???????????????????????????
 * @param  null
 * @return null
*************************************************/
void DeviceInfoItem::DevConnectFunc()
{
    if (_MDev)
    {
        if (_MDev.data()->type() == BluezQt::Device::AudioVideo ||
            _MDev.data()->type() == BluezQt::Device::Headphones ||
            _MDev.data()->type() == BluezQt::Device::Headset    ) {

            BluezQt::AdapterPtr MDevAdapter = _MDev.data()->adapter();

            for (auto dev : MDevAdapter.data()->devices()) {
                if (dev.data()->isConnected() && dev.data()->isPaired()) {
                    if (dev.data()->type() == BluezQt::Device::AudioVideo ||
                        dev.data()->type() == BluezQt::Device::Headphones ||
                        dev.data()->type() == BluezQt::Device::Headset    ) {

                        BluezQt::PendingCall *pending = dev.data()->disconnectFromDevice();
                        connect(pending,&BluezQt::PendingCall::finished,this,[&](BluezQt::PendingCall *call){
                            if (!call->error()) {
                                _MDev.data()->connectToDevice();
                            }
                        });
                        return;
                    }
                }
            }
            _MDev.data()->connectToDevice();
        } else {
            _MDev.data()->connectToDevice();
        }
    }
}

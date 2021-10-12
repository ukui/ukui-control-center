#include "deviceinfoitem.h"
#include <QStyle>
#include <QPushButton>
#include <QAbstractButton>

DeviceInfoItem::DeviceInfoItem(QWidget *parent, BluezQt::DevicePtr dev):
    QFrame(parent),
    _MDev(dev)
{

    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        item_gsettings = new QGSettings("org.ukui.style");
        connect(item_gsettings,&QGSettings::changed,this,&DeviceInfoItem::GSettingsChanges);
    }
    QPalette palette;
    if(item_gsettings->get("style-name").toString() == "ukui-black" ||
       item_gsettings->get("style-name").toString() == "ukui-dark")
    {
        palette.setColor(QPalette::Background,QColor(Qt::white));
    }
    else
    {
        palette.setColor(QPalette::Background,QColor(Qt::black));

    }

    m_str_dev_connecting = tr("Connecting");
    m_str_dev_disconnecting = tr("Disconnecting");
    m_str_dev_connected = tr("Connected");
    m_str_dev_ununited = tr("Ununited");
    m_str_dev_conn_fail = tr("Connect fail");

    this->setMinimumSize(580,64);
    this->setMaximumSize(1800,64);
    //this->setFixedSize(parent->width(),64);
    this->setObjectName(_MDev? _MDev.data()->address(): "null");

    //devFuncBtn  = new QToolButton(this);
    devFuncBtn  = new QPushButton(this);
    devFuncBtn->setText("...");
    devFuncBtn->show();
    connect(devFuncBtn,SIGNAL(released()),this,SLOT(MouseClickedDevFunc()));
    InitMemberVariables();
    setDeviceConnectSignals();

//    this->setProperty("useIconHighlightEffect", 0x8);
//    this->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::black));

}

DeviceInfoItem::~DeviceInfoItem()
{

}


void DeviceInfoItem::InitMemberVariables()
{
    _MStatus = Status::Nomal;
    _clicked = false;
    _pressFlag = false;
    _connDevTimeOutFlag = false;

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
        _connDevTimeOutFlag = true ;

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
    qDebug() << Q_FUNC_INFO << action->text() ;
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


void DeviceInfoItem::setDeviceCurrentStatus()
{
    if (NULL != _MDev)
    {
        if (_connDevTimeOutFlag)
        {
            _DevStatus = DEVICE_STATUS::ERROR;
            QTimer::singleShot(8000,this,[=]{
                _connDevTimeOutFlag = false ;
                update();
            });
        }
        else
        {
            if(_MDev.data()->isPaired())
            {
                _DevStatus = DEVICE_STATUS::PAIRED;
                if (_MDev.data()->isConnected())
                    _DevStatus = DEVICE_STATUS::LINK;
            }
            else
            {
                _DevStatus = DEVICE_STATUS::NONE;
            }
        }
    }
}


void DeviceInfoItem::GSettingsChanges(const QString &key)
{

    qDebug() << Q_FUNC_INFO << key;
    QPalette palette;
    if(key == "styleName"){
        if(item_gsettings->get("style-name").toString() == "ukui-black" ||
           item_gsettings->get("style-name").toString() == "ukui-dark")
        {
            palette.setColor(QPalette::Background,QColor(Qt::white));
        }
        else
        {
            palette.setColor(QPalette::Background,QColor(Qt::black));

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
                emit devPaired(_MDev->address());
            }
            _clicked = false;
            _connDevTimeOutFlag = false ;
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
            _connDevTimeOutFlag = false ;
            update();

            emit devConnectComplete();
        });
    }
}

void DeviceInfoItem::initInfoPage(QString d_name, DEVICE_STATUS status, BluezQt::DevicePtr device)
{
    _MDev      = device;
    _DevStatus = status;
    devName    = device->name();
    devFuncBtn->setVisible(_MDev.data()->isPaired());

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
    update();
}

void DeviceInfoItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        _pressFlag = true;
    }
    if (event->button() == Qt::RightButton)
    {
        _rightFlag = true;
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
    if(item_gsettings->get("style-name").toString() == "ukui-black" ||
       item_gsettings->get("style-name").toString() == "ukui-dark")
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
    QAction *sendfile = new QAction(dev_Menu);
    sendfile->setText(tr("Send files"));
    QAction *remove = new QAction(dev_Menu);
    remove->setText(tr("Remove"));

    dev_Menu->addAction(sendfile);
    dev_Menu->addSeparator();
    dev_Menu->addAction(remove);

    //qDebug () << this->x() << this->y() << "======x ======y";
    dev_Menu->move(qAbs(sreenPoint.x())+this->width()-200,qAbs(sreenPoint.y())+this->y()+40);
    dev_Menu->exec();

}

void DeviceInfoItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && _pressFlag) {
        MouseClickedFunc();
    }

    if (event->button() == Qt::RightButton && _rightFlag) {
        MouseClickedDevFunc();
    }

}

void DeviceInfoItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor(Qt::transparent));

    DrawBackground(painter);
    DrawStatusIcon(painter);
    DrawText(painter);
    DrawStatusText(painter);

    if (_MDev.data()->isPaired())
    {
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

        if(item_gsettings->get("style-name").toString() == "ukui-black" ||
           item_gsettings->get("style-name").toString() == "ukui-dark")
            color = QColor(Qt::black);//("#EBEBEB");
        else
            color = QColor(Qt::white);//("#EBEBEB");

        //color = QColor(Qt::black);//("#EBEBEB");

        break;
    case Status::Hover:
        color = QColor("#D7D7D7");
        break;

    //case :

    default:
        if(item_gsettings->get("style-name").toString() == "ukui-black" ||
           item_gsettings->get("style-name").toString() == "ukui-dark")
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
        if (_clicked) {
            icon = QIcon::fromTheme("ukui-loading-" + QString::number(iconFlag));
        } else {
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
        }
    } else {
        if (_clicked) {
            icon = QIcon::fromTheme("ukui-loading-" + QString::number(iconFlag));
        } else {
            icon = QIcon::fromTheme("bluetooth-active-symbolic");
        }
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
 * @brief  DrawStatusIcon
 * @param  painter
 * @return null
*************************************************/
void DeviceInfoItem::DrawStatusIcon(QPainter &painter)
{
    painter.save();
    painter.setBrush(getDevStatusColor());
    painter.drawEllipse(14,8,45,45);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    style()->drawItemPixmap(&painter, QRect(20, 12, 35, 35), Qt::AlignCenter, getDevTypeIcon());
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
    if(item_gsettings->get("style-name").toString() == "ukui-black" ||
       item_gsettings->get("style-name").toString() == "ukui-dark")
        painter.setPen(QColor(Qt::white));
    else
        painter.setPen(QColor(Qt::black));

    painter.drawText(64,20,350,24,Qt::AlignLeft,_MDev? _MDev.data()->name(): QString("Example"));
    painter.restore();
}

void DeviceInfoItem::DrawStatusText(QPainter &painter)
{
    setDeviceCurrentStatus();

    painter.save();
    if(item_gsettings->get("style-name").toString() == "ukui-black" ||
       item_gsettings->get("style-name").toString() == "ukui-dark")
        painter.setPen(QColor(Qt::white));
    else
    painter.setPen(QColor(Qt::black));

    if (_clicked)
    {
        if (LINK == _DevStatus)
            painter.drawText(this->width()-210,16,150,24,Qt::AlignRight,(m_str_dev_disconnecting));
        else
        {
            if (_MDev.data()->isPaired())
                painter.drawText(this->width()-210,18,150,24,Qt::AlignRight,(m_str_dev_connecting));
            else
                painter.drawText(this->width()-210,18,180,24,Qt::AlignRight,(m_str_dev_connecting));
        }
    }
    else if (DEVICE_STATUS::ERROR == _DevStatus)
    {
        if (_MDev.data()->isPaired())
            painter.drawText(this->width()-210,18,150,24,Qt::AlignRight,(m_str_dev_conn_fail));
        else
            painter.drawText(this->width()-210,18,180,24,Qt::AlignRight,(m_str_dev_conn_fail));
    }
    else if (DEVICE_STATUS::PAIRED == _DevStatus)
        painter.drawText(this->width()-210,18,150,24,Qt::AlignRight,(m_str_dev_ununited));
    else if (DEVICE_STATUS::LINK == _DevStatus)
        painter.drawText(this->width()-210,18,150,24,Qt::AlignRight,(m_str_dev_connected));
    else
        painter.drawText(this->width()-210,18,150,24,Qt::AlignRight,"");

    painter.restore();
}

void DeviceInfoItem::DrawFuncBtn(QPainter &painter)
{
    devFuncBtn->setGeometry(this->width()-55,12,40,35);

    if (_MDev)
	devFuncBtn->setVisible(_MDev->isPaired());

//    painter.save();
//    if(item_gsettings->get("style-name").toString() == "ukui-black" ||
//       item_gsettings->get("style-name").toString() == "ukui-dark")
//        painter.setPen(QColor(Qt::white));
//    else
//        painter.setPen(QColor(Qt::black));
//    //painter.setBrush(getDevStatusColor());
//    //painter.drawEllipse(this->width()-60,12,40,24);
//    painter.drawText(this->width()-60,12,40,24,Qt::AlignRight,". . .");
//    //painter.setRenderHint(QPainter::SmoothPixmapTransform);
//    //style()->drawItemText(&painter, QRect(this->width()-60,12,40,24), Qt::AlignCenter,painter,0, ". . .");
//    painter.restore();
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

    if (_MDev) {
        if (_MDev.data()->isConnected()) {
            qDebug() << Q_FUNC_INFO << "devDisconnect: "<< _MDev.data()->name() <<  _MDev.data()->address() << __LINE__;
            //_MDev.data()->disconnectFromDevice();
            emit devDisconnect(_MDev.data()->address());
        } else {
            qDebug() << Q_FUNC_INFO << "devConnect: "<< _MDev.data()->name() <<  _MDev.data()->address() << __LINE__;
           // _MDev.data()->connectToDevice();
            emit devConnect(_MDev.data()->address());
        }
    }

    //_iconTimer loading
    if (!_iconTimer->isActive())
    {
        _iconTimer->start();
        _devConnTimer->start();
    }
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
                    }
                }
            }

        } else {
            _MDev.data()->connectToDevice();
        }
    }
}

#include "inteldeviceinfoitem.h"
#include <QStyle>

#include "intelmsgbox.h"
#include "bluetoothmain.h"

IntelDeviceInfoItem::IntelDeviceInfoItem(QWidget *parent, bluetoothdevice * dev):
    QFrame(parent),
    _MDev(dev)
{
    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        item_gsettings = new QGSettings("org.ukui.style");
        connect(item_gsettings,&QGSettings::changed,this,&IntelDeviceInfoItem::GSettingsChanges);
    }

    m_str_dev_connecting    = tr("Connecting");
    m_str_dev_disconnecting = tr("Disconnecting");
    m_str_dev_connected     = tr("Connected");
    m_str_dev_ununited      = tr("Not connected");
    m_str_dev_conn_fail     = tr("Connect fail");
    m_str_dev_disconn_fail  = tr("Disconnect fail");

    this->setMinimumSize(580,64);
    this->setMaximumSize(1800,64);
    this->setObjectName(_MDev? _MDev->getDevAddress(): "null");

    InitMemberVariables();
    setDeviceConnectSignals();
}

IntelDeviceInfoItem::~IntelDeviceInfoItem()
{

}


void IntelDeviceInfoItem::InitMemberVariables()
{
    _MStatus = Status::Nomal;

    devName    = _MDev->getDevName();

    if (_MDev->isPaired()) {

        _DevStatus = DEVSTATUS::Paired;

        if (_MDev->isConnected()) {
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

    _fontFamily = item_gsettings->get("system-font").toString();
    _fontSize = item_gsettings->get("system-font-size").toString().toInt();

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
    _devConnTimer->setInterval(32000);
    connect(_devConnTimer,&QTimer::timeout,this,[=]{
//        if(BlueToothMain::m_device_pin_flag)
//        {
//            _devConnTimer->stop();
//            _devConnTimer->start();
//            return;
//        }
        emit devConnectionComplete();

        _devConnTimer->stop();
        _iconTimer->stop();
        _clicked = false;

        if (DEVSTATUS::Connecting == _DevStatus) {
            _DevStatus = DEVSTATUS::ConnectFailed;
        } else if (DEVSTATUS::DisConnecting == _DevStatus) {
            _DevStatus = DEVSTATUS::DisConnectFailed;
        }

        update();
        TimedRestoreConnectionErrorDisplay();

    });

    dev_Menu = new QMenu(this);
    connect(dev_Menu,&QMenu::triggered,this,&IntelDeviceInfoItem::MenuSignalDeviceFunction);

}

void IntelDeviceInfoItem::MenuSignalDeviceFunction(QAction *action)
{
    if(NULL == _MDev)
        return;

    if(action->text() == tr("Send files"))
    {
        qDebug() << Q_FUNC_INFO << "To :" << _MDev->getDevName() << "Send files" << __LINE__;
        emit devSendFiles(_MDev->getDevAddress());
    }
    else if(action->text() == tr("remove"))
    {
        MsgBox *box = new MsgBox(this,devName);
        connect(box,&MsgBox::accepted,this,[=]{
            qDebug() << Q_FUNC_INFO << "To :" << _MDev->getDevName() << "Remove" << __LINE__;
            emit devRemove(_MDev->getDevAddress());
        });

        box->exec();
    }
}

void IntelDeviceInfoItem::GSettingsChanges(const QString &key)
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
    } else if (key == "systemFont") {
        _fontFamily = item_gsettings->get("system-font").toString();
    } else if (key == "systemFontSize") {
        _fontSize = item_gsettings->get("system-font-size").toString().toInt();
    }
}

void IntelDeviceInfoItem::setDeviceConnectSignals()
{
    if (_MDev)
    {
        connect(_MDev,&bluetoothdevice::nameChanged,this,[=](const QString &name)
        {
            devName = name ;
            update();
        });

        connect(_MDev,&bluetoothdevice::typeChanged,this,[=](bluetoothdevice::DEVICE_TYPE type)
        {
            qDebug() << Q_FUNC_INFO << "typeChanged" << __LINE__;
            update();
        });

        connect(_MDev,&bluetoothdevice::pairedChanged,this,[=](bool paired)
        {
            qDebug() << Q_FUNC_INFO << "pairedChanged" << __LINE__;
            //BlueToothMain::m_device_pin_flag = false;

            if(_devConnTimer->isActive())
                _devConnTimer->stop();

            if (_iconTimer->isActive())
                _iconTimer->stop();

            if (paired)
            {
                _DevStatus = DEVSTATUS::Paired;
                emit devPaired(_MDev->getDevAddress());

                if (_MDev->isConnected())
                    _DevStatus = DEVSTATUS::Connected;

            } else {
                _DevStatus = DEVSTATUS::NoPaired;
            }

            _clicked = false;

            update();
        });

        connect(_MDev,&bluetoothdevice::connectedChanged,this,[=](bool connected)
        {
            qDebug() << Q_FUNC_INFO << "connectedChanged" << __LINE__;
            //BlueToothMain::m_device_pin_flag = false;
            if(_devConnTimer->isActive())
                _devConnTimer->stop();

            if (_iconTimer->isActive())
                _iconTimer->stop();

            _clicked = false;

            if (_MDev->isPaired() && connected) {
                _DevStatus = DEVSTATUS::Connected;
            } else if (!_MDev->isPaired() && connected) {
                _DevStatus = DEVSTATUS::Connecting;
            } else if (_MDev->isPaired() && !connected){
                if(_DevStatus == DEVSTATUS::ConnectFailed || _DevStatus == DEVSTATUS::DisConnectFailed )
                    ;
                else
                _DevStatus = DEVSTATUS::Paired;
            } else {
                _DevStatus = DEVSTATUS::ConnectFailed;
            }

            update();

            emit devConnectionComplete();
            TimedRestoreConnectionErrorDisplay();

        });

        connect(_MDev,&bluetoothdevice::errorInfoRefresh,this,[=](int errorId , QString errorText)
        {
            qDebug () << Q_FUNC_INFO << "error:" << errorId << errorText << __LINE__;
            emit devConnectionComplete();
            if (errorId)
            {
                if(_devConnTimer->isActive())
                    _devConnTimer->stop();
                if (_iconTimer->isActive())
                    _iconTimer->stop();
                _clicked = false;

                qDebug () << Q_FUNC_INFO << "error:" << errorId << errorText << __LINE__;
                //BlueToothMain::m_device_pin_flag = false;
                if (!_MDev->isConnected())
                {
                    _DevStatus = DEVSTATUS::ConnectFailed;
                    update();
                    TimedRestoreConnectionErrorDisplay();
                }
            }
        });
    }
}

bool IntelDeviceInfoItem::mouseEventIntargetAera(QPoint p)
{
    QRect *targte = new QRect(this->width()-55,14,36,36);
    if (targte->contains(p)) {
        return true;
    } else {
        return false;
    }
}

QRect IntelDeviceInfoItem::getStatusTextRect(QRect rect)
{
    if (_MDev && _MDev->isPaired()) {
        return QRect(this->width()-226,20,150,24);
    } else {
        return QRect(this->width()-120,20,105,24);
    }
}

QRect IntelDeviceInfoItem::getStatusIconRect(QRect rect)
{
    if (_MDev && _MDev->isPaired()) {

        if (QLocale::system().name() == "zh_CN")
            return QRect(this->width()-170,20,25,25);
        else
            return QRect(this->width()-210,20,25,25);

    } else {

        if (QLocale::system().name() == "zh_CN")
            return QRect(this->width()-110,20,25,25);
        else
            return QRect(this->width()-135,20,25,25);

    }
}

QColor IntelDeviceInfoItem::getStatusColor(DEVSTATUS status)
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

QPixmap IntelDeviceInfoItem::getDevConnectedIcon(DEVSTATUS status,QSize size)
{
    QString iconName;
    switch (status) {
    case DEVSTATUS::Paired:
        iconName = ":/img/plugins/bluetooth/not-connected.svg";
        break;
    case DEVSTATUS::Connected:
        iconName = ":/img/plugins/bluetooth/connected.svg";
        break;
    case DEVSTATUS::DisConnectFailed:
    case DEVSTATUS::ConnectFailed:
        iconName = ":/img/plugins/bluetooth/connect-fail.svg";
        break;
    case DEVSTATUS::Connecting:
    case DEVSTATUS::DisConnecting:
        iconName = ":/img/plugins/bluetooth/connecting.svg";
        break;
    default:
        iconName = "";
        break;
    }

    //if (":/img/plugins/bluetooth/not-connected.svg" == iconName && _themeIsBlack && (Status::Hover != _MStatus)) {
    if (":/img/plugins/bluetooth/not-connected.svg" == iconName && _themeIsBlack) {
        return iconName.isEmpty() ? \
                QPixmap() : \
                ImageUtil::loadSvg(iconName,"white",size.width());
    } else
        return iconName.isEmpty() ? \
                QPixmap() : \
                ImageUtil::loadSvg(iconName,"default",size.width());
}

QPixmap IntelDeviceInfoItem::convertIconColor(QPixmap icon, QColor rgb)
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

QRect IntelDeviceInfoItem::getFontPixelQPoint(QString str)
{
    QFont font;
    font.setFamily(_fontFamily);
    font.setPointSize(_fontSize);
    QFontMetrics fm(font);

    return fm.boundingRect(str);
}

void IntelDeviceInfoItem::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (!BlueToothMain::m_device_operating)
        _MStatus = Status::Hover;
    else
        _MStatus = Status::Nomal;
    update();
}

void IntelDeviceInfoItem::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    _MStatus = Status::Nomal;
    _inBtn = false;
    update();
}

void IntelDeviceInfoItem::mouseReleaseEvent(QMouseEvent *event)
{
    long long _releaseCurrentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qDebug() << Q_FUNC_INFO << "_releaseCurrentTime" << _releaseCurrentTime << "_pressCurrentTime:" << _pressCurrentTime;
    if((_releaseCurrentTime - _pressCurrentTime) >= 200)
    {
        qDebug() << Q_FUNC_INFO << "press to release time too long！！！！！！！";
        _pressBtnFlag = false;

        _pressFlag = false;
        _MStatus = Status::Hover;
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (mouseEventIntargetAera(event->pos()) && _pressBtnFlag && !BlueToothMain::m_device_operating) {
            MouseClickedDevFunc();
            _pressBtnFlag = false;
        } else {
            qDebug() << Q_FUNC_INFO << BlueToothMain::m_device_operating ;
            if (!BlueToothMain::m_device_operating)
            {
                MouseClickedFunc();
                _pressFlag = false;
            }
        }
    }
}

void IntelDeviceInfoItem::mousePressEvent(QMouseEvent *event)
{
    //获取当前时间
    _pressCurrentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qDebug() << Q_FUNC_INFO << _pressCurrentTime;

    if (event->button() == Qt::LeftButton) {
        if (mouseEventIntargetAera(event->pos())) {
            _pressBtnFlag = true;
        } else {
            if (!BlueToothMain::m_device_operating)
            {
                _pressFlag = true;
                _MStatus = Status::Check;
            }        }
    }
}

void IntelDeviceInfoItem::MouseClickedDevFunc()
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

    if (bluetoothdevice::DEVICE_TYPE::phone == _MDev->getDevType() ||
        bluetoothdevice::DEVICE_TYPE::computer == _MDev->getDevType())
    {
        QAction *sendfile = nullptr;
        if (_themeIsBlack) {
            QIcon sendfileIcon;
            sendfileIcon.addFile(":/image/icon-bluetooth/ukui-bluetooth-send-file-selected.svg");
            sendfile= new QAction(sendfileIcon.pixmap(18,18),tr("Send files"),dev_Menu);
        } else {
            QIcon sendfileIcon;
            sendfileIcon.addFile(":/image/icon-bluetooth/ukui-bluetooth-send-file.png");
            sendfile= new QAction(sendfileIcon.pixmap(18,18),tr("Send files"),dev_Menu);

        }
        dev_Menu->addAction(sendfile);
        dev_Menu->addSeparator();
    }

    QAction *remove = nullptr;
    if (_themeIsBlack) {
        QIcon sendfileIcon;
        sendfileIcon.addFile(":/image/icon-bluetooth/ukui-bluetooth-delete-selected.svg");
        remove= new QAction(sendfileIcon.pixmap(18,18),tr("remove"),dev_Menu);
    } else {
        QIcon sendfileIcon;
        sendfileIcon.addFile(":/image/icon-bluetooth/ukui-bluetooth-delete.png");
        remove= new QAction(sendfileIcon.pixmap(18,18),tr("remove"),dev_Menu);
    }
    //remove->setText(tr("Remove"));
    dev_Menu->addAction(remove);

    dev_Menu->move(qAbs(sreenPoint.x())+this->width()-200,qAbs(sreenPoint.y())+this->y()+40);
    dev_Menu->exec();

}


void IntelDeviceInfoItem::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseEventIntargetAera(event->pos())) {
        _inBtn = true;
        _MStatus = Status::Nomal;
        this->update();
    } else {
        _MStatus = Status::Hover;
        if (_inBtn) {
            _inBtn = false;
            this->update();
        }
    }

}

void IntelDeviceInfoItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor(Qt::transparent));

    DrawBackground(painter);
    DrawDevTypeIcon(painter);
    DrawText(painter);

    DrawStatusText(painter);

    if (_MDev->isPaired())
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
QColor IntelDeviceInfoItem::getPainterBrushColor()
{
    QColor color;
    switch (_MStatus) {
    case Status::Nomal:

        if(_themeIsBlack)
            color = QColor("#1F2022");//("#EBEBEB");
        else
            color = QColor(Qt::white);//("#EBEBEB");

        //color = QColor(Qt::black);//("#EBEBEB");

        break;
    case Status::Hover:
        if (_inBtn) {
            if(_themeIsBlack)
                color = QColor("#1F2022");//("#EBEBEB");
            else
                color = QColor(Qt::white);//("#EBEBEB");
        } else {
            color = QColor("#D7D7D7");
        }
        break;

    default:
        if(_themeIsBlack)
            color = QColor("#1F2022");//("#EBEBEB");
        else
            color = QColor(Qt::white);//("#EBEBEB");
        //color = QColor(Qt::white);//("#EBEBEB");
        break;
    }

    if (_MStatus == Status::Hover || _MStatus == Status::Check)
        color.setAlpha(50);

    return color;
}

/************************************************
 * @brief  getDevStatusColor
 * @param  null
 * @return QColor
*************************************************/
QColor IntelDeviceInfoItem::getDevStatusColor()
{
    QColor color;

    if (_MDev) {
        if (_MDev->isPaired() && _MDev->isConnected()) {
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
QPixmap IntelDeviceInfoItem::getDevTypeIcon()
{
    QPixmap icon;
    QString iconName;
    if (_MDev) {
        switch (_MDev->getDevType()) {
        case bluetoothdevice::DEVICE_TYPE::phone:
            iconName = "phone-symbolic";
            break;
        case bluetoothdevice::DEVICE_TYPE::computer:
            iconName = "video-display-symbolic";
            break;
        case bluetoothdevice::DEVICE_TYPE::headset:
            iconName = "audio-headset-symbolic";
            break;
        case bluetoothdevice::DEVICE_TYPE::headphones:
            iconName = "audio-headphones-symbolic";
            break;
        case bluetoothdevice::DEVICE_TYPE::audiovideo:
            iconName = "audio-speakers-symbolic";
            break;
        case bluetoothdevice::DEVICE_TYPE::keyboard:
            iconName = "input-keyboard-symbolic";
            break;
        case bluetoothdevice::DEVICE_TYPE::mouse:
            iconName = "input-mouse-symbolic";
            break;
        default:
            //iconName = "bluetooth-active-symbolic";
            iconName = "bluetooth-symbolic";
            break;
        }
    } else {
        //iconName = "bluetooth-active-symbolic";
        iconName = "bluetooth-symbolic";
    }

    //if (_themeIsBlack && (Status::Nomal == _MStatus)) {
    if (_themeIsBlack) {
        icon = ImageUtil::drawSymbolicColoredPixmap(QIcon::fromTheme(iconName).pixmap(18,18),"white");
    } else {
        icon = QIcon::fromTheme(iconName).pixmap(18,18);
    }

    return icon;
}

/************************************************
 * @brief  DrawBackground
 * @param  painter
 * @return null
*************************************************/
void IntelDeviceInfoItem::DrawBackground(QPainter &painter)
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
void IntelDeviceInfoItem::DrawDevTypeIcon(QPainter &painter)
{
    painter.save();
//    painter.setBrush(getDevStatusColor());
//    painter.drawEllipse(14,9,45,45);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    style()->drawItemPixmap(&painter, QRect(10, 13, 36, 36), Qt::AlignCenter, getDevTypeIcon());
    painter.restore();
}

QString IntelDeviceInfoItem::getDeviceName(QString devName)
{
    QString showRealName;
    int nCount = devName.count();
    //qDebug() << Q_FUNC_INFO << "DevName Count:" << nCount << __LINE__;

    int zhCount = 0;
    int charMCount = 0;
    for(int i = 0; i < nCount; i++)
    {
        QChar ch = devName.at(i);
        ushort uNum = ch.unicode();
        if(uNum >= 0x4E00 && uNum <= 0x9FA5)
        {
            // 这个字符是中文
            zhCount ++;
            //qDebug() << Q_FUNC_INFO << "this is zh:" << uNum << zhCount << __LINE__;
        }

        if (uNum == 0x004D || uNum == 0x006D)
        {
            // 这个字符是m/M
            charMCount++;
            //qDebug() << Q_FUNC_INFO << "this is char m/M:" << uNum << charMCount << __LINE__;
        }
    }
    if(devName.length() > (ShowNameTextNumberMax() - zhCount - charMCount/2)) /*减去中文长度 或者 m字符显示长度*/
    {
        showRealName = devName.left(ShowNameTextNumberMax()- zhCount - charMCount/2 - 3);
        showRealName.append("...");
        toptipflag = true;
        this->setToolTip(_MDev->getDevName());
    }
    else
    {
        showRealName = devName;
        toptipflag = false;
        this->setToolTip("");
    }
    //qDebug() << Q_FUNC_INFO << "showRealName:" << showRealName << __LINE__;

    return showRealName;
}

int IntelDeviceInfoItem::ShowNameTextNumberMax()
{
    float display_coefficient = (this->width() - 350)/450.00;
    //qDebug() << Q_FUNC_INFO << "display coefficient:" << display_coefficient << __LINE__;
    int max_text_number = 0;
    switch (_fontSize)
    {
    case 10:
        max_text_number += 6 ;
    case 11:
        max_text_number += 5 ;
    case 12:
        max_text_number += 4 ;
    case 13:
        max_text_number += 3 ;
    case 14:
        max_text_number += 2 ;
    case 15:
    default:
        max_text_number += 35 ;
        break;
    }

    int showMaxNameLength = max_text_number*display_coefficient;
    //qDebug() << Q_FUNC_INFO << "The max length :" <<  showMaxNameLength << __LINE__;

    return (showMaxNameLength);

}

/************************************************
 * @brief  DrawText
 * @param  painter
 * @return null
*************************************************/
void IntelDeviceInfoItem::DrawText(QPainter &painter)
{
    painter.save();
    if(_themeIsBlack)
        painter.setPen(QColor(Qt::white));
    else
        painter.setPen(QColor(Qt::black));

//    if (_MStatus == Status::Hover) {
//        painter.setPen(QColor(Qt::black));
//    }

    painter.drawText(60,20,this->width()-300,24,Qt::AlignLeft,_MDev?getDeviceName( _MDev->getDevName() ): QString("Example"));
    painter.restore();
}

void IntelDeviceInfoItem::DrawStatusText(QPainter &painter)
{
    painter.save();
    if(_themeIsBlack)
        painter.setPen(QColor(Qt::white));
    else
        painter.setPen(QColor(Qt::black));

    QString str;

    switch (_DevStatus) {
    case DEVSTATUS::Paired:
        str = m_str_dev_ununited;
        break;
    case DEVSTATUS::Connected:
        str = m_str_dev_connected;
        break;
    case DEVSTATUS::Connecting:
        str = m_str_dev_connecting;
        break;
    case DEVSTATUS::DisConnecting:
        str = m_str_dev_disconnecting;
        break;
    case DEVSTATUS::DisConnectFailed:
        str = m_str_dev_disconn_fail;
        break;
    case DEVSTATUS::ConnectFailed:
        str = m_str_dev_conn_fail;
        break;
    default:
        break;
    }

    QRect rect = getFontPixelQPoint(str);

    painter.setPen(getStatusColor(_DevStatus));
    style()->drawItemPixmap(&painter,getStatusIconRect(rect), Qt::AlignCenter, getDevConnectedIcon(_DevStatus,QSize(20,20)));
    painter.drawText(getStatusTextRect(rect),Qt::AlignRight,(str));

    painter.restore();
}

void IntelDeviceInfoItem::DrawFuncBtn(QPainter &painter)
{
    QString iconName;

    painter.save();

    if (_inBtn) {
//        painter.setPen(QColor("#2FB3E8"));
        iconName = ":/img/plugins/bluetooth/more-blue.svg";
    } else {
//        painter.setBrush(getPainterBrushColor());
        iconName = ":/img/plugins/bluetooth/more.svg";
    }

    //painter.drawRoundRect(this->width()-55,14,0,0,30,30);

    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (_themeIsBlack && !_inBtn)
        style()->drawItemPixmap(&painter, QRect(this->width()-48,23,20,20), Qt::AlignCenter, ImageUtil::loadSvg(iconName,"white",20));
    else

        style()->drawItemPixmap(&painter, QRect(this->width()-48,23,20,20), Qt::AlignCenter, ImageUtil::loadSvg(iconName,"default",20));

    painter.restore();
}

/************************************************
 * @brief  MouseClickedFunc
 * @param  null
 * @return null
*************************************************/
void IntelDeviceInfoItem::MouseClickedFunc()
{
    _clicked = true;
    _pressFlag = false;

    update();

    BlueToothMain::m_device_operating = true;

    if (_MDev) {
        if (_MDev->isConnected()) {
            qDebug() << Q_FUNC_INFO << "devDisconnect: "<<  _MDev->getDevName() <<  _MDev->getDevAddress() << __LINE__;
            //_MDev.data()->disconnectFromDevice();
            _DevStatus = DEVSTATUS::DisConnecting;
            emit devDisconnect(_MDev->getDevAddress());
        } else {
            qDebug() << Q_FUNC_INFO << "devConnect: "<< _MDev->getDevName() <<  _MDev->getDevAddress() << __LINE__;
            //DevConnectFunc();
            _DevStatus = DEVSTATUS::Connecting;
            emit devConnect(_MDev->getDevAddress());
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
void IntelDeviceInfoItem::DevConnectFunc()
{
//    if (_MDev)
//    {
//        if (_MDev.data()->type() == BluezQt::Device::AudioVideo ||
//            _MDev.data()->type() == BluezQt::Device::Headphones ||
//            _MDev.data()->type() == BluezQt::Device::Headset    ) {

//            BluezQt::AdapterPtr MDevAdapter = _MDev.data()->adapter();

//            for (auto dev : MDevAdapter.data()->devices()) {
//                if (dev.data()->isConnected() && dev.data()->isPaired()) {
//                    if (dev.data()->type() == BluezQt::Device::AudioVideo ||
//                        dev.data()->type() == BluezQt::Device::Headphones ||
//                        dev.data()->type() == BluezQt::Device::Headset    ) {

//                        BluezQt::PendingCall *pending = dev.data()->disconnectFromDevice();
//                        connect(pending,&BluezQt::PendingCall::finished,this,[&](BluezQt::PendingCall *call){
//                            if (!call->error()) {
//                                _MDev.data()->connectToDevice();
//                            }
//                        });
//                        return;
//                    }
//                }
//            }
//            _MDev.data()->connectToDevice();
//        } else {
//            _MDev.data()->connectToDevice();
//        }
//    }
}

void IntelDeviceInfoItem::TimedRestoreConnectionErrorDisplay()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    //错误信息显示超时后，显示错误操作后的设备状态
    QTimer::singleShot(8000,this,[=]{

        if((_DevStatus != DEVSTATUS::Connecting) && (_DevStatus != DEVSTATUS::Connecting))
        {
            if (_MDev->isPaired())
            {
                _DevStatus = DEVSTATUS::Paired;

            if (_MDev->isConnected())
                _DevStatus = DEVSTATUS::Connected;

            } else {
                _DevStatus = DEVSTATUS::NoPaired;
            }
            update();
        }
    });
}

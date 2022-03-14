#include "devicebase.h"

//devicebase
devicebase::devicebase()
{

}
//devicebase end

//bluetoothadapter
bluetoothadapter::bluetoothadapter(QString dev_name ,
                                   const QString dev_address ,
                                   bool    dev_power ,
                                   bool    dev_discovering ,
                                   bool    dev_discoverable)
    :m_dev_name(dev_name)
    ,m_dev_address(dev_address)
    ,m_dev_power(dev_power)
    ,m_dev_discovering(dev_discovering)
    ,m_dev_discoverable(dev_discoverable)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    this->setObjectName(dev_address);

}

void bluetoothadapter::resetDeviceName(QString new_dev_name)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    this->m_dev_name = new_dev_name;
}

QString bluetoothadapter::getDevName()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    return this->m_dev_name;
}

QString bluetoothadapter::getDevAddress()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    return this->m_dev_address;
}


void bluetoothadapter::setDevPower(bool value)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    this->m_dev_power = value;
}

bool bluetoothadapter::getDevPower()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    return this->m_dev_power;
}

void bluetoothadapter::setDevDiscovering(bool value)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    this->m_dev_discovering = value;
}

bool bluetoothadapter::getDevDiscovering()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    return this->m_dev_discovering;
}

void bluetoothadapter::setDevDiscoverable(bool value)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    this->m_dev_discoverable = value;
}

bool bluetoothadapter::getDevDiscoverable()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    return this->m_dev_discoverable;
}

//bluetoothadapter end


//bluetoothdevice
bluetoothdevice::bluetoothdevice(QString   dev_name ,
                                 QString   dev_address ,
                                 DEVICE_TYPE   dev_type ,
                                 bool            dev_paired_status ,
                                 bool            dev_connected_status ,
                                 //DEVICE_STATUS dev_status ,
                                 bool            dev_trust)
    :m_dev_name(dev_name)
    ,m_dev_address(dev_address)
    ,m_dev_type(dev_type)
    ,m_dev_isPaired(dev_paired_status)
    ,m_dev_isConnected(dev_connected_status)
    //,m_dev_status(dev_status)
    ,m_dev_trust(dev_trust)
{
    this->setObjectName(dev_address);
    clearErrorInfo();
    //qDebug() << Q_FUNC_INFO << __LINE__;
//    if (DEVICE_STATUS::PairedAndUnlinked == dev_status ||
//        DEVICE_STATUS::PairedAndLinked == dev_status )
//    {
//        this->m_dev_isPaired = true ;
//        this->m_dev_trust = true ;
//    }
//    else
//    {
//        this->m_dev_isPaired = false ;
//        this->m_dev_trust = false ;
//    }

//    if (DEVICE_STATUS::UnpairAndLinked == dev_status ||
//        DEVICE_STATUS::PairedAndLinked == dev_status )
//        this->m_dev_isConnected = true ;
//    else
//        this->m_dev_isConnected = false ;

}

void bluetoothdevice::resetDeviceName(QString new_dev_name)
{
    //qDebug() << Q_FUNC_INFO << __LINE__;
    this->m_dev_name = new_dev_name;
    emit nameChanged(new_dev_name);
}

QString bluetoothdevice::getDevName()
{
    //qDebug() << Q_FUNC_INFO << __LINE__;
    return this->m_dev_name;
}

QString bluetoothdevice::getDevAddress()
{
    //qDebug() << Q_FUNC_INFO << __LINE__;
    return this->m_dev_address;
}

void bluetoothdevice::setDevType(DEVICE_TYPE type)
{
    //qDebug() << Q_FUNC_INFO << __LINE__;
    this->m_dev_type = type;
    emit typeChanged(type);
}
bluetoothdevice::DEVICE_TYPE bluetoothdevice::getDevType()
{
    //qDebug() << Q_FUNC_INFO << __LINE__;
    return this->m_dev_type;

}
//void bluetoothdevice::setDevStatus(DEVICE_STATUS status)
//{
//    //qDebug() << Q_FUNC_INFO << __LINE__;

//}
//bluetoothdevice::DEVICE_STATUS bluetoothdevice::getDevStatus()
//{
//    //qDebug() << Q_FUNC_INFO << __LINE__;

//}

void bluetoothdevice::setDevTrust(bool value)
{
    //qDebug() << Q_FUNC_INFO << __LINE__;
    this->m_dev_trust = value;
    emit trustChanged(value);

}
bool bluetoothdevice::getDevTrust()
{
    //qDebug() << Q_FUNC_INFO << __LINE__;
}

bool bluetoothdevice::isPaired()
{
    //qDebug() << Q_FUNC_INFO << __LINE__;
    return m_dev_isPaired;
}
void bluetoothdevice::devPairedChanged(bool value)
{
    qDebug() << Q_FUNC_INFO << value << __LINE__;
    if(this->m_dev_isPaired != value)
    {
        this->m_dev_isPaired = value;
        emit pairedChanged(value);
    }
}

bool bluetoothdevice::isConnected()
{
    //qDebug() << Q_FUNC_INFO << __LINE__;
    return m_dev_isConnected;
}

void bluetoothdevice::devConnectedChanged(bool value)
{
    if(this->m_dev_isConnected != value)
    {
        this->m_dev_isConnected = value;
        emit connectedChanged(value);
    }
}

void bluetoothdevice::setErrorInfo(int errorId,QString errorText)
{
    this->m_errorId = errorId;
    this->m_errorText = errorText;
    emit errorInfoRefresh(errorId,errorText);
}

void bluetoothdevice::clearErrorInfo()
{
    this->m_errorId = 0;
    this->m_errorText = "";
}

//bluetoothdevice end


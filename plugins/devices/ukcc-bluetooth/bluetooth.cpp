#include "bluetooth.h"

#include <QDebug>


static QString executeLinuxCmd(QString strCmd)
{
    QProcess p;
    p.start("bash", QStringList() <<"-c" << strCmd);
    p.waitForFinished();
    QString strResult = p.readAllStandardOutput();
    return strResult.toLower();
}

static bool IsHuawei()
{
    QString str = executeLinuxCmd("cat /proc/cpuinfo | grep Hardware");
    if(str.length() == 0)
    {
        return false;
    }

    if(str.indexOf("huawei") != -1 || str.indexOf("pangu") != -1 ||
            str.indexOf("kirin") != -1)
    {
        return true;
    }
    else
    {
        return false;
    }
    return false;
}

Bluetooth::Bluetooth() : mFirstLoad(true) {

    qDebug() << Q_FUNC_INFO << "++ukccBluetooth========================" << __LINE__;
    //global_ishuawei = IsHuawei();
    //qDebug() << "Ishuawei: "<< global_ishuawei;

    QList <quint64> btServiceProcessId;
    bool m_bluetooth_service_process_flag = ukccbluetoothconfig::checkProcessRunning(BluetoothServiceExePath/*"bluetoothService"*/,btServiceProcessId);

    for(quint64 tempId : btServiceProcessId)
    {
        qDebug () << Q_FUNC_INFO << "========================" << tempId;
    }

    if (m_bluetooth_service_process_flag)
    {
        qDebug () << Q_FUNC_INFO << BluetoothServiceExePath << "is Running" ;
    }
    else
    {
        qDebug () << Q_FUNC_INFO << BluetoothServiceExePath << "is not Running" ;

        QList <quint64> btTrayProcessId;
        bool m_ukui_bluetooth_process_flag = ukccbluetoothconfig::checkProcessRunning(BluetoothTrayExePath,btTrayProcessId);
        if (m_ukui_bluetooth_process_flag)
        {
            qDebug () << Q_FUNC_INFO << BluetoothTrayExePath << "is Running" ;
            for (quint64 processId:btTrayProcessId)
            {
                qDebug () << Q_FUNC_INFO << "ProcessId:" << btTrayProcessId << "is Running" ;
                ukccbluetoothconfig::killAppProcess(processId);
            }
        }
        else
        {
            qDebug () << Q_FUNC_INFO << BluetoothTrayExePath << "is not Running" ;
        }
        ukccbluetoothconfig::launchBluetoothServiceStart(BluetoothServiceExePath);
    }

    pluginName = tr("Bluetooth");
//#ifdef DEVICE_IS_INTEL
    //ukccbluetoothconfig::m_isIntel = QFile::exists("/etc/apt/ota_version");
//    ukccbluetoothconfig::m_isIntel = true;
//    pluginType = NETWORK;
//#else
    ukccbluetoothconfig::m_isIntel = false;
    pluginType = DEVICES;
//#endif
}

Bluetooth::~Bluetooth() {
    if (!mFirstLoad) {
//        delete pluginWidget;
    }
}

QString Bluetooth::get_plugin_name() {
    return pluginName;
}

int Bluetooth::get_plugin_type() {
    return pluginType;
}

QWidget *Bluetooth::get_plugin_ui() {

    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new BlueToothMain;
    }
    //if (!mFirstLoad && (nullptr != pluginWidget))
    //    pluginWidget->setbluetoothAdapterDiscoveringStatus(true);
    return pluginWidget;
}

void Bluetooth::plugin_delay_control() {

}

const QString Bluetooth::name() const {
    return QStringLiteral("bluetooth");
}

//Bluetooth::Bluetooth() : mFirstLoad(true) {

//    QList <quint64> btServiceProcessId;
//    bool m_bluetooth_service_process_flag = ukccbluetoothconfig::checkProcessRunning(BluetoothServiceExePath/*"bluetoothService"*/,btServiceProcessId);

//    for(quint64 tempId : btServiceProcessId)
//    {
//        qDebug () << Q_FUNC_INFO << "========================" << tempId;
//    }

//    if (m_bluetooth_service_process_flag)
//    {
//        qDebug () << Q_FUNC_INFO << BluetoothServiceExePath << "is Running" ;
//    }
//    else
//    {
//        qDebug () << Q_FUNC_INFO << BluetoothServiceExePath << "is not Running" ;

//        QList <quint64> btTrayProcessId;
//        bool m_ukui_bluetooth_process_flag = ukccbluetoothconfig::checkProcessRunning(BluetoothTrayExePath,btTrayProcessId);
//        if (m_ukui_bluetooth_process_flag)
//        {
//            qDebug () << Q_FUNC_INFO << BluetoothTrayExePath << "is Running" ;
//            for (quint64 processId:btTrayProcessId)
//            {
//                qDebug () << Q_FUNC_INFO << "ProcessId:" << btTrayProcessId << "is Running" ;
//                ukccbluetoothconfig::killAppProcess(processId);
//            }
//        }
//        else
//        {
//            qDebug () << Q_FUNC_INFO << BluetoothTrayExePath << "is not Running" ;
//        }
//        ukccbluetoothconfig::launchBluetoothServiceStart(BluetoothServiceExePath);
//    }

//    pluginName = tr("Bluetooth");
//    ukccbluetoothconfig::m_isIntel = QFile::exists("/etc/apt/ota_version");
//    if (ukccbluetoothconfig::m_isIntel)
//        pluginType = NETWORK;
//    else
//        pluginType = DEVICES;
//}

//Bluetooth::~Bluetooth() {
//    if (!mFirstLoad) {
////        delete pluginWidget;
//    }
//}

//QString Bluetooth::get_plugin_name() {
//    return pluginName;
//}

//int Bluetooth::get_plugin_type() {
//    return pluginType;
//}

//QWidget *Bluetooth::get_plugin_ui() {

//    if (mFirstLoad) {
//        mFirstLoad = false;
//        pluginWidget = new BlueToothMain;
////        pluginWidget = new BlueToothMainWindow;
//    }
//    //if (!mFirstLoad && (nullptr != pluginWidget))
//    //    pluginWidget->setbluetoothAdapterDiscoveringStatus(true);
//    return pluginWidget;
//}

//const QString Bluetooth::name() const {
//    return QStringLiteral("bluetooth");
//}


//void Bluetooth::plugin_delay_control() {

//}




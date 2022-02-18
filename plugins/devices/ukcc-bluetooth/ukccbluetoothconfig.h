#ifndef UKCCBLUETOOTHCONFIG_H
#define UKCCBLUETOOTHCONFIG_H

#include <QProcess>
#include <QDebug>

#include "config.h"

class ukccbluetoothconfig
{
public:
    ukccbluetoothconfig();

    static void launchBluetoothServiceStart(const QString &processName);
    static void killAppProcess(const quint64 &processId);
    static bool checkProcessRunning(const QString &processName, QList<quint64> &listProcessId);

    static bool m_isIntel;
};

#endif // UKCCBLUETOOTHCONFIG_H

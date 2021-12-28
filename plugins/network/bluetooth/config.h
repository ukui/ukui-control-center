#ifndef CONFIG_H
#define CONFIG_H
#include <QString>

#define BluetoothServiceExePath "/usr/bin/bluetoothService -o"
#define BluetoothTrayExePath    "/usr/bin/ukui-bluetooth"
#define BluetoothServiceName    "bluetoothService"
#define BluetoothTrayName       "ukui-bluetooth"

const QString SERVICE   = "com.ukui.bluetooth";
const QString PATH      = "/com/ukui/bluetooth";
const QString INTERFACE = "com.ukui.bluetooth";

#endif // CONFIG_H

#ifndef CONFIG_H
#define CONFIG_H
#include <QString>
#include <QPixmap>
#include <QIcon>

#define BluetoothServiceExePath "/usr/bin/bluetoothService -o"
#define BluetoothTrayExePath    "/usr/bin/ukui-bluetooth"
#define BluetoothServiceName    "bluetoothService"
#define BluetoothTrayName       "ukui-bluetooth"

#define SYSTEMSTYLESCHEMA "org.ukui.style"
#define SYSTEMSTYLENAME "styleName"
#define SYSTEMFONTSIZE  "systemFontSize"
#define SYSTEMFONT      "systemFont"

#define MAX_DEVICE_CONECTIONS_TIMES 3

#define DELAYED_SCANNING_TIME_S (2*1000)
#define DEVICE_CONNECTION_TIMEOUT_S (30*1000)
#define LOADING_ICON_TIMEOUT_INTERVAL_MS 110

const QString SERVICE   = "com.ukui.bluetooth";
const QString PATH      = "/com/ukui/bluetooth";
const QString INTERFACE = "com.ukui.bluetooth";

const QPixmap renderSvg(const QIcon &icon, QString cgColor);
//全局变量，是否是华为机器, 默认false, 在bluetooth.cpp文件里面定义
extern bool global_ishuawei;
#endif // CONFIG_H

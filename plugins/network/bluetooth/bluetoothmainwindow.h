#ifndef BLUETOOTHMAINWINDOW_H
#define BLUETOOTHMAINWINDOW_H

#include <ukcc/widgets/switchbutton.h>

#include "bluetoothnamelabel.h"
#include "loadinglabel.h"

#include <QMainWindow>
#include <QWidget>
#include <QStackedWidget>
#include <QWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QFrame>
#include <QStringList>
#include <QDebug>
#include <QIcon>
#include <QMetaEnum>

class BlueToothMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum DEVTYPESHOW {
        All = 0,
        Audio,
        Peripherals,
        Computer,
        Phone,
        Other,
    };
    Q_ENUM(DEVTYPESHOW);

    explicit BlueToothMainWindow(QWidget *parent = nullptr);
    ~BlueToothMainWindow();
protected:
    void InitMainWindow();
    void InitError0Widget();
    void InitError1Widget();
    void InitNormalWidget();
    void InitNormalWidgetTop();
    void InitNormalWidgetMiddle();
    void InitNormalWidgetBottom();

    void InitInterfaceUIStatus();

private:
    QStackedWidget *_MCentralWidget = nullptr;
    QWidget        *_MError0Widget  = nullptr;
    QWidget        *_MError1Widget  = nullptr;
    QWidget        *_MNormalWidget  = nullptr;

    QVBoxLayout      *_NormalWidgetMainLayout = nullptr;
    QVBoxLayout *_NormalWidgetPairedDevLayout = nullptr;
    QVBoxLayout  *_NormalWidgetCacheDevLayout = nullptr;

    QWidget       *_MNormalWidgetTop = nullptr;
    QWidget *_MNormalWidgetTopMiddle = nullptr;
    QWidget *_MNormalWidgetTopBottom = nullptr;
    QWidget         *_CacheDevWidget = nullptr;

    SwitchButton       *_BtSwitchBtn = nullptr;
    SwitchButton    *_BtTrayIconShow = nullptr;
    LoadingLabel          *_LoadIcon = nullptr;
    SwitchButton    *_BtDiscoverable = nullptr;
    BluetoothNameLabel *_BtNameLabel = nullptr;
    QComboBox    *_AdapterListSelect = nullptr;
    QComboBox        *_DevTypeSelect = nullptr;
};

#endif // BLUETOOTHMAINWINDOW_H

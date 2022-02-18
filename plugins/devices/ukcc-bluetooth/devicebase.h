#ifndef DEVICEBASE_H
#define DEVICEBASE_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QString>
#include <QDebug>

#include "config.h"

class devicebase;
class bluetoothadapter;
class bluetoothdevice;

//devicebase
class devicebase : public QObject
{
    Q_OBJECT
public:
    devicebase();
    virtual ~devicebase(){}

    virtual void resetDeviceName(QString)    = 0 ;
    virtual QString getDevName()      = 0;
    virtual QString getDevAddress()   = 0;

//signals:
//    void deviceNameChanged();

};
//devicebase end


//bluetoothadapter
class bluetoothadapter : public devicebase
{
    Q_OBJECT
public:
    bluetoothadapter(QString dev_name ,
                     QString dev_address,
                     bool    dev_power ,
                     bool    dev_discovering,
                     bool    dev_discoverable);

    ~bluetoothadapter(){}

    void resetDeviceName(QString)  Q_DECL_OVERRIDE ;
    QString getDevName()    Q_DECL_OVERRIDE ;
    QString getDevAddress() Q_DECL_OVERRIDE ;

    void setDevPower(bool);
    bool getDevPower();
    void setDevDiscovering(bool);
    bool getDevDiscovering();
    void setDevDiscoverable(bool);
    bool getDevDiscoverable();

    QList<bluetoothdevice *> m_bluetooth_device_list;
    QList<bluetoothdevice *> m_bluetooth_device_paired_list;

private:
    QString m_dev_name;
    QString m_dev_address;
    bool m_dev_power;
    bool m_dev_discovering;
    bool m_dev_discoverable;


};
//bluetoothadapter end

//bluetoothdevice
class bluetoothdevice : public devicebase
{
    Q_OBJECT

public:
    enum DEVICE_TYPE{
        /** The device is a phone. */
        phone = 0,
        /** The device is a modem. */
        modem,
        /** The device is a computer. */
        computer,
        /** The device is a network. */
        network,
        /** The device is a headset. */
        headset,
        /** The device is a headphones. */
        headphones,
        /** The device is an uncategorized audio video device. */
        audiovideo,
        /** The device is a keyboard. */
        keyboard,
        /** The device is a mouse. */
        mouse,
        /** The device is a joypad. */
        joypad,
        /** The device is a graphics tablet (input device). */
        tablet,
        /** The deivce is an uncategorized peripheral device. */
        peripheral,
        /** The device is a camera. */
        camera,
        /** The device is a printer. */
        printer,
        /** The device is an uncategorized imaging device. */
        imaging,
        /** The device is a wearable device. */
        wearable,
        /** The device is a toy. */
        toy,
        /** The device is a health device. */
        health,
        /** The device is not of any of the known types. */
        uncategorized
    };
    Q_ENUM(DEVICE_TYPE)


//    enum DEVICE_STATUS{
//        Unkown = 0,
//        UnpairAndUnlinked,    //
//        UnpairAndLinked,
//        PairedAndUnlinked,
//        PairedAndLinked,
//        Error
//    };
//    Q_ENUM(DEVICE_STATUS)

    enum Error {
        /** Indicates there is no error. */
        NoError = 0,
        /** Indicates that the device is not ready. */
        NotReady = 1,
        /** Indicates that the action have failed. */
        Failed = 2,
        /** Indicates that the action was rejected. */
        Rejected = 3,
        /** Indicates that the action was canceled. */
        Canceled = 4,
        /** Indicates that invalid arguments were passed. */
        InvalidArguments = 5,
        /** Indicates that an agent or pairing record already exists. */
        AlreadyExists = 6,
        /** Indicates that an agent, service or pairing operation does not exists. */
        DoesNotExist = 7,
        /** Indicates that the action is already in progress. */
        InProgress = 8,
        /** Indicates that the action is not in progress. */
        NotInProgress = 9,
        /** Indicates that the device is already connected. */
        AlreadyConnected = 10,
        /** Indicates that the connection to the device have failed. */
        ConnectFailed = 11,
        /** Indicates that the device is not connected. */
        NotConnected = 12,
        /** Indicates that the action is not supported. */
        NotSupported = 13,
        /** Indicates that the caller is not authorized to do the action. */
        NotAuthorized = 14,
        /** Indicates that the authentication was canceled. */
        AuthenticationCanceled = 15,
        /** Indicates that the authentication have failed. */
        AuthenticationFailed = 16,
        /** Indicates that the authentication was rejected. */
        AuthenticationRejected = 17,
        /** Indicates that the authentication timed out. */
        AuthenticationTimeout = 18,
        /** Indicates that the connection attempt have failed. */
        ConnectionAttemptFailed = 19,
        /** Indicates that the data provided generates a data packet which is too long. */
        InvalidLength = 20,
        /** Indicates that the action is not permitted (e.g. maximum reached or socket locked). */
        NotPermitted = 21,
        /** Indicates an error with D-Bus. */
        DBusError = 98,
        /** Indicates an internal error. */
        InternalError = 99,
        /** Indicates an unknown error. */
        UnknownError = 100
    };
    Q_ENUM(Error)

    bluetoothdevice(QString         device_name ,
                    QString         device_address ,
                    DEVICE_TYPE     dev_type ,
                    bool            dev_paired_status ,
                    bool            dev_connected_status ,
                    //DEVICE_STATUS dev_status ,
                    bool            dev_trust);
    ~bluetoothdevice(){}

    void resetDeviceName(QString)  Q_DECL_OVERRIDE ;
    QString getDevName()    Q_DECL_OVERRIDE ;
    QString getDevAddress() Q_DECL_OVERRIDE ;

    void setDevType(DEVICE_TYPE);
    DEVICE_TYPE getDevType();
    //void setDevStatus(DEVICE_STATUS);
    //DEVICE_STATUS getDevStatus();

    bool isPaired();
    void devPairedChanged(bool);
    bool isConnected();
    void devConnectedChanged(bool);

    void setDevTrust(bool);
    bool getDevTrust();
    void setErrorInfo(int,QString);
    void clearErrorInfo();

signals:

    void nameChanged(QString);
    void typeChanged(DEVICE_TYPE);
    void pairedChanged(bool);
    void connectedChanged(bool);
    void trustChanged(bool);
    void errorInfoRefresh(int,QString);

private:

    QString         m_dev_name;
    QString         m_dev_address;
    DEVICE_TYPE     m_dev_type;
    //DEVICE_STATUS   m_dev_status;
    bool            m_dev_trust ;
    bool            m_dev_isPaired;
    bool            m_dev_isConnected;
    bool            m_dev_support_sendFile;
    int             m_errorId;
    QString         m_errorText;
};
//bluetoothdevice end

#endif // DEVICEBASE_H

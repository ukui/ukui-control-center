#ifndef KEYVALUECONVERTER_H
#define KEYVALUECONVERTER_H

#include <QObject>
#include <QMetaEnum>


class KeyValueConverter : public QObject
{
    Q_OBJECT

public:
    explicit KeyValueConverter();
    ~KeyValueConverter();

public:
    QString keycodeTokeystring(int code);
    int keystringTokeycode(QString string);
    QString keycodeTokeyi18nstring(int code);

public:
    QMetaEnum metaModule;

    enum FunType{
        SYSTEM,
        DEVICES,
        PERSONALIZED,
        NETWORK,
        ACCOUNT,
        DATETIME,
        UPDATE,
        MESSAGES,
        FUNCTOTALNUM,
    };
    Q_ENUM(FunType)

};

#endif // KEYVALUECONVERTER_H

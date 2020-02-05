#include "keyvalueconverter.h"

KeyValueConverter::KeyValueConverter()
{
    metaModule = QMetaEnum::fromType<KeyValueConverter::FunType>();
}

KeyValueConverter::~KeyValueConverter()
{
}

QString KeyValueConverter::keycodeTokeystring(int code){
    //未匹配到则返回空
    return metaModule.valueToKey(code);
}

int KeyValueConverter::keystringTokeycode(QString string){
    //QString to const char *
    QByteArray ba = string.toLocal8Bit(); const char * str = ba.data();
    return metaModule.keyToValue(str);
}

QString KeyValueConverter::keycodeTokeyi18nstring(int code){
    QString nameString;
    switch (code) {
    case SYSTEM:
        nameString = tr("system");
        break;
    case DEVICES:
        nameString = tr("devices");
        break;
    case PERSONALIZED:
        nameString = tr("personalized");
        break;
    case NETWORK:
        nameString = tr("network");
        break;
    case ACCOUNT:
        nameString = tr("account");
        break;
    case DATETIME:
        nameString = tr("datetime");
        break;
    case UPDATE:
        nameString = tr("update");
        break;
    case MESSAGES:
        nameString = tr("messages");
        break;
    default:
        break;
    }
    return nameString;
}

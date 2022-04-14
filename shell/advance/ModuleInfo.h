#ifndef MODULEINFO_H
#define MODULEINFO_H

#include <QMap>
#include <QVariant>
#include <QString>
#include <QDBusArgument>

struct ModuleInfo
{
    QString arg;
    QDBusVariant out;
};

QDBusArgument &operator<<(QDBusArgument &argument, const ModuleInfo &mystruct)
{
    argument.beginStructure();
    argument << mystruct.arg << mystruct.out;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ModuleInfo &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.arg >> mystruct.out;
    argument.endStructure();
    return argument;
}

Q_DECLARE_METATYPE(ModuleInfo)

#endif // MODULEINFO_H

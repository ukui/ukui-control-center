#ifndef SYSDBUSREGISTER_H
#define SYSDBUSREGISTER_H

#include <QObject>
#include <QCoreApplication>
//#include <QProcess>

#include <QFile>

class SysdbusRegister : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.control.center.interface")

public:
    explicit SysdbusRegister();
    ~SysdbusRegister();

private:
//    QString m_name;

signals:
    Q_SCRIPTABLE void nameChanged(QString);
    Q_SCRIPTABLE void computerinfo(QString);

public slots:
//    Q_SCRIPTABLE QString name() const;
//    Q_SCRIPTABLE void SetName(QString name);

    Q_SCRIPTABLE void exitService();
    Q_SCRIPTABLE QString GetComputerInfo();
};

#endif // SYSDBUSREGISTER_H

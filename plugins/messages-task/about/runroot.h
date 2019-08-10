#ifndef RUNROOT_H
#define RUNROOT_H

#include <QObject>

class RunRoot : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.ukcc.interface")

public:
    explicit RunRoot();
    ~RunRoot();

private:
    QString m_name;

signals:
    void nameChanged(QString);

public slots:
    QString name() const {return m_name;}
    void SetName(QString name){
        m_name = name;
    }
};

#endif // RUNROOT_H

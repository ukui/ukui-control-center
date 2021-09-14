#ifndef MTHREAD_H
#define MTHREAD_H

#include <QObject>
#include <QtDBus>

class MThread : public QObject
{
    Q_OBJECT
public:
    explicit MThread(QObject *parent = nullptr);
    ~MThread();

    void run();

private:
    QDBusInterface *m_cloudInterface;

public slots:
    void keychanged(QString  str);

Q_SIGNALS:
    void keychangedsignal(QString  str);
};

#endif // MTHREAD_H

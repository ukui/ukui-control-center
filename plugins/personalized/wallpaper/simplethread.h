#ifndef SIMPLETHREAD_H
#define SIMPLETHREAD_H

#include <QThread>
#include <QMap>
#include <QListWidgetItem>

class SimpleThread : public QThread
{
    Q_OBJECT

public:
    explicit SimpleThread(QMap<QString, QMap<QString, QString>> _wpinfo, QObject * parent);
    ~SimpleThread();

protected:
    void run();

private:
    QMap<QString, QMap<QString, QString>> wallpaperinfosMap;

Q_SIGNALS:
    void widgetItemCreate(QPixmap pixmap, QString filename);


};

#endif // SIMPLETHREAD_H

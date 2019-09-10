#ifndef CUSTDOMFRAME_H
#define CUSTDOMFRAME_H

#include <QFrame>

class CustdomFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CustdomFrame(const QString &t, QWidget *parent = 0);
    ~CustdomFrame();

    QString getText();

private:
    QString text;
};

#endif // CUSTDOMFRAME_H

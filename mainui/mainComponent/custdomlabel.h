#ifndef CUSTDOMLABEL_H
#define CUSTDOMLABEL_H

#include <QLabel>
#include <QMouseEvent>

class CustdomLabel : public QLabel{
    Q_OBJECT

public:
    explicit CustdomLabel(const QString &text, QWidget * parent=0);
    ~CustdomLabel();

protected:
    void mousePressEvent(QMouseEvent * event);

signals:
    void clicked();
};

#endif // CUSTDOMLABEL_H

#ifndef CLICKLABEL_H
#define CLICKLABEL_H
#include <QLabel>
#include <QMouseEvent>

class clickLabel : public QLabel
{
     Q_OBJECT
public:
    explicit clickLabel();
    ~clickLabel();
protected:
    void mousePressEvent(QMouseEvent * event);

signals:
    void clicked();
};
#endif // CLICKLABEL_H

#ifndef ICONLABEL_H
#define ICONLABEL_H

#include <QObject>
#include <QWidget>
#include <QLabel>

class IconLabel : public QLabel
{
    Q_OBJECT
public:
    IconLabel(QWidget *parent = nullptr);


protected:
    void leaveEvent(QEvent * event);
    void enterEvent(QEvent * event);

signals:
    void enterWidget();
    void leaveWidget();
};

#endif // ICONLABEL_H

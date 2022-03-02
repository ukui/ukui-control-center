#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include <QDebug>
#include <QComboBox>
#include <QWheelEvent>

class myComboBox : public QComboBox
{
public:
    explicit myComboBox(QWidget * parent = nullptr);

private:
    void wheelEvent(QWheelEvent *event);
};

#endif // MYCOMBOBOX_H

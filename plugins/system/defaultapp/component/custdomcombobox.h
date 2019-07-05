#ifndef CUSTDOMCOMBOBOX_H
#define CUSTDOMCOMBOBOX_H

#include <QComboBox>
#include <QMouseEvent>

class CustdomComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit CustdomComboBox(QWidget *parent = 0);
    ~CustdomComboBox();

public slots:
    void itemchangedSlot(int itemindex);

Q_SIGNALS:
    void itemchangedSignal(int index);

};

#endif // CUSTDOMCOMBOBOX_H

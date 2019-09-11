#ifndef CUSTOMCOMBOBOX_H
#define CUSTOMCOMBOBOX_H

#include <QWidget>
#include <QComboBox>

#include "support-customcombobox/comboboxitem.h"
#include "support-customcombobox/nofocusframedelegate.h"

class QListWidget;
class QListWidgetItem;

class CustomComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit CustomComboBox(QWidget *parent = 0);
    ~CustomComboBox();

//    void setCurrentItem(QString text);
    void addwidgetItem(QString text);
    void setcurrentwidgetIndex(int index);

private:
//    QString currentitemtext;

    QListWidget * partListWidget;

public slots:
//    void currentItemChanged(QListWidgetItem * current, QListWidgetItem * previous);
    void onChooseItem(QString text);

};

#endif // CUSTOMCOMBOBOX_H

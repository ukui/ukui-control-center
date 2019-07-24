#include "custdomcombobox.h"
#include <QDebug>

CustdomComboBox::CustdomComboBox(QWidget *parent) :
    QComboBox(parent)
{
    QSize size(422, 62);
    this->setMinimumSize(size);
    this->setMaximumSize(size);
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(itemchangedSlot(int)));
}

CustdomComboBox::~CustdomComboBox()
{
}

void CustdomComboBox::itemchangedSlot(int itemindex){
    if (CustdomComboBox::currentText() == "add")
        qDebug() << "---------------->";

    emit itemchangedSignal(itemindex);
}

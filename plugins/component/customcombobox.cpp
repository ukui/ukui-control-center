#include "customcombobox.h"

#include <QListWidget>
#include <QListWidgetItem>

#include <QDebug>


CustomComboBox::CustomComboBox(QWidget *parent) :
    QComboBox(parent)
{
    this->setStyleSheet("QComboBox{border: 1px solid #cccccc; padding: 1px 2px 1px 2px; background-color: #eeeeee;}"
                        "QComboBox QAbstractItemView::item{height: 30px}"
                        "QListView::item{background: white}"
                        "QListView::item:hover{background: #BDD7FD}");

    partListWidget = new QListWidget(this);

    partListWidget->setItemDelegate(new NoFocusFrameDelegate(this));

    this->setModel(partListWidget->model());
    this->setView(partListWidget);
    this->setEditable(true);
}

CustomComboBox::~CustomComboBox()
{
}

void CustomComboBox::onChooseItem(QString text){
    this->setEditText(text);
    this->hidePopup();
}

void CustomComboBox::addwidgetItem(QString text){

    ComboboxItem * item = new ComboboxItem(this);
    item->setLabelContent(text);
    connect(item, SIGNAL(chooseItem(QString)), this, SLOT(onChooseItem(QString)));
    QListWidgetItem * widgetItem = new QListWidgetItem(partListWidget);
    partListWidget->setItemWidget(widgetItem, item);
}

void CustomComboBox::setcurrentwidgetIndex(int index){
    QListWidgetItem * item = partListWidget->takeItem(index);
    this->setEditText(item->text());
}


#include "listwidgetitem.h"

ListWidgetItem::ListWidgetItem(QWidget *parent) :
    QWidget(parent)
{
//    this->setFixedSize(QSize(198,50));
    this->setStyleSheet("background: none;");

    iconLabel = new QLabel(this);
    QSizePolicy policy = iconLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Fixed);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    iconLabel->setSizePolicy(policy);

    textLabel = new QLabel(this);
    QSizePolicy policy1 = textLabel->sizePolicy();
    policy1.setHorizontalPolicy(QSizePolicy::Fixed);
    policy1.setVerticalPolicy(QSizePolicy::Fixed);
    textLabel->setSizePolicy(policy1);
    textLabel->setScaledContents(true);

    QHBoxLayout * mainlayout = new QHBoxLayout(this);
    mainlayout->addWidget(iconLabel);
    mainlayout->addWidget(textLabel);
    mainlayout->addStretch();
    mainlayout->setSpacing(10);
    mainlayout->setContentsMargins(20, 0, 0, 0);

    setLayout(mainlayout);
}

ListWidgetItem::~ListWidgetItem()
{
}

void ListWidgetItem::setLabelPixmap(QString filename){
    iconLabel->setPixmap(QPixmap(filename));
}

void ListWidgetItem::setLabelText(QString text){
    textLabel->setText(text);
}

QString ListWidgetItem::text(){
    return textLabel->text();
}

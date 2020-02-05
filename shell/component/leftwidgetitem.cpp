#include "leftwidgetitem.h"

LeftWidgetItem::LeftWidgetItem(QWidget *parent) :
    QWidget(parent)
{
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
    mainlayout->setSpacing(0);
    mainlayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainlayout);
}

LeftWidgetItem::~LeftWidgetItem()
{
}

void LeftWidgetItem::setLabelPixmap(QString filename){
    iconLabel->setPixmap(QPixmap(filename));
}

void LeftWidgetItem::setLabelText(QString text){
    textLabel->setText(text);
}

QString LeftWidgetItem::text(){
    return textLabel->text();
}

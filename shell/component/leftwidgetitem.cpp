#include "leftwidgetitem.h"

#include <QPainter>
#include <QStyleOption>

LeftWidgetItem::LeftWidgetItem(QWidget *parent) :
    QWidget(parent)
{
//    this->setStyleSheet("background: none;");
    widget = new QWidget(this);
    widget->setFixedSize(120, 40);

    iconLabel = new QLabel(widget);
    QSizePolicy policy = iconLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Fixed);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    iconLabel->setSizePolicy(policy);
    iconLabel->setFixedSize(24, 24);

    textLabel = new QLabel(widget);
    QSizePolicy policy1 = textLabel->sizePolicy();
    policy1.setHorizontalPolicy(QSizePolicy::Fixed);
    policy1.setVerticalPolicy(QSizePolicy::Fixed);
    textLabel->setSizePolicy(policy1);
    textLabel->setScaledContents(true);


    QHBoxLayout * mainlayout = new QHBoxLayout(widget);
    mainlayout->setSpacing(8);
    mainlayout->setContentsMargins(8, 0, 0, 0);
    mainlayout->addWidget(iconLabel, Qt::AlignVCenter);
    mainlayout->addWidget(textLabel, Qt::AlignVCenter);
    mainlayout->addStretch();

    widget->setLayout(mainlayout);


    QVBoxLayout * baseVerLayout = new QVBoxLayout(this);
    baseVerLayout->setSpacing(0);
    baseVerLayout->setMargin(0);

    baseVerLayout->addWidget(widget);
    baseVerLayout->addStretch();

    setLayout(baseVerLayout);
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

void LeftWidgetItem::setSelected(bool selected){
    if (selected)
        widget->setStyleSheet("QWidget{background: #3D6BE5; border-radius: 4px;}");
    else
        widget->setStyleSheet("QWidget{background: none;}");
}

QString LeftWidgetItem::text(){
    return textLabel->text();
}

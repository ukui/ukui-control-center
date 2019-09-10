#include "comboboxitem.h"

ComboboxItem::ComboboxItem(QWidget *parent) :
    QWidget(parent)
{
    mpress = false;

    imgLabel = new QLabel(this);
    textLabel = new QLabel(this);

    mainlayout = new QHBoxLayout(this);
    mainlayout->addWidget(imgLabel);
//    mainlayout->addStretch();
    mainlayout->addWidget(textLabel);
    mainlayout->addStretch();

    mainlayout->setSpacing(5);
    mainlayout->setContentsMargins(5, 5, 5, 5);

    setLayout(mainlayout);
}

ComboboxItem::~ComboboxItem()
{
}

void ComboboxItem::setLabelContent(QString content){
    textLabel->setText(content);
}

QString ComboboxItem::getLabelContent(){
    return textLabel->text();
}

void ComboboxItem::mousePressEvent(QMouseEvent * event){
    if (event->button() == Qt::LeftButton){
        mpress = true;
//        emit chooseItem(textLabel->text());
    }
//    QWidget::mousePressEvent(event);
}

void ComboboxItem::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        ;//un used warning
    }
    if (mpress){
        emit chooseItem(textLabel->text());
        mpress = false;
    }
}

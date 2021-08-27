#include "namelabel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>

NameLabel::NameLabel(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *nameLayout = new QHBoxLayout(this);

    nameLabel = new QLabel(this);
    QSizePolicy nameSizePolicy = nameLabel->sizePolicy();
    nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    nameLabel->setSizePolicy(nameSizePolicy);

    changeNameLabel = new QLabel(this);
    changeNameLabel->setMinimumSize(QSize(15,22));
    changeNameLabel->setMaximumSize(QSize(15,22));
    changeNameLabel->setPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(changeNameLabel->size()));
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(changeNameLabel);
}

void NameLabel::mouseReleaseEvent(QMouseEvent * ev){
   emit clicked();
}

void NameLabel::setText(QString text)
{
    nameLabel->setText(text);
}

#include "themewidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

ThemeWidget::ThemeWidget(QSize iSize, QString name, QStringList iStringList, QWidget *parent) :
    QWidget(parent)
{
    ////创建图标主题
    //构建基础Widget为了达到中间8px的空白效果
    QWidget * baseWidget = new QWidget(this);
//    baseWidget->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout * baseVerLayout = new QVBoxLayout(baseWidget);
    baseVerLayout->setSpacing(0);
    baseVerLayout->setContentsMargins(0, 0, 0, 8);

    //构建图标主题Widget
    QWidget * widget = new QWidget;
    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->setFixedHeight(66);
    widget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");

    QHBoxLayout * mainHorLayout = new QHBoxLayout(widget);
    mainHorLayout->setSpacing(16);
    mainHorLayout->setContentsMargins(16, 9, 16, 9);

    //占位Label，解决隐藏选中图标后文字Label位置的变化
    placeHolderLabel = new QLabel(widget);
    QSizePolicy phSizePolicy = placeHolderLabel->sizePolicy();
    phSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    phSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    placeHolderLabel->setSizePolicy(phSizePolicy);
    placeHolderLabel->setFixedSize(QSize(16, 16)); //选中图标的大小为16*16

    selectedLabel = new QLabel(widget);
    QSizePolicy sSizePolicy = selectedLabel->sizePolicy();
    sSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    sSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    selectedLabel->setSizePolicy(sSizePolicy);
    selectedLabel->setScaledContents(true);
    selectedLabel->setPixmap(QPixmap("://img/plugins/theme/selected.png"));

    QLabel * nameLabel = new QLabel(widget);
    QSizePolicy nameSizePolicy = nameLabel->sizePolicy();
    nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    nameLabel->setSizePolicy(nameSizePolicy);
    nameLabel->setFixedWidth(102);
    nameLabel->setText(name);

    QHBoxLayout * iconHorLayout = new QHBoxLayout;
    iconHorLayout->setSpacing(16);
    iconHorLayout->setMargin(0);
    for (QString icon : iStringList){
        QLabel * label = new QLabel(widget);
        label->setFixedSize(iSize);
        label->setPixmap(QPixmap(icon));
        iconHorLayout->addWidget(label);
    }

    mainHorLayout->addWidget(placeHolderLabel);
    mainHorLayout->addWidget(selectedLabel);
    mainHorLayout->addWidget(nameLabel);
    mainHorLayout->addLayout(iconHorLayout);
    mainHorLayout->addStretch();

    widget->setLayout(mainHorLayout);

    baseVerLayout->addWidget(widget);
    baseVerLayout->addStretch();

    baseWidget->setLayout(baseVerLayout);

}

ThemeWidget::~ThemeWidget()
{
}

void ThemeWidget::setSelectedStatus(bool status){
    placeHolderLabel->setHidden(status);
    selectedLabel->setVisible(status);
}

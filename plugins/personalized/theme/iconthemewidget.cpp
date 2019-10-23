#include "iconthemewidget.h"

#define LINENUM 8

IconThemeWidget::IconThemeWidget()
{
    themeFullName = "";
    this->setProperty("selected", QVariant(false));

//    this->setFixedHeight(139);
    this->setObjectName("themeFrame");
    this->setStyleSheet("QFrame#themeFrame{background-color: #ffffff;}"
                        "QFrame#themeFrame[selected=false]{border: 1px solid #cccccc;}"
                        "QFrame#themeFrame[selected=true]{border: 5px solid #daebff;}");

    flHBoxLayout = new QHBoxLayout();
    flHBoxLayout->setContentsMargins(0, 0, 0, 0);
    flHBoxLayout->setSpacing(25);
    slHBoxLayout = new QHBoxLayout();
    slHBoxLayout->setContentsMargins(0, 0, 0, 0);
    slHBoxLayout->setSpacing(25);
    leftVBoxLayout = new QVBoxLayout();
    leftVBoxLayout->setContentsMargins(0, 0, 0, 0);
    leftVBoxLayout->setSpacing(20);
    leftVBoxLayout->addLayout(flHBoxLayout);
    leftVBoxLayout->addLayout(slHBoxLayout);

    rightVBoxLayout = new QVBoxLayout();
    rightVBoxLayout->setContentsMargins(0, 0, 0, 0);
    rightVBoxLayout->setSpacing(25);
    QHBoxLayout * selectHBoxLayout = new QHBoxLayout();
    selectHBoxLayout->setContentsMargins(0, 0, 0, 0);
    selectHBoxLayout->setSpacing(10);

    themeName = new QLabel();
    QSizePolicy policy = themeName->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Fixed);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    themeName->setSizePolicy(policy);
    themeName->setFixedWidth(90);

    themeStatus = new QLabel();
    QSizePolicy policy1 = themeStatus->sizePolicy();
    policy1.setHorizontalPolicy(QSizePolicy::Fixed);
    policy1.setHorizontalPolicy(QSizePolicy::Fixed);
    themeStatus->setSizePolicy(policy1);

    selectedIcon = new QLabel();
    selectedIcon->setFixedSize(20, 20);
    selectedIcon->setPixmap(QPixmap("://theme/itselected.png"));

    selectHBoxLayout->addWidget(themeStatus);
    selectHBoxLayout->addWidget(selectedIcon);
    selectHBoxLayout->addStretch();

    rightVBoxLayout->addStretch();
    rightVBoxLayout->addWidget(themeName);
    rightVBoxLayout->addLayout(selectHBoxLayout);
    rightVBoxLayout->addStretch();

    mainHBoxLayout = new QHBoxLayout(this);
    mainHBoxLayout->setContentsMargins(20, 20, 20, 20);
    mainHBoxLayout->setSpacing(28);

    QFrame * line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);

    mainHBoxLayout->addLayout(leftVBoxLayout);
    mainHBoxLayout->addWidget(line);
    mainHBoxLayout->addLayout(rightVBoxLayout);

    this->setLayout(mainHBoxLayout);
}

IconThemeWidget::~IconThemeWidget()
{
}

void IconThemeWidget::set_icontheme_name(QString name){
    themeName->setText(name);
}

void IconThemeWidget::set_icontheme_selected(bool status){
    if (status){
        themeStatus->setText(tr("selected"));
        selectedIcon->show();
    }
    else{
        themeStatus->setText(tr("not selected"));
        selectedIcon->hide();
    }
    this->setProperty("selected", QVariant(status));
}

void IconThemeWidget::set_icontheme_example(QStringList firsticonList, QStringList secondiconList){
    for (QString icon : firsticonList){
        QLabel * label = new QLabel();
        label->setFixedSize(48, 48);
        label->setPixmap(QPixmap(icon));
        flHBoxLayout->addWidget(label);
    }

    for (QString icon : secondiconList){
        QLabel * label = new QLabel();
        label->setFixedSize(48, 48);
        label->setPixmap(QPixmap(icon));
        slHBoxLayout->addWidget(label);
    }
}

void IconThemeWidget::set_icontheme_fullname(QString fullname){
    themeFullName = fullname;
}

QString IconThemeWidget::get_icontheme_fullname(){
    return themeFullName;
}

void IconThemeWidget::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        emit clicked(themeFullName);
    }
}

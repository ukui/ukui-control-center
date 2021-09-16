#include "changeusertype.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFrame>

#include <QDBusInterface>
#include <QDBusReply>

#include "elipsemaskwidget.h"

ChangeUserType::ChangeUserType(QString objpath, QWidget *parent) :
    QDialog(parent),
    _objpath(objpath)
{
    setFixedSize(QSize(520, 446));
    setWindowTitle(tr("UserType"));

    cutiface = new QDBusInterface("org.freedesktop.Accounts",
                                  _objpath,
                                  "org.freedesktop.Accounts.User",
                                  QDBusConnection::systemBus());

    initUI();
    setConnect();
}

ChangeUserType::~ChangeUserType()
{
    delete cutiface;
}

void ChangeUserType::initUI(){


    cutNickNameLabel = new QLabel();
    cutNickNameLabel->setFixedHeight(24);
    cutUserTypeLabel = new QLabel();
    cutUserTypeLabel->setFixedHeight(24);

    cutUserLogoBtn = new QPushButton();
    cutUserLogoBtn->setFixedSize(QSize(56, 56));
    cutUserLogoBtn->setIconSize(QSize(48, 48));

    cutUserInfoVerLayout = new QVBoxLayout;
    cutUserInfoVerLayout->setSpacing(0);
    cutUserInfoVerLayout->setContentsMargins(0, 0, 0, 0);
    cutUserInfoVerLayout->addStretch();
    cutUserInfoVerLayout->addWidget(cutNickNameLabel);
    cutUserInfoVerLayout->addWidget(cutUserTypeLabel);
    cutUserInfoVerLayout->addStretch();

    cutUserHorLayout = new QHBoxLayout;
    cutUserHorLayout->setSpacing(16);
    cutUserHorLayout->setContentsMargins(0, 0, 0, 0);
    cutUserHorLayout->addWidget(cutUserLogoBtn);
    cutUserHorLayout->addLayout(cutUserInfoVerLayout);
    cutUserHorLayout->addStretch();

    cutNoteLabel = new QLabel;
    cutNoteLabel->setFixedHeight(24);
    cutNoteLabel->setText(tr("Ensure that must have admin on system"));

    cutAdminRadioBtn = new QRadioButton;
    cutAdminRadioBtn->setFixedSize(QSize(16, 16));
    cutStandardRadioBtn = new QRadioButton;
    cutStandardRadioBtn->setFixedSize(QSize(16, 16));
    cutTypesBtnGroup = new QButtonGroup;
    cutTypesBtnGroup->addButton(cutAdminRadioBtn, 1);
    cutTypesBtnGroup->addButton(cutStandardRadioBtn, 0);
    cutAdminLabel = new QLabel;
//    cutAdminLabel->setFixedHeight(24);
    cutAdminLabel->setText(tr("administrator"));
    cutStandardLabel = new QLabel;
//    cutStandardLabel->setFixedHeight(24);
    cutStandardLabel->setText(tr("standard user"));
    cutAdminNoteLabel = new QLabel;
    cutAdminNoteLabel->setText(tr("Users can make any changes they need"));
    cutStandardNoteLabel = new QLabel;
    cutStandardNoteLabel->setText(tr("Users cannot change system settings"));


    cutAdminInfoHorLayout = new QHBoxLayout;
    cutAdminInfoHorLayout->setSpacing(9);
    cutAdminInfoHorLayout->setContentsMargins(0, 0, 0, 0);
    cutAdminInfoHorLayout->addWidget(cutAdminRadioBtn);
    cutAdminInfoHorLayout->addWidget(cutAdminLabel);
    cutAdminInfoHorLayout->addStretch();
    cutAdminInfo2HorLayout = new QHBoxLayout;
    cutAdminInfo2HorLayout->setSpacing(0);
    cutAdminInfo2HorLayout->setContentsMargins(cutAdminRadioBtn->width() + 9, 0, 0, 0);
    cutAdminInfo2HorLayout->addWidget(cutAdminNoteLabel);
    cutStandardInfoHorLayout = new QHBoxLayout;
    cutStandardInfoHorLayout->setSpacing(9);
    cutStandardInfoHorLayout->setContentsMargins(0, 0, 0, 0);
    cutStandardInfoHorLayout->addWidget(cutStandardRadioBtn);
    cutStandardInfoHorLayout->addWidget(cutStandardLabel);
    cutStandardInfoHorLayout->addStretch();
    cutStandardInfo2HorLayout = new QHBoxLayout;
    cutStandardInfo2HorLayout->setSpacing(0);
    cutStandardInfo2HorLayout->setContentsMargins(cutStandardRadioBtn->width() + 9, 0, 0, 0);
    cutStandardInfo2HorLayout->addWidget(cutStandardNoteLabel);

    cutAdminVerLayout = new QVBoxLayout;
    cutAdminVerLayout->setSpacing(9);
    cutAdminVerLayout->setContentsMargins(20, 0, 0, 0);
    cutAdminVerLayout->addLayout(cutAdminInfoHorLayout);
    cutAdminVerLayout->addLayout(cutAdminInfo2HorLayout);
    cutStandardVerLayout = new QVBoxLayout;
    cutStandardVerLayout->setSpacing(9);
    cutStandardVerLayout->setContentsMargins(20, 0, 0, 0);
    cutStandardVerLayout->addLayout(cutStandardInfoHorLayout);
    cutStandardVerLayout->addLayout(cutStandardInfo2HorLayout);

    cutAdminFrame = new QFrame;
    cutAdminFrame->setMinimumSize(QSize(550, 60));
    cutAdminFrame->setMaximumSize(QSize(16777215, 60));
    cutAdminFrame->setFrameShape(QFrame::Box);
    cutAdminFrame->setFrameShadow(QFrame::Plain);
    cutAdminFrame->setLayout(cutAdminVerLayout);
    cutStandardFrame = new QFrame;
    cutStandardFrame->setMinimumSize(QSize(550, 60));
    cutStandardFrame->setMaximumSize(QSize(16777215, 60));
    cutStandardFrame->setFrameShape(QFrame::Box);
    cutStandardFrame->setFrameShadow(QFrame::Plain);
    cutStandardFrame->setLayout(cutStandardVerLayout);

    cutConfirmBtn = new QPushButton;
    cutCancelBtn = new QPushButton;

    cutBtnGroupsHorLayout = new QHBoxLayout;
    cutBtnGroupsHorLayout->setSpacing(16);
    cutBtnGroupsHorLayout->setContentsMargins(0, 0, 0, 0);
    cutBtnGroupsHorLayout->addStretch();
    cutBtnGroupsHorLayout->addWidget(cutCancelBtn);
    cutBtnGroupsHorLayout->addWidget(cutConfirmBtn);

    cutMainVerLayout = new QVBoxLayout;
    cutMainVerLayout->setSpacing(8);
    cutMainVerLayout->setContentsMargins(25, 28, 22, 25);
    cutMainVerLayout->addLayout(cutUserHorLayout);
    cutMainVerLayout->addWidget(cutNoteLabel);
    cutMainVerLayout->addWidget(cutAdminFrame);
    cutMainVerLayout->addWidget(cutStandardFrame);
    cutMainVerLayout->addStretch();
    cutMainVerLayout->addLayout(cutBtnGroupsHorLayout);

    setLayout(cutMainVerLayout);
}

void ChangeUserType::setConnect(){
    connect(cutCancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });
    connect(cutConfirmBtn, &QPushButton::clicked, this, [=]{
        cutiface->call("SetAccountType", cutTypesBtnGroup->checkedId());

        close();
    });
}

void ChangeUserType::requireUserInfo(QString logo, QString nname, int id, QString utype){

    //设置头像
    cutUserLogoBtn->setIcon(QIcon(logo));
    ////圆形头像
    ElipseMaskWidget * currentElipseMaskWidget = new ElipseMaskWidget(cutUserLogoBtn);
    currentElipseMaskWidget->setGeometry(0, 0, cutUserLogoBtn->width(), cutUserLogoBtn->height());

    //设置昵称
    if (setTextDynamic(cutNickNameLabel, nname)){
        cutNickNameLabel->setToolTip(nname);
    }

    //
    cutUserTypeLabel->setText(utype);

    //设置类型
    cutTypesBtnGroup->blockSignals(true);
    if (id >= 0 && id < cutTypesBtnGroup->buttons().length()){
        cutTypesBtnGroup->button(id)->setChecked(true);
    }
    cutTypesBtnGroup->blockSignals(false);
}

bool ChangeUserType::setTextDynamic(QLabel *label, QString string){

    bool isOverLength = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);

    QString str = string;
    if (fontSize > 80) {
        label->setFixedWidth(80);
        str = fontMetrics.elidedText(string, Qt::ElideRight, 80);
        isOverLength = true;
    } else {
        label->setFixedWidth(fontSize);
    }
    label->setText(str);
    return isOverLength;

}

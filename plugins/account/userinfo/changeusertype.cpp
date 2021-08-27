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

    cutAdminRadioBtn = new QRadioButton;
    cutStandardRadioBtn = new QRadioButton;
    cutTypesBtnGroup = new QButtonGroup;
    cutTypesBtnGroup->addButton(cutAdminRadioBtn, 1);
    cutTypesBtnGroup->addButton(cutStandardRadioBtn, 0);
    cutAdminLabel = new QLabel;
    cutStandardLabel = new QLabel;
    cutAdminNoteLabel = new QLabel;
    cutStandardNoteLabel = new QLabel;


    cutAdminInfoVerLayout = new QVBoxLayout;
    cutAdminInfoVerLayout->setSpacing(0);
    cutAdminInfoVerLayout->setContentsMargins(0, 0, 0, 0);
    cutAdminInfoVerLayout->addWidget(cutAdminRadioBtn);
    cutAdminInfoVerLayout->addStretch();
    cutAdminInfo2VerLayout = new QVBoxLayout;
    cutAdminInfo2VerLayout->setSpacing(4);
    cutAdminInfo2VerLayout->setContentsMargins(0, 0, 0, 0);
    cutAdminInfo2VerLayout->addWidget(cutAdminLabel);
    cutAdminInfo2VerLayout->addWidget(cutAdminNoteLabel);
    cutStandardInfoVerLayout = new QVBoxLayout;
    cutStandardInfoVerLayout->setSpacing(0);
    cutStandardInfoVerLayout->setContentsMargins(0, 0, 0, 0);
    cutStandardInfoVerLayout->addWidget(cutStandardRadioBtn);
    cutStandardInfoVerLayout->addStretch();
    cutStandardInfo2VerLayout = new QVBoxLayout;
    cutStandardInfo2VerLayout->setSpacing(4);
    cutStandardInfo2VerLayout->setContentsMargins(0, 0, 0, 0);
    cutStandardInfo2VerLayout->addWidget(cutStandardLabel);
    cutStandardInfo2VerLayout->addWidget(cutStandardNoteLabel);

    cutAdminHorLayout = new QHBoxLayout;
    cutAdminHorLayout->setSpacing(4);
    cutAdminHorLayout->setContentsMargins(20, 0, 0, 0);
    cutAdminHorLayout->addLayout(cutAdminInfoVerLayout);
    cutStandardHorLayout = new QHBoxLayout;
    cutStandardHorLayout->setSpacing(4);
    cutStandardHorLayout->setContentsMargins(20, 0, 0, 0);
    cutStandardHorLayout->addLayout(cutStandardInfoVerLayout);

    cutAdminFrame = new QFrame;
    cutAdminFrame->setMinimumSize(QSize(550, 60));
    cutAdminFrame->setMaximumSize(QSize(16777215, 60));
    cutAdminFrame->setFrameShape(QFrame::Box);
    cutAdminFrame->setFrameShadow(QFrame::Plain);
    cutAdminFrame->setLayout(cutAdminHorLayout);
    cutStandardFrame = new QFrame;
    cutStandardFrame->setMinimumSize(QSize(550, 60));
    cutStandardFrame->setMaximumSize(QSize(16777215, 60));
    cutStandardFrame->setFrameShape(QFrame::Box);
    cutStandardFrame->setFrameShadow(QFrame::Plain);
    cutStandardFrame->setLayout(cutStandardHorLayout);

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
    cutMainVerLayout->setContentsMargins(25, 58, 22, 25);
    cutMainVerLayout->addLayout(cutUserHorLayout);
    cutMainVerLayout->addWidget(cutNoteLabel);
    cutMainVerLayout->addWidget(cutAdminFrame);
    cutMainVerLayout->addWidget(cutStandardFrame);
    cutMainVerLayout->addLayout(cutBtnGroupsHorLayout);

    setLayout(cutMainVerLayout);
}

void ChangeUserType::setConnect(){
    connect(cutCancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });
    connect(cutConfirmBtn, &QPushButton::clicked, this, [=]{
//        cutiface->call("SetAccountType", cutTypesBtnGroup->checkedId());
    });
}

void ChangeUserType::requireUserInfo(QString logo, QString nname, QString utype){

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
}

bool ChangeUserType::setTextDynamic(QLabel *label, QString string){

    bool isOverLength = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);

    QString str = string;
    if (fontSize > 100) {
        label->setFixedWidth(100);
        str = fontMetrics.elidedText(string, Qt::ElideRight, 100);
        isOverLength = true;
    } else {
        label->setFixedWidth(fontSize);
    }
    label->setText(str);
    return isOverLength;

}

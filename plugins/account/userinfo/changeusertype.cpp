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

#include <QMouseEvent>

#include "elipsemaskwidget.h"
#include "../../../shell/utils/utils.h"

#include <QDebug>

ChangeUserType::ChangeUserType(QString objpath, QWidget *parent) :
    QDialog(parent),
    _objpath(objpath)
{
    setMinimumSize(QSize(520, 390));
    setWindowTitle(tr("UserType"));

    if (Utils::isCommunity()) {
        cutiface = new QDBusInterface("com.control.center.qt.systemdbus",
                                      "/",
                                      "com.control.center.interface",
                                      QDBusConnection::systemBus());
    } else {
        cutiface = new QDBusInterface("org.freedesktop.Accounts",
                                      _objpath,
                                      "org.freedesktop.Accounts.User",
                                      QDBusConnection::systemBus());
    }

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
    cutUserTypeLabel = new LightLabel();
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
    cutNoteLabel->setText(tr("Select account type (Ensure have admin on system):"));

    cutAdminRadioBtn = new QRadioButton;
    cutAdminRadioBtn->setFixedSize(QSize(16, 16));
    cutStandardRadioBtn = new QRadioButton;
    cutStandardRadioBtn->setFixedSize(QSize(16, 16));
    cutTypesBtnGroup = new QButtonGroup;
    cutTypesBtnGroup->addButton(cutAdminRadioBtn, 1);
    cutTypesBtnGroup->addButton(cutStandardRadioBtn, 0);
    cutAdminLabel = new QLabel;
    cutAdminLabel->setText(tr("administrator"));
    cutStandardLabel = new QLabel;
    cutStandardLabel->setText(tr("standard user"));
    cutAdminNoteLabel = new LightLabel;
    cutAdminNoteLabel->setText(tr("change system settings, install and upgrade software."));
    cutStandardNoteLabel = new LightLabel;
    cutStandardNoteLabel->setText(tr("use most software, cannot change system settings."));


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
    cutAdminVerLayout->setSpacing(5);
    cutAdminVerLayout->setContentsMargins(20, 0, 0, 0);
    cutAdminVerLayout->addLayout(cutAdminInfoHorLayout);
    cutAdminVerLayout->addLayout(cutAdminInfo2HorLayout);
    cutAdminVerLayout->addStretch();
    cutStandardVerLayout = new QVBoxLayout;
    cutStandardVerLayout->setSpacing(5);
    cutStandardVerLayout->setContentsMargins(20, 0, 0, 0);
    cutStandardVerLayout->addLayout(cutStandardInfoHorLayout);
    cutStandardVerLayout->addLayout(cutStandardInfo2HorLayout);
    cutStandardVerLayout->addStretch();

    cutAdminFrame = new QFrame;
    cutAdminFrame->setMinimumSize(QSize(473, 80));
    cutAdminFrame->setMaximumSize(QSize(16777215, 80));
    cutAdminFrame->setFrameShape(QFrame::Box);
    cutAdminFrame->setFrameShadow(QFrame::Plain);
    cutAdminFrame->setLayout(cutAdminVerLayout);
    cutAdminFrame->installEventFilter(this);
    cutStandardFrame = new QFrame;
    cutStandardFrame->setMinimumSize(QSize(473, 80));
    cutStandardFrame->setMaximumSize(QSize(16777215, 80));
    cutStandardFrame->setFrameShape(QFrame::Box);
    cutStandardFrame->setFrameShadow(QFrame::Plain);
    cutStandardFrame->setLayout(cutStandardVerLayout);
    cutStandardFrame->installEventFilter(this);

    tipLabel = new QLabel;
    tipLabel->setText(tr("Note: Effective After Logout!!!"));
    tipLabel->setStyleSheet("color:red;");
    tipLabel->hide();

    tipHorLayout = new QHBoxLayout;
    tipHorLayout->setSpacing(0);
    tipHorLayout->setContentsMargins(0, 0, 0, 0);
    tipHorLayout->addStretch();
    tipHorLayout->addWidget(tipLabel);

    cutConfirmBtn = new QPushButton;
    cutConfirmBtn->setText(tr("Confirm"));
    cutConfirmBtn->setEnabled(false);
    cutCancelBtn = new QPushButton;
    cutCancelBtn->setText(tr("Cancel"));

    cutBtnGroupsHorLayout = new QHBoxLayout;
    cutBtnGroupsHorLayout->setSpacing(16);
    cutBtnGroupsHorLayout->setContentsMargins(0, 0, 0, 0);
    cutBtnGroupsHorLayout->addStretch();
    cutBtnGroupsHorLayout->addWidget(cutCancelBtn);
    cutBtnGroupsHorLayout->addWidget(cutConfirmBtn);

    cutMainVerLayout = new QVBoxLayout;
    cutMainVerLayout->setSpacing(0);
    cutMainVerLayout->setContentsMargins(25, 24, 22, 25);
    cutMainVerLayout->addLayout(cutUserHorLayout);
    cutMainVerLayout->addSpacing(35);
    cutMainVerLayout->addWidget(cutNoteLabel);
    cutMainVerLayout->addSpacing(16);
    cutMainVerLayout->addWidget(cutAdminFrame);
    cutMainVerLayout->addSpacing(0);
    cutMainVerLayout->addWidget(cutStandardFrame);
    cutMainVerLayout->addSpacing(8);
    cutMainVerLayout->addLayout(tipHorLayout);
    cutMainVerLayout->addStretch();
    cutMainVerLayout->addLayout(cutBtnGroupsHorLayout);

    setLayout(cutMainVerLayout);
}

void ChangeUserType::setConnect(){
    connect(cutCancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });
    connect(cutConfirmBtn, &QPushButton::clicked, this, [=]{
        if (Utils::isCommunity()) {
            cutiface->call("SetAccountType", _objpath, cutTypesBtnGroup->checkedId());
        } else {
            cutiface->call("SetAccountType", cutTypesBtnGroup->checkedId());
        }

        accept();
    });

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(cutTypesBtnGroup), static_cast<void (QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled), [=](int id, bool status){
#else
    connect(cutTypesBtnGroup, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), [=](int id, bool status){
#endif
        if (id != oldid){
            if (status){
                cutConfirmBtn->setEnabled(true);
            } else {
                cutConfirmBtn->setEnabled(false);
            }
        }
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
        //记录原始状态
        oldid = id;
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

bool ChangeUserType::eventFilter(QObject *watched, QEvent *event){
    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton ){
            if (watched == cutStandardFrame){
                cutStandardRadioBtn->setChecked(true);
            } else if (watched == cutAdminFrame){
                cutAdminRadioBtn->setChecked(true);
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

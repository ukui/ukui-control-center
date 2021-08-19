#include "inputpwddialog.h"

#include <QDebug>
#include <QLineEdit>
#include <QSpacerItem>
#include <QLabel>
#include <QPushButton>
#include "sharemain.h"
#include "Label/fixlabel.h"


InputPwdDialog::InputPwdDialog(QGSettings *Keygsettiings,QWidget *parent) :
    QDialog(parent),
    mgsettings(Keygsettiings)
{

    setupInit();
    initConnect();
}

InputPwdDialog::~InputPwdDialog()
{

}

void InputPwdDialog::setupInit()
{
    setWindowTitle(tr("Set"));
    this->setFixedSize(480, 160);

    QVBoxLayout *mInputPwdLyt = new QVBoxLayout(this);
    mInputPwdLyt->setContentsMargins(24, 24, 24, 24);
    mInputPwdLyt->setSpacing(8);

    QFrame *mInputPwdFrame = new QFrame(this);
    mInputPwdFrame->setFixedSize(432, 36);
    mInputPwdFrame->setFrameShape(QFrame::Box);

    QHBoxLayout *mLyt_1= new QHBoxLayout(mInputPwdFrame);
    mLyt_1->setContentsMargins(0, 0, 0, 0);
    mLyt_1->setSpacing(8);

    FixLabel *mSetPwdLabel = new FixLabel(mInputPwdFrame);
    mSetPwdLabel->setFixedSize(72, 36);
    mSetPwdLabel->setText(tr("Set Password"));

    mpwd = new QLineEdit(mInputPwdFrame);
    mpwd->setFixedSize(352, 36);
    mpwd->installEventFilter(this);

    mLyt_1->addWidget(mSetPwdLabel);
    mLyt_1->addWidget(mpwd);

    mfirstload = true;
    mstatus = false;

    mHintLabel = new QLabel(this);
    mHintLabel->setFixedSize(432,36);
    mHintLabel->setContentsMargins(80, 0, 0, 0);
    mHintLabel->setStyleSheet("color:red;");

    QFrame *mInputPwdFrame_1 = new QFrame(this);
    mInputPwdFrame_1->setFixedSize(432, 36);
    mInputPwdFrame_1->setFrameShape(QFrame::Box);

    QHBoxLayout *mLyt_2= new QHBoxLayout(mInputPwdFrame_1);
    mLyt_2->setContentsMargins(0, 0, 0, 0);
    mLyt_2->setSpacing(16);


    mLyt_1->addWidget(mSetPwdLabel);
    mLyt_1->addWidget(mpwd);

    mCancelBtn = new QPushButton(mInputPwdFrame_1);
    mCancelBtn->setFixedSize(96, 36);
    mCancelBtn->setText(tr("Cancel"));

    mConfirmBtn = new QPushButton(mInputPwdFrame_1);
    mConfirmBtn->setFixedSize(96, 36);
    mConfirmBtn->setText(tr("Confirm"));

    mLyt_2->addStretch();
    mLyt_2->addWidget(mCancelBtn);
    mLyt_2->addWidget(mConfirmBtn);

    mInputPwdLyt->addWidget(mInputPwdFrame);
    mInputPwdLyt->addWidget(mHintLabel);
    mInputPwdLyt->addStretch();
    mInputPwdLyt->addWidget(mInputPwdFrame_1);


    if(QByteArray::fromBase64(mgsettings->get(kVncPwdKey).toString().toLatin1()).length() == 8) {
        mpwd->setText(QByteArray::fromBase64(mgsettings->get(kVncPwdKey).toString().toLatin1()));
        mHintLabel->setText(tr("less than or equal to 8"));
        mHintLabel->setVisible(true);
    } else if (mgsettings->get(kVncPwdKey).toString() == "keyring") {
        mpwd->setText("");
        mConfirmBtn->setEnabled(false);
        mHintLabel->setText(tr("Password can not be blank"));
        mHintLabel->setVisible(true);
    } else {
        mpwd->setText(QByteArray::fromBase64(mgsettings->get(kVncPwdKey).toString().toLatin1()));
    }

}

void InputPwdDialog::mpwdInputSlot(const QString &pwd)
{
    Q_UNUSED(pwd);
    mstatus = true;
    mConfirmBtn->setEnabled(true);
    if (pwd.length() <= 7 && !pwd.isEmpty()) {
        QByteArray text = pwd.toLocal8Bit();
        secPwd = text.toBase64();
        mHintLabel->setText("");
    } else if (pwd.isEmpty()) {
        mConfirmBtn->setEnabled(false);
        mHintLabel->setText(tr("Password can not be blank"));
        mHintLabel->setStyleSheet("color:red;");
        secPwd = NULL;
    } else {
        mHintLabel->setText(tr("less than or equal to 8"));
        mHintLabel->setStyleSheet("color:red;");
        mpwd->setText(pwd.mid(0, 8));
        QByteArray text = pwd.mid(0, 8).toLocal8Bit();
        secPwd = text.toBase64();
    }
}

void InputPwdDialog::initConnect() {

    connect(mCancelBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        this->close();
    });

    connect(mConfirmBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        if (mstatus && secPwd.length() == 0) {
            return;
        } else if (!mstatus){
            mgsettings->set(kAuthenticationKey, "vnc");
            this->close();
        } else {
            mgsettings->set(kVncPwdKey, secPwd);
            mgsettings->set(kAuthenticationKey, "vnc");
            this->close();
        }
    });
    connect(mpwd, &QLineEdit::textChanged, this, &InputPwdDialog::mpwdInputSlot);
}

bool InputPwdDialog::eventFilter(QObject *wcg, QEvent *event)
{
    //过滤
       if(wcg==mpwd){
           if(event->type() == QEvent::MouseButtonPress){
               if(mpwd->hasFocus()){
                   if (mfirstload) {
                       mpwd->setText("");
                       mfirstload = false;
                   }
               }
           }
       }
       return QWidget::eventFilter(wcg,event);
}

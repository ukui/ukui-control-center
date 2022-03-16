#include "inputpwddialog.h"

#include <QDebug>
#include <QLineEdit>
#include <QSpacerItem>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include "sharemain.h"
#include <ukcc/widgets/fixlabel.h>


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
    mInputPwdLyt->setSpacing(0);

    QFrame *mInputPwdFrame = new QFrame(this);
    mInputPwdFrame->setFixedSize(432, 36);
    mInputPwdFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mLyt_1= new QHBoxLayout(mInputPwdFrame);
    mLyt_1->setContentsMargins(0, 0, 0, 0);
    mLyt_1->setSpacing(8);

    FixLabel *mSetPwdLabel = new FixLabel(mInputPwdFrame);
    mSetPwdLabel->setFixedSize(72, 36);
    mSetPwdLabel->setText(tr("Set Password"));

    mpwd = new QLineEdit(mInputPwdFrame);
    mpwd->setAttribute(Qt::WA_InputMethodEnabled, false);  //限制中文输入法
    mpwd->setFixedSize(352, 36);
    mpwd->installEventFilter(this);

    mLyt_1->addWidget(mSetPwdLabel);
    mLyt_1->addWidget(mpwd);

    mfirstload = true;
    mstatus = false;

    mHintLabel = new QLabel(this);
    mHintLabel->setFixedSize(432,24);
    mHintLabel->setContentsMargins(84, 0, 0, 0);
    mHintLabel->setText(tr("Must be 1-8 characters long"));
    mHintLabel->setStyleSheet("QLabel{color:red; font-size : 14px}");

    QFrame *mInputPwdFrame_1 = new QFrame(this);
    mInputPwdFrame_1->setFixedSize(432, 48);
    mInputPwdFrame_1->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mLyt_2= new QHBoxLayout(mInputPwdFrame_1);
    mLyt_2->setContentsMargins(0, 0, 0, 0);
    mLyt_2->setSpacing(16);


    mLyt_1->addWidget(mSetPwdLabel);
    mLyt_1->addWidget(mpwd);

    mCancelBtn = new QPushButton(mInputPwdFrame_1);
    mCancelBtn->setFixedWidth(96);
    mCancelBtn->setText(tr("Cancel"));

    mConfirmBtn = new QPushButton(mInputPwdFrame_1);
    mConfirmBtn->setFixedWidth(96);
    mConfirmBtn->setText(tr("Confirm"));

    mLyt_2->addStretch();
    mLyt_2->addWidget(mCancelBtn);
    mLyt_2->addWidget(mConfirmBtn);

    mInputPwdLyt->addWidget(mInputPwdFrame);
    mInputPwdLyt->addWidget(mHintLabel);
    mInputPwdLyt->addStretch();
    mInputPwdLyt->addWidget(mInputPwdFrame_1);


    if(QByteArray::fromBase64(mgsettings->get(kVncPwdKey).toString().toLatin1()).length() <= 8) {
        if (mgsettings->get(kVncPwdKey).toString() == "keyring") {
            mpwd->setText("");
            mConfirmBtn->setEnabled(false);
            mHintLabel->setVisible(true);
        } else {
            mpwd->setText(QByteArray::fromBase64(mgsettings->get(kVncPwdKey).toString().toLatin1()));
            mHintLabel->setVisible(false);
        }
    }

}

void InputPwdDialog::mpwdInputSlot(const QString &pwd)
{
    Q_UNUSED(pwd);
    mstatus = true;
    mConfirmBtn->setEnabled(true);
    if (pwd.length() <= 8 && !pwd.isEmpty()) {
        QByteArray text = pwd.toLocal8Bit();
        secPwd = text.toBase64();
        mHintLabel->setVisible(false);
    } else if (pwd.isEmpty()) {
        mConfirmBtn->setEnabled(false);
        mHintLabel->setVisible(true);
        secPwd = NULL;
    } else {
        mpwd->setText(pwd.mid(0, 8));
        QByteArray text = pwd.mid(0, 8).toLocal8Bit();
        mHintLabel->setVisible(true);
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
    //使用textEdited信号是为了防止密码框setText时触发信号
    connect(mpwd, &QLineEdit::textEdited, this, &InputPwdDialog::mpwdInputSlot);
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
       // 回车键触发确定按钮点击事件
       if (event->type() == QEvent::KeyPress) {
           QKeyEvent *mEvent = static_cast<QKeyEvent *>(event);
           if (mEvent->key() == Qt::Key_Enter || mEvent->key() == Qt::Key_Return) {
               emit mConfirmBtn->clicked();
           }

       }
       return QWidget::eventFilter(wcg,event);
}

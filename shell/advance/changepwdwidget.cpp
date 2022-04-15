#include "changepwdwidget.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QRegExpValidator>
#include <QDBusReply>
#include <QKeyEvent>

ChangePwdWidget::ChangePwdWidget(QWidget *parent) : QWidget(parent)
{
    initUi();
    setConnect();
}

void ChangePwdWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        emit mConfirmBtn->click();
        return;
    }
    return QWidget::keyPressEvent(event);
}

void ChangePwdWidget::initUi()
{
    setMinimumSize(440, 400);
    QVBoxLayout *Lyt = new QVBoxLayout(this);
    Lyt->setContentsMargins(0, 0, 0, 24);

    QLabel *titlelabel = new QLabel(tr("Change password"));

    QFrame *originalframe = new QFrame(this);
    QHBoxLayout *Lyt_1 = new QHBoxLayout(originalframe);
    Lyt_1->setContentsMargins(0, 0, 0, 0);

    QLabel *originallabel = new QLabel(tr("Orignal password"), originalframe);
    originallabel->setFixedWidth(120);
    mOriginalEdit = new QLineEdit(originalframe);
    mOriginalEdit->setEchoMode(QLineEdit::Password);

    Lyt_1->addWidget(originallabel);
    Lyt_1->addSpacing(16);
    Lyt_1->addWidget(mOriginalEdit);

    QFrame *newpwdframe = new QFrame(this);
    QHBoxLayout *Lyt_2 = new QHBoxLayout(newpwdframe);
    Lyt_2->setContentsMargins(0, 0, 0, 0);

    QLabel *newpwdlabel = new QLabel(tr("New password"), newpwdframe);
    newpwdlabel->setFixedWidth(120);
    mNewPwdEdit = new QLineEdit(newpwdframe);
    mNewPwdEdit->setEchoMode(QLineEdit::Password);
    QRegExp rx("[a-zA-Z0-9]*");
    QRegExpValidator *validator = new QRegExpValidator(rx , this);
    mNewPwdEdit->setValidator(validator);

    Lyt_2->addWidget(newpwdlabel);
    Lyt_2->addSpacing(16);
    Lyt_2->addWidget(mNewPwdEdit);

    QFrame *verifyframe = new QFrame(this);
    QHBoxLayout *Lyt_3 = new QHBoxLayout(verifyframe);
    Lyt_3->setContentsMargins(0, 0, 0, 0);

    QLabel *verifydlabel = new QLabel(tr("Verify password"), verifyframe);
    verifydlabel->setFixedWidth(120);
    mVerifyEdit = new QLineEdit(verifyframe);
    mVerifyEdit->setEchoMode(QLineEdit::Password);

    Lyt_3->addWidget(verifydlabel);
    Lyt_3->addSpacing(16);
    Lyt_3->addWidget(mVerifyEdit);

    QHBoxLayout *Lyt_4 = new QHBoxLayout;
    Lyt_4->setContentsMargins(8, 0, 8, 0);
    Lyt_4->setSpacing(0);
    mCancelBtn = new QPushButton(tr("Cancel"), this);
    mCancelBtn->setFixedWidth(96);
    mConfirmBtn = new QPushButton(tr("Confirm"), this);
    mConfirmBtn->setFixedWidth(96);
    mConfirmBtn->setEnabled(false);
    Lyt_4->addStretch();
    Lyt_4->addWidget(mCancelBtn);
    Lyt_4->addSpacing(10);
    Lyt_4->addWidget(mConfirmBtn);

    mHintLabel_1 = new FixLabel(this);
    mHintLabel_1->setContentsMargins(142, 0, 0, 0);
    mHintLabel_1->setFixedSize(440, 24);
    mHintLabel_1->setText("");
    mHintLabel_1->setStyleSheet("FixLabel{color:red; font-size : 14px}");
    mHintLabel_2 = new FixLabel(this);
    mHintLabel_2->setContentsMargins(142, 0, 0, 0);
    mHintLabel_2->setFixedSize(440, 24);
    mHintLabel_2->setText(" ");
    mHintLabel_2->setStyleSheet("FixLabel{color:red; font-size : 14px}");
    mHintLabel_3 = new FixLabel(this);
    mHintLabel_3->setContentsMargins(142, 0, 0, 0);
    mHintLabel_3->setFixedSize(440, 24);
    mHintLabel_3->setText(" ");
    mHintLabel_3->setStyleSheet("FixLabel{color:red; font-size : 14px}");

    Lyt->setSpacing(2);;
    Lyt->addWidget(titlelabel);
    Lyt->addSpacing(8);
    Lyt->addWidget(originalframe);
    Lyt->addWidget(mHintLabel_1);
    Lyt->addWidget(newpwdframe);
    Lyt->addWidget(mHintLabel_2);
    Lyt->addWidget(verifyframe);
    Lyt->addWidget(mHintLabel_3);
    Lyt->addStretch();
    Lyt->addLayout(Lyt_4);
}

void ChangePwdWidget::setConnect()
{
    connect(mCancelBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        this->close();
    });

    connect(mConfirmBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        if (mOriginalEdit->text().isEmpty()) {
            mHintLabel_1->setText(tr("The original password cannot be empty"));
            mOriginalEdit->setFocus();
        } else if (mOriginalEdit->text() == getPwd()) {
            setPwd(mNewPwdEdit->text());
            resultNotice();
        } else {
            mHintLabel_1->setText(tr("Wrong password"));
            mOriginalEdit->clear();
            mOriginalEdit->setFocus();
        }
    });

    connect(mNewPwdEdit, &QLineEdit::textEdited, this, &ChangePwdWidget::newpwdInputSlot);
    connect(mVerifyEdit, &QLineEdit::textEdited, this, &ChangePwdWidget::verifypwdInputSlot);
    connect(mOriginalEdit, &QLineEdit::textEdited, this, &ChangePwdWidget::orignalpwdinoutSlot);

}

void ChangePwdWidget::resultNotice()
{
    QString iconname = "ukui-dialog-success";
    QString tips = tr("Password changed successfully !");

    QDialog *dialog = new QDialog(this);
    dialog->setFixedSize(400, 150);
    QVBoxLayout *Lyt = new QVBoxLayout(dialog);
    Lyt->setContentsMargins(16, 16, 16, 24);

    QHBoxLayout *Lyt_1 = new QHBoxLayout;
    Lyt_1->setSpacing(8);
    QLabel *iconlabel = new QLabel(dialog);
    iconlabel->setPixmap(QIcon::fromTheme(iconname).pixmap(24, 24));
    QLabel *textlabel = new QLabel(dialog);
    textlabel->setStyleSheet("QLabel{text-align: left}");
    textlabel->setText(tips);
    Lyt_1->addWidget(iconlabel);
    Lyt_1->addWidget(textlabel);
    Lyt_1->addStretch();

    QHBoxLayout *Lyt_2 = new QHBoxLayout();
    QPushButton *confirmbtn = new QPushButton(tr("Confirm"), dialog);;
    Lyt_2->addStretch();
    Lyt_2->addWidget(confirmbtn);

    connect(dialog, &QDialog::destroyed, [=](){
        this->close();
    });

    connect(confirmbtn, &QPushButton::clicked, [=](){
        dialog->close();
        this->close();
    });

    Lyt->addLayout(Lyt_1);
    Lyt->addStretch();
    Lyt->addLayout(Lyt_2);
    dialog->exec();

}

void ChangePwdWidget::isActive()
{
    if (mHintLabel_3->text().isEmpty() && mHintLabel_2->text().isEmpty() && mHintLabel_1->text().isEmpty()) {
        mConfirmBtn->setEnabled(true);
    } else {
        mConfirmBtn->setEnabled(false);
    }
}

QString ChangePwdWidget::getPwd()
{
    mUkccInterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                                        "/",
                                                        "com.control.center.interface",
                                                        QDBusConnection::systemBus(), this);
    if (mUkccInterface->isValid()) {
        QDBusReply<QString> result = mUkccInterface->call("getAdvancePwd");
        return result;
    }
    return "admin";
}

void ChangePwdWidget::setPwd(QString pwd)
{
    mUkccInterface->call("setAdvancePwd", pwd);
    return;
}

void ChangePwdWidget::newpwdInputSlot(const QString &pwd)
{
    if (pwd.length() < 8) {
        if (pwd == mOriginalEdit->text() && !mNewPwdEdit->text().isEmpty()) { //新密码不为空的时候，判断与原密码框输入是否相同
            mHintLabel_2->setText(tr("Same as original password"));
        } else {
            mHintLabel_2->setText(tr("Must be 8-16 characters long"));
        }
    } else {
        mNewPwdEdit->setText(pwd.mid(0, 16));
        if (mNewPwdEdit->text() == mOriginalEdit->text()) {
            mHintLabel_2->setText(tr("Same as original password"));
        } else {
            mHintLabel_2->setText("");
        }
    }
    if (!mVerifyEdit->text().isEmpty()) {
        if (mVerifyEdit->text() != mNewPwdEdit->text()) { //验证密码不为空时，在新密码改变的时候，与其作比较
            mHintLabel_3->setText("Inconsistent input");
        } else {
            mHintLabel_3->setText("");
        }

    }
    isActive();
}

void ChangePwdWidget::verifypwdInputSlot(const QString &pwd)
{
    if (pwd != mNewPwdEdit->text()) {
        mHintLabel_3->setText(tr("Inconsistent input"));
    } else {
        mHintLabel_3->setText("");
    }
    if (mVerifyEdit->text().isEmpty()) {
        mHintLabel_3->setText(" ");
    }
    isActive();
}

void ChangePwdWidget::orignalpwdinoutSlot(const QString &pwd)
{
    mOriginalEdit->setText(pwd.mid(0, 16));
    mHintLabel_1->setText("");
    if (!mNewPwdEdit->text().isEmpty()) {
        newpwdInputSlot(mNewPwdEdit->text()); //新密码不为空的时候，原始密码变化的时候，与新密码作比较
    }
}

void ChangePwdWidget::clearContent()
{
    mOriginalEdit->clear();
    mNewPwdEdit->clear();
    mVerifyEdit->clear();
    mHintLabel_1->setText("");
    mHintLabel_2->setText(" ");
    mHintLabel_3->setText(" ");
    mConfirmBtn->setEnabled(false);
}



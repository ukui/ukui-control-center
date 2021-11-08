#include "hostnamedialog.h"
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QProcess>
#include <QRegExp>
#include <QRegExpValidator>

HostNameDialog::HostNameDialog(QWidget *parent):
    QDialog(parent)
{
    this->setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("Set HostName"));
    InitUi();
    setEdit();
    setupComponent();
    initConnect();
}

HostNameDialog::~HostNameDialog()
{

}

void HostNameDialog::InitUi()
{
    setFixedSize(480 ,160 );
    QVBoxLayout *mInputPwdLyt = new QVBoxLayout(this);
    mInputPwdLyt->setContentsMargins(24, 24, 24, 24);
    mInputPwdLyt->setSpacing(8);

    QFrame *mInputPwdFrame = new QFrame(this);
    mInputPwdFrame->setFixedSize(432, 36);
    mInputPwdFrame->setFrameShape(QFrame::Box);

    QHBoxLayout *mLyt_1= new QHBoxLayout(mInputPwdFrame);
    mLyt_1->setContentsMargins(0, 0, 0, 0);
    mLyt_1->setSpacing(8);

    QLabel *mHostNameLabel = new QLabel(mInputPwdFrame);
    mHostNameLabel->setFixedSize(102, 36);
    mHostNameLabel->setText(tr("HostName"));

    mHostNameEdit = new QLineEdit(mInputPwdFrame);
    mHostNameEdit->setAttribute(Qt::WA_InputMethodEnabled, false);  //限制中文输入法
    mHostNameEdit->setFixedSize(322, 36);
    mHostNameEdit->installEventFilter(this);

    mLyt_1->addWidget(mHostNameLabel);
    mLyt_1->addWidget(mHostNameEdit);


    QFrame *mInputPwdFrame_1 = new QFrame(this);
    mInputPwdFrame_1->setFixedSize(432, 36);
    mInputPwdFrame_1->setFrameShape(QFrame::Box);

    QHBoxLayout *mLyt_2= new QHBoxLayout(mInputPwdFrame_1);
    mLyt_2->setContentsMargins(0, 0, 0, 0);
    mLyt_2->setSpacing(16);

    mCancelBtn = new QPushButton(mInputPwdFrame_1);
    mCancelBtn->setFixedSize(120, 36);
    mCancelBtn->setText(tr("Cancel"));

    mConfirmBtn = new QPushButton(mInputPwdFrame_1);
    mConfirmBtn->setFixedSize(120, 36);
    mConfirmBtn->setText(tr("Confirm"));

    mLyt_2->addStretch();
    mLyt_2->addWidget(mCancelBtn);
    mLyt_2->addWidget(mConfirmBtn);

    mInputPwdLyt->addWidget(mInputPwdFrame);
    mInputPwdLyt->addStretch();
    mInputPwdLyt->addWidget(mInputPwdFrame_1);
}

void HostNameDialog::initConnect()
{
    connect(mHostNameEdit, &QLineEdit::textEdited, this, [=]() {
        if (mHostNameEdit->text().isEmpty()) {
             mConfirmBtn->setEnabled(false);
        } else {
            mConfirmBtn->setEnabled(true);
        }
    });

    connect(mCancelBtn, &QPushButton::clicked, this, [=]() {
        this->close();
    });

    connect(mConfirmBtn, &QPushButton::clicked, this, [=]() {
        if (mfirsthostname != mHostNameEdit->text()) {
            setHostname(mHostNameEdit->text());
        }
        this->close();
    });
}

void HostNameDialog::setEdit()
{
    mHostNameEdit->setMaxLength(32);
    QRegExp rx("[a-z_][a-zA-Z0-9_-]*[$]");
    QRegExpValidator *validator = new QRegExpValidator(rx , this);
    mHostNameEdit->setValidator(validator);
}

void HostNameDialog::setupComponent()
{
    mfirsthostname = Utils::getHostName();
    mHostNameEdit->setText(mfirsthostname);
}

void HostNameDialog::setHostname(QString hostname)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","en_US");
    QProcess *process = new QProcess;
    process->setProcessEnvironment(env);
    process->start(QString("%1%2").arg("hostnamectl set-hostname ").arg(hostname));
    process->waitForFinished();
    delete process;

    //修改/etc/hosts文件中的主机名

    QDBusInterface *sethostnameDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                                             "/",
                                                             "com.control.center.interface",
                                                             QDBusConnection::systemBus());

    QDBusReply<bool> reply = sethostnameDbus->call("sethostname", hostname);
    delete sethostnameDbus;
    sethostnameDbus = nullptr;

}












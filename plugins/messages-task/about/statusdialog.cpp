#include "statusdialog.h"
#include <QVBoxLayout>

StatusDialog::StatusDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("About"));
    initUI();
}

StatusDialog::~StatusDialog()
{

}

void StatusDialog::initUI()
{
    this->setFixedSize(450 , 384);
    QVBoxLayout *vLyt = new QVBoxLayout(this);
    vLyt->setContentsMargins(24 , 16 , 24 , 40);
    vLyt->setSpacing(8);

    mLogoLabel = new QLabel(this);

    // 版本号
    mVersionFrame = new QFrame(this);
    QHBoxLayout *mVersionLyt = new QHBoxLayout(mVersionFrame);
    mVersionLyt->setSpacing(16);
    mVersionFrame->setFixedHeight(36);
    mVersionFrame->setFrameShape(QFrame::NoFrame);
    mVersionLabel_1 = new QLabel(mVersionFrame);
    mVersionLabel_2 = new FixLabel(mVersionFrame);
    mVersionLabel_1->setFixedWidth(100);
    mVersionLyt->addWidget(mVersionLabel_1);
    mVersionLyt->addWidget(mVersionLabel_2);

    // 系统状态
    mStatusFrame = new QFrame(this);
    QHBoxLayout *mStatusLyt = new QHBoxLayout(mStatusFrame);
    mStatusLyt->setSpacing(16);
    mStatusFrame->setFixedHeight(36);
    mStatusFrame->setFrameShape(QFrame::NoFrame);
    mStatusLabel_1 = new FixLabel(mStatusFrame);
    mStatusLabel_2 = new QLabel(mStatusFrame);
    mStatusLabel_1->setFixedWidth(100);
    mStatusLyt->addWidget(mStatusLabel_1);
    mStatusLyt->addWidget(mStatusLabel_2);

    // 序列号
    mSerialFrame = new QFrame(this);
    QHBoxLayout *mSerialLyt = new QHBoxLayout(mSerialFrame);
    mSerialLyt->setSpacing(16);
    mSerialFrame->setFixedHeight(36);
    mSerialFrame->setFrameShape(QFrame::NoFrame);
    mSerialLabel_1 = new QLabel(mSerialFrame);
    mSerialLabel_2 = new FixLabel(mSerialFrame);
    mSerialLabel_1->setFixedWidth(100);
    mSerialLyt->addWidget(mSerialLabel_1);
    mSerialLyt->addWidget(mSerialLabel_2);

    // 服务到期时间
    mTimeFrame = new QFrame(this);
    QHBoxLayout *mTimeLyt = new QHBoxLayout(mTimeFrame);
    mTimeLyt->setSpacing(16);
    mTimeFrame->setFixedHeight(36);
    mTimeFrame->setFrameShape(QFrame::NoFrame);
    mTimeLabel_1 = new FixLabel(mTimeFrame);
    mTimeLabel_2 = new QLabel(mTimeFrame);
    mTimeLabel_1->setFixedWidth(100);
    mTimeLyt->addWidget(mTimeLabel_1);
    mTimeLyt->addWidget(mTimeLabel_2);

    mExtentBtn = new QPushButton(this);
    mExtentBtn->setFixedHeight(40);

    vLyt->addWidget(mLogoLabel);
    vLyt->addSpacing(24);
    vLyt->addWidget(mVersionFrame);
    vLyt->addWidget(mStatusFrame);
    vLyt->addWidget(mSerialFrame);
    vLyt->addWidget(mTimeFrame);
    vLyt->addStretch();
    vLyt->addWidget(mExtentBtn);
}

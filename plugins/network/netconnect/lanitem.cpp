#include "lanitem.h"
#define FRAME_SPEED 150
#define LIMIT_TIME 60*1000
#define TOTAL_PAGE 8
LanItem::LanItem(QWidget *parent)
{
    this->setMinimumSize(550, 58);
    this->setProperty("useButtonPalette", true);
    setStyleSheet("QPushButton:!checked{background-color: palette(base)}");
    QHBoxLayout *mLanLyt = new QHBoxLayout(this);
    mLanLyt->setSpacing(16);
    iconLabel = new QLabel(this);
    titileLabel = new FixLabel(this);
    statusLabel = new FixLabel(this);
    statusLabel->setMinimumSize(36,36);
    infoLabel = new InfoButton(this);
    mLanLyt->addWidget(iconLabel);
    mLanLyt->addWidget(titileLabel,Qt::AlignLeft);
    mLanLyt->addStretch();
    mLanLyt->addWidget(statusLabel);
    mLanLyt->addWidget(infoLabel);
    waitTimer = new QTimer(this);
    connect(waitTimer, &QTimer::timeout, this, &LanItem::waitAnimStep);
}

LanItem::~LanItem()
{

}

void LanItem::setWaitPage(int waitPage)
{
    this->waitPage = waitPage;
}

void LanItem::setCountCurrentTime(int countCurrentTime)
{
    this->countCurrentTime = countCurrentTime;
}

void LanItem::startLoading()
{
    waitTimer->start(FRAME_SPEED);
    loading = true;
}

void LanItem::stopLoading(){
    waitTimer->stop();
    loading = false;
}

void LanItem::waitAnimStep()
{
    QString qpmQss = "QLabel{background-image:url(':/img/plugins/netconnect/";
    qpmQss.append(QString::number(this->waitPage));
    qpmQss.append(".png');}");
    QImage img;
    img.load(":/img/plugins/netconnect/1.png");
    statusLabel->setText("");
    statusLabel->setFixedSize(img.size());
    statusLabel->setStyleSheet(qpmQss);
    this->waitPage ++;
    if (this->waitPage > TOTAL_PAGE) {
        this->waitPage = 1; //循环播放8张图片
    }
    this->countCurrentTime += FRAME_SPEED;
    if (this->countCurrentTime >= LIMIT_TIME) {
        this->stopLoading(); //动画超出时间限制，强制停止动画
    }
}


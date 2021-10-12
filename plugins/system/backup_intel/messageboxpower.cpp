#include "messageboxpower.h"
#include <QtDBus/QtDBus>


MessageBoxPower::MessageBoxPower(QWidget *parent)
    : QDialog(parent)
{
    //设置任务栏无显示
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    QDesktopWidget *deskdop=QApplication::desktop();
    //this->move((deskdop->width()-this->width())/2, (deskdop->height()-this->height())/2);
    //this->move(this->geometry().center() - this->rect().center());
    this->setFixedSize(550,250);
    initUI();
    //QDBusConnection::systemBus().connect(QString(),QString("/"),"com.kylin.update.notification","DownloadFinish",this,SLOT(otaevent()));
}

MessageBoxPower::~MessageBoxPower()
{

}
void MessageBoxPower::initUI()
{
    m_pmainlayout = new QVBoxLayout();
    m_pfirstlayout = new QHBoxLayout();
    m_ptestlayout = new QVBoxLayout();
    buttonlayout = new QHBoxLayout();
    m_pleftIconlayout = new QVBoxLayout();
    m_pcenterlayout = new QHBoxLayout();

    topWidget = new QWidget();
    iconLabel = new QLabel();
    closeButton = new QPushButton();
    closeButton = new QPushButton(this);
    closeButton->setProperty("useIconHighlightEffect", true);
    closeButton->setProperty("iconHighlightEffectMode", 1);
    closeButton->setFlat(true);
    closeButton->setAutoDefault(false);
    closeButton->setIcon(QIcon(":/img/plugins/backup/window-close-symbolic.svg"));;
    connect(closeButton, &QPushButton::clicked, [=]{
        close();
    });
    contentWidget = new QWidget();

    QImage *img=new QImage; //新建一个image对象
    img->load("://img/plugins/backup/System Recovery.svg"); //将图像资源载入对象img，注意路径，可点进图片右键复制路径
    iconLabel->setPixmap(QPixmap::fromImage(*img)); //将图片放入label，使用setPixmap,注意指针*img

    topLabel = new QLabel();
    //topLabel->setText("系统升级");
    topLabel->setText(tr("System Recovery"));

    linebutton = new QPushButton();
    linebutton->setMaximumSize(550,1);
    linebutton->setFocusPolicy(Qt::NoFocus);

    firstlabel = new QLabel();
    seclabel   = new QLabel();
    //firstlabel->setText("系统更新已准备就绪，请重启安装最新版本");
    firstlabel->setText(tr("The battery is low,please connect the power"));
    //seclabel->setText("请注意保存您的文件");
    seclabel->setText(tr("Keep the power connection, or the power is more than 25%."));

    remindButton = new QPushButton();
    //remindButton->setText("30分钟后提醒我");
    remindButton->setText(tr("Remind in 30 minutes"));
    remindButton->setFixedSize(160,40);
    remindButton->setStyleSheet("background-color: rgb(255,255,255);color:black;border:1px solid grey;border-radius:10px;");
    //connect(remindButton,&QPushButton::clicked,this,&MessageBoxPower::remindslots);

//    connect(remindButton, &QPushButton::clicked, this, [=]() {
//        hide();
//    });


    confirmButton = new QPushButton();
    //restartButton->setText("立即重启");
    confirmButton->setText(tr("Got it"));
    confirmButton->setFixedSize(160,40);
    confirmButton->setStyleSheet("background-color: rgb(85,173,222);color:white;");
    connect(confirmButton,&QPushButton::clicked,this,&MessageBoxPower::restartslots);
    buttonWidget = new QWidget();

    iconBigLabel = new QLabel();
    QImage *img2=new QImage; //新建一个image对象
    img2->load("://img/plugins/backup/Low battery.svg"); //将图像资源载入对象img，注意路径，可点进图片右键复制路径
    iconBigLabel->setPixmap(QPixmap::fromImage(*img2)); //将图片放入label，使用setPixmap,注意指针*img
    leftIconWidget = new QWidget();
    centerWidget = new QWidget();

    //标题布局
    m_pfirstlayout->addWidget(iconLabel);
    m_pfirstlayout->addWidget(topLabel);
    m_pfirstlayout->addItem(new QSpacerItem(550,2));
    m_pfirstlayout->addWidget(closeButton);
    topWidget->setLayout(m_pfirstlayout);

    //按钮布局
    //buttonlayout->addWidget(remindButton);
    remindButton->hide();
    buttonlayout->addStretch();
    buttonlayout->addWidget(confirmButton);
    buttonWidget->setLayout(buttonlayout);

    //左侧图标布局
    m_pleftIconlayout->addWidget(iconBigLabel);
    m_pleftIconlayout->addItem(new QSpacerItem(2,250));
    leftIconWidget->setFixedWidth(70);
    leftIconWidget->setLayout(m_pleftIconlayout);

    //右侧文字主体内容布局
    m_ptestlayout->addWidget(firstlabel);
    m_ptestlayout->addWidget(seclabel);
    m_ptestlayout->addItem(new QSpacerItem(2,20));
    m_ptestlayout->addWidget(buttonWidget);
    contentWidget->setLayout(m_ptestlayout);

    //增加图标主体内容布局
    m_pcenterlayout->addWidget(leftIconWidget);
    m_pcenterlayout->addWidget(contentWidget);
    centerWidget->setLayout(m_pcenterlayout);

    //整体布局
    m_pmainlayout->addWidget(topWidget);
    m_pmainlayout->addWidget(linebutton);
    m_pmainlayout->addWidget(centerWidget);

    this->setLayout(m_pmainlayout);
}

void MessageBoxPower::paintEvent(QPaintEvent *e)
{
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    /* 获取当前剪贴板中字体的颜色，作为背景色；
//     * 白字体 --> 黑背景
//     * 黑字体 --> 白字体
//    */
//    p.setBrush(opt.palette.color(QPalette::Base));
//    p.setOpacity(1);
//    p.setPen(QPen(QColor(255,255,255)));
//    QPainterPath path;
//    opt.rect.adjust(0,0,0,0);
//    path.addRoundedRect(opt.rect,16,16);
//    p.setRenderHint(QPainter::Antialiasing);                        //反锯齿
//    p.drawRoundedRect(opt.rect,16,16);
//    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
//    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
//    QWidget::paintEvent(e);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 16, 16);

    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::black);
    pixmapPainter.setOpacity(0.65);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();


    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    // 绘制一个背景
    p.save();
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.restore();
}

void MessageBoxPower::remindslots()
{

    //timer->start(1800*1000);

}
void MessageBoxPower::restartslots()
{
    /*QDBusMessage msg = QDBusMessage::createMethodCall("org.gnome.SessionManager",
                                                    "/org/gnome/SessionManager",
                                                    "org.gnome.SessionManager",
                                                    "reboot");
    */
//    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.login1",
//                                                    "/org/freedesktop/login1",
//                                                    "org.freedesktop.login1.Manager",
//                                                    "Reboot");
//    bool arg=1;
//    msg<<arg;

//    QDBusMessage response = QDBusConnection::systemBus().call(msg);
    this->close();

}

void MessageBoxPower::otaevent()
{

}

void MessageBoxPower::timetask()
{
//    timer->stop();
//    otaevent();
    //QDBusMessage msg =QDBusMessage::createSignal("/", "com.kylin.update.notification", "DownloadFinish");
    //QDBusConnection::systemBus().send(msg);
}

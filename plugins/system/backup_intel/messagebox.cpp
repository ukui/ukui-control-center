#include "messagebox.h"
#include "ui_messagebox.h"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
MessageBox::MessageBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageBox)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->closeBtn_3->setProperty("useIconHighlightEffect", true);
    ui->closeBtn_3->setProperty("iconHighlightEffectMode", 1);
    ui->closeBtn_3->setFlat(true);
    m_rebootInterface = new QDBusInterface("org.gnome.SessionManager",
                                         "/org/gnome/SessionManager",
                                         "org.gnome.SessionManager",
                                         QDBusConnection::sessionBus());
    connect(ui->closeBtn_3,&QPushButton::clicked,this,[=]{
        close();
    });
    connect(ui->cancelBtn,&QPushButton::clicked,[=]{
        close();
    });
    connect(ui->rebootBtn,&QPushButton::clicked,[=]{
        hide();
        m_rebootInterface->call("reboot");
    });
    ui->rebootBtn->setText(tr("Reboot Now"));
    ui->cancelBtn->setText(tr("Cancel"));
    ui->attentionInfoBtn->setText(tr("This cleanup and restore need to be done after the system restarts, whether to restart and restore immediately?"));
    ui->attentionInfoBtn->setWordWrap(true);
    ui->attentionInfoBtn->setAlignment(Qt::AlignHCenter);
    ui->attention->setText(tr("System Backup Tips"));
    initUi();
}

MessageBox::~MessageBox()
{
    delete ui;
}
void MessageBox::initUi(){
    //关闭按钮在右上角，窗体radius 6px，所以按钮只得6px
    ui->closeBtn_3->setIcon(QIcon("://img/titlebar/close.svg"));
    ui->closeBtn_3->setStyleSheet("QPushButton:hover:!pressed#closeBtn{background: #FA6056; border-radius: 4px;}"
                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border-radius: 4px;}");
}
void MessageBox::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

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

    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    // 绘制一个背景
    p.save();
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.restore();

}

#include "messageboxdialog.h"
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
MessageDialog::MessageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->closeBtn->setProperty("useIconHighlightEffect", true);
    ui->closeBtn->setProperty("iconHighlightEffectMode", 1);
    ui->closeBtn->setFlat(true);
    m_areaInterface = new QDBusInterface("org.gnome.SessionManager",
                                         "/org/gnome/SessionManager",
                                         "org.gnome.SessionManager",
                                         QDBusConnection::sessionBus());
    connect(ui->closeBtn,&QPushButton::clicked,this,[=]{
        close();
    });
    connect(ui->pushButton_2,&QPushButton::clicked,[=]{
        close();
    });
    connect(ui->pushButton,&QPushButton::clicked,[=]{
        close();
        QDBusMessage result = m_areaInterface->call("logout");
        if (QDBusMessage::ErrorMessage == result.type()) {
            CT_SYSLOG(LOG_DEBUG,"QDBusMessage::ErrorMessage");
        } else {
            QString logStr;
            logStr = "result.type = " + QString::number(result.type());
            CT_SYSLOG(LOG_DEBUG,logStr.toStdString().c_str());
        }
    });
    ui->pushButton->setText(tr("Logout Now"));
    ui->pushButton_2->setText(tr("Cancel"));
    ui->attentionLabel->setText(tr("This language change should be made after the system is logged off. Do you want to log off immediately?"));
    ui->attentionLabel->setWordWrap(true);
    ui->attentionLabel->setAlignment(Qt::AlignHCenter);
    ui->tipsLabel->setText(tr("Language Change Tips"));
    initUi();
}
MessageDialog::~MessageDialog(){

}
void MessageDialog::initUi(){
    //关闭按钮在右上角，窗体radius 6px，所以按钮只得6px
    ui->closeBtn->setIcon(QIcon("://img/titlebar/close.svg"));
    ui->closeBtn->setStyleSheet("QPushButton:hover:!pressed#closeBtn{background: #FA6056; border-radius: 4px;}"
                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border-radius: 4px;}");
}
void MessageDialog::paintEvent(QPaintEvent *event) {
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

#include "delautoboot.h"
#include "ui_delautoboot.h"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

DelAutoBoot::DelAutoBoot(QString appname, QString bname, QPixmap pixmap, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DelAutoBoot)
{
    ui->setupUi(this);
    app_name = appname;
    b_name = bname;
    ui->iconlabel->setPixmap(pixmap);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    qDebug()<<"Deleting "<<app_name<<".";
    initUI();
}

DelAutoBoot::~DelAutoBoot()
{
    delete ui;
}

void DelAutoBoot::initUI(){
    ui->namelabel->setText(app_name);
    ui->tiplabel->setText(tr("Are you sure to delete this program?"));
    connect(ui->cancelBtn, &QPushButton::clicked, [=]{
        close();
    });
    ui->confirmBtn->setStyleSheet("QPushButton{border-radius: 4px; background-color: #FD625E; color: #FFFFFF;}"
                                  "QPushButton:hover{background-color: #E6332D;}");
    ui->closeBtn->setIcon(QIcon("://img/titlebar/close.svg"));
    ui->closeBtn->setProperty("useIconHighlightEffect", true);
    ui->closeBtn->setProperty("iconHighlightEffectMode", 1);
    ui->closeBtn->setFlat(true);

    ui->closeBtn->setStyleSheet("QPushButton:hover:!pressed#closeBtn{background: #FA6056; border-radius: 4px;}"
                                "QPushButton:hover:pressed#closeBtn{background: #E54A50; border-radius: 4px;}");
    connect(ui->closeBtn, &QPushButton::clicked, [=]{
        close();
    });
    connect(ui->confirmBtn, &QPushButton::clicked, [=]{
        emit autoboot_deleted(app_name, b_name);
        close();
    });
}
void DelAutoBoot::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 10, 10);

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

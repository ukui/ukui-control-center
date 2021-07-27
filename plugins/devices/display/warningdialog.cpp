#include "warningdialog.h"
#include "ui_warningdialog.h"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
WarningDialog::WarningDialog(QString warningText, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WarningDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    warning = warningText;
    ui->confirmBtn_2->setText(tr("Confirm"));
    ui->cancelBtn_2->setText(tr("Cancel"));
    connect(ui->confirmBtn_2,&QPushButton::clicked,this,[=]{
        close();
    });
    connect(ui->cancelBtn_2,&QPushButton::clicked,this,[=]{
        close();
    });
    initUI();
}

WarningDialog::~WarningDialog()
{
    delete ui;
}

void WarningDialog::initUI(){
    textLabel = QSharedPointer<QLabel>(new QLabel());
    iconLabel = QSharedPointer<QLabel>(new QLabel());
//    textLabel.get()->setText(tr("please insure at least one output!"));
    textLabel.get()->setText(warning);
    textLabel.get()->setWordWrap(true);
    iconLabel.get()->setFixedSize(24, 24);
    iconLabel.get()->setStyleSheet("QLabel{border-radius: 12px; background: #FD625E; font-size:18px; color: #FFFFFF;}");
    iconLabel.get()->setText("!");
    iconLabel.get()->setAlignment(Qt::AlignCenter);
    ui->mainLyt->addStretch();
    ui->mainLyt->addWidget(iconLabel.get());
    ui->mainLyt->addWidget(textLabel.get());
    ui->mainLyt->addStretch();
}

void WarningDialog::paintEvent(QPaintEvent *event) {
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

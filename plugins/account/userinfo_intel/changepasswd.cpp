#include "changepasswd.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

changepasswd::changepasswd(QString username,QWidget *parent) : QDialog(parent)
{
    this->resize(360, 576);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    initUI(username);
    setQSS();

    connect(m_pdigitalAuthWidget, &DigitalAuthDialog::forgetpassword, [=](){
        m_pStackedWidget->setCurrentIndex(1);
    });
    connect(m_pPhoneAuthWidget, &PhoneAuthDialog::returnSignal, [=](){
        m_pStackedWidget->setCurrentIndex(0);
    });
    connect(m_pPhoneAuthWidget, &PhoneAuthDialog::confirmSignal, [=](){
        m_pdigitalAuthWidget->gotonext();
        m_pStackedWidget->setCurrentIndex(0);
    });
    connect(m_pdigitalAuthWidget, &DigitalAuthDialog::ended, [=](){
        emit changepwd();
        close();
    });
}

void changepasswd::initUI(QString username){
    QWidget * titleWidget = new QWidget(this);
    titleWidget->resize(360, 47);
//    titleWidget->move(0, 0);

    title_label = new QLabel(this);
    title_label->setText(tr("Change Password"));
    title_label->setObjectName("titleLabel");
    closeBtn = new QPushButton(this);
    closeBtn->setProperty("useIconHighlightEffect", true);
    closeBtn->setProperty("iconHighlightEffectMode", 1);
    closeBtn->setFlat(true);
    closeBtn->setIcon(QIcon(":/img/plugins/userinfo/close.svg"));
    connect(closeBtn, &QPushButton::clicked, [=]{
        close();
    });
    const QByteArray id_1(UKUI_QT_STYLE);
    if (QGSettings::isSchemaInstalled(id_1)) {
        m_style =new QGSettings(id_1);
        connect(m_style, &QGSettings::changed, this, &changepasswd::setpwdstyle);
    }
    QHBoxLayout *hlayout = new QHBoxLayout(titleWidget);
//    hlayout->setContentsMargins(0,0,0,0);
    hlayout->addWidget(title_label);
    hlayout->addStretch();
    hlayout->addWidget(closeBtn);

    m_pdigitalAuthWidget = new DigitalAuthDialog(username);
    m_pPhoneAuthWidget = new PhoneAuthDialog(username);
    m_pStackedWidget = new QStackedWidget(this);
    m_pStackedWidget->addWidget(m_pdigitalAuthWidget);
    m_pStackedWidget->addWidget(m_pPhoneAuthWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(m_pStackedWidget);
}

void changepasswd::setQSS(){
}
void changepasswd::setpwdstyle(QString key)
{
    if(key == "styleName") {
        update();
    }
}
void changepasswd::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 16, 16);
//    rectPath.addRoundedRect(this->rect(), 16, 16);

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
    qt_blurImage(img, 16, false, false);

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

changepasswd::~changepasswd(){
  delete m_style;
}

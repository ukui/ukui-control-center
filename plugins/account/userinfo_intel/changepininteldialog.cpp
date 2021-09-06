#include "changepininteldialog.h"
#include "ui_changepininteldialog.h"
#include <QDebug>

#define INPUT_OLD_PWD 1
#define INPUT_NEW_PWD 2
#define REPEAT_NEW_PWD 3

int ChangePinIntelDialog::m_isThemeChanged = 0;

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

ChangePinIntelDialog::ChangePinIntelDialog(QString username, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangePinIntelDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    closeBtn = new QPushButton(this);
    closeBtn->setFixedSize(32, 32);
    ui->horizontalLayout->addWidget(closeBtn);
    closeBtn->setProperty("useIconHighlightEffect", true);
    closeBtn->setProperty("iconHighlightEffectMode", 1);
    closeBtn->setFlat(true);
    closeBtn->setIcon(QIcon(":/img/plugins/userinfo_intel/close.svg"));
    closeBtn->setAutoDefault(false);
    connect(closeBtn, &QPushButton::clicked, [=]{
        close();
    });

    m_pdigitalAuthWidget = new DigitalAuthIntelDialog(username);
    m_pPhoneAuthWidget = new PhoneAuthIntelDialog(username);
    ui->stackedWidget->addWidget(m_pdigitalAuthWidget);
    ui->stackedWidget->addWidget(m_pPhoneAuthWidget);
    ui->stackedWidget->setCurrentIndex(0);


    connect(m_pdigitalAuthWidget, &DigitalAuthIntelDialog::forgetpassword, [=](){
        ui->stackedWidget->setCurrentIndex(1);
    });
    connect(m_pPhoneAuthWidget, &PhoneAuthIntelDialog::returnSignal, [=](){
        ui->stackedWidget->setCurrentIndex(0);
    });
    connect(m_pPhoneAuthWidget, &PhoneAuthIntelDialog::confirmSignal, [=](){
        m_pdigitalAuthWidget->gotonext();
        ui->stackedWidget->setCurrentIndex(0);
    });
    connect(m_pdigitalAuthWidget, &DigitalAuthIntelDialog::ended, [=](){
        emit changepwd();
        close();
    });
    connect(this, &ChangePinIntelDialog::EmitThemeChanged, m_pPhoneAuthWidget, &PhoneAuthIntelDialog::themeChanged);

    const QByteArray id_1(UKUI_QT_STYLE);
    if (QGSettings::isSchemaInstalled(id_1)) {
        m_style =new QGSettings(id_1);
        connect(m_style, &QGSettings::changed, this, &ChangePinIntelDialog::setpwdstyle);
    }
    setpwdstyle("styleName");
}

ChangePinIntelDialog::~ChangePinIntelDialog()
{
    delete ui;
    delete m_style;
}

void ChangePinIntelDialog::paintEvent(QPaintEvent *event) {
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

void ChangePinIntelDialog::setpwdstyle(QString key){
    if(key == "styleName") {
        QString currentTheme = m_style->get(UKUI_STYLE_KEY).toString();
        if(currentTheme == "ukui-default" || currentTheme == "ukui-white"
                || currentTheme == "ukui-light" || currentTheme == "ukui"){
            ChangePinIntelDialog::m_isThemeChanged = 0;
        }else if(currentTheme == "ukui-dark" || currentTheme == "ukui-black"){
            ChangePinIntelDialog::m_isThemeChanged = 1;
        }
        emit EmitThemeChanged(ChangePinIntelDialog::m_isThemeChanged);
        update();
    }
}

#include "changeusernickname.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>

#include <QDBusInterface>
#include <QDBusReply>

#include <QPainter>
#include <QPainterPath>

extern "C" {
#include <glib.h>
}

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

ChangeUserNickname::ChangeUserNickname(QString nn, QStringList ns, QString op, QWidget *parent) :
    QDialog(parent),
    namesIsExists(ns),
    realname(nn)
{
    setFixedSize(QSize(480, 296));
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    cniface = new QDBusInterface("org.freedesktop.Accounts",
                                  op,
                                  "org.freedesktop.Accounts.User",
                                  QDBusConnection::systemBus());


    initUI();
    setConnect();
    setupStatus();
}

ChangeUserNickname::~ChangeUserNickname()
{
    delete cniface;
}

void ChangeUserNickname::initUI(){

    //右上角关闭按钮
    closeBtn = new QPushButton();
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setFlat(true);
    closeBtn->setFixedSize(QSize(30, 30));
    closeBtn->setProperty("isWindowButton", 0x2);
    closeBtn->setProperty("useIconHighlightEffect", 0x08);

    titleHorLayout = new QHBoxLayout;
    titleHorLayout->setSpacing(0);
    titleHorLayout->setContentsMargins(0, 0, 14, 0);
    titleHorLayout->addStretch();
    titleHorLayout->addWidget(closeBtn);

    //用户名
    userNameLabel = new QLabel();
    userNameLabel->setMinimumWidth(60);
    userNameLabel->setMaximumWidth(120);
    userNameLabel->setText(tr("UserName"));

    userNameLineEdit = new QLineEdit();
    userNameLineEdit->setFixedSize(QSize(300, 36));
    userNameLineEdit->setPlaceholderText(QString(g_get_user_name()));
    userNameLineEdit->setReadOnly(true);

    userNameHorLayout = new QHBoxLayout;
    userNameHorLayout->setSpacing(25);
    userNameHorLayout->setContentsMargins(0, 0, 0, 0);
    userNameHorLayout->addWidget(userNameLabel);
    userNameHorLayout->addWidget(userNameLineEdit);

    //用户昵称
    nickNameLabel = new QLabel();
    nickNameLabel->setMinimumWidth(60);
    nickNameLabel->setMaximumWidth(120);
    nickNameLabel->setText(tr("NickName"));

    tipLabel = new QLabel();
    tipLabel->setFixedSize(QSize(300, 24));
    QString tipinfo = tr("Name already in use, change another one.");
    if (setTextDynamicInNick(tipLabel, tipinfo)){
        tipLabel->setToolTip(tipinfo);
    }

    nickNameLineEdit = new QLineEdit();
    nickNameLineEdit->setFixedSize(QSize(300, 36));
    nickNameLineEdit->setText(realname);

    nickNameHorLayout = new QHBoxLayout;
    nickNameHorLayout->setSpacing(25);
    nickNameHorLayout->setContentsMargins(0, 0, 0, 0);
    nickNameHorLayout->addWidget(nickNameLabel);
    nickNameHorLayout->addWidget(nickNameLineEdit);

    tipHorLayout = new QHBoxLayout;
    tipHorLayout->setSpacing(0);
    tipHorLayout->setContentsMargins(0, 0, 0, 0);
    tipHorLayout->addStretch();
    tipHorLayout->addWidget(tipLabel);

    nickNameWithTipVerLayout = new QVBoxLayout;
    nickNameWithTipVerLayout->setSpacing(4);
    nickNameWithTipVerLayout->setContentsMargins(0, 0, 0, 0);
    nickNameWithTipVerLayout->addLayout(nickNameHorLayout);
    nickNameWithTipVerLayout->addLayout(tipHorLayout);

    //计算机名
    computerNameLabel = new QLabel();
    computerNameLabel->setMinimumWidth(60);
    computerNameLabel->setMaximumWidth(120);
    computerNameLabel->setText(tr("ComputerName"));

    computerNameLineEdit = new QLineEdit();
    computerNameLineEdit->setFixedSize(QSize(300, 36));
    computerNameLineEdit->setPlaceholderText(QString(g_get_host_name()));
    computerNameLineEdit->setReadOnly(true);

    computerNameHorLayout = new QHBoxLayout;
    computerNameHorLayout->setSpacing(25);
    computerNameHorLayout->setContentsMargins(0, 0, 0, 0);
    computerNameHorLayout->addWidget(computerNameLabel);
    computerNameHorLayout->addWidget(computerNameLineEdit);

    //中部输入区域
    contentVerLayout = new QVBoxLayout;
    contentVerLayout->setSpacing(10);
    contentVerLayout->setContentsMargins(24, 0, 35, 24);
    contentVerLayout->addLayout(userNameHorLayout);
    contentVerLayout->addLayout(nickNameWithTipVerLayout);
    contentVerLayout->addLayout(computerNameHorLayout);

    //底部“取消”、“确定”按钮
    cancelBtn = new QPushButton();
    cancelBtn->setMinimumWidth(96);
    cancelBtn->setText(tr("Cancel"));
    confirmBtn = new QPushButton();
    confirmBtn->setMinimumWidth(96);
    confirmBtn->setText(tr("Confirm"));

    bottomBtnsHorLayout = new QHBoxLayout;
    bottomBtnsHorLayout->setSpacing(16);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 25, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(cancelBtn);
    bottomBtnsHorLayout->addWidget(confirmBtn);

    //主布局
    mainVerLayout = new QVBoxLayout;
    mainVerLayout->setSpacing(20);
    mainVerLayout->setContentsMargins(0, 14, 0, 24);
    mainVerLayout->addLayout(titleHorLayout);
    mainVerLayout->addLayout(contentVerLayout);
    mainVerLayout->addLayout(bottomBtnsHorLayout);

    setLayout(mainVerLayout);

}

void ChangeUserNickname::setConnect(){
    nickNameLineEdit->installEventFilter(this);

    connect(nickNameLineEdit, &QLineEdit::editingFinished, this, [=]{
        if (nickNameLineEdit->text().isEmpty()){
            nickNameLineEdit->setText(QString(g_get_real_name()));
        }
    });

    connect(nickNameLineEdit, &QLineEdit::textEdited, this, [=](QString txt){
        if (namesIsExists.contains(txt)){
            tipLabel->show();
            confirmBtn->setEnabled(false);
        } else {
            tipLabel->hide();
            confirmBtn->setEnabled(true);
        }
    });

    connect(closeBtn, &QPushButton::clicked, this, [=]{
        close();
    });
    connect(cancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });

    connect(confirmBtn, &QPushButton::clicked, this, [=]{
        cniface->call("SetRealName", nickNameLineEdit->text());
        close();
    });
}

void ChangeUserNickname::setupStatus(){
    tipLabel->hide();
}

bool ChangeUserNickname::setTextDynamicInNick(QLabel *label, QString string){

    bool isOverLength = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);

    QString str = string;
    int pSize = label->width();
    if (fontSize > pSize) {
        str = fontMetrics.elidedText(string, Qt::ElideRight, pSize);
        isOverLength = true;
    } else {

    }
    label->setText(str);
    return isOverLength;

}

void ChangeUserNickname::keyPressEvent(QKeyEvent * event){
    switch (event->key())
    {
    case Qt::Key_Escape:
        break;
    case Qt::Key_Enter:
        if (confirmBtn->isEnabled())
            confirmBtn->clicked();
        break;
    case Qt::Key_Return:
        if (confirmBtn->isEnabled())
            confirmBtn->clicked();
        break;
    default:
        QDialog::keyPressEvent(event);
    }
}


bool ChangeUserNickname::eventFilter(QObject *watched, QEvent *event){
    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton ){
            if (watched == nickNameLineEdit){
                if (QString::compare(nickNameLineEdit->text(), g_get_real_name()) == 0){
                    nickNameLineEdit->setText("");
                }
            }
        }
    }
}

void ChangeUserNickname::paintEvent(QPaintEvent *event){
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 6, 6);

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

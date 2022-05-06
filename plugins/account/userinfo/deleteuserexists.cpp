#include "deleteuserexists.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QButtonGroup>
#include <unistd.h>

#include <QDBusInterface>

#include <QPainter>
#include <QPainterPath>

#include <QMouseEvent>

#include <QDebug>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

DeleteUserExists::DeleteUserExists(QString name, QString nick, qint64 uid, QWidget *parent) :
    QDialog(parent),
    _name(name),
    _nick(nick),
    _id(uid)
{
    setFixedHeight(280);
    setMinimumWidth(520);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    initUI();
    setConnect();
}

DeleteUserExists::~DeleteUserExists()
{
}

void DeleteUserExists::initUI(){

    removeBtnGroup = new QButtonGroup;

    //标题
    closeBtn = new QPushButton;
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

    //提示信息
    QString note1 = tr("Delete user '");
    QString note2 = tr("'? And:");
    noteLabel = new QLabel;
    noteLabel->setFixedHeight(27);
    if (!_nick.isEmpty()){
        noteLabel->setText(note1 + _nick + note2);
    } else {
        noteLabel->setText(note1 + _name + note2);
    }

    noteHorLayout = new QHBoxLayout;
    noteHorLayout->setSpacing(0);
    noteHorLayout->setMargin(10);
    noteHorLayout->addWidget(noteLabel);

    //
    removeButKeepFilesRadioBtn = new QRadioButton;
    removeButKeepFilesRadioBtn->setFixedSize(QSize(16, 16));
    removeWholeRadioBtn = new QRadioButton;
    removeWholeRadioBtn->setFixedSize(QSize(16, 16));

    removeBtnGroup->addButton(removeButKeepFilesRadioBtn, 0);
    removeBtnGroup->addButton(removeWholeRadioBtn, 1);
    removeWholeRadioBtn->setChecked(true);

    removeButKeepFilesLabel = new QLabel;
    removeButKeepFilesLabel->setText(tr("Keep desktop, files, favorites, music of the user"));
    removeWholeLabel = new QLabel;
    removeWholeLabel->setText(tr("Delete whole data belong user"));

    removeButKeepFilesHorLayout = new QHBoxLayout;
    removeButKeepFilesHorLayout->setSpacing(9);
    removeButKeepFilesHorLayout->setContentsMargins(10, 0, 0, 0);
    removeButKeepFilesHorLayout->addWidget(removeButKeepFilesRadioBtn);
    removeButKeepFilesHorLayout->addWidget(removeButKeepFilesLabel);

    removeWholeHorLayout = new QHBoxLayout;
    removeWholeHorLayout->setSpacing(9);
    removeWholeHorLayout->setContentsMargins(10, 0, 0, 0);
    removeWholeHorLayout->addWidget(removeWholeRadioBtn);
    removeWholeHorLayout->addWidget(removeWholeLabel);

    removeButKeepFilesFrame = new QFrame;
    removeButKeepFilesFrame->setMinimumSize(QSize(472, 60));
    removeButKeepFilesFrame->setMaximumSize(QSize(16777215, 60));
    removeButKeepFilesFrame->setFrameShape(QFrame::Box);
    removeButKeepFilesFrame->setFrameStyle(QFrame::Plain);
    removeButKeepFilesFrame->setLayout(removeButKeepFilesHorLayout);
    removeButKeepFilesFrame->installEventFilter(this);

    removeWholeFrame = new QFrame;
    removeWholeFrame->setMinimumSize(QSize(472, 60));
    removeWholeFrame->setMaximumSize(QSize(16777215, 60));
    removeWholeFrame->setFrameShape(QFrame::Box);
    removeWholeFrame->setFrameStyle(QFrame::Plain);
    removeWholeFrame->setLayout(removeWholeHorLayout);
    removeWholeFrame->installEventFilter(this);

    //底部按钮
    cancelBtn = new QPushButton;
    cancelBtn->setText(tr("Cancel"));
    confirmBtn = new QPushButton;
    confirmBtn->setText(tr("Confirm"));

    bottomBtnsHorLayout = new QHBoxLayout;

    bottomBtnsHorLayout->setContentsMargins(0, 0, 0, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(cancelBtn);
    bottomBtnsHorLayout->addSpacing(16);
    bottomBtnsHorLayout->addWidget(confirmBtn);
    bottomBtnsHorLayout->addSpacing(8);

    contentVerLayout = new QVBoxLayout;
    contentVerLayout->setSpacing(0);
    contentVerLayout->setContentsMargins(24, 0, 24, 12);
    contentVerLayout->addLayout(noteHorLayout);
    contentVerLayout->addSpacing(8);
    contentVerLayout->addWidget(removeButKeepFilesFrame);
    contentVerLayout->addSpacing(8);
    contentVerLayout->addWidget(removeWholeFrame);
    contentVerLayout->addSpacing(33);
    contentVerLayout->addLayout(bottomBtnsHorLayout);

    mainVerLayout = new QVBoxLayout;
    mainVerLayout->setSpacing(0);
    mainVerLayout->setContentsMargins(0, 14, 0, 24);
    mainVerLayout->addLayout(titleHorLayout);
    mainVerLayout->addLayout(contentVerLayout);

    setLayout(mainVerLayout);
}

void DeleteUserExists::setConnect(){

    connect(closeBtn, &QPushButton::clicked, this, [=]{
        close();
    });

    connect(cancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });

    connect(confirmBtn, &QPushButton::clicked, this, [=]{
        QDBusInterface *tmpSysinterface = nullptr;
        if (Utils::isCommunity()) {
            tmpSysinterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                           "/",
                                           "com.control.center.interface",
                                           QDBusConnection::systemBus());
        } else {
            tmpSysinterface = new QDBusInterface("org.freedesktop.Accounts",
                                           "/org/freedesktop/Accounts",
                                           "org.freedesktop.Accounts",
                                           QDBusConnection::systemBus());
        }

        //底层删除用户存在延时，先隐藏掉删除用户界面
        this->hide();

        if (tmpSysinterface->isValid()){
            qDebug() << "call" << "method: deleteuser";
            QDBusReply<bool> ret = tmpSysinterface->call("DeleteUser", _id, removeBtnGroup->checkedId() == 1 ? true : false);

            if (!ret.isValid()) {
                qDebug() << "call DeleteUser failed" << ret.error();
            }
        } else {
            qCritical() << "Create Client Interface Failed When : " << QDBusConnection::systemBus().lastError();
        }

        close();
    });
}

bool DeleteUserExists::eventFilter(QObject *watched, QEvent *event){
    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton ){
            if (watched == removeButKeepFilesFrame){
                removeButKeepFilesRadioBtn->setChecked(true);
            } else if (watched == removeWholeFrame){
                removeWholeRadioBtn->setChecked(true);
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

void DeleteUserExists::paintEvent(QPaintEvent *event){
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

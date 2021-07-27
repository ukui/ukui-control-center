#include "applydialog.h"
#include "ui_applydialog.h"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
ApplyDialog::ApplyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ApplyDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    initUI();
    initTimer(15);
}

void ApplyDialog::initUI() {
    mainLyt = QSharedPointer<QVBoxLayout>(new QVBoxLayout(this));
    mainLyt.get()->setSpacing(0);
    mainLyt.get()->setContentsMargins(10, 10, 10, 10);

    //标题栏
    titleFrame = QSharedPointer<QFrame>(new QFrame());
    titleLyt = QSharedPointer<QHBoxLayout>(new QHBoxLayout(titleFrame.get()));
    minimizeBtn = new QPushButton;
    closeBtn = new QPushButton;
    titleFrame.get()->setFixedHeight(47);
    titleLyt.get()->setSpacing(32);
    titleLyt.get()->setContentsMargins(16, 8, 16, 7);
    minimizeBtn->setFixedSize(24, 24);
    minimizeBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    minimizeBtn->setStyleSheet("QPushButton:!hover{background: transparent;}"
                                "QPushButton:hover:!pressed{background: rgba(47,179,232,0.5); border-radius: 4px;}"
                                "QPushButton:hover:pressed{background: rgba(47,179,232,1); border-radius: 4px;}");
//    connect(minimizeBtn, &QPushButton::clicked, this, [ = ] () {
//        showMinimized();
//    });
    closeBtn->setFixedSize(24, 24);
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setStyleSheet("QPushButton:!hover{background: transparent;}"
                                "QPushButton:hover:!pressed{background: #FA6056; border-radius: 4px;}"
                                "QPushButton:hover:pressed{background: #E54A50; border-radius: 4px;}");
    connect(closeBtn, &QPushButton::clicked, this, [ = ] () {
        m_itimer->stop();
        emit do_not_apply();
        close();
    });
    titleLyt.get()->addStretch();
//    titleLyt.get()->addWidget(minimizeBtn);
    titleLyt.get()->addWidget(closeBtn);
    titleFrame.get()->setLayout(titleLyt.get());
    titleFrame.get()->setLineWidth(0);
    //分割线
    hLine = QSharedPointer<QFrame>(new QFrame);
    hLine.get()->setFrameShape(QFrame::Shape::HLine);
    hLine.get()->setFixedHeight(1);
    hLine.get()->setLineWidth(0);
    //内容控件
    contentFrame = QSharedPointer<QFrame>(new QFrame);
    contentFrame.get()->setFixedHeight(227);
    contentFrame.get()->setFrameShape(QFrame::Shape::Box);
    contentFrame.get()->setLineWidth(0);
    contentLyt = QSharedPointer<QVBoxLayout>(new QVBoxLayout(contentFrame.get()));
    contentLyt.get()->setContentsMargins(24, 40, 24, 24);
    contentLyt.get()->setSpacing(0);
    tipLabel = QSharedPointer<QLabel>(new QLabel(tr("Whether to apply this modification?")));
    tipLabel.get()->setAlignment(Qt::AlignHCenter);
    QFont tipfont;
    tipfont.setPointSize(15);
    tipfont.setBold(true);
    tipLabel.get()->setFont(tipfont);
    countDownLabel = QSharedPointer<QLabel>(new QLabel);
    QFont countfont;
    countfont.setPointSize(13);
    countDownLabel.get()->setAlignment(Qt::AlignHCenter);
    countDownLabel.get()->setFont(countfont);
    countDownLabel.get()->setWordWrap(true);
    //按钮
    buttonFrame = QSharedPointer<QFrame>(new QFrame);
    buttonFrame.get()->setLineWidth(0);
    buttonLyt = QSharedPointer<QHBoxLayout>(new QHBoxLayout(buttonFrame.get()));
    buttonLyt.get()->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
    buttonLyt.get()->setContentsMargins(0, 0, 0, 0);
    buttonLyt.get()->setSpacing(24);
    cancelBtn = new QPushButton;
    applyBtn = new QPushButton;
    cancelBtn->setFixedSize(164, 48);
    cancelBtn->setText(tr("Cancel"));
    connect(cancelBtn, &QPushButton::clicked, this, [ = ] () {
        m_itimer->stop();
        emit do_not_apply();
        close();
    });
    applyBtn->setFixedSize(164, 48);
    applyBtn->setText(tr("Apply"));
    connect(applyBtn, &QPushButton::clicked, this, [ = ] () {
        m_itimer->stop();
        emit do_apply();
        close();
    });
    buttonLyt.get()->addWidget(cancelBtn);
    buttonLyt.get()->addWidget(applyBtn);
    buttonFrame.get()->setLayout(buttonLyt.get());

    contentLyt.get()->addWidget(tipLabel.get());
    contentLyt.get()->addWidget(countDownLabel.get());
    contentLyt.get()->addWidget(buttonFrame.get());
    contentFrame.get()->setLayout(contentLyt.get());

    mainLyt.get()->addWidget(titleFrame.get());
    mainLyt.get()->addWidget(hLine.get());
    mainLyt.get()->addWidget(contentFrame.get());
    this->setLayout(mainLyt.get());
}

void ApplyDialog::initTimer(int second) {
    sec = second;
    countDownLabel.get()->setText(QString(tr("This operation will be undone after %1 seconds.")).arg(sec));
    m_itimer = new QTimer();
    m_itimer->start(1000);
    connect(m_itimer,SIGNAL(timeout()), this, SLOT(updateLabel()));
}

void ApplyDialog::updateLabel() {
    sec--;
    if (sec == 0) {
        m_itimer->stop();
        emit do_not_apply();
        close();
    } else if (sec == 1) {
        countDownLabel.get()->setText(QString(tr("This operation will be undone after %1 second.")).arg(sec));
    } else {
        countDownLabel.get()->setText(QString(tr("This operation will be undone after %1 seconds.")).arg(sec));
    }
}

ApplyDialog::~ApplyDialog()
{
    delete ui;
    if (m_itimer) {
        m_itimer->stop();
        delete m_itimer;
        m_itimer = NULL;
    }
    if (minimizeBtn) {
        delete minimizeBtn;
        minimizeBtn = NULL;
    }
    if (closeBtn) {
        delete closeBtn;
        closeBtn = NULL;
    }
    if (cancelBtn) {
        delete cancelBtn;
        cancelBtn = NULL;
    }
    if (applyBtn) {
        delete applyBtn;
        applyBtn = NULL;
    }
}

void ApplyDialog::paintEvent(QPaintEvent *event) {
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

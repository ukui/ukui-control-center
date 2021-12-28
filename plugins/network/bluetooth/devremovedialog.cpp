#include "devremovedialog.h"

#include <QFont>
#include <QFontMetrics>

DevRemoveDialog::DevRemoveDialog(QWidget *parent):QDialog(parent)
{
    initGsettings();
    this->setFixedSize(380,180);
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    initUI();
}

DevRemoveDialog::~DevRemoveDialog()
{

}

void DevRemoveDialog::initUI()
{
    tipLabel = new QLabel(this);
    tipLabel->setGeometry(56,25,300,60);
    tipLabel->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    tipLabel->setWordWrap(true);


    txtLabel = new QLabel(this);
    txtLabel->setGeometry(55,80,300,50);
    txtLabel->setWordWrap(true);
    txtLabel->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    txtLabel->setText(tr("After it is removed, the PIN code must be matched for the next connection."));
    txtLabel->setStyleSheet("font-size: 14px;\
                            font-family: Noto Sans CJK SC;\
                            font-weight: 400;\
                            line-height: 30px;\
                            color: #818181;\
                            opacity: 1;");

    iconLabel = new QLabel(this);
    iconLabel->setGeometry(16,45,22,22);
    iconLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(22,22));

    closeBtn = new QPushButton(this);
    closeBtn->setGeometry(350,8,20,20);
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setFlat(true);
    //closeBtn->setFixedSize(QSize(20,20));
    closeBtn->setProperty("isWindowButton",0x2);
    closeBtn->setProperty("useIconHighlihtEffect",0x8);
    connect(closeBtn,&QPushButton::clicked,this,[=]{
        this->close();
    });

    acceptBtn = new QPushButton(this);
    acceptBtn->setGeometry(252,128,96,36);
    acceptBtn->setText(tr("Remove"));
    connect(acceptBtn,&QPushButton::clicked,this,[=]{
        emit accepted();
        this->close();
    });

    rejectBtn = new QPushButton(this);
    rejectBtn->setGeometry(140,128,96,36);
    rejectBtn->setText(tr("Cancel"));
    connect(rejectBtn,&QPushButton::clicked,this,[=]{
        this->close();
    });
}

void DevRemoveDialog::initGsettings()
{
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        gsettings = new QGSettings("org.ukui.style");

        if(gsettings->get("style-name").toString() == "ukui-default"
           || gsettings->get("style-name").toString() == "ukui-light")
            isblack = false;
        else
            isblack = true;
    }

    connect(gsettings,&QGSettings::changed,this,&DevRemoveDialog::gsettingsSlot);
}

void DevRemoveDialog::setDialogText(const QString &str)
{
    QString txt = QString(tr("Are you sure to remove %1 ?")).arg(str);

    txt = QFontMetrics(this->font()).elidedText(txt,Qt::ElideMiddle,tipLabel->width());

    tipLabel->setText(txt);
}

void DevRemoveDialog::gsettingsSlot(const QString &key)
{
    if (key == "styleName") {
        QPalette palette;
        if(gsettings->get("style-name").toString() == "ukui-default" ||
           gsettings->get("style-name").toString() == "ukui-light") {

            palette.setBrush(QPalette::Base,QBrush(Qt::white));
            palette.setColor(QPalette::Text,QColor(Qt::black));

            isblack = false;

        } else {

            palette.setBrush(QPalette::Base,QBrush(Qt::black));
            palette.setColor(QPalette::Text,QColor(Qt::white));

            isblack = true;
        }

        this->setPalette(palette);
    }
}

void DevRemoveDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setPen(Qt::transparent);

    if (isblack)
        painter.setBrush(Qt::black);
    else
        painter.setBrush(Qt::white);

    painter.setRenderHint(QPainter::Antialiasing);

    painter.drawRoundedRect(this->rect(),12,12);
}

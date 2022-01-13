#include "intelmsgbox.h"

#include <QFont>
#include <QFontMetrics>

#include "config.h"

#define STYLE "org.ukui.style"

MsgBox::MsgBox(QWidget *parent, const QString txt):
    QDialog(parent),
    devname(txt)
{
    this->setFixedSize(510,292);
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);//设置背景透明

    if (QGSettings::isSchemaInstalled(STYLE)) {
        gsettings = new QGSettings("org.ukui.style");
        connect(gsettings,&QGSettings::changed,this,&MsgBox::gsettingsChanged);

        if(gsettings->get("style-name").toString() == "ukui-black" ||
           gsettings->get("style-name").toString() == "ukui-dark")
        {
            isBlack = true;
        }
        else
        {
            isBlack = false;
        }
    }

    QString text = QString(tr("Sure to remove %1 ?")).arg(devname);

    if (QFontMetrics(this->font()).width(text) > 485) {
        text = QFontMetrics(this->font()).elidedText(text,Qt::ElideMiddle,485);
    }

    QLabel *label = new QLabel(this);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label->resize(390,50);
    label->setGeometry(96,53,390,50);
    label->setText(text);
    label->setStyleSheet("font-style: normal;\
                         font-weight: bold;\
                         font-size: 18px;\
                         line-height: 27px;");

    QLabel *label1 = new QLabel(this);
    label1->setWordWrap(true);
    label1->setAlignment(Qt::AlignTop);
    label1->resize(390,40);
    label1->setGeometry(96,108,390,QFontMetrics(this->font()).height()*2);
    label1->setText(tr("After removal, the next connection requires matching PIN code !"));

    QLabel *icon = new QLabel(this);
    icon->resize(48,48);
    icon->setGeometry(24,73,48,48);
    icon->setPixmap(QIcon(":/image/icon-bluetooth/ukui-bluetooth-warning.svg").pixmap(48,48));

    closeBtn = new QPushButton(this);
    closeBtn->setGeometry(470,16,30,30);

    if (isBlack)
        closeBtn->setIcon(renderSvg(QIcon::fromTheme("window-close-symbolic"),"white"));
    else
        closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));

    closeBtn->setStyleSheet("QPushButton:hover{background:transparent; border-radius: 4px;}"
                            "QPushButton:hover{background:rgba(251,80,80,50%); border-radius: 4px;}"
                            "QPushButton:pressed{background-color:rgba(251,80,80,80%); border-radius: 4px;}");

    connect(closeBtn,&QPushButton::clicked,this,[=]{
        emit rejected();
        this->close();
    });

    noBtn = new QPushButton(this);
    noBtn->setGeometry(246,212,112,56);
    noBtn->setText(tr("Cancel"));
    connect(noBtn,&QPushButton::clicked,this,[=]{
        emit rejected();
        this->close();
    });

    okBtn = new QPushButton(this);
    okBtn->setGeometry(374,212,112,56);
    okBtn->setText(tr("Remove"));
    connect(okBtn,&QPushButton::clicked,this,[=]{
        emit accepted();
        this->close();
    });
}

MsgBox::~MsgBox()
{

}

void MsgBox::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (isBlack) {
        painter.setBrush(QColor(Qt::black));
    } else {
        painter.setBrush(QColor(Qt::white));
    }

    painter.setPen(QColor(Qt::transparent));
    painter.drawRoundedRect(0,0,this->width(),this->height(),16,16);
}

void MsgBox::showEvent(QShowEvent *event)
{

}

void MsgBox::gsettingsChanged(const QString &key)
{
    if ("styleName" == key) {
        if(gsettings->get("style-name").toString() == "ukui-black" ||
           gsettings->get("style-name").toString() == "ukui-dark")
        {
            isBlack = true;
            closeBtn->setIcon(renderSvg(QIcon::fromTheme("window-close-symbolic"),"white"));
        }
        else
        {
            isBlack = false;
            closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
        }
    }
}

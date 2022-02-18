#include "devrenamedialog.h"

DevRenameDialog::DevRenameDialog(QWidget *parent):QDialog(parent)
{
    this->setFixedSize(480,192);
    this->adjustSize();
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    initUI();
    initGsettings();
}

DevRenameDialog::~DevRenameDialog()
{

}

void DevRenameDialog::setDevName(const QString &str)
{
    lineEdit->setText(str);
    adapterOldName = str;
    //test name
    //lineEdit->setText("strdddd123456788991111000000");
    //adapterOldName = "strdddd123456788991111000000";
}

void DevRenameDialog::initGsettings()
{
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        gsettings = new QGSettings("org.ukui.style");

        if(gsettings->get("style-name").toString() == "ukui-default"
           || gsettings->get("style-name").toString() == "ukui-light")
            isblack = false;
        else
            isblack = true;

        _fontSize = gsettings->get("system-font-size").toString().toInt();

    }

    connect(gsettings,&QGSettings::changed,this,&DevRenameDialog::gsettingsSlot);
}

void DevRenameDialog::gsettingsSlot(const QString &key)
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

void DevRenameDialog::initUI()
{
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setGeometry(10,6,20,20);
    iconLabel->setPixmap(QIcon::fromTheme("bluetooth").pixmap(20,20));

    QLabel *titleLabel = new QLabel(this);
    titleLabel->setGeometry(36,5,100,20);
    titleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    titleLabel->setText(tr("Rename"));

    closeBtn = new QPushButton(this);
    closeBtn->setGeometry(453,8,20,20);
    //closeBtn->setIcon(QIcon::fromTheme("application-exit-symbolic"));
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setFlat(true);
    //closeBtn->setFixedSize(QSize(20,20));
    closeBtn->setProperty("isWindowButton",0x2);
    closeBtn->setProperty("useIconHighlihtEffect",0x8);
    connect(closeBtn,&QPushButton::clicked,this,[=]{
        this->close();
    });

    QLabel *textLabel = new QLabel(this);
    textLabel->setGeometry(24,64,60,20);
    textLabel->setText(tr("Name"));
    textLabel->setAlignment(Qt::AlignHCenter|Qt::AlignRight);
    textLabel->adjustSize();

    int wigth = textLabel->width();
    wigth = wigth - 60;
    lineEdit = new QLineEdit(this);
    lineEdit->setGeometry(100 + wigth,55,355 - wigth,36);
    //lineEdit->setGeometry(100,55,355,36);
    //connect(lineEdit,&QLineEdit::textChanged,this,&DevRenameDialog::lineEditSlot);
    //输入变化时进行长度提示
    connect(lineEdit,&QLineEdit::textEdited,this,&DevRenameDialog::lineEditSlot);

    tipLabel = new QLabel(this);
    tipLabel->setGeometry(96,94,300,20);
    tipLabel->setText(tr("The input character length exceeds the limit"));
    tipLabel->setVisible(false);
    tipLabel->setStyleSheet("font-size: 14px;\
                            font-family: Noto Sans CJK SC;\
                            font-weight: 400;\
                            line-height: 24px;\
                            color: rgba(255, 0, 0, 0.85);\
                            opacity: 1;");

    acceptBtn = new QPushButton(tr("OK"),this);
    acceptBtn->setGeometry(359,130,96,36);
    connect(acceptBtn,&QPushButton::clicked,this,[=]{

        if ((lineEdit->text().length() > 0) &&
            (lineEdit->text().length() < 21) &&
            (lineEdit->text() != adapterOldName))

            emit nameChanged(lineEdit->text());

        this->close();
    });

    rejectBtn = new QPushButton(tr("Cancel"),this);
    rejectBtn->setGeometry(247,130,96,36);
    connect(rejectBtn,&QPushButton::clicked,this,[=]{
        this->close();
    });
}


void DevRenameDialog::lineEditSlot(const QString &str)
{
    if (str.length() > 0 && str.length() < 21) {
        acceptBtn->setDisabled(false);
        tipLabel->setVisible(false);
    } else if (0 == str.length()) {
        acceptBtn->setDisabled(true);
        tipLabel->setVisible(false);
    }else {
        tipLabel->setVisible(true);
        acceptBtn->setDisabled(true);
    }
}

void DevRenameDialog::paintEvent(QPaintEvent *event)
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

void DevRenameDialog::keyPressEvent(QKeyEvent * event)
{
    Q_UNUSED(event)
    switch(event->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        //if(acceptBtn->isEnabled())//setEnable状态后，emit click 信号不相应,无需判断
            emit acceptBtn->click();
        break;
    case Qt::Key_Escape:
        //if(rejectBtn->isEnabled())//setEnable状态后，emit click 信号不相应,无需判断
            emit rejectBtn->click();
        break;
    }

}

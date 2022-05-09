#include "addbtn.h"
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QVariant>
#include <QPainter>
#include <QGSettings/QGSettings>

AddBtn::AddBtn(QWidget *parent):
    QPushButton(parent)
{
    this->setObjectName("this");
    this->setMinimumSize(QSize(580, 60));
    this->setMaximumSize(QSize(16777215, 60));
    this->setBtnStyle(Box);
    this->setProperty("useButtonPalette", true);

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel *iconLabel = new QLabel();
    QLabel *textLabel = new QLabel(tr("Add"));

    QIcon mAddIcon = QIcon::fromTheme("list-add-symbolic");
    iconLabel->setPixmap(mAddIcon.pixmap(mAddIcon.actualSize(QSize(16, 16))));
    iconLabel->setProperty("iconHighlightEffectMode", 1);

    const QByteArray idd(THEME_QT_SCHEMA);
    QGSettings *qtSettings  = new QGSettings(idd, QByteArray(), this);
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode){
        iconLabel->setProperty("useIconHighlightEffect", true);
    }
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "styleName") {
            QString currentThemeMode = qtSettings->get(key).toString();
            if ("ukui-black" == currentThemeMode || "ukui-dark" == currentThemeMode) {
                iconLabel->setProperty("useIconHighlightEffect", true);
            } else if("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode) {
                iconLabel->setProperty("useIconHighlightEffect", false);
            }
        }
    });

    addLyt->addStretch();
    addLyt->addWidget(iconLabel);
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    this->setLayout(addLyt);

}

AddBtn::~AddBtn()
{

}

void AddBtn::enterEvent(QEvent *event){
    Q_EMIT enterWidget();

    QPushButton::enterEvent(event);
}

void AddBtn::setBtnStyle(AddBtn::Shape type)
{
    switch (type) {
    case None:
        this->setStyleSheet("AddBtn::hover:!pressed{background-color: palette(button); border-rdius: 0px}"
                            "AddBtn:!checked{background-color: palette(base);border-rdius: 0px}");
        break;
    case Top:
        this->setStyleSheet("AddBtn::hover:!pressed{background-color: palette(button); border-top-left-radius: 6px; border-top-right-radius: 6px;}"
                            "AddBtn:!checked{background-color: palette(base); border-top-left-radius: 6px; border-top-right-radius: 6px;}");
        break;
    case Bottom:
        this->setStyleSheet("AddBtn::hover:!pressed{background-color: palette(button); border-bottom-left-radius: 6px; border-bottom-right-radius: 6px;}"
                            "AddBtn:!checked{background-color: palette(base); border-bottom-left-radius: 6px; border-bottom-right-radius: 6px;}");
        break;
    case Box:
        this->setStyleSheet("QPushButton:!checked{background-color: palette(base)}");
        break;
    }
}

void AddBtn::leaveEvent(QEvent *event){
    Q_EMIT leaveWidget();

    QPushButton::leaveEvent(event);
}

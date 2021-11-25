#include "addbtn.h"
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QVariant>

AddBtn::AddBtn(QWidget *parent):
    QPushButton(parent)
{
    this->setObjectName("this");
    this->setMinimumSize(QSize(580, 60));
    this->setMaximumSize(QSize(16777215, 60));
    this->setStyleSheet("QPushButton{background: palette(base);}"
                        "QPushButton:hover:!pressed{background:#3790FA;}"
                        "QPushButton:pressed{background:#3790FA;}");
//    this->setProperty("useButtonPalette", true);

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel *iconLabel = new QLabel();
    QLabel *textLabel = new QLabel(tr("Add"));

    QIcon mAddIcon = QIcon::fromTheme("list-add-symbolic");
    iconLabel->setPixmap(mAddIcon.pixmap(mAddIcon.actualSize(QSize(16, 16))));
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

    iconLabel->setProperty("iconHighlightEffectMode", 1);

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

void AddBtn::leaveEvent(QEvent *event){
    Q_EMIT leaveWidget();

    QPushButton::leaveEvent(event);
}

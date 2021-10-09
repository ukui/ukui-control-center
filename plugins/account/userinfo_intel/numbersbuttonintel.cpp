#include "numbersbuttonintel.h"

#include <QGridLayout>
#include <QVariant>

NumbersButtonIntel::NumbersButtonIntel(QWidget *parent):
    QWidget(parent)
{
    initUI();
    initConnect();
    const QByteArray id_1(UKUI_QT_STYLE);
    if (QGSettings::isSchemaInstalled(id_1)) {
        m_style = new QGSettings(id_1);
    }
    setQSS();
}

void NumbersButtonIntel::initUI()
{
    QGridLayout* mainLayout = new QGridLayout(this);
    setLayout(mainLayout);

    for(int i = 1;i <= 9;i++)
    {
        m_pNumerPressBT[i] = new QPushButton(this);
        m_pNumerPressBT[i]->setText(QString(QChar(i + '0')));
        m_pNumerPressBT[i]->setProperty("class", "numberPressBT");
        mainLayout->addWidget(m_pNumerPressBT[i], (i - 1) / 3, (i - 1) % 3);
    }
    m_pNumerPressBT[0] = new QPushButton(this);
    m_pNumerPressBT[0]->setText(QString(QChar('0')));
    m_pNumerPressBT[0]->setProperty("class", "numberPressBT");
    mainLayout->addWidget(m_pNumerPressBT[0], 3, 1);

    m_pNumerPressBT[10] = new QPushButton(this);
    m_pPictureToWhite = new PictureToWhite();
    m_pNumerPressBT[10]->setIcon(QIcon(m_pPictureToWhite->drawSymbolicColoredPixmap(QPixmap(":/img/plugins/userinfo_intel/num-delete.svg").scaled(30,30))));
    m_pNumerPressBT[10]->setProperty("class", "numberPressBT");
    m_pNumerPressBT[11] = new QPushButton(this);
    m_pNumerPressBT[11]->setText(tr("clean"));
    m_pNumerPressBT[11]->setProperty("class", "numberPressCL");
    mainLayout->addWidget(m_pNumerPressBT[11], 3, 0);
    mainLayout->setSpacing(12);
    mainLayout->setVerticalSpacing(10);
    mainLayout->addWidget(m_pNumerPressBT[10], 3, 2);
}

void NumbersButtonIntel::initConnect()
{
    connect(m_pNumerPressBT[0], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(0); });
    connect(m_pNumerPressBT[1], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(1); });
    connect(m_pNumerPressBT[2], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(2); });
    connect(m_pNumerPressBT[3], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(3); });
    connect(m_pNumerPressBT[4], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(4); });
    connect(m_pNumerPressBT[5], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(5); });
    connect(m_pNumerPressBT[6], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(6); });
    connect(m_pNumerPressBT[7], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(7); });
    connect(m_pNumerPressBT[8], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(8); });
    connect(m_pNumerPressBT[9], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(9); });
    connect(m_pNumerPressBT[10], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(10); });
    connect(m_pNumerPressBT[11], &QPushButton::clicked, this, [=](){ emit numbersButtonPress(11); });
}

void NumbersButtonIntel::setQSS()
{
    QString themeName = m_style->get(UKUI_STYLE_KEY).toString();
    if( themeName == "ukui-light" || themeName == "ukui-default" | themeName == "ukui" ) {
        setStyleSheet(".numberPressBT{"
                          "font-size:24px;"
                          "min-width:96px;"
                          "min-height:64px;"
                          "max-width:96px;"
                          "max-height:64px;"
                          "background:rgba(0, 0, 0, 0.05);"
                          "border-radius:16px;"
                          "}"
                          ".numberPressBT:hover{"
                          "background:rgba(0, 0, 0, 0.15)"
                          "}"
                       ".numberPressCL{"
                           "font-size:16px;"
                           "min-width:96px;"
                           "min-height:64px;"
                           "max-width:96px;"
                           "max-height:64px;"
                           "background:rgba(0, 0, 0, 0.05);"
                           "border-radius:16px;"
                           "}"
                           ".numberPressCL:hover{"
                           "background:rgba(0, 0, 0, 0.15)"
                           "}"

                      );         // isNightMode=false
    } else {
        setStyleSheet(".numberPressBT{"
                          "font-size:24px;"
                          "min-width:96px;"
                          "min-height:64px;"
                          "max-width:96px;"
                          "max-height:64px;"
                          "background:rgba(58, 58, 61, 255);"
                          "border-radius:16px;"
                          "}"
                          ".numberPressBT:hover{"
                          "background:rgba(98, 98, 103, 255)"
                          "}"
                       ".numberPressCL{"
                           "font-size:16px;"
                           "min-width:96px;"
                           "min-height:64px;"
                           "max-width:96px;"
                           "max-height:64px;"
                           "background:rgba(58, 58, 61, 255);"
                           "border-radius:16px;"
                           "}"
                           ".numberPressCL:hover{"
                           "background:rgba(98, 98, 98, 255)"
                           "}"

                      );   // isNightMode=true
    }

}


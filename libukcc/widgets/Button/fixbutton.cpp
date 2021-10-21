#include "fixbutton.h"
#include <QFontMetrics>
#include <QDebug>

FixButton::FixButton(QWidget *parent):
    QPushButton(parent)
{

}

FixButton::~FixButton()
{

}

void FixButton::paintEvent(QPaintEvent *event)
{
    QFontMetrics fontMetrics(this->font());
    int fontSize = fontMetrics.width(mStr);
    int showTextSize = this->width() - 32;
    if (fontSize > showTextSize) {
        this->setText(fontMetrics.elidedText(mStr, Qt::ElideRight, showTextSize), false);
        this->setToolTip(mStr);
    } else {
        this->setText(mStr, false);
        this->setToolTip("");
    }
    QPushButton::paintEvent(event);
}

void FixButton::setText(const QString & text, bool saveTextFlag)
{
    if (saveTextFlag)
        mStr = text;
    QPushButton::setText(text);
}

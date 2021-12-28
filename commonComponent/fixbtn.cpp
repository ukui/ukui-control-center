#include "fixbtn.h"
#include <QFontMetrics>
#include <QDebug>

FixBtn::FixBtn(QWidget *parent):
    QPushButton(parent)
{

}

FixBtn::~FixBtn()
{

}

void FixBtn::paintEvent(QPaintEvent *event)
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

void FixBtn::setText(const QString & text, bool saveTextFlag)
{
    if (saveTextFlag)
        mStr = text;
    QPushButton::setText(text);
}

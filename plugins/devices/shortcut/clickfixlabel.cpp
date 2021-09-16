#include "clickfixlabel.h"
#include <QDebug>

ClickFixLabel::ClickFixLabel(QWidget *parent):
    QLabel(parent)
{
    this->setContentsMargins(8,0,0,0);
}

ClickFixLabel::~ClickFixLabel()
{

}

void ClickFixLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    Q_EMIT doubleClicked();
    return;
}

void ClickFixLabel::paintEvent(QPaintEvent *event)
{
    QFontMetrics fontMetrics(this->font());
    int fontSize = fontMetrics.width(mStr);
    int m_width = this->width() - 8;
    if (fontSize > m_width) {
        this->setText(fontMetrics.elidedText(mStr, Qt::ElideRight, m_width), false);
        this->setToolTip(mStr);
    } else {
        this->setText(mStr, false);
        this->setToolTip("");
    }
    QLabel::paintEvent(event);
}

void ClickFixLabel::setText(const QString & text, bool saveTextFlag)
{
    if (saveTextFlag)
        mStr = text;
    QLabel::setText(text);
}

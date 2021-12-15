#include "tristatelabel.h"

tristateLabel::tristateLabel(const QString &text, QWidget *parent)
{
    setText(abridge(text));
    adjustSize();
}

tristateLabel::~tristateLabel()
{

}

QString tristateLabel::abridge(QString text)
{
    /* 设计要求，部分首页显示插件名和导航显示名不一致*/
    if (text == "时间和日期") {
        text = "时间日期";
    } else if (text == "区域语言") {
        text = "语言";
    }

    return text;
}

void tristateLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {      
        mClicked = true;
    }
}

void tristateLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mClicked = false;
        emit clicked();
    }
}

void tristateLabel::enterEvent(QEvent *e)
{
    Q_UNUSED(e)
    mMoved = true;
}
void tristateLabel::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)
    mMoved = false;
}

static inline qreal mixQreal(qreal a, qreal b, qreal bias)
{
    return a + (b - a) * bias;
}

QColor mixColor(const QColor &c1, const QColor &c2, qreal bias)
{
    if (bias <= 0.0) {
        return c1;
    }
    if (bias >= 1.0) {
        return c2;
    }
    if (qIsNaN(bias)) {
        return c1;
    }

    qreal r = mixQreal(c1.redF(),   c2.redF(),   bias);
    qreal g = mixQreal(c1.greenF(), c2.greenF(), bias);
    qreal b = mixQreal(c1.blueF(),  c2.blueF(),  bias);
    qreal a = mixQreal(c1.alphaF(), c2.alphaF(), bias);

    return QColor::fromRgbF(r, g, b, a);
}

void tristateLabel::paintEvent(QPaintEvent *event)
{
    QPushButton *button = new QPushButton;
    QPalette pal;
    QBrush brush = pal.placeholderText();
    QColor textColor = brush.color();
    if (mMoved) {
        QColor highlight = button->palette().color(QPalette::Active, QPalette::Highlight);
        QColor mix = button->palette().color(QPalette::Active, QPalette::BrightText);
        textColor = mixColor(highlight, mix, 0.05);
    }
    if (mClicked) {
        QColor highlight = button->palette().color(QPalette::Active, QPalette::Highlight);
        QColor mix = button->palette().color(QPalette::Active, QPalette::BrightText);
        textColor = mixColor(highlight, mix, 0.2);
    }

    QString stringColor = QString("color: rgba(%1,%2,%3,%4)")
           .arg(textColor.red())
           .arg(textColor.green())
           .arg(textColor.blue())
           .arg(textColor.alphaF());

    this->setStyleSheet(stringColor);



    QLabel::paintEvent(event);
}

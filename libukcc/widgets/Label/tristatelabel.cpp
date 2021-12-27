#include "tristatelabel.h"

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


TristateLabel::TristateLabel(const QString &text, QWidget *parent)
{
    setText(abridge(text));
    adjustSize();
    QPalette pal;
    QBrush brush = pal.placeholderText();
    QColor textColor = brush.color();
    QString stringColor = QString("color: rgba(%1,%2,%3,%4)")
           .arg(textColor.red())
           .arg(textColor.green())
           .arg(textColor.blue())
           .arg(textColor.alphaF());
    this->setStyleSheet(stringColor);
    const QByteArray idd(THEME_QT_SCHEMA);
    QGSettings *qtSettings  = new QGSettings(idd, QByteArray(), this);
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
        if ("styleName" == key) {
            QPalette pal;
            QBrush brush = pal.placeholderText();
            QColor textColor = brush.color();
            QString stringColor = QString("color: rgba(%1,%2,%3,%4)")
                           .arg(textColor.red())
                           .arg(textColor.green())
                           .arg(textColor.blue())
                           .arg(textColor.alphaF());
            this->setStyleSheet(stringColor);
        }
    });
}

TristateLabel::~TristateLabel()
{

}

QString TristateLabel::abridge(QString text)
{
    /* 设计要求，部分首页显示插件名和导航显示名不一致*/
    if (text == "时间和日期") {
        text = "时间日期";
    } else if (text == "区域语言") {
        text = "语言";
    }

    return text;
}

void TristateLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {

        QPalette pal;
        QBrush brush = pal.placeholderText();
        QColor textColor = brush.color();
        QPushButton *button = new QPushButton;
        QColor highlight = button->palette().color(QPalette::Active, QPalette::Highlight);
        QColor mix = button->palette().color(QPalette::Active, QPalette::BrightText);
        textColor = mixColor(highlight, mix, 0.2);
        QString stringColor = QString("color: rgba(%1,%2,%3,%4)")
               .arg(textColor.red())
               .arg(textColor.green())
               .arg(textColor.blue())
               .arg(textColor.alphaF());
        this->setStyleSheet(stringColor);
        mClicked = true;
    }
}

void TristateLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mClicked = false;
        QPalette pal;
        QBrush brush = pal.placeholderText();
        QColor textColor = brush.color();
        QString stringColor = QString("color: rgba(%1,%2,%3,%4)")
               .arg(textColor.red())
               .arg(textColor.green())
               .arg(textColor.blue())
               .arg(textColor.alphaF());
        this->setStyleSheet(stringColor);
        emit clicked();
    }
}

void TristateLabel::enterEvent(QEvent *e)
{
    Q_UNUSED(e)
    QPalette pal;
    QBrush brush = pal.placeholderText();
    QColor textColor = brush.color();
    QPushButton *button = new QPushButton;
    QColor highlight = button->palette().color(QPalette::Active, QPalette::Highlight);
    QColor mix = button->palette().color(QPalette::Active, QPalette::BrightText);
    textColor = mixColor(highlight, mix, 0.05);
    QString stringColor = QString("color: rgba(%1,%2,%3,%4)")
           .arg(textColor.red())
           .arg(textColor.green())
           .arg(textColor.blue())
           .arg(textColor.alphaF());
    this->setStyleSheet(stringColor);
    mMoved = true;
}
void TristateLabel::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)
    mMoved = false;
    QPalette pal;
    QBrush brush = pal.placeholderText();
    QColor textColor = brush.color();
    QString stringColor = QString("color: rgba(%1,%2,%3,%4)")
           .arg(textColor.red())
           .arg(textColor.green())
           .arg(textColor.blue())
           .arg(textColor.alphaF());
    this->setStyleSheet(stringColor);
}


#include "dateedit.h"
#include <QPainter>
#include <QBrush>
#include <QDebug>
#include <QRect>
#include <QApplication>
#include <QSvgRenderer>
#include <QLineEdit>

DateEdit::DateEdit(QWidget *parent) : QDateEdit(parent){
    this->setButtonSymbols(QAbstractSpinBox::NoButtons);
}

DateEdit::~DateEdit() {

}

void DateEdit::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
    QPainter painter(this);
    QBrush brush = QBrush(palette().color(QPalette::Button));
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawRoundedRect(this->rect(),6,6);
    QPixmap pix = loadSvg(":/img/dropArrow/ukui-down-symbolic.svg",16);
    QRect rect = QRect(125,10,16,16);
    painter.drawPixmap(rect, pix);

}


QPixmap DateEdit::loadSvg(const QString &path, int size)
{
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();
    if ( 2 == ratio) {
        size += origSize;
    } else if (3 == ratio) {
        size += origSize;
    }
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap);
}

QPixmap DateEdit::drawSymbolicColoredPixmap(const QPixmap &source)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                QColor colorSet = palette().color(QPalette::ButtonText);
                color.setRed(colorSet.red());
                color.setGreen(colorSet.green());
                color.setBlue(colorSet.blue());
                img.setPixelColor(x, y, color);
            }
        }
    }
    return QPixmap::fromImage(img);
}


#include "iconbutton.h"
#include <QApplication>
#include <QSvgRenderer>
#include <QPainter>

IconButton::IconButton(QString iconFileName, QWidget *parent):
    QPushButton(parent)
{
    this->iconName = iconFileName;
    QString iconBtnQss = QString("QPushButton:checked{border:  none;}"
                                 "QPushButton:!checked{border: none;}");
    this->setStyleSheet(iconBtnQss);
}

IconButton::~IconButton()
{

}

void IconButton::reLoadIcon()
{
    QString path = QString("://img/secondaryleftmenu/%1.svg").arg(iconName);
    QPixmap pix = loadSvg(path);
    this->setIcon(pix);
}

QPixmap IconButton::loadSvg(const QString &path, int size) {
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();
    if (  ratio >= 2) {
        size += origSize;
    } else {
        size *= ratio;
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

QPixmap IconButton::drawSymbolicColoredPixmap(const QPixmap &source) {
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                QColor colorSet = palette().color(QPalette::ButtonText);
                if (this->isChecked()) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                } else {
                    color.setRed(colorSet.red());
                    color.setGreen(colorSet.green());
                    color.setBlue(colorSet.blue());
                }
                img.setPixelColor(x, y, color);

            }
        }
    }
    return QPixmap::fromImage(img);
}

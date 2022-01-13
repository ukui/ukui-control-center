#include "config.h"
#include <QApplication>
#include <QImage>

const QPixmap renderSvg(const QIcon &icon, QString cgColor) {
    int size = 24;
    const auto ratio = qApp->devicePixelRatio();
    if ( 2 == ratio) {
        size = 48;
    } else if (3 == ratio) {
        size = 96;
    }
    QPixmap iconPixmap = icon.pixmap(size,size);
    iconPixmap.setDevicePixelRatio(ratio);
    QImage img = iconPixmap.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ("white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if ("black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
//                    color.setAlpha(0.1);
                    color.setAlphaF(0.9);
                    img.setPixelColor(x, y, color);
                } else if ("gray" == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor){
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                } else {
                    return iconPixmap;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

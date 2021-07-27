#include "border_shadow_effect.h"
#include <QPainter>
#include <QWidget>
#include <QDebug>
#include <QRect>
#include <QGSettings>

#define THEME_QT_SCHEMA "org.ukui.style"
#define MODE_QT_KEY "style-name"
#define PERSONALISE  "org.ukui.control-center.personalise"
//qt's global function
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

BorderShadowEffect::BorderShadowEffect(QObject *parent) : QGraphicsEffect(parent)
{
    const QByteArray idd(THEME_QT_SCHEMA);
    if  (QGSettings::isSchemaInstalled(idd)){
        qtSettings = new QGSettings(idd);
        qtSettingsCreate = true;
    }
}
BorderShadowEffect::~BorderShadowEffect(){
    if (settingsCreate)
        delete gsettings;
    if (qtSettingsCreate)
        delete qtSettings;
}
void BorderShadowEffect::setBorderRadius(int radius)
{
    m_x_border_radius = radius;
    m_y_border_radius = radius;
}

void BorderShadowEffect::setBorderRadius(int xradius, int yradius)
{
    m_x_border_radius = xradius;
    m_y_border_radius = yradius;
}

void BorderShadowEffect::setBlurRadius(int radius)
{
    m_blur_radius = radius;
}

void BorderShadowEffect::setPadding(int padding)
{
    m_padding = padding;
}

void BorderShadowEffect::setShadowColor(const QColor &color)
{
    if (color != m_shadow_color) {
        m_force_update_cache = true;
    }
    m_shadow_color = color;
}

void BorderShadowEffect::setWindowBackground(const QColor &color)
{
    m_window_bg = color;
}

void BorderShadowEffect::setTransParentPath(const QPainterPath &path)
{
    m_transparent_path = path;
}

void BorderShadowEffect::setTransParentAreaBg(const QColor &transparentBg)
{
    m_transparent_bg = transparentBg;
}

void BorderShadowEffect::drawWindowShadowManually(QPainter *painter, const QRect &windowRect, bool fakeShadow,bool isMax)
{
    //draw window bg;
    QRect sourceRect = windowRect;
    auto contentRect = sourceRect.adjusted(m_padding, m_padding, -m_padding, -m_padding);
    //qDebug()<<contentRect;
    QPainterPath sourcePath;
    QPainterPath contentPath;
    sourcePath.addRect(sourceRect);
    if(!isMax){
        contentPath.addRoundedRect(contentRect, m_x_border_radius, m_y_border_radius);
        auto targetPath = sourcePath - contentPath;
        auto bgPath = contentPath - m_transparent_path;
        painter->fillPath(bgPath, m_window_bg);
        painter->fillPath(m_transparent_path, m_transparent_bg);
    } else {
        contentPath.addRoundedRect(contentRect, 0, 0);
        auto targetPath = sourcePath - contentPath;
        auto bgPath = contentPath - m_transparent_path;
        painter->fillPath(bgPath, m_window_bg);
        painter->fillPath(m_transparent_path, m_transparent_bg);

    }
    if (fakeShadow) {
        currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        connect(qtSettings, &QGSettings::changed, this, [=](const QString &key){
            currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        });
            if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
                painter->save();
                painter->setBrush(QColor("#F6F6F6"));
                auto color = m_shadow_color;
                color.setAlphaF(0);
                painter->setPen(color);
                const QByteArray id(PERSONALISE);
                if (QGSettings::isSchemaInstalled(id)){
                    settingsCreate = true;
                    gsettings = new QGSettings(id);
                    double transparency=gsettings->get("transparency").toDouble();
                    painter->setOpacity(transparency);
                } else {
                    painter->setOpacity(0.45);
                }
                painter->drawPath(m_transparent_path);
                painter->restore();
                auto color_1 = m_shadow_color;
                color_1.setAlphaF(0);
                painter->setPen(color_1);
                painter->setBrush(QColor("#F6F6F6"));
                painter->drawPath(contentPath - m_transparent_path);
            } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode) {
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setBrush(QColor(33,32,33));
                painter->setPen(Qt::transparent);
                if (QGSettings::isSchemaInstalled(PERSONALISE)){
                    settingsCreate = true;
                    gsettings = new QGSettings(PERSONALISE);
                    double transparency=gsettings->get("transparency").toDouble();
                    painter->setOpacity(transparency);
                } else {
                    painter->setOpacity(0.45);
                }
                painter->drawPath(m_transparent_path);
                painter->restore();
                auto color_1 = m_shadow_color;
                color_1.setAlphaF(0);
                painter->setPen(color_1);
                painter->setBrush(QColor(65,65,65));
                painter->setPen(Qt::transparent);
                painter->setOpacity(0.24);
                painter->drawPath(contentPath - m_transparent_path);
            }
            return;
        }

        //qDebug()<<this->boundingRect()<<offset;
        if (m_padding > 0) {
            if (m_cache_shadow.size() == windowRect.size() && !m_force_update_cache) {
                //use cache pixmap draw shadow.
                painter->save();
                painter->setClipPath(sourcePath - contentPath);
                painter->drawImage(QPoint(), m_cache_shadow);
                painter->restore();
            } else {
                //draw shadow and cache shadow pixmap
                QPixmap pixmap(sourceRect.size().width(), sourceRect.height());
                pixmap.fill(Qt::transparent);
                QPainter p(&pixmap);
                p.fillPath(contentPath, m_shadow_color);
                p.end();
                QImage img = pixmap.toImage();
                qt_blurImage(img, m_blur_radius, false, false);
                pixmap.convertFromImage(img);
                m_cache_shadow = img;
                painter->save();
                painter->setClipPath(sourcePath - contentPath);
                painter->drawImage(QPoint(), img);
                painter->restore();
                m_force_update_cache = false;
            }
        }
    }
    void BorderShadowEffect::draw(QPainter *painter)
    {
        //draw window bg;
        auto sourceRect = boundingRect();
        auto contentRect = boundingRect().adjusted(m_padding, m_padding, -m_padding, -m_padding);
        //qDebug()<<contentRect;
        QPainterPath sourcePath;
        QPainterPath contentPath;
        sourcePath.addRect(sourceRect);
        contentPath.addRoundedRect(contentRect, m_x_border_radius, m_y_border_radius);
        auto targetPath = sourcePath - contentPath;
        //qDebug()<<contentPath;
        painter->fillPath(contentPath, m_window_bg);

        QPoint offset;
        if (sourceIsPixmap()) {
            // No point in drawing in device coordinates (pixmap will be scaled anyways).
            const QPixmap pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset, QGraphicsEffect::PadToTransparentBorder);
            painter->drawPixmap(offset, pixmap);
        } else {
            // Draw pixmap in device coordinates to avoid pixmap scaling;
            const QPixmap pixmap = sourcePixmap(Qt::DeviceCoordinates, &offset, QGraphicsEffect::PadToTransparentBorder);
            painter->setWorldTransform(QTransform());
            painter->drawPixmap(offset, pixmap);
        }
        //qDebug()<<this->boundingRect()<<offset;
        if (m_padding > 0) {
            //draw shadow
            QPixmap pixmap(sourceRect.size().width(), sourceRect.height());
            pixmap.fill(Qt::transparent);
            QPainter p(&pixmap);
            p.fillPath(contentPath, m_shadow_color);
            p.end();
            QImage img = pixmap.toImage();
            qt_blurImage(img, m_blur_radius, false, false);
            pixmap.convertFromImage(img);
            painter->save();
            painter->setClipPath(sourcePath - contentPath);
            painter->drawImage(QPoint(), img);
            painter->restore();
        }
    }

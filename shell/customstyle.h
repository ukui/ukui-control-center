#ifndef CUSTOMSTYLE_H
#define CUSTOMSTYLE_H

#include <QProxyStyle>

class CustomStyle : public QProxyStyle
{
    Q_OBJECT
public:
    explicit CustomStyle(QObject *parent = nullptr);

    void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const;

    void polish(QPalette &pal);

signals:

};

#endif // CUSTOMSTYLE_H

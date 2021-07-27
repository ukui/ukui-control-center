#ifndef COUSTOMSTYLE_H
#define COUSTOMSTYLE_H
#include <QProxyStyle>

class CoustoMstyle : public QProxyStyle
{
public:
    CoustoMstyle();
    // 画出Slider中groove、handle和填充条外观
    void drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option,
                                QPainter *painter, const QWidget *widget) const;
};

#endif // COUSTOMSTYLE_H

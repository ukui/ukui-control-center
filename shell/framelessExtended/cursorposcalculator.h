#ifndef CURSORPOSCALCULATOR_H
#define CURSORPOSCALCULATOR_H

#include <QPoint>
#include <QRect>

class CursorPosCalculator
{

public:
    explicit CursorPosCalculator();

public:
    void reset();
    void recalculate(const QPoint &globalMousePos, const QRect &frameRect);

public:
    bool onEdges : true;

    bool onLeftEdges : true;
    bool onTopEdges : true;
    bool onRightEdges : true;
    bool onBottomEdges : true;

    bool onLeftTopEdges :  true;
    bool onRightTopEdges : true;
    bool onRightBottomEdges : true;
    bool onLeftBottomEdges : true;

    static int borderWidth;
};

#endif // CURSORPOSCALCULATOR_H

#include "cursorposcalculator.h"

int CursorPosCalculator::borderWidth = 5;

CursorPosCalculator::CursorPosCalculator()
{
    reset();
}

void CursorPosCalculator::reset(){
    onEdges = false;

    onLeftEdges = false;
    onTopEdges = false;
    onRightEdges = false;
    onBottomEdges = false;

    onLeftTopEdges =false;
    onRightTopEdges = false;
    onRightBottomEdges = false;
    onLeftBottomEdges = false;
}

void CursorPosCalculator::recalculate(const QPoint &globalMousePos, const QRect &frameRect){
    int globalMouseX = globalMousePos.x();
    int globalMouseY = globalMousePos.y();

    int frameX = frameRect.x();
    int frameY = frameRect.y();

    int frameWidth = frameRect.width();
    int frameHeight = frameRect.height();

    onLeftEdges = (globalMouseX >= frameX &&
                   globalMouseX <= frameX + borderWidth);

    onRightEdges = (globalMouseX >= frameX + frameWidth - borderWidth &&
                    globalMouseX <= frameX + frameWidth);

    onTopEdges = (globalMouseY >= frameY &&
                  globalMouseY <= frameY + borderWidth);

    onBottomEdges = (globalMouseY >= frameY + frameHeight - borderWidth &&
                     globalMouseY <= frameY + frameHeight);

    onLeftTopEdges = onTopEdges && onLeftEdges;
    onRightTopEdges = onRightEdges && onTopEdges;
    onRightBottomEdges = onRightEdges && onBottomEdges;
    onLeftBottomEdges = onLeftEdges && onBottomEdges;

    onEdges = onLeftEdges || onRightEdges || onTopEdges || onBottomEdges;
}

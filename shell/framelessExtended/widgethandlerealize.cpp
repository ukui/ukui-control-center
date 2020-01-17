#include "widgethandlerealize.h"

WidgetHandleRealize::WidgetHandleRealize(FramelessHandlePrivate *_fpri, QWidget * pTopLevelWidget )
{
    fpri = _fpri;
    widgetInAction = pTopLevelWidget;

    leftBtnPressed = false;
    cursorShapeChanged = false;

    currentWindowFlags = widgetInAction->windowFlags();

    widgetInAction->setMouseTracking(true);
    widgetInAction->setAttribute(Qt::WA_Hover, true);

}

WidgetHandleRealize::~WidgetHandleRealize()
{
    widgetInAction->setMouseTracking(false);
    widgetInAction->setWindowFlags(currentWindowFlags);
    widgetInAction->setAttribute(Qt::WA_Hover, false);
}

QWidget * WidgetHandleRealize::currentWidget(){
    return widgetInAction;
}

void WidgetHandleRealize::handleWidgetEvent(QEvent *event){
    switch (event->type()){
    case QEvent::MouseButtonPress:
        handleMousePressEvent(dynamic_cast<QMouseEvent *>(event));
        break;
    case QEvent::MouseButtonRelease:
        handleMouseReleaseEvent(dynamic_cast<QMouseEvent *>(event));
        break;
    case QEvent::MouseMove:
        handleMouseMoveEvent(dynamic_cast<QMouseEvent *>(event));
        break;
    case QEvent::Leave:
        handleLeaveEvent(dynamic_cast<QMouseEvent *>(event));
        break;
    case QEvent::HoverMove:
        handleHoverMoveEvent(dynamic_cast<QHoverEvent *>(event));
        break;
    default:
        break;
    }
}

void WidgetHandleRealize::updateCursorShape(const QPoint &gMousePos){
    if (widgetInAction->isFullScreen() || widgetInAction->isMaximized()){
        if (cursorShapeChanged)
            widgetInAction->unsetCursor();
        return;
    }

    moveMousePos.recalculate(gMousePos, widgetInAction->frameGeometry());

    if (moveMousePos.onLeftTopEdges || moveMousePos.onRightBottomEdges){
        widgetInAction->setCursor(Qt::SizeFDiagCursor);
        cursorShapeChanged = true;
    }
    else if (moveMousePos.onRightTopEdges || moveMousePos.onLeftBottomEdges){
        widgetInAction->setCursor(Qt::SizeBDiagCursor);
        cursorShapeChanged = true;
    }
    else if (moveMousePos.onLeftEdges || moveMousePos.onRightEdges){
        widgetInAction->setCursor(Qt::SizeHorCursor);
        cursorShapeChanged = true;
    }
    else if (moveMousePos.onTopEdges || moveMousePos.onBottomEdges){
        widgetInAction->setCursor(Qt::SizeVerCursor);
        cursorShapeChanged = true;
    }
    else{
        if (cursorShapeChanged){
            widgetInAction->unsetCursor();
            cursorShapeChanged = false;
        }
    }
}

void WidgetHandleRealize::moveWidget(const QPoint &gMousePos){
    widgetInAction->move(gMousePos - dragPos);
}

void WidgetHandleRealize::resizeWidget(const QPoint &gMousePos){
    QRect origRect;

    origRect = widgetInAction->frameGeometry();

    int left = origRect.left();
    int top = origRect.top();
    int right = origRect.right();
    int bottom = origRect.bottom();

    origRect.getCoords(&left, &top, &right, &bottom);

    int minWidth = widgetInAction->minimumWidth();
    int minHeight = widgetInAction->minimumHeight();

    if (pressedMousePos.onLeftTopEdges){
        left = gMousePos.x();
        top = gMousePos.y();
    }
    else if (pressedMousePos.onRightTopEdges){
        right = gMousePos.x();
        top = gMousePos.y();
    }
    else if (pressedMousePos.onRightBottomEdges){
        right = gMousePos.x();
        bottom = gMousePos.y();
    }
    else if (pressedMousePos.onLeftBottomEdges){
        left = gMousePos.x();
        bottom = gMousePos.y();
    }
    else if (pressedMousePos.onLeftEdges){
        left = gMousePos.x();
    }
    else if (pressedMousePos.onTopEdges){
        top = gMousePos.y();
    }
    else if (pressedMousePos.onRightEdges){
        right = gMousePos.x();
    }
    else if (pressedMousePos.onBottomEdges){
        bottom = gMousePos.y();
    }

    QRect newRect(QPoint(left, top), QPoint(right, bottom));

    if (newRect.isValid()){
        if (minWidth > newRect.width()){
            if (left != origRect.left())
                newRect.setLeft(origRect.left());
            else
                newRect.setRight(origRect.right());
        }
        if (minHeight > newRect.height()){
            if (top != origRect.top())
                newRect.setTop(origRect.top());
            else
                newRect.setBottom(origRect.bottom());
        }

        widgetInAction->setGeometry(newRect);
    }
}

void WidgetHandleRealize::handleMousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        leftBtnPressed = true;

        QRect frameRect = widgetInAction->frameGeometry();
        pressedMousePos.recalculate(event->globalPos(), frameRect);

        dragPos = event->globalPos() - frameRect.topLeft();

    }
}

void WidgetHandleRealize::handleMouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        leftBtnPressed = false;
        pressedMousePos.reset();
    }
}

void WidgetHandleRealize::handleMouseMoveEvent(QMouseEvent *event){
    if (leftBtnPressed){
        if (fpri->widgetResizable && pressedMousePos.onEdges){
            resizeWidget(event->globalPos());
        }
        else if (fpri->widgetMovable && leftBtnPressed){
            moveWidget(event->globalPos());
        }
    }
    else if (fpri->widgetResizable){
        updateCursorShape(event->globalPos());
    }
}

void WidgetHandleRealize::handleLeaveEvent(QEvent *event){
    Q_UNUSED(event)
    if (!leftBtnPressed)
        widgetInAction->unsetCursor();
}

void WidgetHandleRealize::handleHoverMoveEvent(QHoverEvent *event){
    if (fpri->widgetResizable){
        updateCursorShape(widgetInAction->mapToGlobal(event->pos()));
    }
}

#ifndef WIDGETHANDLEREALIZE_H
#define WIDGETHANDLEREALIZE_H

#include <QWidget>

#include <QEvent>
#include <QMouseEvent>
#include <QHoverEvent>

#include "framelesshandleprivate.h"
#include "cursorposcalculator.h"

class WidgetHandleRealize : public QWidget
{

public:
    explicit WidgetHandleRealize(FramelessHandlePrivate * _fpri, QWidget *pTopLevelWidget);
    ~WidgetHandleRealize();

public:
    QWidget * currentWidget();

    void handleWidgetEvent(QEvent * event);

private:
    void updateCursorShape(const QPoint &gMousePos);
    void resizeWidget(const QPoint &gMousePos);
    void moveWidget(const QPoint &gMousePos);
    void handleMousePressEvent(QMouseEvent * event);
    void handleMouseReleaseEvent(QMouseEvent * event);
    void handleMouseMoveEvent(QMouseEvent * event);
    void handleLeaveEvent(QEvent * event);
    void handleHoverMoveEvent(QHoverEvent * event);

private:
    FramelessHandlePrivate * fpri;
    QWidget * widgetInAction;
    QPoint dragPos;

    CursorPosCalculator pressedMousePos;
    CursorPosCalculator moveMousePos;

    bool leftBtnPressed;
    bool cursorShapeChanged;

    Qt::WindowFlags currentWindowFlags;

};

#endif // WIDGETHANDLEREALIZE_H

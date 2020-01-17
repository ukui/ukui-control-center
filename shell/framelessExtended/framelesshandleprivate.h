#ifndef FRAMELESSHANDLEPRIVATE_H
#define FRAMELESSHANDLEPRIVATE_H

#include <QHash>

class QWidget;
class WidgetHandleRealize;

class FramelessHandlePrivate
{

public:
    QHash<QWidget *, WidgetHandleRealize *> widgethandleHash;

    bool widgetResizable : true;
    bool widgetMovable : true;

};

#endif // FRAMELESSHANDLEPRIVATE_H

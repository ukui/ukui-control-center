#ifndef FRAMELESSHANDLE_H
#define FRAMELESSHANDLE_H

#include <QObject>

class QWidget;
class FramelessHandlePrivate;

class FramelessHandle : public QObject
{
    Q_OBJECT

public:
    explicit FramelessHandle(QObject *parent = 0);
    ~FramelessHandle();

    void activateOn(QWidget * topLevelWidget);
    void setWidgetResizable(bool resizable);
    void setWidgetMovable(bool movable);
    void setBorderWidget(uint width);

    bool currentWidgetResizable();
    bool currentWidgetMovable();
    uint currentBorderWidth();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event);

private:
    FramelessHandlePrivate * fpri;

};

#endif // FRAMELESSHANDLE_H

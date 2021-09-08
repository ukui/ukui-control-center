#ifndef CLICKFIXLABEL_H
#define CLICKFIXLABEL_H
#include "Label/fixlabel.h"

class ClickFixLabel : public FixLabel
{
    Q_OBJECT
public:
    ClickFixLabel(QWidget *parent = nullptr);
    ~ClickFixLabel();
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);

Q_SIGNALS:
    void doubleClicked();
};



#endif // CLICKFIXLABEL_H

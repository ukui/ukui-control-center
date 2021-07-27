#ifndef TIMEBTN_H
#define TIMEBTN_H
#include <QPushButton>
#include "Label/fixlabel.h"
#include <QGSettings>
#include <QTimeZone>

class TimeBtn : public QPushButton
{
    Q_OBJECT
public:
    TimeBtn(const QString &timezone);
    ~TimeBtn();
    void updateTime(bool hour_24);
    QPushButton *deleteBtn = nullptr;

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

private:
    QTimeZone thisZone;
    FixLabel    *labelInfo = nullptr;
    FixLabel    *labelTime = nullptr;
};


#endif // TIMEBTN_H

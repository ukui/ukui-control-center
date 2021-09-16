#ifndef CUSTOMCALENDAR_H
#define CUSTOMCALENDAR_H

//qcustomcalendarwidget.h

#include <QCalendarWidget>

class QPushButton;
class QLabel;
class CustomCalendarWidget : public QCalendarWidget
{
    Q_OBJECT

public:
    CustomCalendarWidget(QWidget *parent = nullptr);
    ~CustomCalendarWidget();
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);
    QPixmap loadSvg(const QString &path, int size);
protected:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;
    void mouseMoveEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *event);
//    bool even
};




#endif // CUSTOMCALENDAR_H

#ifndef MSTACKWIDGET_H
#define MSTACKWIDGET_H

#include <QStackedWidget>
#include <QObject>


class MStackWidget : public QStackedWidget
{
    Q_OBJECT
public:
    MStackWidget(QWidget *parent = nullptr);
    ~MStackWidget();
    void paintEvent(QPaintEvent *event);
public:
    bool status = true;
};


#endif // MSTACKWIDGET_H

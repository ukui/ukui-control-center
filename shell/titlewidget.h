#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>
#include <QObject>
#include <QPainterPath>

class TitleWidget : public QWidget
{
    Q_OBJECT
public:
    TitleWidget(QWidget *parent = nullptr);
    ~TitleWidget();
    void paintEvent(QPaintEvent *event);

public:
    bool status = true;
};

#endif // TITLEWIDGET_H

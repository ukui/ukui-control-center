#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <QWidget>

class CustomWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomWidget(QWidget *parent = 0);
    ~CustomWidget();

    void emitting_toggle_signal(QString name, int type, int page);

Q_SIGNALS:
    void transmit(QString pluginname, int plugintype, int page);

};

#endif // CUSTOMWIDGET_H
